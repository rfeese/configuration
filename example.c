/*
 * Copyright 2023 Roger Feese
 */
#include <stdio.h>
#include <stdlib.h>
#include "src/configuration.h"

int main(int argc, char* argv[]){

	// create a configuration directory in an appropriate place
	if(!configuration_init("configuration_example", "config.ini")){
		printf("Error while initializing configuration: %s\n", configuration_get_error());
	}

	// initial load will fail as config does not exist yet
	if(!configuration_load()){
		printf("Error while loading configuration: %s\n", configuration_get_error());
	}

	// update configuration values
	int times_executed = 0;
	if(!configuration_get_int_value("times_executed", &times_executed)){
		printf("Error accessing configuration: %s\n", configuration_get_error());
	}

	printf("I've been executed %d times.\n", times_executed);

	if(!configuration_set_int_value("times_executed", ++times_executed)){
		printf("Error accessing configuration: %s\n", configuration_get_error());
	}

	// save configuration
	if(!configuration_save()){
		printf("Error saving configuration: %s\n", configuration_get_error());
	}

	return EXIT_SUCCESS;
}
