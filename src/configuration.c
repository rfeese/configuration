/*
 * Copyright 2023 Roger Feese
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "configuration.h"
#ifdef WIN32
#include <direct.h> /* for _mkdir */
#endif

char configuration_dirname[32] = "configuration";
char configuration_filename[32] = "configuration.ini";

char configdir[256] = "config"; //default config dir name
int configdirok = 0;

t_configuration_index_mapping configuration_mappings[CONFIGURATION_ITEMS_MAX] = {};
t_configuration configuration = {};

#define CONFIGURATION_ERROR_MSG_LEN 128
char configuration_error_msg[CONFIGURATION_ERROR_MSG_LEN] = {};


//---------------------------------------------------------------------------
int _configdir_init(int create_configdir){

	if(configdirok){
		return 1;
	}

	/* find config directory */
#ifndef WIN32
	/* for UNIX systems */
	char config_base[200] = { '\0' };

	/* try XDG standard */
	char *xdg_config = getenv("XDG_CONFIG_HOME");
	if(xdg_config != NULL){
		if(strlen(xdg_config) > sizeof(config_base)){
			snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Your XDG config path is too long for me to use.");
		}
		else{
			snprintf(config_base, sizeof(config_base), "%s", xdg_config);
		}
	}

	// if we don't have a config_base yet, try HOME
	if(strlen(config_base) == 0){
		/* find HOME directory */
		char *homedir = getenv("HOME");
		if(homedir == NULL){
			snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Unable to find HOME directory for configuration.");
			printf("Unable to find HOME directory for configuration.\n");
			//exit?
			configdirok = 0;
		}
		else{
			if(strlen(homedir) + strlen("/.config")  > sizeof(config_base)){
				snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN,"Your XDG config path is too long for me to use.");
			}
			else{
				snprintf(config_base, sizeof(config_base), "%s/.config", homedir);

				// create HOME/.config dir if it doesn't exist
				struct stat st;
				if(stat(config_base, &st) != 0){
					if(create_configdir){
#ifdef WIN32
						if(_mkdir(config_base) != 0){
#else
						if(mkdir(config_base, 0755) != 0){
#endif
							snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Unable to create HOME./config configuration directory %s.", config_base);
							configdirok = 0;
							return 0;
						}
					}
				}
			}
		}
	}

	if(strlen(config_base) > 0){
		if(strlen(config_base) + strlen("/") + strlen(configuration_dirname) > sizeof(configdir)){
			snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Your config directory path is too long for me to use.");
			printf("Your config directory path is too long for me to use.\n");
			return 0;
		}
		snprintf(configdir, sizeof(configdir), "%s/%s", config_base, configuration_dirname);
	}
#endif

	//create dir if it doesn't exist
	struct stat st;
	if(stat(configdir, &st) == 0){
		configdirok = 1; //found
	}
	else{ //create, if requested
		if(!create_configdir){
			snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Configuration directory does not exist and not created (as requested).");
			return 0;
		}
#ifdef WIN32
		if(_mkdir(configdir) == 0){
#else
		if(mkdir(configdir, 0755) == 0){
#endif
			configdirok = 1;
		}
		else{
			snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Unable to create configuration directory %s.", configdir);
			configdirok = 0;
			return 0;
		}
	}
	return 1;
}
//---------------------------------------------------------------------------
int configuration_init(char config_dirname[], char config_filename[]){

	if(!strlen(config_dirname)){
		return 0;
	}
	if(!strlen(config_filename)){
		return 0;
	}
	snprintf(configuration_dirname, 32, "%s", config_dirname);
	snprintf(configuration_filename, 32, "%s", config_filename);
	return _configdir_init(1);
}
//---------------------------------------------------------------------------
int configuration_init_indexes(t_configuration_index_mapping mappings[CONFIGURATION_ITEMS_MAX]){
	for(int i = 0; i < CONFIGURATION_ITEMS_MAX; i++){
		if(strnlen(mappings[i].key, CONFIGURATION_KEY_MAX)){
			if((mappings[i].index < CONFIGURATION_ITEMS_MAX)){
				configuration_mappings[i] = mappings[i];
				snprintf(configuration.items[mappings[i].index].key, CONFIGURATION_KEY_MAX, "%s", mappings[i].key);
			}
			else {
				snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Invalid mapping to index %d.", mappings[i].index);
				printf("Invalid configuration mapping to %d.\n", mappings[i].index);
			}
		}
	}
	return 1;
}
//---------------------------------------------------------------------------
int configuration_load(){

	_configdir_init(0);

	//can't load if configdir not ok
	if(!configdirok){
		return 0;
	}

	//don't load more than once
	if(configuration.loaded){
		return 1;
	}

	char fqconfigname[288]; // configdir + configfile
	snprintf(fqconfigname, sizeof(fqconfigname), "%s/%s", configdir, configuration_filename);

	// init configuration
	configuration.num_items = 0;
	// count mapped items
	int num_mapped_items = 0;
	for(int i = 0; i < CONFIGURATION_ITEMS_MAX; i++){
		if(strnlen(configuration_mappings[i].key, CONFIGURATION_KEY_MAX)){
			num_mapped_items++;
		}
	}
	// start non-indexed items after mappings
	configuration.num_items = num_mapped_items;

	FILE *configfile = NULL;
	configfile = fopen(fqconfigname, "r");
	if(!configfile){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Unable to open configfile.");
		return 0;
	}

	int at_eof = 0;
	int line = 0;
	while(!at_eof){
		char tmpkey[CONFIGURATION_KEY_MAX];
		char tmpval[CONFIGURATION_VAL_STR_LEN];
		int charsmatching = 0; // number of chars matching format
		int signal = fscanf(configfile, "%32s %32s\n", &tmpkey[0], &tmpval[0]);

		if(signal == EOF){
			// reached end of file
			at_eof = 1;
			continue;
		}

		if(signal != 2){
			printf("ERROR: reading configuration entry %d from %s\n", line, fqconfigname);
			// try next line
			line++;
			continue;
		}
		line++;

		int insert_index = configuration.num_items;
		// if key matches a mapping, insert in mapped position
		if(num_mapped_items > 0){
			for(int i = 0; i < num_mapped_items; i++){
				if(strncmp(configuration_mappings[i].key, tmpkey, CONFIGURATION_KEY_MAX) == 0){
					insert_index = configuration_mappings[i].index;
				}
			}
		}

		// copy the key
		snprintf(configuration.items[insert_index].key, CONFIGURATION_KEY_MAX, "%s", tmpkey);

		// convert tmpval
		// check for integer
		sscanf(tmpval, "%d%n", &configuration.items[insert_index].val.int_value, &charsmatching);
		if(charsmatching == strlen(tmpval)){
			// all chars were int
			configuration.items[insert_index].val_type = CONFIGURATION_VAL_INT;
			if(insert_index >= num_mapped_items){
				configuration.num_items = configuration.num_items + 1;
			}
			continue;
		}

		sscanf(tmpval, "%f%n", &configuration.items[insert_index].val.float_value, &charsmatching);
		if(charsmatching == strlen(tmpval)){
			// all chars were float
			configuration.items[insert_index].val_type = CONFIGURATION_VAL_FLOAT;
			if(insert_index >= num_mapped_items){
				configuration.num_items = configuration.num_items + 1;
			}
			continue;
		}

		// if not int or float, assume string
		snprintf(configuration.items[insert_index].val.str_value, CONFIGURATION_VAL_STR_LEN, "%s", tmpval);
		configuration.items[insert_index].val_type = CONFIGURATION_VAL_STR;
		if(insert_index >= num_mapped_items){
			configuration.num_items = configuration.num_items + 1;
		}
	}

	fclose(configfile);
	configuration.loaded = 1;
	return 1;	
}
//---------------------------------------------------------------------------
int configuration_save(){
	FILE *configfile;
	int i = 0;
	char fqconfigname[288]; //configdir + configfile

	_configdir_init(1);

	//configdirok?
	if(!configdirok){
		return 0;
	}

	snprintf(fqconfigname, sizeof(fqconfigname), "%s/%s", configdir, configuration_filename);
	configfile = fopen(fqconfigname, "w");

	if(configfile == NULL){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Unable to open configfile for save.");
		printf("Unable to open configfile for save.\n");
		return 0;
	}

	for(i = 0; i < configuration.num_items; i++){
		switch(configuration.items[i].val_type){
			case CONFIGURATION_VAL_INT:
				fprintf(configfile, "%s %d\n", configuration.items[i].key, configuration.items[i].val.int_value);
				break;
			case CONFIGURATION_VAL_FLOAT:
				fprintf(configfile, "%s %0.4f\n", configuration.items[i].key, configuration.items[i].val.float_value);
				break;
			case CONFIGURATION_VAL_STR:
				fprintf(configfile, "%s %s\n", configuration.items[i].key, configuration.items[i].val.str_value);
				break;
		}
	}

	fclose(configfile);
	configuration.saved = 1;
	return 1;	
}
//---------------------------------------------------------------------------
int configuration_get_by_index_int_value(const int index){
	if(index >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Index out of bounds.");
		printf("ERROR: index >= CONFIGURATION_ITEMS_MAX.\n");
	}

	return configuration.items[index].val.int_value;
}
//---------------------------------------------------------------------------
int configuration_get_int_value(const char *key){
	int i = 0;

	for(i = 0; i < configuration.num_items; i++){
		if(strcmp(configuration.items[i].key, key) == 0){
			if(configuration.items[i].val_type == CONFIGURATION_VAL_INT){
				return configuration.items[i].val.int_value;
			}
			else {
				snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "conf value type is not int for %s.", key);
				printf("ERROR: conf value type is not int for %s.\n", key);
			}
		}
	}

	//not found
	return -1;
}
//---------------------------------------------------------------------------
void configuration_set_by_index_int_value(const int index, int value){
	if(index >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Index out of bounds.");
		printf("ERROR: index >= CONFIGURATION_ITEMS_MAX.\n");
	}

	configuration.items[index].val_type = CONFIGURATION_VAL_INT;
	configuration.items[index].val.int_value = value;
	configuration.saved = 0;
}
//---------------------------------------------------------------------------
void configuration_set_int_value(const char *key, int value){
	int i = 0;
	int key_found = 0;

	for(i = 0; i < configuration.num_items; i++){
		if(strcmp(configuration.items[i].key, key) == 0){
			key_found = 1;
			break;
		}
	}

	if(i >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "No more space in configuration.");
		printf("ERROR: no more space in configuration.\n");
		return;
	}

	if(!key_found){ //add new item
		snprintf(configuration.items[i].key, 32, "%s", key);
		configuration.num_items = configuration.num_items + 1;
	}

	configuration.items[i].val_type = CONFIGURATION_VAL_INT;
	configuration.items[i].val.int_value = value;
	configuration.saved = 0;
}
//---------------------------------------------------------------------------
float configuration_get_by_index_float_value(const int index){
	if(index >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Index out of bounds.");
		printf("ERROR: index >= CONFIGURATION_ITEMS_MAX.\n");
		return 0;
	}

	return configuration.items[index].val.float_value;
}
//---------------------------------------------------------------------------
float configuration_get_float_value(const char *key){
	int i = 0;

	for(i = 0; i < configuration.num_items; i++){
		if(strcmp(configuration.items[i].key, key) == 0){
			if(configuration.items[i].val_type == CONFIGURATION_VAL_FLOAT){
				return configuration.items[i].val.float_value;
			}
			else {
				snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "conf value type is not float for %s.", key);
				printf("ERROR: conf value type is not float for %s.\n", key);
			}
		}
	}

	//not found
	return -1;
}
//---------------------------------------------------------------------------
void configuration_set_by_index_float_value(const int index, float value){

	if(index >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Index out of bounds.");
		printf("ERROR: index >= CONFIGURATION_ITEMS_MAX.\n");
		return;
	}

	configuration.items[index].val_type = CONFIGURATION_VAL_FLOAT;
	configuration.items[index].val.float_value = value;
	configuration.saved = 0;
}
//---------------------------------------------------------------------------
void configuration_set_float_value(const char *key, float value){
	int i = 0;
	int key_found = 0;

	for(i = 0; i < configuration.num_items; i++){
		if(strcmp(configuration.items[i].key, key) == 0){
			key_found = 1;
			break;
		}
	}

	if(i >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "No more space in configuration.");
		printf("ERROR: no more space in configuration.\n");
		return;
	}

	if(!key_found){ //add new item
		snprintf(configuration.items[i].key, CONFIGURATION_KEY_MAX, "%s", key);
		configuration.num_items = configuration.num_items + 1;
	}

	configuration.items[i].val_type = CONFIGURATION_VAL_FLOAT;
	configuration.items[i].val.float_value = value;
	configuration.saved = 0;
}
//---------------------------------------------------------------------------
const char *configuration_get_by_index_str_value(const int index){

	if(index >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Index out of bounds.");
		printf("ERROR: index >= CONFIGURATION_ITEMS_MAX.\n");
		return "";
	}

	return configuration.items[index].val.str_value;
}
//---------------------------------------------------------------------------
const char *configuration_get_str_value(const char *key){
	int i = 0;

	for(i = 0; i < configuration.num_items; i++){
		if(strcmp(configuration.items[i].key, key) == 0){
			if(configuration.items[i].val_type == CONFIGURATION_VAL_STR){
				return configuration.items[i].val.str_value;
			}
			else {
				snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Conf value type is not str for %s.", key);
				printf("ERROR: conf value type is not str for %s.\n", key);
			}
		}
	}

	//not found
	return "";
}
//---------------------------------------------------------------------------
void configuration_set_by_index_str_value(const int index, const char *value){

	if(index >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "Index out of bounds.");
		printf("ERROR: index >= CONFIGURATION_ITEMS_MAX.\n");
		return;
	}

	configuration.items[index].val_type = CONFIGURATION_VAL_STR;
	snprintf(configuration.items[index].val.str_value, CONFIGURATION_VAL_STR_LEN, "%s", value);
	configuration.saved = 0;
}
//---------------------------------------------------------------------------
void configuration_set_str_value(const char *key, const char *value){
	int i = 0;
	int key_found = 0;

	for(i = 0; i < configuration.num_items; i++){
		if(strcmp(configuration.items[i].key, key) == 0){
			key_found = 1;
			break;
		}
	}

	if(i >= CONFIGURATION_ITEMS_MAX){
		snprintf(configuration_error_msg, CONFIGURATION_ERROR_MSG_LEN, "No more space in configuration.");
		printf("ERROR: no more space in configuration.\n");
		return;
	}

	if(!key_found){ //add new item
		snprintf(configuration.items[i].key, CONFIGURATION_KEY_MAX, "%s", key);
		configuration.num_items = configuration.num_items + 1;
	}

	configuration.items[i].val_type = CONFIGURATION_VAL_STR;
	snprintf(configuration.items[i].val.str_value, CONFIGURATION_VAL_STR_LEN, "%s", value);
	configuration.saved = 0;
}
//---------------------------------------------------------------------------
char *configuration_get_error(){
	return configuration_error_msg;
}
//---------------------------------------------------------------------------
