#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SnapPea.h"
#include "triangulation.h"

#include "unix_file_io.h"

int main(int argc, char ** argv)
{
    Boolean ok = FALSE;
    Triangulation * trig = NULL;

    Tetrahedron *tet;
    int i, j;

    char outFilename[1024];

    if (argc != 2) {
        printf("Usage: solution_for_snappea_finite_triangulation TRIANGULATION.tri\n");
        exit(1);
    }

    trig = get_triangulation( argv[1] );
    if (!trig) {
        fprintf(stderr, "Not a valid triangulation file\n");
        exit(1);
    }
    
    printf("Solution type: %d\n", find_structure(trig, FALSE));

    printf("Volume: %lf\n", my_volume(trig, &ok));
    printf("Vol ok: %d\n", ok);

    strcpy(outFilename, argv[1]);
    strcat(outFilename, ".vgm");

    printf("Writing vertex gram matrices to file %s\n", outFilename);

    FILE *o = fopen(outFilename, "w");

    if (!o) {
        fprintf(stderr, "Could not open file for writing.");
        exit(1);
    }

    fprintf(o, "[\n");
    for (  tet  = trig->tet_list_begin.next;
           tet != &trig->tet_list_end;
           tet  = tet->next) {

        fprintf(o, "    [\n");
        for (i = 0; i < 4; i++) {
            fprintf(o, "     [ ");
            for (j = 0; j < 4; j++) {
                fprintf(o, "%.15f, ", tet->Gram_matrix[i][j]);
            }
            fprintf(o, " ],\n");
        }
        fprintf(o, "   ],\n");
    }
    fprintf(o, "]\n");

    fclose(o);

    return 0;
}
