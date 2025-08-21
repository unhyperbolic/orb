#ifndef _diagram_
#define _diagram_

#include "diagram_typedefs.h"

void initialize_diagram(Diagram *);
void free_diagram(Diagram *);
void initialize_diagram_vertex(DiagramVertex *vertex);
void add_end_data_to_vertex(DiagramEndData * data, DiagramVertex * vertex);
void add_crossing_to_edge(DiagramCrossing * crossing, DiagramEdge * edge);
void initialize_diagram_edge(DiagramEdge * edge);

char * dump_diagram(Diagram * diagram);

// Corresponds to DiagramCanvas::assign_arcs.
void assign_diagram_arcs(Diagram *);
// Corresponds to DiagramCanvas::assign_links.
void assign_diagram_links(Diagram *);

void
diagram_get_crossing_signs(Diagram * diagram);

void
diagram_ed_angles(Diagram * diagram);

void
assign_diagram_crossings_to_edges(Diagram * diagram);

void
prepare_diagram_components_for_output(Diagram * diagram);

int diagram_get_strand(DiagramEdge * e, DiagramVertex * v);

DiagramCrossing * get_next_crossing(DiagramEdge *e, DiagramCrossing *c);
DiagramCrossing * get_prev_crossing(DiagramEdge *e, DiagramCrossing *c);

#endif
