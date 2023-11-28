/*
 * Copyright 2023 Roger Feese
 */
#ifndef CONFIGURATION_H
#define CONFIGURATION_H


/* TODO: make config items array automatically expandable */
#define CONFIGURATION_ITEMS_MAX 128
#define CONFIGURATION_VAL_STR_LEN	33

// Configuration item key to index mapping
#define CONFIGURATION_KEY_MAX	33
typedef struct configuration_index_mapping {
	char key[CONFIGURATION_KEY_MAX];
	int index;
} t_configuration_index_mapping;


typedef enum config_val_type { CONFIGURATION_VAL_INT, CONFIGURATION_VAL_FLOAT, CONFIGURATION_VAL_STR } t_conf_val_type;

typedef struct s_config_item {
	char key[32];
	t_conf_val_type val_type;
	union {
		int int_value;
		float float_value;
		char str_value[CONFIGURATION_VAL_STR_LEN];
	} val;
} t_config_item;

#define CONFIGURATION_ERROR_MSG_LEN 128

typedef struct s_configuration {
	// directory to contain configuration file(s)
	char dirname[32];
	// name of configuration file
	char filename[32];
	char configdir[256];
	int configdirok;
	int loaded;
	int saved;
	int num_items;
	t_config_item items[CONFIGURATION_ITEMS_MAX];
	t_configuration_index_mapping mappings[CONFIGURATION_ITEMS_MAX];
	char error_msg[CONFIGURATION_ERROR_MSG_LEN];
} t_configuration;

extern t_configuration configuration;

int configuration_init(char config_dirname[], char config_filename[]);
int configuration_init_indexes(t_configuration_index_mapping mappings[CONFIGURATION_ITEMS_MAX]);
int configuration_load();
int configuration_save();

int configuration_get_by_index_int_value(const int index);
int configuration_get_int_value(const char *key);
void configuration_set_by_index_int_value(const int index, int value);
void configuration_set_int_value(const char *key, int value);

float configuration_get_by_index_float_value(const int index);
float configuration_get_float_value(const char *key);
void configuration_set_by_index_float_value(const int index, float value);
void configuration_set_float_value(const char *key, float value);

const char *configuration_get_by_index_str_value(const int index);
const char *configuration_get_str_value(const char *key);
void configuration_set_by_index_str_value(const int index, const char *value);
void configuration_set_str_value(const char *key, const char *value);

char *configuration_get_error();
#endif //CONFIGURATION_H
