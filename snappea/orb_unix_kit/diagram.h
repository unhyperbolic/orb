#ifndef _diagram_
#define _diagram_

#include "diagram_typedefs.h"

// Corresponds to DiagramCanvas::assign_arcs.
void assign_diagram_args(Diagram *);
// Corresponds to DiagramCanvas::assign_links.
void assign_diagram_links(Diagram *);

void free_diagram(Diagram *);

#endif
