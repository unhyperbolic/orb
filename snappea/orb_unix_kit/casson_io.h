#ifndef _casson_io_
#define _casson_io_

#include "SnapPea.h"

typedef struct OStream OStream;

Triangulation * read_casson_format(char ** str);

char * write_casson_format_to_string(
    Triangulation * manifold,
    Boolean ae,
    Boolean ex,
    Boolean curves);

void write_casson_format_to_stream(
    OStream * stream,
    Triangulation * manifold,
    Boolean ae,
    Boolean ex,
    Boolean curves);

#endif
