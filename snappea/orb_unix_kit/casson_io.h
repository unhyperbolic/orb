#ifndef _casson_io_
#define _casson_io_

#include "SnapPea.h"

typedef struct OStream OStream;

Triangulation * read_casson_format(char ** str);

void write_casson_format_to_stream(
    OStream * stream,
    Triangulation * manifold,
    Boolean include_angular_error,
    Boolean include_geometric_structure_and_cusps,
    Boolean include_peripheral_curves);

char * write_casson_format_to_string(
    Triangulation * manifold,
    Boolean include_angular_error,
    Boolean include_geometric_structure_and_cusps,
    Boolean include_peripheral_curves);


#endif
