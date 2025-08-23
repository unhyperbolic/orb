#include "kernel.h"

#include "parse_orb.h"

#include "casson.h"
#include "diagram.h"

#include "parse_util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* What is happening in gui code:

   loadOrbSlot (in organizer.cpp):
       * reads the line "% orb"
       * reads the next line for the name (but ignores it)
       - readTriangulation
            - readCassonFormat
            - verifyCassonFormat
            - cassonToTriangulation
            - freeCassonFormat
       - readDiagram
            - DiagramCanvas::readDiagram (in interface.cpp)
    	         * seems to be somewhat similar to the plink format (vertices, edges, crossings)
		 * EndData, Edge, Vertex, Crossing in diagram_canvas.h
	         * DiagramCanvas::outputTriangulation creates a Graph

*/

static SolutionType string_to_solution_type(char *s)
{
    #define _SOL_TYPE(t)          \
                                  \
        if (strcmp(s, #t) == 0) { \
            return t;             \
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

    if (strcmp(s, "partially_flat_solution") == 0) {
        return geometric_solution;
    }

    return not_attempted;
}

Boolean
is_new_line_after_skipping_blanks(char **string)
{
    while (isspace(**string))
    {
	if (**string == '\n')
	{
	    return TRUE;
	}
	(*string)++;
    }
    return FALSE;
}

static Boolean fill_casson_from_string_destructive(
        CassonFormat *cf,
        char **file_data)
{
    cf->head = NULL;
    cf->num_tet = 0;
    cf->vertices_known = FALSE;
    cf->type = not_attempted;

    int consumed;

    {
	char solution_type[128];
	if (sscanf(*file_data, " SolutionType %127s%n", solution_type, &consumed) == 1)
	{
	    // Set cf->type based on section
	    // see branches at 712
	    cf->type = string_to_solution_type(solution_type);
	    *file_data += consumed;
	}
    }

    if (sscanf(*file_data, " vertices_known%n", &consumed) == 0)
    {
        cf->vertices_known = TRUE;
	*file_data += consumed; 
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
		*file_data,
		" %d %d %lf%n",
		&e->index, &e->singular_index, &e->singular_order, &consumed) != 3) {
	    return FALSE;
	}

	*file_data += consumed;
	
	e->index--;
	e->singular_index--;

        if (cf->vertices_known)
	{
	    if (sscanf(
		    *file_data,
		    " %d %d%n",
		    &e->one_vertex, &e->other_vertex, &consumed) != 2)
	    {
                return FALSE;
            }

	    *file_data += consumed;

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
	    
            if (sscanf(*file_data,
		       "%d%c%c%n",
                       &t->tet_index, &f1, &f2, &consumed) != 3) {
                return FALSE;
            }

	    *file_data += consumed;
	    
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

	    if (is_new_line_after_skipping_blanks(file_data))
	    {
		break;
	    }
        }

	(*file_data)++;

	if (is_new_line_after_skipping_blanks(file_data))
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
	    if (sscanf(*file_data,
		       " %d %lf %lf %lf%n",
		       &index,
		       &e->e_inner_product,
		       &e->v_inner_product1,
		       &e->v_inner_product2,
		       &consumed) != 4)
	    {
		return FALSE;
	    }
	    *file_data += consumed;

	    for (TetEdgeInfo * t = e->head; t != NULL; t = t->next)
	    {
		if (sscanf(*file_data, " %lf%n", &t->dihedral_angle, &consumed) != 1)
		{
		    return FALSE;
		}
		*file_data += consumed;
		printf("%f\n", t->dihedral_angle);
	    }
	    
	    printf("E\n");
	}
	
        // Line 873 in organizer.cpp
    }

    if (cf->vertices_known) {
	for (EdgeInfo * e = cf->head; e != NULL; e = e->next)
	{
	    int index;
	    if (sscanf(*file_data, " %d%n", &index, &consumed) != 1)
	    {
		return FALSE;
	    }
	    *file_data += consumed;

	    for (TetEdgeInfo * t = e->head; t != NULL; t = t->next)
	    {
                for (int i = 0; i < 8; i++)
		{
		    if (sscanf(*file_data, " %d%n", &t->curves[i], &consumed) != 1) {
			return FALSE;
		    }
		    *file_data += consumed;
		}
	    }
	}
    }

    printf("Scanned\n");
    
    return TRUE;
}

static CassonFormat *read_casson_from_string_destructive(
        char **file_data)
{
    // readCassonFormat( QTextStream &ts)
    CassonFormat *cf = NEW_STRUCT(CassonFormat);
    if (fill_casson_from_string_destructive(cf, file_data)) {
        return cf;
    }

    free_casson(cf);
    return NULL;
}

