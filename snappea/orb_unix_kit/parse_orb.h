#ifndef _parse_orb_
#define _parse_orb_

#include "casson_typedefs.h"
#include "diagram_typedefs.h"

extern void read_orb_from_string(
        char *file_data,
        char **name,
        CassonFormat ** cf,
	Diagram ** diagram);

extern void read_orb(
        const char *file_name,
        char **name,
        CassonFormat ** cf,
	Diagram ** diagram);

#endif

