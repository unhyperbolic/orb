/*
 *  cassion_io.c
 *
 */

#include "casson.h"

#include "kernel.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static SolutionType string_to_solution_type(char *str);
static Boolean skip_blanks(char **str);
static Boolean fill_casson_struct(CassonFormat *cf, char **str);

static SolutionType string_to_solution_type(
    char *str)
{
    #define _SOL_TYPE(t)            \
                                    \
        if (strcmp(str, #t) == 0) { \
            return t;               \
        }

    // Branches similar to line 712...
    _SOL_TYPE(geometric_solution);
    _SOL_TYPE(nongeometric_solution);
    _SOL_TYPE(flat_solution);
    _SOL_TYPE(degenerate_solution);
    _SOL_TYPE(other_solution);
    _SOL_TYPE(no_solution);
    _SOL_TYPE(step_failed);
    _SOL_TYPE(invalid_solution);

    if (strcmp(str, "partially_flat_solution") == 0) {
        return geometric_solution;
    }

    return not_attempted;
}

static Boolean skip_blanks(
    char **str)
{
    while (isspace(**str))
    {
	if (**str == '\n')
	{
	    (*str)++;
	    return TRUE;
	}
	if (**str == '\0')
	{
	    return TRUE;
	}
	(*str)++;
    }
    return FALSE;
}

static Boolean fill_casson_struct(
    CassonFormat *cf,
    char **str)
{
    cf->head = NULL;
    cf->num_tet = 0;
    cf->vertices_known = FALSE;
    cf->type = not_attempted;

    int consumed;

    {
	char solution_type[128];
	if (sscanf(
		*str, " SolutionType %127s%n", solution_type, &consumed) == 1)
	{
	    // Set cf->type based on section
	    // see branches at 712
	    cf->type = string_to_solution_type(solution_type);
	    *str += consumed;
	}
    }

    if (sscanf(*str, " vertices_known%n", &consumed) == 0)
    {
        cf->vertices_known = TRUE;
	*str += consumed;
    }

    printf("F\n");

    EdgeInfo *e = NULL;

    while (1)
    {
	if (e)
	{
	    e = e->next = NEW_STRUCT(EdgeInfo);
	}
	else
	{
	    e = cf->head = NEW_STRUCT(EdgeInfo);
	}
        e->next = NULL;
        e->head = NULL;

	if (sscanf(
		*str,
		" %d %d %lf%n",
		&e->index, &e->singular_index, &e->singular_order,
		&consumed) != 3) {
	    return FALSE;
	}

	*str += consumed;

	e->index--;
	e->singular_index--;

        if (cf->vertices_known)
	{
	    if (sscanf(
		    *str,
		    " %d %d%n",
		    &e->one_vertex, &e->other_vertex, &consumed) != 2)
	    {
                return FALSE;
            }

	    *str += consumed;

            if (e->one_vertex > 0)
	    {
		e->one_vertex--;
	    }
            if (e->other_vertex > 0)
	    {
		e->other_vertex--;
	    }
        }

	printf("Scanning edge\n");

	TetEdgeInfo * t = NULL;

        while (1)
	{
	    if (t)
	    {
		t = t->next = NEW_STRUCT(TetEdgeInfo);
	    }
	    else
	    {
		t = e->head = NEW_STRUCT(TetEdgeInfo);
	    }

            for(int i = 0; i < 8; i++ ) {
                t->curves[i] = 0;
            }

	    char f1, f2;

            if (sscanf(
		    *str, "%d%c%c%n", &t->tet_index, &f1, &f2, &consumed) != 3) {
                return FALSE;
            }

	    *str += consumed;

            if (t->tet_index > cf->num_tet)
	    {
                cf->num_tet = t->tet_index;
            }

            t->tet_index--;

            if ('u' <= f1 && f1 <= 'x') {
                t->f1 = f1 - 'u';
            } else {
                return FALSE;
            }

            if ('u' <= f2 && f2 <= 'x') {
                t->f2 = f2 - 'u';
            } else {
                return FALSE;
            }

	    printf("Scanned tet edge info %d %d\n", t->f1, t->f2);

	    if (skip_blanks(str))
	    {
		break;
	    }
        }

	if (skip_blanks(str))
	{
	    break;
	}
    }

    if (cf->type != not_attempted) {
        // Note that organizer.cpp returns early if the solution
        // type is not attempted.
        // This is not correct since we still need to parse the
        // peripheral curves.
        // The peripheral curves are always there
        // (every call to saveTriangulation says so).

	for (EdgeInfo * e = cf->head; e != NULL; e = e->next)
	{
	    int index;
	    if (sscanf(
		    *str,
		    " %d %lf %lf %lf%n",
		    &index,
		    &e->e_inner_product,
		    &e->v_inner_product1,
		    &e->v_inner_product2,
		    &consumed) != 4)
	    {
		return FALSE;
	    }
	    *str += consumed;

	    for (TetEdgeInfo * t = e->head; t != NULL; t = t->next)
	    {
		if (sscanf(*str, " %lf%n", &t->dihedral_angle, &consumed) != 1)
		{
		    return FALSE;
		}
		*str += consumed;
		printf("%f\n", t->dihedral_angle);
	    }

	    printf("E\n");
	}

        // Line 873 in organizer.cpp
    }

    if (cf->vertices_known)
    {
	for (EdgeInfo * e = cf->head; e != NULL; e = e->next)
	{
	    int index;
	    if (sscanf(*str, " %d%n", &index, &consumed) != 1)
	    {
		return FALSE;
	    }
	    *str += consumed;

	    for (TetEdgeInfo * t = e->head; t != NULL; t = t->next)
	    {
                for (int i = 0; i < 8; i++)
		{
		    if (sscanf(*str, " %d%n", &t->curves[i], &consumed) != 1)
		    {
			return FALSE;
		    }
		    *str += consumed;
		}
	    }
	}
    }

    printf("Scanned\n");

    return TRUE;
}

CassonFormat *read_casson_struct(
    char **str)
{
    // readCassonFormat( QTextStream &ts)
    CassonFormat * cf = NEW_STRUCT(CassonFormat);
    if (fill_casson_struct(cf, str))
    {
        return cf;
    }

    free_casson(cf);
    return NULL;
}
