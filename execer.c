#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

static char **buildCommand(token_t *token)
{
    size_t i = 0;
    char **command = (char **) malloc((token->list_size + 1) * sizeof(char *));

    for (i = 0; i < token->list_size; i++)
        command[i] = stringCpy(token->cmd_list[i], 0, stringLength(token->cmd_list[i]) - 1);
        
    command[i] = NULL;

    return command;
}

static void distroyCommand(char **command, size_t size)
{
    size_t parser = 0;
    for (parser = 0; parser < size + 1; parser++)
    {
        free(command[parser]);
    }
    free(command);
}

static int getAbsolutePath(char *command, char **env, char **absolute_path)
{
    char **env_parser = NULL;
    char *path_tokens = NULL;
    char abs_path[1024];

    *absolute_path = NULL;

    /* Search for PATH variable */
    env_parser = env;
    while (*env_parser)
    {
        if (stringNCompare(*env_parser, "PATH=", 5) == 0)
            break;
        env_parser++;
    }
    /* Parse paths */
    path_tokens = strToken(&(*env_parser)[5], ":");
    
    while (path_tokens)
    {
        snprintf(abs_path, sizeof(abs_path), "%s/%s", path_tokens, command);
        if (access(abs_path, F_OK) == 0 && access(abs_path, X_OK) == 0)
        {
            /* Command found */
            *absolute_path = stringCpy(abs_path, 0, stringLength(abs_path) - 1);
            return 1;
        }
        path_tokens = strToken(NULL, ":");
    }

    return 0;
}

static int execute_cd(token_t **token, char **env)
{
    static char prev_path[1024];
    char **env_parser = NULL;
    char *path = NULL, *token_path = NULL;

    if ((*token)->list_size == 1)
    {
        /* Get HOME env */
        env_parser = env;
        while (*env_parser)
        {
            if (stringNCompare(*env_parser, "HOME=", 5) == 0)
            {
                strToken(*env_parser, "=");
                token_path = strToken(NULL, "=");
                path = stringCpy(token_path, 0, stringLength(token_path) - 1);
                break;
            }
            env_parser++;
        }
        /* Check if HOME exist */ 
        if (path == NULL)
        {
            fprintf(stderr, "HOME variable is unset\n");
            goto err;
        }
    }
    else if ((*token)->list_size == 2)
    {
        if (stringCompare((*token)->cmd_list[1], "-") == 0)
            path = stringCpy(prev_path, 0, stringLength(prev_path) - 1);
        else
            path = stringCpy((*token)->cmd_list[1], 0, stringLength((*token)->cmd_list[1]) - 1);
    }
    else
    {
        fprintf(stderr, "Invalid syntax\n");
        goto err;
    }

    /* Save current path */
    if (getcwd(prev_path, sizeof(prev_path)) == NULL)
    {
        fprintf(stderr, "Can't get current directory\n");
        goto err;
    }
    
    if (chdir(path) != 0)
    {
        fprintf(stderr, "Can't change current directory\n");
        goto err;
    }
    
    free(path);
    return 0;
    
    err:
    free(path);
    return 1;
}

