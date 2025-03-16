#ifndef _mg_STRING_UTILS_H_
#define _mg_STRING_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

extern const char* AllSeparators;

const char* mg_find_string(const char* pBuffer, const char* S);
char* mg_find_string_nc(char* pBuffer, const char* S);
int mg_count_string(const char* pBuffer, const char* S);
char* mg_resize_if_needed(char* pBuffer, int *size, int addsize);
char* mg_inplace_replace(char* pBuffer, int* size, const char* S, const char* D);
char* mg_append(char* pBuffer, int* size, const char* S);
char* mg_inplace_insert(char* pBuffer, const char* S, char* master, int* size);
char* mg_getline(char* pBuffer, int num);
int mg_countline(const char* pBuffer);
int mg_getline_for(const char* pBuffer, const char* S); // get the line number for 1st occurent of S in pBuffer
char* mg_str_next(char *pBuffer, const char* S); // mostly as strstr, but go after the substring if found
//"blank" (space, tab, cr, lf,":", ",", ";", ".", "/")
char* mg_next_str(char* pBuffer);   // go to next non "blank"
char* mg_prev_str(char* Str, char* pBuffer);    // go to previous non "blank"
char* mg_next_blank(char* pBuffer);   // go to next "blank"
char* mg_next_line(char* pBuffer);   // go to next new line (crlf not included)

const char* mg_get_next_str(char* pBuffer); // get a (static) copy of next str (until next separator), can be a simple number or separator also

// those function don't try to be smart with separators...
int mg_countstring_simple(char* pBuffer, const char* S);
char* mg_inplace_replace_simple(char* pBuffer, int* size, const char* S, const char* D);

#ifdef __cplusplus
}
#endif

#endif // _mg_STRING_UTILS_H_
