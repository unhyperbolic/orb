#ifndef _orb_io_
#define _orb_io_

#include "diagram.h"

#include "SnapPea.h"

/* Corresponds to Organizer::loadOrbSlot in gui/organizer/cpp */
extern void read_orb_from_string(
    char *file_data,
    Triangulation ** trig,
    Diagram ** diagram);

extern void read_orb(
    const char *file_name,
    Triangulation ** trig,
    Diagram ** diagram);

/* Corresponds to ManifoldInterface::saveSlot in gui/manifold_interface.cpp */
char * write_orb_to_string(
    Triangulation *trig,
    Diagram * diagram);

#endif

