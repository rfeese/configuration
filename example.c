/*
 * Copyright 2023 Roger Feese
 */
#include <stdio.h>
#include <stdlib.h>
#include "src/configuration.h"

// use X macro to define configuration mapping indexes
# define CONFIGS \
	X(CONFIG_TIMES_EXECUTED, "times_executed", CONFIGURATION_VAL_INT, "0") \
	X(CONFIG_RATIO, "ratio", CONFIGURATION_VAL_FLOAT, "1.2345") \
	X(CONFIG_MESSAGE, "message", CONFIGURATION_VAL_STR, "Hello,_world!") \

#define X(a,b,c,d) a,
enum configs {
	CONFIGS
};
#undef X
#define X(a,b,c,d) { b, a, c, d },
struct configuration_index_mapping confmap[CONFIGURATION_ITEMS_MAX] = {
	CONFIGS
};
#undef X

int main(int argc, char* argv[]){

	// create a configuration directory in an appropriate place
	if(!configuration_init("configuration_example", "config.ini")){
		printf("Error while initializing configuration: %s\n", configuration_get_error());
	}

	// load the configuration mappings for quick access
	configuration_init_indexes(confmap);

	// initial load will fail as config does not exist yet
	if(!configuration_load()){
		printf("Error while loading configuration: %s\n", configuration_get_error());
	}

	// update configuration values
	int times_executed = 0;
	if(!configuration_get_by_index_int_value(CONFIG_TIMES_EXECUTED, &times_executed)){
		printf("Error accessing configuration: %s\n", configuration_get_error());
	}

	printf("I've been executed %d times.\n", times_executed);

	if(!configuration_set_by_index_int_value(CONFIG_TIMES_EXECUTED, ++times_executed)){
		printf("Error accessing configuration: %s\n", configuration_get_error());
	}

	// save configuration
	if(!configuration_save()){
		printf("Error saving configuration: %s\n", configuration_get_error());
	}

	return EXIT_SUCCESS;
}
