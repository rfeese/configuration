#include <string.h>
#include <stdlib.h>
#include "../../Unity/src/unity.h"
#include "../src/configuration.h"

char *xdg_config_home_orig = NULL;
char *home_orig = NULL;

// runs before each test
void setUp(void){
	// save original env
	xdg_config_home_orig = getenv("XDG_CONFIG_HOME");
	home_orig = getenv("HOME");
	// set test env
	setenv("XDG_CONFIG_HOME", "./fixtures", 1);

	configuration_reset();

	// snprintf(configuration.configdir, 256, "fixtures");
	configuration_init("configurationtest", "test_configuration.ini");
}

//runs after each test
void tearDown(void){
	if(xdg_config_home_orig){
		// restore original env
		setenv("XDG_CONFIGURATION_HOME", xdg_config_home_orig, 1);
	}
	else {
		// unset if env did not exist previously
		unsetenv("XDG_CONFIGURATION_HOME");
	}
	if(home_orig){
		// restore original env
		setenv("HOME", home_orig, 1);
	}
	else {
		// unset if env did not exist previously
		unsetenv("HOME");
	}
}

void test_configuration_init(){
	configuration_reset();
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_init("","fail"), "Configuration init for empty dirname should fail.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, strnlen(configuration_get_error(), 32), "There should be an error message.");
	configuration_reset();
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_init("fail",""), "Configuration init for empty filename should fail.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, strnlen(configuration_get_error(), 32), "There should be an error message.");

	// test usage of XDG_CONFIG_HOME
	configuration_reset();
	setenv("XDG_CONFIG_HOME", "./fixtures", 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_init("configurationtest","configurationtest.ini"), "Configuration init with XDG_CONFIG_HOME should succeed.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("./fixtures/configurationtest", configuration_get_configdir(), "XDG_CONFIG_HOME specified configdir should be returned.");

	// test usage of HOME
	configuration_reset();
	unsetenv("XDG_CONFIG_HOME");
	setenv("HOME", "./fixtures", 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_init("configurationtest","configurationtest.ini"), "Configuration init with HOME should succeed.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("./fixtures/.config/configurationtest", configuration_get_configdir(), "HOME specified configdir should be returned.");
}

void test_configuration_load(){

	// test file does not exist	
	configuration_init("configurationtest", "fake_configuration.ini");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_load(), "Fake configuration file should not have been loaded.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, strnlen(configuration_get_error(), 32), "There should be an error message.");

	configuration_init("configurationtest", "test_configuration.ini");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_load(), "Configuration should have been loaded.");

	// retrieve values from loaded config
	int intval = 8;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_int_value("testint", &intval), "Get testint should succeed.");
	float floatval = 8.0f;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_float_value("testfloat", &floatval), "Get testfloat should succeed.");
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(2.0f, floatval, "Retrieved floatval should have been 2.0.");
	char strval[32];
	snprintf(strval, 32, "eight");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_str_value("teststr", &strval[0], 32), "Get teststr should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("three", strval, 32), "Retrieved strval should have been three.");
}

void test_set_get(){
	int intval;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_int_value("non-existant", &intval), "Getting non-existant int should fail.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, strnlen(configuration_get_error(), 32), "There should be an error message.");

	intval = 5;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_set_int_value("testint1", intval), "Set testint1 should succeed.");
	intval = 6;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_int_value("testint1", &intval), "Get testint1 should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(5, intval, "Retrieved intval should have been 5.");

	float floatval;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_float_value("non-existant", &floatval), "Getting non-existant float should fail.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, strnlen(configuration_get_error(), 32), "There should be an error message.");

	floatval = 5.0f;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_set_float_value("testfloat1", floatval), "Set testfloat1 should succeed.");
	floatval = 6.0f;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_float_value("testfloat1", &floatval), "Get testfloat1 should succeed.");
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(5.0f, floatval, "Retrieved floatval should have been 5.0.");

	char strval[32];
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_str_value("non-existant", &strval[0], 32), "Getting non-existant float should fail.");
	TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, strnlen(configuration_get_error(), 32), "There should be an error message.");

	snprintf(strval, 32, "five");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_set_str_value("teststr1", strval), "Set teststr1 should succeed.");
	snprintf(strval, 32, "six");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_str_value("teststr1", &strval[0], 32), "Get teststr1 should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("five", strval, 32), "Retrieved strval should have been five.");


	// get values of wrong type
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_int_value("testfloat1", &intval), "Getting float as int should fail.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_int_value("teststr1", &intval), "Getting str as int should fail.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_float_value("testint1", &floatval), "Getting int as float should fail.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_float_value("teststr1", &floatval), "Getting str as float should fail.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_str_value("testint1", &strval[0], 32), "Getting int as str should fail.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_str_value("testfloat1", &strval[0], 32), "Getting float as str should fail.");
}

void test_configuration_save(){
	configuration_init("configurationtest", "test_configuration_saved.ini");

	configuration_set_int_value("testint", 1);
	configuration_set_float_value("testfloat", 2.0f);
	configuration_set_str_value("teststr", "three");
	
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_save(), "Configuration save should succeed.");

	// load what was saved
	configuration_reset();
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_load(), "Saved configuration should have been loaded.");
	int intval = 8;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_int_value("testint", &intval), "Get testint should succeed.");
	float floatval = 8.0f;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_float_value("testfloat", &floatval), "Get testfloat should succeed.");
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(2.0f, floatval, "Retrieved floatval should have been 2.0.");
	char strval[32];
	snprintf(strval, 32, "eight");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_str_value("teststr", &strval[0], 32), "Get teststr should succeed.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("three", strval, 32), "Retrieved strval should have been three.");
}

