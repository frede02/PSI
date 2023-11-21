/*
  Projet minishell - Licence 3 Info - PSI 2023
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Parsing de la ligne de commandes utilisateur (implémentation).
 
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int trim(char* str) {
    int len = strlen(str);

    // Remove leading spaces
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\n') {
        start++;
    }

    // If the string contains only spaces
    if (start == len) {
        str[0] = '\0';
        return 0;
    }

    // Remove trailing spaces
    int end = len - 1;
    while (str[end] == ' ' || str[end] == '\t' || str[end] == '\n') {
        end--;
    }

    // Shift characters to the beginning of the string
    int i;
    for (i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[i] = '\0';

    return 0;
}


int clean(char* str) {
    int i, j;
    int insideWord = 0; // Flag to indicate if we're inside a word

    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
            if (!insideWord) {
                // Found the start of consecutive spaces, skip
                continue;
            } else {
                // Found the end of a word
                insideWord = 0;
                str[j++] = ' '; // Replace multiple spaces with one
            }
        } else {
            insideWord = 1;
            str[j++] = str[i]; // Copy non-space characters
        }
    }

    // Null-terminate the modified string
    str[j] = '\0';

    return 0;
}


int separate(char* str, char s, size_t max) {
    size_t len = strlen(str);
    char* result = (char*)malloc(max + 1);
    if (result == NULL) {
        return -1; // Memory allocation failed
    }

    size_t i, j = 0;
    for (i = 0; i < len && j < max; i++) {
        if (str[i] == s) {
            if (j + 2 < max) {
                result[j++] = s;
                result[j++] = ' ';
            } else {
                break; // Not enough space to insert separator and space
            }
        } else if (j < max) {
            result[j++] = str[i];
        } else {
            break; // Not enough space for remaining characters
        }
    }

    // Copy remaining characters if there is space
    while (i < len && j < max) {
        result[j++] = str[i++];
    }

    result[j] = '\0';

    // Copy the modified string back to original
    strcpy(str, result);

    free(result);
    return 0;
}


int separate_s(char* str, const char* separators, size_t max) {
    size_t len = strlen(str);
    char* result = (char*)malloc(max + 1);
    if (result == NULL) {
        return -1; // Memory allocation failed
    }

    size_t i, j = 0;
    for (i = 0; i < len && j < max; i++) {
        char currentChar = str[i];
        if (strchr(separators, currentChar) != NULL) {
            if (j + 2 < max) {
                result[j++] = currentChar;
                result[j++] = ' ';
            } else {
                break; // Not enough space to insert separator and space
            }
        } else if (j < max) {
            result[j++] = currentChar;
        } else {
            break; // Not enough space for remaining characters
        }
    }

    // Copy remaining characters if there is space
    while (i < len && j < max) {
        result[j++] = str[i++];
    }

    result[j] = '\0';

    // Copy the modified string back to original
    strcpy(str, result);

    free(result);
    return 0;
}



int strcut(char* str, char sep, char** tokens, size_t max) {
    size_t token_count = 0;
    char* token = strtok(str, &sep);
    while (token != NULL && token_count < max - 1) {
        tokens[token_count++] = token;
        token = strtok(NULL, &sep);
    }
    tokens[token_count] = NULL;
    return token_count;
}


// Function to replace environment variables in a string
int substenv(char* str, size_t max) {
    char* result = (char*)malloc(max + 1);
    if (result == NULL) {
        return -1; // Memory allocation failed
    }

    size_t i = 0, j = 0;
    while (str[i] != '\0' && j < max) {
        if (str[i] == '$' && str[i + 1] != '\0') {
            // Found a potential environment variable
            i++; // Skip the '$'
            char var[64]; // Assuming the maximum length of an environment variable name is 63
            size_t k = 0;

            while (str[i] != ' ' && str[i] != '\0' && k < 63) {
                var[k++] = str[i++];
            }
            var[k] = '\0';

            char* env_value = getenv(var);

            if (env_value != NULL) {
                size_t len = strlen(env_value);
                if (j + len < max) {
                    strcpy(result + j, env_value);
                    j += len;
                } else {
                    // Not enough space to insert the environment variable value
                    break;
                }
            }
            // If the environment variable is not found, just skip it
        } else {
            result[j++] = str[i++];
        }
    }

    result[j] = '\0';

    strcpy(str, result);
    free(result);

    return 0;
}


