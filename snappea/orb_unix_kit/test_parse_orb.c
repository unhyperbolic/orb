#include "parse_orb.h"

#include "casson.h"
#include "diagram.h"

#include <stdio.h>

int main()
{
    char *name = NULL;
    CassonFormat * casson = NULL;
    Diagram * diagram = NULL;
    Triangulation * trig = NULL;
    Boolean ok = FALSE;

    read_orb("example.orb", &name, &casson, &diagram);

    if (casson) {
        printf("Status: Got it\n");
    }

    if (name) {
        printf("Status: Name: %s\n", name);
    }

    if (diagram) {
        printf("Status: Got diagram\n");
	printf("%s\n", dump_diagram(diagram));
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
