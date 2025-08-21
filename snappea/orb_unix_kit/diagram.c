#include "diagram.h"

/* for my_free */
#include "kernel.h"
#include "kernel_typedefs.h"

#include <stdio.h>

void initialize_diagram(Diagram * diagram)
{
    diagram->num_arcs = 0;
    diagram->num_links = 0;
    diagram->num_vertices = 0;
    diagram->vertices = NULL;
    diagram->num_edges = 0;
    diagram->edges = NULL;
    diagram->num_crossings = 0;
    diagram->crossings = NULL;
}

void initialize_diagram_vertex(DiagramVertex *vertex)
{
    vertex->vertex_id = -1;
    vertex->link_id = -1;
    vertex->num_incident_end_data = 0;
    vertex->incident_end_data = NULL;
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

void add_end_data_to_vertex(DiagramEndData * data, DiagramVertex * vertex)
{
    DiagramEndData ** new_incident_end_data =
	NEW_ARRAY(vertex->num_incident_end_data + 1, DiagramEndData*);
    for (int i = 0; i < vertex->num_incident_end_data; i++) {
	new_incident_end_data[i] = vertex->incident_end_data[i];
    }
    new_incident_end_data[vertex->num_incident_end_data++] = data;
    my_free(vertex->incident_end_data);	
    vertex->incident_end_data = new_incident_end_data;
}

void assign_diagram_arcs(Diagram * diagram)
{
    diagram->num_arcs = 0;
    
    Boolean drilled_arc;

    int queue_begin = 0;
    int queue_end = 0;
    
    /* Each edge appears on the queue at most ones */
    DiagramEdge ** queue = NEW_ARRAY(diagram->num_edges, DiagramEdge *);

    Boolean * visited = NEW_ARRAY(diagram->num_edges, Boolean);

    for (int i = 0; i < diagram->num_edges; i++)
    {
	DiagramEdge * edge = diagram->edges[i];
	edge->edge_id = i;
	edge->arc_id = -1;
	visited[i] = FALSE;
    }

    {
	DiagramEdge * edge = diagram->edges[0];
	queue[queue_begin++] = edge;
	visited[0] = TRUE;
	drilled_arc = edge->type == diagramDrilled;
	if (!drilled_arc)
	{
	    edge->arc_id = diagram->num_arcs;
	}
    }

    while (queue_begin < queue_end)
    {
	while (queue_begin < queue_end)
	{
	    DiagramEdge * e = queue[queue_begin++];
	    for (int i = 0; i < 2; i++)
	    {
		DiagramVertex * v = e->vertex[i];

		if (e->vertex[i]->num_incident_end_data == 2)
		{
		    int j = (v->incident_end_data[0]->edge == e) ? 1 : 0;
		    DiagramEdge *e1 = v->incident_end_data[j]->edge;
		    Boolean needsSwitch = v->incident_end_data[j]->type == i;
		    if (!visited[e1->edge_id])
		    {
			visited[e1->edge_id] = TRUE;

			if (!drilled_arc)
			{
			    e1->arc_id = diagram->num_arcs;
			}

			if (needsSwitch)
			{
			    DiagramVertex *temp;
			    temp = e1->vertex[0];
			    e1->vertex[0] = e1->vertex[1];
			    e1->vertex[1] = temp;

			    for (int j = 0; j < 2; j++)
			    {
				for (int k = 0; k < e1->vertex[j]->num_incident_end_data; k++)
				{
				    if (e1->vertex[j]->incident_end_data[k]->edge == e1)
				    {
					e1->vertex[j]->incident_end_data[k]->type =
					    1 - e1->vertex[j]->incident_end_data[k]->type;
				    }
				}
			    }
			}

			queue[queue_end++] = e1;
		    }
		}
	    }
	}

	if (!drilled_arc)
	{
	    diagram->num_arcs++;
	}

	for (int i = 0; i < diagram->num_edges; i++)
	{
	    if (visited[i])
	    {
		continue;
	    }

	    DiagramEdge * edge = diagram->edges[i];
		
	    queue[queue_begin++] = edge;
	    visited[edge->edge_id] = TRUE;
		
	    drilled_arc = edge->type == diagramDrilled;
	    if (!drilled_arc)
	    {
		edge->arc_id = diagram->num_arcs;
	    }

	    break;
	}
    }

    my_free(visited);
    my_free(queue);
}

void assign_diagram_links(Diagram * diagram)
{
    diagram->num_links = 0;

    int queue_begin = 0;
    int queue_end = 0;
    
    /* Each edge appears on the queue at most ones */
    DiagramEdge ** queue = NEW_ARRAY(diagram->num_edges, DiagramEdge *);

    Boolean * visited = NEW_ARRAY(diagram->num_edges, Boolean);

    for (int i = 0; i < diagram->num_edges; i++)
    {
	DiagramEdge * edge = diagram->edges[i];
	edge->edge_id = i;
	edge->link_id = -2;
	visited[i] = edge->type == diagramDrilled;

	if (queue_begin == queue_end && !visited[i])
	{
	    queue[queue_begin++] = edge;
	    visited[i] = TRUE;
	    edge->link_id = diagram->num_links;
	}
    }

    while (queue_begin < queue_end)
    {
	while (queue_begin < queue_end)
	{
	    DiagramEdge * e = queue[queue_begin++];
	    for (int i = 0; i < 2; i++)
	    {
		DiagramVertex * v = e->vertex[i];

		for (int j = 0; j < v->num_incident_end_data; j++)
		{
		    DiagramEdge *e1 = v->incident_end_data[j]->edge;
		    if (!visited[e1->edge_id])
		    {
			visited[e1->edge_id] = TRUE;
			e1->link_id = diagram->num_links;
			queue[queue_end++] = e1;
		    }
		}
	    }
	}

	diagram->num_links++;
	
	for (int i = 0; i < diagram->num_edges; i++)
	{
	    if (visited[i])
	    {
		continue;
	    }
	    DiagramEdge * e = diagram->edges[i];
	    queue[queue_end++] = e;
	    e->link_id = diagram->num_links;
	}
    }

    for (int i = 0; i < diagram->num_vertices; i++) {
	DiagramVertex * v = diagram->vertices[i];
	v->link_id = -1;
	for (int j = 0; j < v->num_incident_end_data; j++)
	{
	    if (v->incident_end_data[j]->edge->type == diagramDrilled) {
		v->link_id = v->incident_end_data[j]->edge->link_id;
		break;
	    }
	}
	if (v->link_id >= 0) {
	    continue;
	}
	if (v->num_incident_end_data > 2) {
	    v->link_id = diagram->num_links++;
	    continue;
	}
	if (v->num_incident_end_data > 2) {
	    Boolean singular_loop = TRUE;
	    int arc = v->incident_end_data[0]->edge->arc_id;

	    for (int j = 0; j < diagram->num_edges; j++)
	    {
		DiagramEdge * e = diagram->edges[j];
		if (e->arc_id != arc)
		{
		    continue;
		}
		for (int k = 0; k < 2; k++) {
		    DiagramVertex * v1 = e->vertex[k];
		    if (v1->num_incident_end_data != 2 ||
			v1->link_id > -1 ||
			v1->incident_end_data[0]->edge->type == diagramDrilled ||
			v1->incident_end_data[1]->edge->type == diagramDrilled)
		    {
			singular_loop = FALSE;
			break;
		    }
		}
		if (!singular_loop)
		{
		    break;
		}
	    }	
	    if (singular_loop)
	    {
		v->link_id = diagram->num_links++;
	    }
	}
    }

    my_free(visited);
    my_free(queue);
}

char * dump_diagram(Diagram * diagram)
{
    size_t size = 10000000;
    
    char * buffer = my_malloc(size);
    char * p = buffer;
    char * end = buffer + size - 1;

    p += snprintf(p, end - p, "num_arcs = %d\n", diagram->num_arcs);
    p += snprintf(p, end - p, "num_links = %d\n", diagram->num_links);
    
    for (int i = 0; i < diagram->num_vertices; i++) {
	DiagramVertex * v = diagram->vertices[i];
	p += snprintf(p, end - p, "Vertex:\n");
	p += snprintf(p, end - p, "    %d %d\n", v->x, v->y);
	p += snprintf(p, end - p, "    %d %d %d\n", v->connected_component, v->vertex_id, v->link_id);
	for (int j = 0; j < v->num_incident_end_data; j++) {
	    DiagramEndData * e = v->incident_end_data[j];
	    p += snprintf(p, end - p, "    End data:\n");
	    p += snprintf(p, end - p, "        Edge: %d\n", e->edge->edge_id);
	    p += snprintf(p, end - p, "        %d %d %lf\n",
			  e->type, e->singular, e->angle);
	}
    }

    for (int i = 0; i < diagram->num_edges; i++) {
	DiagramEdge * e = diagram->edges[i];
	p += snprintf(p, end - p, "Edge:\n");
	p += snprintf(p, end - p, "    %d %d\n", e->vertex[0]->vertex_id, e->vertex[1]->vertex_id);
	p += snprintf(p, end - p, "    %d %d %d %d\n", e->edge_id, e->arc_id, e->link_id, e->type);
	for (int j = 0; j < e->num_crossings; j++) {
	    DiagramCrossing * c = e->crossings[j];
	    p += snprintf(p, end - p, "    Crossing %d\n", c->crossing_id);
	}
    }

    for (int i = 0; i < diagram->num_crossings; i++) {
	DiagramCrossing * c = diagram->crossings[i];
	p += snprintf(p, end - p, "Crossing:\n");
	p += snprintf(p, end - p, "    %d\n", c->crossing_id);
	p += snprintf(p, end - p, "    %d %d  %d\n", c->x, c->y, c->crossing_sign);
	p += snprintf(p, end - p, "    %d %d\n", c->over->edge_id, c->under->edge_id);
	p += snprintf(p, end - p, "    %lf %lf\n", c->position_on_overstrand, c->position_on_understrand);
    }
    
    return buffer;
}
