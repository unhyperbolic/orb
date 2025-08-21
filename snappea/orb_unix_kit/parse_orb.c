#include "kernel.h"

#include "parse_orb.h"

#include "casson.h"
#include "diagram.h"

#include "parse_util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

static Boolean fill_casson_from_string_destructive(
        CassonFormat *cf,
        char **file_data)
{
    int             i;
    char            *line,
                    *section;
    EdgeInfo        *nei = NULL,
                    *ei = NULL;
    TetEdgeInfo     *ntei = NULL,
                    *tei = NULL;
    char            f1, f2;
    char            dummy;

    cf->head = NULL;
    cf->num_tet = 0;
    cf->vertices_known = FALSE;
    cf->type = not_attempted;

    if (!(section = parse_token_next_non_empty_line(file_data, &line))) {
        return FALSE;
    }

    if (strcmp(section, "SolutionType") == 0) {
        if (!(section = parse_token(&line))) {
            return FALSE;
        }
        
        // Set cf->type based on section
        // see branches at 712
        cf->type = string_to_solution_type(section);
        
        if (!(section = parse_token_next_non_empty_line(file_data, &line))) {
            return FALSE;
        }
    }

    if (strcmp(section, "vertices_known")  == 0) {
        cf->vertices_known = TRUE;
        
        if (!(section = parse_token_next_non_empty_line(file_data, &line))) {
            return FALSE;
        }
    }

    do {
        nei = NEW_STRUCT(EdgeInfo);
        if (cf->head == NULL) {
            cf->head = nei;
        } else {
            ei->next = nei;
        }
        nei->next = NULL;
        nei->head = NULL;

        ei = nei;

        
        if (sscanf(section, "%d%c", &(ei->index), &dummy) != 1) {
            return FALSE;
        }
        (ei->index)--;

        if (!(section = parse_token(&line))) {
            return FALSE;
        }

        if (sscanf(section, "%d%c", &(ei->singular_index), &dummy) != 1) {
            return FALSE;
        }
        
        (ei->singular_index)--;

        if (!(section = parse_token(&line))) {
            return FALSE;
        }

        if (sscanf(section, "%lf%c", &(ei->singular_order), &dummy) != 1) {
            return FALSE;
        }

        if (!(section = parse_token(&line))) {
            return FALSE;
        }

        if (cf->vertices_known) {
            if (sscanf(section, "%d%c", &(ei->one_vertex), &dummy) != 1) {
                return FALSE;
            }

            if (ei->one_vertex > 0 ) ei->one_vertex--;

            if (!(section = parse_token(&line))) {
                return FALSE;
            }

            if (sscanf(section, "%d%c", &(ei->other_vertex), &dummy) != 1) {
                return FALSE;
            }

            if (ei->other_vertex > 0 ) ei->other_vertex--;

            if (!(section = parse_token(&line))) {
                return FALSE;
            }
        }

        do {
            ntei = NEW_STRUCT(TetEdgeInfo);
            for(i = 0; i < 8; i++ ) {
                ntei->curves[i] = 0;
            }

            if (ei->head==NULL) {
                ei->head        = ntei;
            } else {
                tei->next       = ntei;
            }

            ntei->next = NULL;
            tei = ntei;

            if (sscanf(section, "%d%c%c%c",
                       &tei->tet_index, &f1, &f2, &dummy) != 3) {
                return FALSE;
            }
            
            if (tei->tet_index > cf->num_tet) {
                cf->num_tet = tei->tet_index;
            }
 
            (tei->tet_index)--;
            
            if ('u' <= f1 && f1 <= 'x') {
                tei->f1 = f1 - 'u';
            } else {
                return FALSE;
            }

            if ('u' <= f2 && f2 <= 'x') {
                tei->f2 = f2 - 'u';
            } else {
                return FALSE;
            }

        } while ((section = parse_token(&line)));
        
    } while ((line = parse_line(file_data)) && (section = parse_token(&line)));

    if (cf->type != not_attempted) {
        // Note that organizer.cpp returns early if the solution
        // type is not attempted.
        // This is not correct since we still need to parse the
        // peripheral curves.
        // The peripheral curves are always there
        // (every call to saveTriangulation says so).
  
        line = parse_line_skipping_empty_lines(file_data);
      
//        if (!( section = parse_token_next_non_empty_line(file_data, &line))) {
//            return FALSE;
//        }

        // Line 873 in organizer.cpp

        ei = cf->head;
        while(ei != NULL) {
            // Orb skips 1
            if (!(section = parse_token(&line))) {
                return FALSE;
            }

            if (!(section = parse_token(&line))) {
                return FALSE;
            }

            if (sscanf(section, "%lf%c", &(ei->e_inner_product), &dummy) != 1) {
                return FALSE;
            }

            if (!(section = parse_token(&line))) {
                return FALSE;
            }

            if (sscanf(section, "%lf%c", &(ei->v_inner_product1), &dummy) != 1) {
                return FALSE;
            }

            if (!(section = parse_token(&line))) {
                return FALSE;
            }

            if (sscanf(section, "%lf%c", &(ei->v_inner_product2), &dummy) != 1) {
                return FALSE;
            }

            tei = ei->head;
            while( tei != NULL) {
                if (!(section = parse_token(&line))) {
                    return FALSE;
                }
                
                if (sscanf(section, "%lf%c", &(tei->dihedral_angle), &dummy) != 1) {
                    return FALSE;
                }

                tei = tei->next;
            }

            ei = ei->next;
            line = parse_line(file_data);
        }
    }

    if (cf->vertices_known) {
        /*
        if (!line) {
            return FALSE;
        }
        */
        line = parse_line_skipping_empty_lines(file_data);
        
        ei = cf->head;
        while(ei != NULL) {
            // Orb skips 1
            if (!(section = parse_token(&line))) {
                return FALSE;
            }

            tei = ei->head;
            while( tei != NULL) {
                for (i = 0; i < 8; i++) {
                    if (!(section = parse_token(&line))) {
                        return FALSE;
                    }
                    
                    if (sscanf(section, "%d%s", &(tei->curves[i]), &dummy) != 1) {
                        return FALSE;
                    }
                }

                tei = tei->next;
            }

            ei = ei->next;
            line = parse_line(file_data);
        }

    }

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
