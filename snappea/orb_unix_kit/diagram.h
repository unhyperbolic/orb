#ifndef _diagram_
#define _diagram_

#include "diagram_typedefs.h"

void initialize_diagram(Diagram *);
void free_diagram(Diagram *);
void initialize_diagram_vertex(DiagramVertex *vertex);
void add_end_data_to_vertex(DiagramEndData * data, DiagramVertex * vertex);

// Corresponds to DiagramCanvas::assign_arcs.
void assign_diagram_arcs(Diagram *);
// Corresponds to DiagramCanvas::assign_links.
void assign_diagram_links(Diagram *);


#endif
