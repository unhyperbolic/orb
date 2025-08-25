#ifndef _diagram_io_
#define _diagram_io_

typedef struct Diagram Diagram;

Diagram * read_diagram_from_string(char *str);
char * write_diagram_to_string(Diagram *diagram);

#endif
