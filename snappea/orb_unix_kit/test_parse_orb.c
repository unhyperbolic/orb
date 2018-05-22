#include "parse_orb.h"

#include <stdio.h>

int main()
{
    char *name = NULL;
    CassonFormat * casson = NULL;

    read_orb("example.orb", &name, &casson);

    if (casson) {
        printf("Got it\n");
    }

    if (name) {
        printf("Name: %s\n", name);
    }

    return 0;
}
