#include "kernel.h"

#include "orb_io.h"
#include "casson_io.h"

#include "diagram.h"

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
	Triangulation * tri = diagram_complement(diagram, FALSE);
	if (tri) {
	    printf("Triangulation\n%s\n", write_casson_format_to_string(tri, TRUE, TRUE, TRUE));
	}
    }

    printf("Status: got trig\n");

    printf("Solution type: %d\n", find_structure(trig, FALSE));

    printf("Volume: %lf\n", my_volume(trig, &ok));
    printf("Vol ok: %d\n", ok);

    free_triangulation(trig);

    return 0;
}