static int execBuiltIn(token_t **token, char **env)
{
    char **env_parser = NULL;
    char holder[MAX_WORD_LEN];
    size_t parser = 0, length = 0;
    ssize_t free_index = -1;

    /* Check if built in command */
    if (stringCompare((*token)->cmd_list[0], "exit") == 0)
    {
        if ((*token)->list_size > 1)
            return_value = atoi((*token)->cmd_list[1]);
        distroyToken(token);
        exit(return_value);
    }
    if (stringCompare((*token)->cmd_list[0], "env") == 0)
    {
        env_parser = env;
        while (*env_parser)
        {
            if ((*env_parser)[0] != '\0')
                printf("%s\n", *env_parser);
            env_parser++;
        }
        return 0;
    }
    if (stringCompare((*token)->cmd_list[0], "setenv") == 0)
    {
        if ((*token)->list_size != 3)
        {
            fprintf(stderr, "%s: %d: setenv: Invalid syntax\n", shell_name, 1);
            return 1;
        }
        
        length = stringLength((*token)->cmd_list[1]);
        sprintf(holder, "%s=%s", (*token)->cmd_list[1], (*token)->cmd_list[2]);
        holder[length + stringLength((*token)->cmd_list[2]) + 2] = '\0';

        for (parser = 0; env[parser] != NULL; parser++)
        {
            if (stringNCompare(env[parser], holder, length + 1) == 0)
            {
                stringCpyIn(env[parser], holder, stringLength(holder) + 1);
                return 0;
            }
            if (env[parser][0] == '\0')
                free_index = parser;
        }
        if (free_index == -1)
        {
            fprintf(stderr, "Cannot add new variable\n");
            return 1;   
        }
        stringCpyIn(env[free_index], holder, stringLength(holder) + 1);
        return 0;
    }
    if (stringCompare((*token)->cmd_list[0], "unsetenv") == 0)
    {
        if ((*token)->list_size != 2)
        {
            fprintf(stderr, "%s: %d: unsetenv: Invalid syntax\n", shell_name, 1);
            return 1;
        }
        
        length = stringLength((*token)->cmd_list[1]);
        sprintf(holder, "%s=", (*token)->cmd_list[1]);
        holder[length + 2] = '\0';

        for (parser = 0; env[parser] != NULL; parser++)
        {
            if (stringNCompare(env[parser], holder, length + 1) == 0)
            {
                env[parser][0] = '\0';
                return 0;
            }
        }
        fprintf(stderr, "Cannot unset %s\n", (*token)->cmd_list[1]);
        return 1;
    }
    if (stringCompare((*token)->cmd_list[0], "cd") == 0)
    {
        execute_cd(token, env);
        return 0;
    }
    if (stringCompare((*token)->cmd_list[0], "echo") == 0)
    {
        if ((*token)->list_size == 1)
            return 0;

        if ((*token)->cmd_list[1][0] != '$')
        {
            printf("%s\n", (*token)->cmd_list[1]);
            return 0;
        }

        if (stringCompare((*token)->cmd_list[1], "$?") == 0)
            printf("%d\n", return_value);
        
        else if (stringCompare((*token)->cmd_list[1], "$$") == 0)
            printf("%d\n", getpid());
        else
        {
            for (parser = 0; env[parser]; parser++)
            {
                length = stringLength(&(*token)->cmd_list[1][1]);
                if (stringNCompare(env[parser], &(*token)->cmd_list[1][1], length) == 0 && env[parser][length] == '=')
                {
                    printf("%s\n", env[parser]);
                    return 0;
                }
            }
        }
        return 0;
    }
    return -1;
}

void executeToken(token_t **token, char **env)
{
    pid_t pid = -1;
    size_t length = 0;
    int ret_code = 0;
    char **command = NULL;
    char *absolute_path = NULL;
    char holder[1024];

    /* Check arg */
    if (*token == NULL || (*token)->list_size == 0)
        return;

    /* Check if built in command */
    if ((ret_code = execBuiltIn(token, env)) != -1)
    {
        return_value = ret_code;
        return;
    }
    /* Check if contain absolut path */
    if ((*token)->cmd_list[0][0] == '/')
    {
        length = stringLength((*token)->cmd_list[0]);
        absolute_path = stringCpy((*token)->cmd_list[0], 0, length - 1);
    }
    else if ((*token)->cmd_list[0][0] == '.')
    {
        absolute_path = (char *) malloc(1024);
        getcwd(holder, sizeof(holder));
        sprintf(absolute_path, "%s/%s", holder, (*token)->cmd_list[0]);
    }
    /* Check if the executable exists */
    else if (getAbsolutePath((*token)->cmd_list[0], env, &absolute_path) == 0)
    {
        fprintf(stderr, "%s: %d: %s: not found\n", shell_name, 1, (*token)->cmd_list[0]);
        return_value = 127;
        return;
    }

    command = buildCommand(*token);

    pid = fork();

    if (pid < 0)
    {
        perror(shell_name);
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        ret_code = execve(absolute_path, command, env);
        fprintf(stderr, "%s: %d: ", shell_name, ret_code * -1);
        perror(command[0]);
        exit(ret_code);
    }
    else
    {
        wait(&ret_code);
        /* Get child return value */
        if (WIFEXITED(ret_code))
            return_value = WEXITSTATUS(ret_code);

        distroyCommand(command, (*token)->list_size);
        free(absolute_path);
    }
}