void test_configuration_get_configdir(){
	configuration_reset();
	setenv("XDG_CONFIG_HOME", "./fakedir", 1);
	configuration_init("configurationtest", "test_configuration.ini");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", configuration_get_configdir(), "Configdir should be empty if configdir not ok.");
	configuration_reset();
	setenv("XDG_CONFIG_HOME", "./fixtures", 1);
	configuration_init("configurationtest", "test_configuration.ini");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("./fixtures/configurationtest", configuration_get_configdir(), "Configdir should be returned.");
}

/*
void test_configuration_set_by_index_int_value(){
	configuration_set_by_index_int_value(0, 1);
}

void test_configuration_get_by_index_int_value(){
	configuration.items[0].val.int_value = 1;
	configuration.items[0].val_type = CONFIGURATION_VAL_INT;
	int val = 0;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_by_index_int_value(-1, &val), "should not have successfully got index -1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_by_index_int_value(CONFIGURATION_ITEMS_MAX, &val), "should not have successfully got index CONFIGURATION_ITEMS_MAX.");
	configuration.items[0].val_type = CONFIGURATION_VAL_FLOAT;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_by_index_int_value(0, &val), "should not have successfully got int from float.");
	configuration.items[0].val_type = CONFIGURATION_VAL_INT;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_by_index_int_value(0, &val), "should have successfully got index 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, val, "should have got 1 from index 0.");
}

void test_configuration_set_by_index_float_value(){
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_set_by_index_float_value(-1, 0.1f), "should not have successfully set value at index -1");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_set_by_index_float_value(CONFIGURATION_ITEMS_MAX, 0.1f), "should not have successfully set value at index CONFIGURATION_ITEMS_MAX");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_set_by_index_float_value(0, 0.1f), "should have successfully set value");
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.1f, configuration.items[0].val.float_value, "configuration item at index 0 should have been set to 0.1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_FLOAT, configuration.items[0].val_type, "configuration item at index 0 should have type FLOAT.");
}

void test_configuration_get_by_index_float_value(){
	float val = 0.0f;
	configuration.items[0].val_type = CONFIGURATION_VAL_FLOAT;
	configuration.items[0].val.float_value = 0.1f;
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0, configuration_get_by_index_float_value(-1, &val), "should not have got value from index -1.");
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0, configuration_get_by_index_float_value(CONFIGURATION_ITEMS_MAX, &val), "should not have got value from index CONFIGURATION_ITEMS_MAX.");
	configuration.items[0].val_type = CONFIGURATION_VAL_INT;
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0, configuration_get_by_index_float_value(0, &val), "should not have got FLOAT from INT.");
	configuration.items[0].val_type = CONFIGURATION_VAL_FLOAT;
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(1, configuration_get_by_index_float_value(0, &val), "should have got value from index 0.");
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.1f, val, "should have got 0.1 from index 0.");
}

void test_configuration_set_by_index_str_value(){
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_set_by_index_str_value(-1, "test"), "Should not have successfully set value at index -1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_set_by_index_str_value(CONFIGURATION_ITEMS_MAX, "test"), "Should not have successfully set value at index CONFIGURATION_ITEMS_MAX.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_set_by_index_str_value(0, "test"), "Should have successfully set value at index 0.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("test", configuration.items[0].val.str_value, "configuration item at index 0 should have been set to \"test\".");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_STR, configuration.items[0].val_type, "configuration item at index 0 should have type STR.");
}

void test_configuration_get_by_index_str_value(){
	char val[32] = {};
	configuration.items[0].val_type = CONFIGURATION_VAL_STR;
	snprintf(configuration.items[0].val.str_value, CONFIGURATION_VAL_STR_LEN, "%s", "test");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_by_index_str_value(-1, &val[0], 32), "should not have got value from index -1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_by_index_str_value(CONFIGURATION_ITEMS_MAX, &val[0], 32), "should not have got value from index CONFIGURATION_ITEMS_MAX.");
	configuration.items[0].val_type = CONFIGURATION_VAL_INT;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_get_by_index_str_value(CONFIGURATION_ITEMS_MAX, &val[0], 32), "should not have got STR from INT.");
	configuration.items[0].val_type = CONFIGURATION_VAL_STR;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_by_index_str_value(0, &val[0], 32), "should have got value from index 0.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("test", val, "should have got \"test\"");
}
*/

void test_configuration_get_error(){
	configuration_reset();
        TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("", configuration_get_error(), 1), "Should not be any error message.");
	int intval;
	configuration_get_int_value("non-existant", &intval);
        TEST_ASSERT_LESS_THAN_INT_MESSAGE(0, strncmp("", configuration_get_error(), 1), "Should have an error message.");
}

int main(){
	UNITY_BEGIN();
	RUN_TEST(test_configuration_init);
	RUN_TEST(test_configuration_load);
	RUN_TEST(test_set_get);
	RUN_TEST(test_configuration_save);
	RUN_TEST(test_configuration_get_configdir);
	/*
	RUN_TEST(test_configuration_set_by_index_int_value);
	RUN_TEST(test_configuration_get_by_index_int_value);
	RUN_TEST(test_configuration_set_by_index_float_value);
	RUN_TEST(test_configuration_get_by_index_float_value);
	RUN_TEST(test_configuration_set_by_index_str_value);
	RUN_TEST(test_configuration_get_by_index_str_value);
	*/
	RUN_TEST(test_configuration_get_error);
	return UNITY_END();
}
