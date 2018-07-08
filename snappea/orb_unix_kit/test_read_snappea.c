#include <stdio.h>
#include <stdlib.h>

#include "SnapPea.h"
#include "triangulation.h"

#include "casson.h"


//#include "casson_typedefs.h"

#include "unix_file_io.h"


int main()
{
    Boolean ok = FALSE;
    Triangulation * trig = NULL;

    Tetrahedron *tet;
    int i, j;

    trig = get_triangulation( "test.tri" );
    
    fprintf(stderr, "Solution type: %d\n", find_structure(trig, FALSE));

    fprintf(stderr, "Volume: %lf\n", my_volume(trig, &ok));
    fprintf(stderr, "Vol ok: %d\n", ok);

    printf("[\n");
    for (  tet  = trig->tet_list_begin.next;
           tet != &trig->tet_list_end;
           tet  = tet->next) {

        printf("    [\n");
        for (i = 0; i < 4; i++) {
            printf("     [ ");
            for (j = 0; j < 4; j++) {
                printf("%.15f, ", tet->Gram_matrix[i][j]);
            }
            printf(" ],\n");
        }
        printf("   ],\n");
    }
    printf("]\n");

    free_triangulation(trig);

    return 0;
}
