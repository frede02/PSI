/*
  Projet minishell - Licence 3 Info - PSI 2023
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Gestion des processus (implémentation).
 
 */

#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

int exec_cmd(cmd_t* p) {
    // Exécute la commande en fonction des attributs de la structure
    // Crée un nouveau processus et détourne éventuellement les entrées/sorties
    // Attend si la commande est lancée en "avant-plan" et initialise le code de retour

    pid_t pid = fork();
    if (pid == -1) {
        // Erreur de fork
        perror("fork");
        return -1;
    } else if (pid == 0) {
        // Code du fils
        if (p->in_stream != 0) {
            dup2(p->in_stream, 0);
            close(p->in_stream);
        }
        if (p->out_stream != 1) {
            dup2(p->out_stream, 1);
            close(p->out_stream);
        }
        if (p->err_stream != 2) {
            dup2(p->err_stream, 2);
            close(p->err_stream);
        }
        if (execvp(p->path, p->argv) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
     } else {
        // Code du parent
        if (!p->background) {
            int status;
            waitpid(pid, &status, 0);
            p->status = WEXITSTATUS(status);
        }
    }
    return 0;
}

int init_cmd(cmd_t* p) {
    // Initialise une structure cmd_t avec les valeurs par défaut
    p->pid = 0;
    p->status = 0;
    p->in_stream = 0;
    p->out_stream = 1;
    p->err_stream = 2;
    p->wait = 1;
    p->path = malloc(MAX_LINE_SIZE * sizeof(char)); // Allouer de la mémoire pour le champ path
    memset(p->argv, 0, MAX_CMD_SIZE * sizeof(char*));
    memset(p->fdclose, 0, MAX_CMD_SIZE * sizeof(int));
    p->next = NULL;
    p->next_success = NULL;
    p->next_failure = NULL;
    return 0;
}

int parse_cmd(char* tokens[], cmd_t* cmds, size_t max) {
    int current_cmd = 0;
    int current_arg = 0;
    int redirect_in = 0; // Variable pour la redirection d'entrée
    int redirect_out = 0; // Variable pour la redirection de sortie
    int redirect_err = 0; // Variable pour la redirection d'erreur

    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], ">&2") == 0) {
            // Redirection de la sortie standard vers la sortie d'erreur
            cmds[current_cmd].out_stream = cmds[current_cmd].err_stream;
        } else if (strcmp(tokens[i], "2>&1") == 0) {
            // Redirection de la sortie d'erreur vers la sortie standard
            cmds[current_cmd].err_stream = cmds[current_cmd].out_stream;
        } else if (strcmp(tokens[i], "<") == 0) {
            // Redirection d'entrée détectée
            redirect_in = 1;
        } else if (strcmp(tokens[i], ">") == 0) {
            // Redirection de sortie détectée
            redirect_out = 1;
        } else if (strcmp(tokens[i], ">>") == 0) {
            // Redirection de sortie en mode ajout détectée
            redirect_out = 2;
        } else if (strcmp(tokens[i], "2>") == 0) {
            // Redirection d'erreur détectée
            redirect_err = 1;
        } else if (strcmp(tokens[i], "2>>") == 0) {
            // Redirection d'erreur en mode ajout détectée
            redirect_err = 2;
    }else if (strcmp(tokens[i], "<<") == 0) {
    // Redirection d'entrée ici-document détectée
    i++; // Passer au mot suivant qui marque la fin de l'entrée ici-document
    if (tokens[i] == NULL) {
        fprintf(stderr, "Erreur: mot de fin pour ici-document manquant\n");
        exit(EXIT_FAILURE);
    }
    char* end_word = tokens[i];
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    cmds[current_cmd].in_stream = pipe_fds[0]; // Utiliser le côté lecture du pipe comme entrée standard

    // Créer un processus pour écrire dans le pipe
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Processus enfant
        close(pipe_fds[0]); // Fermer le côté lecture du pipe
        char line[256];
        while (1) {
            printf("> "); // Invite pour l'entrée de l'utilisateur
            if (fgets(line, sizeof(line), stdin) == NULL) {
                break; // Fin de fichier ou erreur
            }
            if (strncmp(line, end_word, strlen(end_word)) == 0 && line[strlen(end_word)] == '\n') {
                break; // Mot de fin atteint
            }
            write(pipe_fds[1], line, strlen(line));
        }
        close(pipe_fds[1]); // Fermer le côté écriture du pipe
        exit(EXIT_SUCCESS);
    } else {
        // Processus parent
        close(pipe_fds[1]); // Fermer le côté écriture du pipe
        waitpid(pid, NULL, 0); // Attendre que le processus enfant termine
    }
}
 else if (strcmp(tokens[i], "&") == 0) {
            // Marquer la commande pour être exécutée en arrière-plan
            cmds[current_cmd].background = 1;
        } else if (strcmp(tokens[i], "|") == 0) {
            // Fin de la commande actuelle, passer à la suivante
            cmds[current_cmd].argv[current_arg] = NULL;
            current_cmd++;
            current_arg = 0;
            redirect_in = 0;
            redirect_out = 0;
            redirect_err = 0;
        } else {
            if (redirect_in) {
                // Redirection d'entrée, ouvrir le fichier et le lier à l'entrée standard
                int fd = open(tokens[i], O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                cmds[current_cmd].in_stream = fd;
                redirect_in = 0;
            } else if (redirect_out) {
                // Redirection de sortie, ouvrir le fichier et le lier à la sortie standard
                int flags = O_WRONLY | O_CREAT;
                if (redirect_out == 2) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                int fd = open(tokens[i], flags, 0644);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                cmds[current_cmd].out_stream = fd;
                redirect_out = 0;
            } else if (redirect_err) {
                // Redirection d'erreur, ouvrir le fichier et le lier à la sortie d'erreur standard
                int flags = O_WRONLY | O_CREAT;
                if (redirect_err == 2) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }
                int fd = open(tokens[i], flags, 0644);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                cmds[current_cmd].err_stream = fd;
                redirect_err = 0;

            } else {
                // Argument normal, l'ajouter à argv
                cmds[current_cmd].argv[current_arg] = tokens[i];
                current_arg++;
                if (current_arg == 1) {
                    strcpy(cmds[current_cmd].path, tokens[i]);
                }
            }
        }
    }

    cmds[current_cmd].argv[current_arg] = NULL;
    return 0;
}

void add_to_history(cmd_history **history, const char *cmd) {
    cmd_history *new_node = malloc(sizeof(cmd_history));
    if (new_node) {
        new_node->command = strdup(cmd); // Copie la commande
        new_node->next = *history;       // Place le nouveau noeud en tête de la liste
        *history = new_node;             // Met à jour le pointeur de la tête de liste
    }
}
