#include "parse_util.h"

#include <stddef.h>

char *parse_line(char **text)
{
    char * result;
    char c;

    if (*text == NULL) {
        return 0;
    }

    result = *text;

    while (**text && (**text != '\r') && (**text != '\n')) {
        (*text) ++;
    }

    c = **text;

    if (!c) {
        *text = 0;
	return result;
    }

    **text = 0;
    (*text)++;

    if ( (c == '\r' && **text == '\n') ||
 	 (c == '\n' && **text == '\r')) {
        (*text)++;
    }
    
    return result;
}

char *parse_token(char **text)
{
    char * result;

    if (*text == NULL) {
        return 0;
    }
    
    while (**text && (**text == ' ' || **text == '\t')) {
      (*text)++;
    }

    if (!**text) {
        return 0;
    }
    
    result = *text;

    while (**text && **text != ' ' && **text != '\t') {
      (*text)++;
    }
    
    if (**text) {
        **text = 0;
	(*text)++;
    }
    
    return result;
}
