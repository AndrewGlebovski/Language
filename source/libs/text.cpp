/**
 * \file
 * \brief Text module source
*/

#include <io.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include "text.hpp"




int read_text(Text *text, int file) {
    size_t size = get_file_size(file);

    char *buffer = nullptr;

    size = read_in_buffer(file, &buffer, (unsigned int) size);

    int lines = 1 + replace_in_buffer(buffer, '\n', '\0');

    text -> lines = (String *) calloc(lines + 1, sizeof(String));

    text -> buffer = buffer;

    for(int l = 0; l < lines; l++) {
        text -> lines[l] = {buffer, 0};

        while(*buffer != '\0')
            buffer++;

        text -> lines[l].len = (int)(buffer - (text -> lines)[l].str);

        buffer++;
    }

    text -> lines[lines] = {nullptr, -1};
    text -> size = lines;

    return 0;
}


size_t read_in_buffer(int file, char **buffer, size_t size) {
    *buffer = (char *) calloc(size + 1, sizeof(char));
    
    size = read(file, *buffer, (unsigned int) size * sizeof(char));

    *buffer = (char *) realloc(*buffer, size + 1);

    (*buffer)[size] = '\0';

    return size;
}


int replace_in_buffer(char *buffer, const char old_char, const char new_char) {
    int count = 0;

    for(size_t i = 0; buffer[i]; i++) {
        if (buffer[i] == old_char) {
            buffer[i] = new_char;
            count++;
        }
    }

    return count;
}


size_t get_file_size(int file) {
    struct stat info = {};
    fstat(file, &info);
    return info.st_size;
}


int print_lines(String lines[], int file) {
    for(int i = 0; lines[i].str != nullptr && lines[i].len != -1; i++) {
        write(file, lines[i].str, lines[i].len * sizeof(char));
        write(file, "\n", sizeof(char));
    }

    return 0;
}


int free_text(Text *text) {
    free(text -> lines);
    text -> lines = nullptr;

    free(text -> buffer);
    text -> buffer = nullptr;

    text -> size = 0;

    return 0;
}


int str_to_int(String *str, int *value) {
    char *end = nullptr;
    *value = (int)(strtod(str -> str, &end));
    return (end == str -> str + str -> len);
}


String get_token(char *origin, const char *solo, const char *extra) {
    String token = {origin, 1};

    while (isspace(*token.str)) token.str++;

    if (*token.str == '\0') return {nullptr, -1};

    if (strchr(solo, *token.str)) return token;

    else if (isalpha(*token.str))
        while (isalnum(*(token.str + token.len)) || *(token.str + token.len) == '_') token.len++;

    else if (isdigit(*token.str) || *token.str == '-')
        while (isdigit(*(token.str + token.len)) || *(token.str + token.len) == '.') token.len++;

    if (strchr(extra, *token.str)) return {nullptr, -1};

    return token;
}
