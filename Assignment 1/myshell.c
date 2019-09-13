#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_SIZE 1024
int array_of_background_processes[MAX_SIZE], count_of_background_processes;
int read_redirection = 0, write_redirection = 0, append_redirection = 0;
char *read_file, *write_file, *append_file;
extern char **environ;
char cwd[MAX_SIZE];
//int killparent;
int dont_run_here;


void end_time(clock_t time_begin) {
    clock_t time_end = clock();
    double time_spent = (double) (time_end - time_begin) / CLOCKS_PER_SEC;
    printf("Time taken to execute command = %f s\n", time_spent);
}

void print_prompt(char cwd[]) {
    getcwd(cwd, MAX_SIZE);
    printf("(%s) MyShell> ", cwd);
}

void removeWS(char *str) {
    //Remove leading whitespaces
    unsigned long i = 0, start = 0, end = 0, lenght = strlen(str);

    for (i = 0; i < lenght; i++) {
        if (str[i] != '\t' && str[i] != ' ') {
            start = i;
            break;
        }
    }
    for (i = 0; start < lenght && start != i; i++, start++) {
        str[i] = str[start];
        str[start] = ' ';
    }

    //Remove trailing whitespaces
    for (i = 0; i < lenght; i++) {
        if (str[i] != '\t' && str[i] != ' ') {
            end = i;
        }
    }
    str[++end] = '\0';
}


void execute(char *cmd, int bg) {
    pid_t pid; // process ID
    if (bg == 1)
        cmd[strlen(cmd) - 1] = '\0';
//    printf("****%s****\n", cmd);
    switch (pid = fork()) {
        case -1:
            printf("fork error");
        case 0: // execution in child process
            pid = getpid(); // get child pid
            if (bg == 1) {
                printf("[%d]\n", pid);
//                print_prompt(cwd);
            }
            char *ptr = strtok(cmd, " ");
            char *array[MAX_SIZE];
            int i = 0;
            while (ptr != NULL) {
                if (!strcmp(ptr, "<") || !strcmp(ptr, ">")) {

                    break;
                }
                array[i++] = ptr;
                ptr = strtok(NULL, " ");
            }
            array[i] = NULL;
            fflush(stdout);

            if (read_redirection) {
                freopen(read_file, "r", stdin);
            }
            if (write_redirection) {
                freopen(write_file, "w", stdout);
            }
            if (append_redirection) {
                freopen(append_file, "a", stdout);
            }

            execvp(array[0], array);
            if (!strcmp(array[0], "quit"))
                exit(0);
            if (!strcmp(array[0], "cd"))
                exit(0);
            if (!strcmp(array[0], "dir")) {
                ptr = array[1];
                char dir_command[MAX_SIZE] = "ls -al ";
                if (ptr == NULL) {
                    system(dir_command);
                } else {
                    strcat(dir_command, ptr);
                    system(dir_command);
//                    end_time(time_begin);
                }
                exit(0);
            }
            if (!strcmp(array[0], "env")) {
                setenv("COURSE", "CS_744", 0);
                setenv("ASSIGNMENT", "ASSIGNMENT_1", 0);
                for (i = 0; environ[i] != NULL; i++) {
                    printf("%s\n", environ[i]);
                }
                exit(0);
            }
            if (!strcmp(array[0], "clr")) {
                write(1, "\33[1;1H\33[2J", 10);
                exit(0);
            }
            printf("ERROR: Command does not exist\n");
            exit(0);
        default:
            removeWS(cmd);
            int ran_cd = 0;
            char *parentcommand = strdup(cmd);
            char *token = strtok_r(parentcommand, " ", &parentcommand);
            if (token != NULL && !strcmp(token, "cd")) {
                ran_cd = 1;
                char *newdir = strtok_r(NULL, " ", &parentcommand);
                if (newdir == NULL) {
                    printf("BEFORE: %s\nAFTER: %s\n", cwd, cwd);
                } else {
                    char *directory = newdir;
                    DIR *dir = opendir(directory);
                    if (dir) {
                        char *oldpwd = getenv("OLDPWD");
                        if (!oldpwd)
                            oldpwd = "Empty";
                        printf("BEFORE:\nOLDPWD = %s\nPWD = %s\n", oldpwd, getenv("PWD"));
                        setenv("OLDPWD", getenv("PWD"), 1);
                        chdir(directory);
                        getcwd(cwd, sizeof(cwd));
                        setenv("PWD", cwd, 1);
                        printf("AFTER:\nOLDPWD = %s\nPWD = %s\n", getenv("OLDPWD"), getenv("PWD"));
                        closedir(dir);
                    } else if (ENOENT == errno) {
                        /* Directory does not exist. */
                        printf("ERROR: Directory does not exist");
                    } else {
                        /* opendir() failed for some other reason. */
                        printf("ERROR: Failed");
                    }
                }

            }
            if (bg == 0) {
                int status;
                if (!ran_cd)
                    waitpid(pid, &status, 0);
            } else if (bg == 1)
                array_of_background_processes[count_of_background_processes++] = pid;
    }
}

