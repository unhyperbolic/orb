#ifndef _parse_orb_
#define _parse_orb_

#include "casson.h"

extern void read_orb_from_string(
        char *file_data,
        char **name,
        CassonFormat ** cf);

extern void read_orb(
        const char *file_name,
        char **name,
        CassonFormat ** cf);

extern void free_casson(
        CassonFormat * cf);

#endif

