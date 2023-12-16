#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"

#define PROMPT      "$ "
#define BUFF_SIZE   (MAX_USER_WORDS * MAX_WORD_LEN)  

char *shell_name = NULL;
int return_value = 0;

int main(int argc, char **argv, char **envp)
{
    char cmd_line[BUFF_SIZE];
    token_t *token = NULL;
    int fd = -1;
    shell_name = argv[0];

    if (argc == 2)
    {
        fd = open(argv[1], O_RDONLY);
        if (fd == -1)
        {
            fprintf(stderr, "%s: ", argv[0]);
            perror(argv[1]);
            return 1;
        }
    }
    
    if (fd == -1)
        fd = STDIN_FILENO;
    
    while (1)
    {
        if (isatty(fileno(stdin)) && fd == STDIN_FILENO)
        {
            printf(PROMPT);
            fflush(stdout);
        }
        /* Check if stdin is broken (EOF) */
        if (getUserLine(cmd_line, BUFF_SIZE, fd) <= 0)
            break;
        /* Tokenize the cmd */
        token = tokenize(cmd_line);
        /* Check if the token (line) is empty */
        if (token == NULL)
            continue;
        /* Execute the command */
        executeToken(&token, envp);
        distroyToken(&token);
    }
    exit(return_value);
}
