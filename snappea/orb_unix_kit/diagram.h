#ifndef _diagram_
#define _diagram_

#include "SnapPea.h"

typedef struct DiagramEndData DiagramEndData;
typedef struct DiagramEdge DiagramEdge;
typedef struct DiagramVertex DiagramVertex;
typedef struct DiagramCrossing DiagramCrossing;
typedef struct Diagram Diagram;
typedef enum DiagramEdgeType DiagramEdgeType;
typedef enum DiagramEndType DiagramEndType;

typedef struct Graph Graph;

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

Graph * diagram_to_graph(Diagram *);
Triangulation * diagram_complement(Diagram *, Boolean remove_vertices);

enum DiagramEndType
{
    diagramBegin = 0,
    diagramEnd
};

enum DiagramEdgeType
{
    diagramSingular = 0,
    diagramDrilled
};

struct DiagramEndData
{
    DiagramEdge *edge;
    DiagramEndType type;
    Boolean singular;
    double angle;
};

struct DiagramVertex
{
    int x, y;
    int connected_component;
    int vertex_id;
    int link_id;
    int num_incident_end_data;
    DiagramEndData **incident_end_data;
};

struct DiagramCrossing
{
    int x, y;
    int crossing_id;
    int crossing_sign;
    DiagramEdge *over, *under;
    double position_on_overstrand, position_on_understrand;
};

struct DiagramEdge
{
    DiagramVertex *vertex[2];
    int num_crossings;
    DiagramCrossing **crossings;
    int arc_id;
    int link_id;
//    int cuff_id;
//    int arc;
    int edge_id;
//    int thickness; /* ?? */
//    bool selected; /* ?? */
//    int a, b, c;
    DiagramEdgeType type;
};

struct Diagram
{
    int num_arcs;
    int num_links;
    int num_vertices;
    DiagramVertex **vertices;
    int num_edges;
    DiagramEdge **edges;
    int num_crossings;
    DiagramCrossing **crossings;
};

#endif
