#include "orb_io.h"

#include "diagram.h"

#include "triangulation_io.h"

#include "graph_complement.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    Diagram * diagram = NULL;
    Triangulation * trig = NULL;
    Boolean ok = FALSE;

    if (argc != 2) {
	printf("Needs filename\n");
	exit(1);
    }

    read_orb(argv[1], &trig, &diagram);

    if (trig) {
        printf("Status: Got trig %s\n", trig->name);
    }

    if (diagram) {
        printf("Status: Got diagram\n");
	Graph * graph = diagram_to_graph(diagram);
	if (graph) {
	    printf("Num comp graph %d\n", graph->num_components);
	}
	Triangulation *tri = triangulate_graph_complement(graph, FALSE);
	if (tri) {
	    printf("Triangulation\n%s\n", triangulation_to_string(tri, TRUE, TRUE, TRUE));
	}
    }

    printf("Status: got trig\n");

    printf("Solution type: %d\n", find_structure(trig, FALSE));

    printf("Volume: %lf\n", my_volume(trig, &ok));
    printf("Vol ok: %d\n", ok);

    free_triangulation(trig);

    return 0;
}
