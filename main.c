#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "cmd.h"
#include "builtin.h"

int main(int argc, char* argv[]) {
    char cmdline[MAX_LINE_SIZE]; // buffer des lignes de commandes
    char* cmdtoks[MAX_CMD_SIZE]; // "mots" de la ligne de commandes
    cmd_t cmds[MAX_CMD_SIZE];
    cmd_t* current;
    cmd_history *history = NULL;

    while (1) {
        // Effacer les contenus de cmdline, cmdtoks et cmds
        memset(cmdline, 0, sizeof(cmdline));
        memset(cmdtoks, 0, sizeof(cmdtoks));
        memset(cmds, 0, sizeof(cmds));

        // Initialiser les valeurs par défaut dans cmds (stdin, stdout, stderr, ...)
        for (int i = 0; i < MAX_CMD_SIZE; i++) {
            init_cmd(&cmds[i]);
        }

        // Afficher un prompt
        printf("$ ");

        // Lire une ligne dans cmdline - Attention fgets enregistre le \n final
        if (fgets(cmdline, MAX_LINE_SIZE, stdin) == NULL) {
            break;
        }


        
        if (strlen(cmdline) > 0 && cmdline[strlen(cmdline) - 1] == '\n') {
            cmdline[strlen(cmdline) - 1] = '\0';
        }

        // Traiter la ligne de commande
        //   - supprimer les espaces en début et en fin de ligne
        if (strlen(cmdline) > 0) {
            trim(cmdline);
            //   - ajouter d'éventuels espaces autour de ; ! || && & ...
            clean(cmdline);
            //   - supprimer les doublons d'espaces
            separate_s(cmdline, " ", MAX_LINE_SIZE);
            //   - traiter les variables d'environnement
            substenv(cmdline, MAX_LINE_SIZE);
        }
        add_to_history(&history, cmdline);
        // Découper la ligne dans cmdtoks
        int num_tokens = strcut(cmdline, ' ', cmdtoks, MAX_CMD_SIZE);

        // Traduire la ligne en structures cmd_t dans cmds
        // Les commandes sont chaînées en fonction des séparateurs
        //   - next -> exécution inconditionnelle
        //   - next_success -> exécution si la commande précédente réussit
        //   - next_failure -> exécution si la commande précédente échoue
        parse_cmd(cmdtoks, cmds, num_tokens);
        int status;
        // Exécuter les commandes dans l'ordre en fonction des opérateurs de flux
        for (current = cmds; current != NULL; current = current->next) {

            if (is_builtin(current->path)) {
                builtin(current, history);
            } else {
                // Exécuter la commande
                exec_cmd(current);
                status = exec_cmd(current);

    }
    }
    }
    fprintf(stderr, "\nAu revoir!\n");
    free_history(history);
    return 0;
}
