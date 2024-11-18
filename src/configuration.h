/*
 * Copyright 2023 Roger Feese
 */
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/**
 * Reset the configuration data and initialization.
 */
void configuration_reset();

/**
 * Initialize the configuration, find location for config file.
 *
 * \param config_dirname The directory name to contain configuration.
 * \param config_filename The configuration filename.
 * \return 1 if suitable configuration directory was found.
 */
int configuration_init(char config_dirname[], char config_filename[]);

/**
 * Load the configuration file.
 *
 * \return 1 if configuration was loaded successfully.
 */
int configuration_load();

/**
 * Save the configuration file.
 *
 * \return 1 if configuration was saved successfully.
 */
int configuration_save();

/**
 * Get the current configuration directory.
 *
 * \return String representing configuration file path.
 */
const char * configuration_get_configdir();

/**
 * Get the integer value stored at the provided index.
 *
 * \param index Configuration array index.
 * \param value Pointer to integer value fetched from configuration.
 * \return 1 if a valid integer value was found.
 */
int configuration_get_by_index_int_value(const unsigned int index, int *value);

/**
 * Get the integer value corresponding to the provided configuration key.
 *
 * \param key Key to search for.
 * \param value Pointer to integer value fetched from configuration.
 * \return 1 if a valid value was found.
 */
int configuration_get_int_value(const char *key, int *value);

/**
 * Set an integer configuration value at the provided index.
 *
 * \param key Key to store a value under.
 * \param value Integer value to store.
 * \return 1 if value was stored successfully.
 */
int configuration_set_by_index_int_value(const unsigned int index, int value);

/**
 * Set an integer configuration value corresponding to the provided key.
 *
 * \param key Key to store a value under.
 * \param value Integer value to store.
 * \return 1 if a valid value stored successfully.
 */
int configuration_set_int_value(const char *key, int value);

int configuration_get_by_index_float_value(const unsigned int index, float *value);

/**
 * Get the float value corresponding to the provided configuration key.
 *
 * \param key Key to search for.
 * \param value Pointer to integer value fetched from configuration.
 * \return 1 if a valid value was found.
 */
int configuration_get_float_value(const char *key, float *value);

int configuration_set_by_index_float_value(const unsigned int index, float value);

/**
 * Set a float configuration value corresponding to the provided key.
 *
 * \param key Key to store a value under.
 * \param value Float value to store.
 * \return 1 if a valid value stored successfully.
 */
int configuration_set_float_value(const char *key, float value);

int configuration_get_by_index_str_value(const unsigned int index, char *value, int size);

/**
 * Get the string value corresponding to the provided configuration key.
 *
 * \param key Key to search for.
 * \param value Pointer to caller-provided char buffer fetched from configuration.
 * \param size of provided caller-provided char buffer.
 * \return 1 if a valid value was found.
 */
int configuration_get_str_value(const char *key, char *value, int size);

int configuration_set_by_index_str_value(const unsigned int index, const char *value);

/**
 * Set a string configuration value corresponding to the provided key.
 *
 * \param key Key to store a value under.
 * \param value Pointer to char buffer value to store.
 * \return 1 if a valid value stored successfully.
 */
int configuration_set_str_value(const char *key, const char *value);

/* function prototypes for getting and setting */
typedef int (config_get_int_t)(const char *key, int *value);
typedef int (config_get_float_t)(const char *key, float *value);
typedef int (config_get_str_t)(const char *key, char *value, int size);
typedef int (config_set_int_t)(const char *key, int value);
typedef int (config_set_float_t)(const char *key, float value);
typedef int (config_set_str_t)(const char *key, const char *value);

/**
 * Get the most recent error.
 *
 * \return Error string.
 */
const char *configuration_get_error();
#endif //CONFIGURATION_H
