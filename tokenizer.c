#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void initToken(token_t *token)
{
    int i = 0;
    token->list_size = 0;
    /* Set all words to NULL */ 
    for (i = 0; i < MAX_USER_WORDS; i++)
        token->cmd_list[i] = NULL;
}

static void fillToken(token_t *token, char *line)
{
    size_t line_size = stringLength(line);
    size_t parser = 0;
    int sub_start = 0, sub_end = 0;

    /* Parse the line */
    while (token->list_size < MAX_USER_WORDS - 1 && parser < line_size)
    {
        /* Delimit the word */
        while (line[parser] != ' ' && parser < line_size)
        {
            sub_end++;
            parser++;
        }
        
        /* Copy the word */
        if (sub_end != 0)
        {
            if (line[sub_start] == '#')
                break;
            token->cmd_list[token->list_size++] = stringCpy(line, sub_start, sub_end - 1);
        }
        
        sub_start = sub_end;
     
        if (parser >= line_size) 
            break;
        
        /* Skip white spaces */
        while (line[parser] == ' ' && parser < line_size)
        {
            sub_start++;
            parser++;
        }

        sub_end = sub_start;
    }
}

token_t *tokenize(char *command_line)
{
    size_t cmd_size = 0;
    token_t *token = NULL;

    /* Check arg */
    if (command_line == NULL)
        return NULL;
    /* Get command size */
    cmd_size = stringLength(command_line);
    /* Check if nothing to do */
    if (cmd_size == 0)
        return NULL;
    /* Build/Init token */
    token = (token_t *) malloc(sizeof(token_t));
    initToken(token);
    /* Fill words */
    fillToken(token, command_line);
    return token;
}

void distroyToken(token_t **token)
{
    size_t parser = 0;
    if (token == NULL || *token == NULL)
        return;
    
    for (parser = 0; parser < (*token)->list_size; parser++)
        free((*token)->cmd_list[parser]);

    free(*token);
}
