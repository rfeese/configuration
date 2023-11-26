/*
 * Copyright 2023 Roger Feese
 */
#include <stdio.h>
#include <stdlib.h>
#include "src/configuration.h"

# define CONFIGS \
	X(CONFIG_SETTING1, "setting1") \
	X(CONFIG_SETTING2, "setting2") \
	X(CONFIG_SETTING3, "setting3") \
	X(CONFIG_SETTING4, "setting4") 

#define X(a,b) a,
enum configs {
	CONFIGS
};
#undef X
#define X(a,b) { b, a },
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
	int setting1 = configuration_get_by_index_int_value(CONFIG_SETTING1);
	configuration_set_by_index_int_value(CONFIG_SETTING1, ++setting1);

	// save configuration
	configuration_save();

	return EXIT_SUCCESS;
}
