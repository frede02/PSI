/*
  Projet minishell - Licence 3 Info - PSI 2023
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Gestion des commandes internes du minishell (implémentation).
 
 */
#include "cmd.h"
#include "builtin.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int is_builtin(const char* cmd) {
    if (cmd == NULL) {
        fprintf(stderr, "Error: Failed to parse command.\n");
        return -1; // Retourne une valeur d'erreur
    }
    if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "export") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "history" ) == 0)  {
        return 1;
    }
    return 0;
}
void display_history(cmd_history *history) {
    cmd_history *current = history;
    while (current) {
        printf("%s\n", current->command);
        current = current->next;
    }
}

int builtin(cmd_t* cmd, cmd_history* history) {
    if (cmd->path == NULL) {
        fprintf(stderr, "Error: Command path is NULL.\n");
        return -1;
    } if (strcmp(cmd->path, "export") == 0) {
        // Vérifiez le nombre d'arguments
            // Récupérez le nom de la variable et sa valeur
       char* var = strtok(cmd->argv[1], "=");
        char* value = strtok(NULL, "=");
        if (value[0] == '"' && value[strlen(value) - 1] == '"') {
        // Créez une nouvelle chaîne sans les guillemets
        char newValue[256];
        strncpy(newValue, value + 1, strlen(value) - 2);
        newValue[strlen(value) - 2] = '\0';
        value = newValue;
    }

            // Appelez la fonction export pour définir la variable d'environnement
            if (export(var, value, cmd->err_stream) != 0) {
                fprintf(stderr, "export: Erreur lors de la définition de la variable d'environnement.\n");
            }


        return 0; // Commande exécutée avec succès
    }
    if (strcmp(cmd->path, "cd") == 0) {
        // Commande interne : cd
        if (cmd->argv[1] == NULL) {
            fprintf(stderr, "cd: Aucun argument fourni.\n");
        } else {
    // Vérifiez si le répertoire existe avant de tenter de s'y déplacer
            if (access(cmd->argv[1], F_OK) != 0) {
                fprintf(stderr, "cd: Le répertoire '%s' n'existe pas.\n", cmd->argv[1]);
            } else if (cd(cmd->argv[1], cmd->err_stream) != 0) {
                fprintf(stderr, "cd: Erreur lors de la modification du répertoire courant.\n");
            }
        }
    }
    if (strcmp(cmd->path, "history") == 0) {
        display_history(history);
    }
     if (strcmp(cmd->path, "exit") == 0) {
        // Commande interne : exit
        int exit_code = (cmd->argc >= 2) ? atoi(cmd->argv[1]) : 0;
        exit_shell(exit_code, cmd->err_stream);
    }
    return -1; // La commande interne n'a pas été reconnue.
}

int cd(const char* path, int fderr) {
    // Change le répertoire de travail courant.
    if (chdir(path) != 0) {
        fprintf(stderr, "cd: Erreur lors de la modification du répertoire courant.\n");
        return -1;
    }
    return 0;
}

int export(const char* var, const char* value, int fderr) {
    // Définit une variable d'environnement.
    setenv(var, value, 1);
    if (var == NULL) {
        fprintf(stderr, "export: Erreur lors de la définition de la variable d'environnement.\n");
        return -1;
    }
    return 0;
}

int exit_shell(int ret, int fderr) {
    // Quitte le shell avec le code de retour spécifié.
    exit(ret);
}



void free_history(cmd_history *history) {
    cmd_history *current = history;
    while (current) {
        cmd_history *temp = current;
        current = current->next;
        free(temp->command);
        free(temp);
    }
}