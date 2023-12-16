#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>

#define MAX_USER_WORDS  (64)
#define MAX_WORD_LEN    (128)

#define UNUSED(var)     ((void *) (&var))

extern char *shell_name;
extern int return_value;

typedef struct
{
    char *cmd_list[MAX_USER_WORDS];
    size_t list_size;
} token_t;


size_t stringLength(char *string);
char *stringCpy(char *src, int start, int end);
void stringCpyIn(char *dest, char *src, size_t size);
int stringCompare(const char *s1, const char *s2);
int stringNCompare(const char *s1, const char *s2, size_t n);
char *strToken(char *str, const char *delim);
ssize_t getUserLine(char *buffer, size_t size, int fd);


token_t *tokenize(char *command_line);
void distroyToken(token_t **token);

void executeToken(token_t **token, char **env);

#endif
