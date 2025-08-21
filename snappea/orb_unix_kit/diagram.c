#include "diagram.h"

/* for my_free */
#include "kernel.h"
#include "kernel_typedefs.h"

void assign_diagram_arcs(Diagram * diagram)
{
    diagram->num_arcs = 0;
    
    Boolean drilled_arc;

    int queue_length = 0;
    
    /* Each edge appears on the queue at most ones */
    DiagramEdge ** queue = NEW_ARRAY(diagram->num_edges, DiagramEdge *);

    Boolean * visited = NEW_ARRAY(diagram->num_edges, Boolean);

    for (int i = 0; i < diagram->num_edges; i++) {
	DiagramEdge * edge = diagram->edges[i];
	edge->edge_id = i;
	edge->arc_id = -1;
	visited[i] = FALSE;
    }

    {
	DiagramEdge * edge = diagram->edges[0];
	queue[queue_length++] = edge;
	visited[0] = TRUE;
	drilled_arc = edge->type == diagramDrilled;
	if (!drilled_arc) {
	    edge->arc_id = diagram->num_arcs;
	}
    }

    while (queue_length) {
	while (queue_length) {
	    DiagramEdge * e = queue[queue_length - 1];

//	    for (

	}
	
    }
    
}

void assign_diagram_links(Diagram * diagram)
{

}

static void free_diagram_vertex(DiagramVertex *vertex)
{
    int i;
    
    if (!vertex) {
	return;
    }

    if (vertex->incident_end_data) {
	for (i = 0; i < vertex->num_incident_end_data; ++i) {
	    my_free(vertex->incident_end_data[i]);
	}
	my_free(vertex->incident_end_data);
    }
    
    my_free(vertex);
}

void free_diagram(Diagram *diagram)
{
    int i;
    
    if (!diagram) {
	return;
    }

    if (diagram->vertices) {
	for (i = 0; i < diagram->num_vertices; ++i) {
	    free_diagram_vertex(diagram->vertices[i]);
	}
	my_free(diagram->vertices);
    }

    if (diagram->edges) {
	for (i = 0; i < diagram->num_edges; ++i) {
	    my_free(diagram->edges[i]);
	}
	my_free(diagram->edges);
    }

    if (diagram->crossings) {
	for (i = 0; i < diagram->num_crossings; ++i) {
	    my_free(diagram->crossings[i]);
	}
	my_free(diagram->crossings);
    }

    my_free(diagram);
}
