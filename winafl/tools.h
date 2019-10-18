#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern char *DICTPATH;

extern int str_in_dictfile(const char *filepath, const char *str);

extern int is_dictitem_valid(const char *str);

extern int add_one_dictitem(const char *dict, const char *filepath = DICTPATH);
