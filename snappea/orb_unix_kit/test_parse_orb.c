#include "parse_orb.h"

#include "casson.h"
#include "diagram.h"
#include "diagram_to_graph.h"

#include "triangulation_io.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char *name = NULL;
    CassonFormat * casson = NULL;
    Diagram * diagram = NULL;
    Triangulation * trig = NULL;
    Boolean ok = FALSE;

    if (argc != 2) {
	printf("Needs filename\n");
	exit(1);
    }

    read_orb(argv[1], &name, &casson, &diagram);

    if (casson) {
        printf("Status: Got it\n");
    }

    if (name) {
        printf("Status: Name: %s\n", name);
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

    if (!verify_casson(casson)) {
        printf("Status: verify_casson failed\n");
        return 1;
    }

    printf("Status: verify_casson passed\n");

    trig = casson_to_triangulation(casson);

    printf("Status: got trig\n");

    printf("Solution type: %d\n", find_structure(trig, FALSE));

    printf("Volume: %lf\n", my_volume(trig, &ok));
    printf("Vol ok: %d\n", ok);

    free_casson(casson);

    free_triangulation(trig);

    return 0;
}
