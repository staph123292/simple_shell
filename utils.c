#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

size_t stringLength(char *string)
{
    size_t length = 0;
    /* Check the arg */ 
    if (string == NULL)
        return 0;
    /* Got to the end of the string */
    while (string[length++] != '\0');
    /* Return the size */    
    return --length;
}

char *stringCpy(char *src, int start, int end)
{
    char *string = NULL;
    size_t length = 0, dest_length = 0, parser = 0;
    /* Check the arg */
    if (src == NULL)
        return NULL;
    length = stringLength(src);
    /* Check if the copy is possible */
    if (start < 0 || end < start || end >= (int) length)
        return NULL;
    /* Build the substing */
    dest_length = end - start + 1;
    string = (char *) malloc(dest_length + 1);
    /* Copy the substing */
    for (parser = 0; parser < dest_length; parser++)
    {
        string[parser] = src[parser + start];
    }
    string[parser] = '\0';

    return string;
}

void stringCpyIn(char *dest, char *src, size_t size)
{
    size_t parser = 0;
    /* No need to check args here, let the shell crush as expected with strcpy */
    for (parser = 0; parser < size; parser++)
        dest[parser] = src[parser];
}


int stringCompare(const char *s1, const char *s2) 
{
    while (*s1 && (*s1 == *s2)) 
    {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int stringNCompare(const char *s1, const char *s2, size_t n) 
{
    if (n == 0)
        return 0;

    while (n-- && *s1 && (*s1 == *s2)) 
    {
        if (n == 0 || *s1 == '\0' || *s2 == '\0') 
            break;
        s1++;
        s2++;
    }

    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strToken(char* str, const char* delim)
{
    static char *token = NULL;
    static char *next_token = NULL;
    char *current_token = NULL;
    int found_delim = 0, i = 0;

    if (str != NULL)
        token = str;
    
    if (token == NULL || *token == '\0') 
        return NULL;
    
    next_token = token;

    while (*next_token != '\0') 
    {
        for (i = 0; delim[i] != '\0'; i++) 
        {
            if (*next_token == delim[i]) 
            {
                *next_token = '\0';
                found_delim = 1;
                break;
            }
        }
        if (found_delim)
            break;
        
        next_token++;
    }

    current_token = token;
    token = next_token + 1;
    
    return current_token;
}


ssize_t getUserLine(char *buffer, size_t size, int fd)
{
    size_t parser = 0;
    ssize_t read_bytes = 0;
    char holder = 0;

    if (buffer == NULL)
        return read_bytes;
    
    /* Clean the buffer */
    for (parser = 0; parser < size; parser++)
        buffer[parser] = '\0';
    
    parser = 0;

    /* Read from stdin to \n */
    while (parser < size)
    {
        read_bytes = read(fd, &holder, 1);
        if (read_bytes == -1)
        {
            perror(shell_name);
            return read_bytes;
        }
        /* Check if EOF */
        if (read_bytes == 0)
        {
            buffer[parser] = '\0';
            return 0;
        }
        
        /* Remove end of line */
        if (holder == '\n' || holder == ';')
        {
            buffer[parser++] = '\0';
            break;
        }
        buffer[parser++] = holder;
    }
    return parser;
}
