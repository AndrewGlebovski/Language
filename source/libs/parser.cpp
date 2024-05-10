/**
 * \file
 * \brief Arguments parser module source
*/

#include <stdio.h>
#include <string.h>
#include "parser.hpp"


int parse_args(int argc, char* argv[], Command command_list[], size_t list_size) {
	if (!argc) {
		printf("Invalid argc!\n");
		return 1;
	}

	if (!argv) {
		printf("Invalid argv!\n");
		return 1;
	}

	if (!command_list) {
		printf("Invalid command list!\n");
		return 1;
	}

	if (!list_size) {
		printf("Invalid list size!\n");
		return 1;
	}

	for(int j = 0; j < argc; j++) {
		for(size_t i = 0; i < list_size; i++) {
			if (!strcmp(command_list[i].short_name, argv[j]) || !strcmp(command_list[i].long_name, argv[j]))
				(*(command_list[i].func))(&argv[j], command_list[i].data);
		}
	}

	return 0;
}




void set_input_file(char *argv[], void *data) {
	if (*(++argv)) {
		*((char **) data) = *argv;
	}
	else {
		printf("No filename after -i, argument ignored!\n");
	}
}


void set_output_file(char *argv[], void *data) {
    if (*(++argv)) {
        *((char **) data) = *argv;
    }
    else {
        printf("No filename after -o, argument ignored!\n");
    }
}


void show_help(char *argv[], void *data) {
    size_t i = 0;

    for(; strcmp(((Command *)(data))[i].short_name, "-h") != 0; i++) {
        printf("%s %s %s\n", ((Command *)(data))[i].short_name, ((Command *)(data))[i].long_name, ((Command *)(data))[i].desc);
    }

    printf("%s %s %s\n", ((Command *)(data))[i].short_name, ((Command *)(data))[i].long_name, ((Command *)(data))[i].desc);
}