//TODO: Free memory
int main(int argc, char **argv) {
    char *command;

    //For child process
    int status;

    char buf[50000];
    memset(buf, 0, 50000);
    while (1) {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            perror(0);
            exit(1);
        }
        print_prompt(cwd);
        fgets(buf, sizeof(buf), stdin);
//        sleep(1);
        command = strdup(buf);
        read_redirection = 0, write_redirection = 0, append_redirection = 0;
        int bg_process = 0, fg_serial_processes = 0, fg_parallel_processes = 0;

        getcwd(cwd, MAX_SIZE);

        if (command[strlen(command) - 1] == '\n')
            command[strlen(command) - 1] = '\0';

        time_t time_begin = clock();
        removeWS(command);
        dont_run_here = 0;
        for (unsigned long ctr = 0; ctr < strlen(command); ctr++) {
            if (command[ctr] == '<' || command[ctr] == '>' || command[ctr] == '&') {
                dont_run_here = 1;
                break;
            }
        }

        for (int ct = 0; ct <= count_of_background_processes; ct++) {
            if (array_of_background_processes[ct] && (waitpid(array_of_background_processes[ct], &status, WNOHANG))) {
                printf("MyShell: Background process [%d] finished\n", array_of_background_processes[ct]);
                array_of_background_processes[ct] = 0;
            }
        }
        if (!strlen(command))
            continue;

        //Analyse
        char *cmdcpy = strdup(command);

        char *ptr = strtok(cmdcpy, " ");

        if (!dont_run_here && ptr && !strcmp(ptr, "quit")) {
            exit(0);
        } else if (!dont_run_here && ptr && !strcmp(ptr, "cd")) {
            removeWS(ptr);
            ptr = strtok(NULL, " ");
            if (ptr == NULL || !strcmp(ptr, "&&") || !strcmp(ptr, "&&&")) {
                printf("BEFORE: %s\nAFTER: %s\n", cwd, cwd);
            } else {
                char *directory = ptr;
                DIR *dir = opendir(directory);
                if (dir) {
                    char *oldpwd = getenv("OLDPWD");
                    if (!oldpwd)
                        oldpwd = "Empty";
                    printf("BEFORE:\nOLDPWD = %s\nPWD = %s\n", oldpwd, getenv("PWD"));
                    setenv("OLDPWD", getenv("PWD"), 1);
                    chdir(directory);
                    getcwd(cwd, sizeof(cwd));
                    setenv("PWD", cwd, 1);
                    printf("AFTER:\nOLDPWD = %s\nPWD = %s\n", getenv("OLDPWD"), getenv("PWD"));
                    closedir(dir);
                } else if (ENOENT == errno) {
                    /* Directory does not exist. */
                    printf("ERROR: Directory does not exist");
                } else {
                    /* opendir() failed for some other reason. */
                    printf("Error");
                }
            }
            end_time(time_begin);
        } //End of cd command
        else if (!dont_run_here && ptr && !strcmp(ptr, "dir")) {
            ptr = strtok(NULL, " ");
            char dir_command[MAX_SIZE] = "ls -al ";
            if (ptr == NULL) {
                system(dir_command);
            } else {
                strcat(dir_command, ptr);
                system(dir_command);
                end_time(time_begin);
            }
        } else if (!dont_run_here && ptr && !strcmp(ptr, "env")) {
            setenv("COURSE", "CS_744", 0);
            setenv("ASSIGNMENT", "ASSIGNMENT_1", 0);
            for (int i = 0; environ[i] != NULL; i++) {
                printf("%s\n", environ[i]);
            }
//            printf("COURSE=%s\nASSIGNMENT=%s\nPWD=%s\n", getenv("COURSE"), getenv("ASSIGNMENT"), getenv("PWD"));
            end_time(time_begin);
        } else if (!dont_run_here && ptr && !strcmp(ptr, "clr")) {
            write(1, "\33[1;1H\33[2J", 10);
            end_time(time_begin);
        } else {
            while (ptr != NULL) {
                if (!strcmp(ptr, "&"))
                    bg_process = 1;
                else if (!strcmp(ptr, "&&"))
                    fg_serial_processes = 1;
                else if (!strcmp(ptr, "&&&"))
                    fg_parallel_processes = 1;

                ptr = strtok(NULL, " ");
            }
            cmdcpy = strdup(command);
            removeWS(command);
            if (bg_process) {
                char *cmdcpy = strdup(command);
                char *word = strtok_r(command, " ", &command);
                while (word != NULL) {
                    if (!strcmp(word, "<")) {
                        read_redirection = 1;
                        read_file = strtok_r(NULL, " ", &command);
                        removeWS(read_file);
                    }
                    if (!strcmp(word, ">")) {
                        write_redirection = 1;
                        write_file = strtok_r(NULL, " ", &command);
                        removeWS(write_file);
                    }
                    if (!strcmp(word, ">>")) {
                        append_redirection = 1;
                        append_file = strtok_r(NULL, " ", &command);
                        removeWS(append_file);
                    }
                    word = strtok_r(NULL, " ", &command);
                }


                if (strcmp(cmdcpy, "&") != 0)
                    execute(cmdcpy, bg_process);
            } else if (fg_serial_processes) {
                ptr = strtok_r(cmdcpy, "&&", &cmdcpy);


                while (ptr != NULL) {

                    char *cmd = strdup(ptr);
                    char *word = strtok_r(cmd, " ", &cmd);
                    while (word != NULL) {
                        if (!strcmp(word, "<")) {
                            read_redirection = 1;
                            read_file = strtok_r(NULL, " ", &cmd);
                            removeWS(read_file);
                        }
                        if (!strcmp(word, ">")) {
                            write_redirection = 1;
                            write_file = strtok_r(NULL, " ", &cmd);
                            removeWS(write_file);
                        }
                        if (!strcmp(word, ">>")) {
                            append_redirection = 1;
                            append_file = strtok_r(NULL, " ", &cmd);
                            removeWS(append_file);
                        }
                        word = strtok_r(NULL, " ", &cmd);
                    }

                    removeWS(ptr);
                    execute(ptr, bg_process);
                    read_redirection = 0;
                    write_redirection = 0;
                    append_redirection = 0;
                    ptr = strtok_r(NULL, "&&", &cmdcpy);
                }
            } else if (fg_parallel_processes) {
                ptr = strtok(cmdcpy, "&&&");
                int count = 1;
                while (ptr != NULL) {
                    char *cmd = strdup(ptr);
                    char *word = strtok_r(cmd, " ", &cmd);
                    while (word != NULL) {
                        if (!strcmp(word, "<")) {
                            read_redirection = 1;
                            read_file = strtok_r(NULL, " ", &cmd);
                            removeWS(read_file);
                        }
                        if (!strcmp(word, ">")) {
                            write_redirection = 1;
                            write_file = strtok_r(NULL, " ", &cmd);
                            removeWS(write_file);
                        }
                        if (!strcmp(word, ">>")) {
                            append_redirection = 1;
                            append_file = strtok_r(NULL, " ", &cmd);
                            removeWS(append_file);
                        }
                        word = strtok_r(NULL, " ", &cmd);
                    }
                    removeWS(ptr);
                    if (strcmp(ptr, "") != 0)
                        execute(ptr, 2);
                    read_redirection = 0;
                    write_redirection = 0;
                    append_redirection = 0;
                    count++;
                    ptr = strtok(NULL, "&&&");
                }
                for (int i = 0; i < count; i++) {
                    wait(NULL);
                }

            } else if (read_redirection || write_redirection || append_redirection) {
                cmdcpy = strdup(command);
                int i = 0;
                while (cmdcpy[i] != '<' && cmdcpy[i] != '>') {
                    i++;
                }
                cmdcpy[i] = '\0';
                execute(cmdcpy, bg_process);
            } else {
                char *cmdcpy = strdup(command);
                char *word = strtok_r(command, " ", &command);
                while (word != NULL) {
                    if (!strcmp(word, "<")) {
                        read_redirection = 1;
                        read_file = strtok_r(NULL, " ", &command);
                        removeWS(read_file);
                    }
                    if (!strcmp(word, ">")) {
                        write_redirection = 1;
                        write_file = strtok_r(NULL, " ", &command);
                        removeWS(write_file);
                    }
                    if (!strcmp(word, ">>")) {
                        append_redirection = 1;
                        append_file = strtok_r(NULL, " ", &command);
                        removeWS(append_file);
                    }
                    word = strtok_r(NULL, " ", &command);
                }
                for(unsigned long i =0; i< strlen(cmdcpy);i++){
                    if(cmdcpy[i]=='<' || cmdcpy[i]=='>')
                    {
                        cmdcpy[i]='\0';
                        break;
                    }
                }
                execute(cmdcpy, bg_process);
            }
        }
        memset(buf, 0, 50000);
    }
}