static Boolean fill_diagram_from_string_destructive(
    Diagram * diagram, char *file_data)
{
    int index;
    int chars_consumed;
    int num_vertices, num_edges, num_crossings;
    int id0, id1, type;

    if (sscanf(file_data, "%d%n", &num_vertices, &chars_consumed) != 1) {
	return FALSE;
    }
    file_data += chars_consumed;

    diagram->vertices = NEW_ARRAY( num_vertices, DiagramVertex* );

    for (diagram->num_vertices = 0;
	 diagram->num_vertices < num_vertices;
	 diagram->num_vertices++) {
	diagram->vertices[diagram->num_vertices] = NEW_STRUCT( DiagramVertex );
	initialize_diagram_vertex(diagram->vertices[diagram->num_vertices]);
	if (sscanf(file_data,
		   "%d%d%d%n",
		   &index,
		   &diagram->vertices[diagram->num_vertices]->x,
		   &diagram->vertices[diagram->num_vertices]->y,
		   &chars_consumed) != 3) {
	    return FALSE;
	}
	file_data += chars_consumed;
    }

    if (sscanf(file_data, "%d%n", &num_edges, &chars_consumed) != 1) {
	return FALSE;
    }
    file_data += chars_consumed;

    diagram->edges = NEW_ARRAY( num_edges, DiagramEdge* );

    for (diagram->num_edges = 0;
	 diagram->num_edges < num_edges;
	 diagram->num_edges++) {
	diagram->edges[diagram->num_edges] = NEW_STRUCT( DiagramEdge );
	initialize_diagram_edge(diagram->edges[diagram->num_edges]);

	if (sscanf(file_data,
		   "%d%d%d%d%n",
		   &index,
		   &id0,
		   &id1,
		   &type,
		   &chars_consumed) != 4) {
	    return FALSE;
	}

	diagram->edges[diagram->num_edges]->vertex[diagramBegin] =
	    diagram->vertices[id0];
	diagram->edges[diagram->num_edges]->vertex[diagramEnd] =
	    diagram->vertices[id1];
	diagram->edges[diagram->num_edges]->type = type;

	DiagramEndData * begin_data = NEW_STRUCT(DiagramEndData);
	begin_data->edge = diagram->edges[diagram->num_edges];
	begin_data->type = diagramBegin;
	begin_data->singular = FALSE;
	begin_data->angle = 0.0;

	DiagramEndData * end_data = NEW_STRUCT(DiagramEndData);
	end_data->edge = diagram->edges[diagram->num_edges];
	end_data->type = diagramEnd;
	end_data->singular = FALSE;
	end_data->angle = 0.0;

	add_end_data_to_vertex(begin_data, diagram->vertices[id0]);
	add_end_data_to_vertex(end_data, diagram->vertices[id1]);

	file_data += chars_consumed;
    }

    if (sscanf(file_data, "%d%n", &num_crossings, &chars_consumed) != 1) {
	return FALSE;
    }
    file_data += chars_consumed;

    diagram->crossings = NEW_ARRAY( num_crossings, DiagramCrossing* );

    for (diagram->num_crossings = 0;
	 diagram->num_crossings < num_crossings;
	 diagram->num_crossings++) {
	diagram->crossings[diagram->num_crossings] = NEW_STRUCT( DiagramCrossing );

	if (sscanf(file_data,
		   "%d%d%d%d%d%lf%lf%n",
		   &index,
		   &diagram->crossings[diagram->num_crossings]->x,
		   &diagram->crossings[diagram->num_crossings]->y,
		   &id0,
		   &id1,
		   &diagram->crossings[diagram->num_crossings]->position_on_overstrand,
		   &diagram->crossings[diagram->num_crossings]->position_on_understrand,
		   &chars_consumed) != 7) {
	    return FALSE;
	}

	diagram->crossings[diagram->num_crossings]->over =
	    diagram->edges[id0];
	diagram->crossings[diagram->num_crossings]->under =
	    diagram->edges[id1];

	file_data += chars_consumed;
    }

    return TRUE;
}

static Diagram *read_diagram_from_string_destructive(
    char * file_data)
{
    Diagram *diagram = NEW_STRUCT(Diagram);
    initialize_diagram(diagram);
    if (fill_diagram_from_string_destructive(diagram, file_data)) {
	assign_diagram_arcs(diagram);
	assign_diagram_links(diagram);

	return diagram;
    }

    free_diagram(diagram);
    return NULL;
}

static Boolean has_non_whitespace(
        char *l)
{
    while (l) {
        if (*l != ' ' && *l != '\t' && *l != '\n' && *l != '\r') {
            return TRUE;
        }
	l++;
    }

    return FALSE;
}

static void read_orb_from_string_destructive(
        char **file_data,
        char ** name,
        CassonFormat ** cf,
	Diagram ** diagram)
{
    char * l;

    l = parse_line(file_data);
    if (!l || strcmp(l, "% orb") != 0) {
        return;
    }

    l = parse_line(file_data);
    if (!l) {
        return;
    }

    *name = my_strdup(l);

    *cf = read_casson_from_string_destructive(file_data);

    if (*cf && *file_data && has_non_whitespace(*file_data)) {
        *diagram = read_diagram_from_string_destructive(
	    *file_data);
    }
}

void read_orb_from_string(
        char *file_data,
        char **name,
        CassonFormat ** cf,
	Diagram ** diagram)
{
    char * copy = my_strdup(file_data);
    char * p = copy;

    read_orb_from_string_destructive(&p, name, cf, diagram);

    free(copy);
}

void read_orb(
        const char *file_name,
        char **name,
        CassonFormat ** cf,
	Diagram ** diagram)
{
    // Follows unit_kit/unix_file_io.c

    FILE *fp;
    long filesize;
    char * buffer, * p;

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        return;
    }

    if ( fseek(fp, 0, SEEK_END) != 0 ||
         (filesize = ftell(fp) ) == -1 ||
         fseek(fp, 0, SEEK_SET) != 0) {
        return;
    }

    buffer = (char*) malloc(filesize + 1);
    buffer[filesize] = '\0';

    if ( fread(buffer, filesize, 1, fp) != 1) {
        free(buffer);
        return;
    }

    p = buffer;
    read_orb_from_string_destructive(&p, name, cf, diagram);

    free(buffer);
}
