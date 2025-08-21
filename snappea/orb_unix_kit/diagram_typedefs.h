#ifndef _diagram_typedefs_
#define _diagram_typedefs_

#include "kernel_typedefs.h"

typedef struct DiagramEndData DiagramEndData;
typedef struct DiagramEdge DiagramEdge;
typedef struct DiagramVertex DiagramVertex;
typedef struct DiagramCrossing DiagramCrossing;
typedef struct Diagram Diagram;

enum DiagramEndType
{
    diagramBegin,
    diagramEnd
};
typedef enum DiagramEndType DiagramEndType;

enum DiagramEdgeType
{
    diagramSingular = 0,
    diagramDrilled
};
typedef enum DiagramEdgeType DiagramEdgeType;

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
    int cuff_id;
    int arc;
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
    int num_cuffs;
    int num_vertices;
    DiagramVertex **vertices;
    int num_edges;
    DiagramEdge **edges;
    int num_crossings;
    DiagramCrossing **crossings;
};

#endif
