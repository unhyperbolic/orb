#include "kernel.h"

#include "parse_orb.h"

#include "casson_io.h"
#include "diagram_io.h"

#include "parse_util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* What is happening in gui code:

   loadOrbSlot (in organizer.cpp):
       * reads the line "% orb"
       * reads the next line for the name (but ignores it)
       - readTriangulation
            - readCassonFormat
            - verifyCassonFormat
            - cassonToTriangulation
            - freeCassonFormat
       - readDiagram
            - DiagramCanvas::readDiagram (in interface.cpp)
    	         * seems to be somewhat similar to the plink format (vertices, edges, crossings)
		 * EndData, Edge, Vertex, Crossing in diagram_canvas.h
	         * DiagramCanvas::outputTriangulation creates a Graph

*/


void read_orb_from_string(
        char *str,
        char ** name,
        Triangulation ** trig,
	Diagram ** diagram)
{
    char * l;

    char *p = str;

    printf("Here\n");
    
    l = parse_line(&p);
    if (!l || strcmp(l, "% orb") != 0) {
        return;
    }

    l = parse_line(&p);
    if (!l) {
        return;
    }

    *name = my_strdup(l);
    
    *trig = read_casson_format(&p);

    while (isspace(*p))
    {
	p++;
    }

    if (*p == '\0')
    {
	return;
    }
    
    *diagram = read_diagram(p);
}

void read_orb(
        const char *file_name,
        char **name,
	Triangulation ** trig,
	Diagram ** diagram)
{
    // Follows unit_kit/unix_file_io.c

    FILE * fp = fopen(file_name, "rb");
    if (fp == NULL) {
        return;
    }

    long filesize;
    if ( fseek(fp, 0, SEEK_END) != 0 ||
         (filesize = ftell(fp) ) == -1 ||
         fseek(fp, 0, SEEK_SET) != 0) {
        return;
    }

    char * buffer = (char*) malloc(filesize + 1);
    buffer[filesize] = '\0';

    if ( fread(buffer, filesize, 1, fp) != 1) {
        free(buffer);
        return;
    }

    read_orb_from_string(buffer, name, trig, diagram);

    free(buffer);
}
