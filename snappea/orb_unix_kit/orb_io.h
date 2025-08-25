#ifndef _orb_io_
#define _orb_io_

#include "diagram.h"

#include "SnapPea.h"

extern void read_orb_from_string(
    char *file_data,
    Triangulation ** trig,
    Diagram ** diagram);

extern void read_orb(
    const char *file_name,
    Triangulation ** trig,
    Diagram ** diagram);

char * write_orb_to_string(
    Triangulation *trig,
    Diagram * diagram);

#endif

