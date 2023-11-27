#include <string.h>
#include <stdlib.h>
#include "../../Unity/src/unity.h"
#include "../src/configuration.h"

void reset_configuration(){
	for(int i = 0; i < CONFIGURATION_ITEMS_MAX; i++){
		configuration.mappings[i].key[0] = '\0'; 
		configuration.mappings[i].index = 0; 
		configuration.items[i].key[0] = '\0'; 
		configuration.items[i].val_type = CONFIGURATION_VAL_INT; 
		configuration.items[i].val.int_value = 0; 
	}
	configuration.num_items = 0;
	configuration.loaded = 0;
}

char *xdg_config_home_orig = NULL;
char *home_orig = NULL;

// runs before each test
void setUp(void){
	// save original env
	xdg_config_home_orig = getenv("XDG_CONFIG_HOME");
	home_orig = getenv("HOME");
	// set test env
	setenv("XDG_CONFIG_HOME", "./fixtures", 1);

	reset_configuration();

	strncpy(configuration.configdir, "fixtures", 256);
	configuration.configdirok = 1;
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
	// configuration.configdirok = 0;
}

void test_configuration_init(){
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_init("","fail"), "Configuration init for empty dirname should fail.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration_init("fail",""), "Configuration init for empty filename should fail.");

	// test usage of XDG_CONFIG_HOME
	configuration.configdirok = 0;
	setenv("XDG_CONFIG_HOME", "./fixtures", 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_init("configurationtest","configurationtest.ini"), "Configuration init with XDG_CONFIG_HOME should succeed.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("./fixtures/configurationtest", configuration.configdir, "configuration_dirname should have been set by XDG_CONFIG_HOME.");

	// test usage of HOME
	configuration.configdirok = 0;
	unsetenv("XDG_CONFIG_HOME");
	setenv("HOME", "./fixtures", 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_init("configurationtest","configurationtest.ini"), "Configuration init with HOME should succeed.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("./fixtures/.config/configurationtest", configuration.configdir, "configuration_dirname should have been set by HOME.");

	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration.num_items, "number of items should b zero after init.");
}

void test_configuration_init_indexes(){
	struct configuration_index_mapping confmap[CONFIGURATION_ITEMS_MAX] = {
		{ "three", 3 },
		{ "two", 2 },
		{ "one", 1 }
	};

	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_init_indexes(confmap), "configuration_init_indexes should succeed.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("three", configuration.mappings[0].key, "configuration mapping key at 0 should be three.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("three", configuration.items[3].key, "configuration item key at 3 should be three.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, configuration.mappings[0].index, "configuration mapping index at 0 should be 3.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("two", configuration.mappings[1].key, "configuration mapping key at 1 should be two.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("two", configuration.items[2].key, "configuration item key at 2 should be two.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, configuration.mappings[1].index, "configuration mapping index at 1 should be 2.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("one", configuration.mappings[2].key, "configuration mapping key at 2 should be one.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("one", configuration.items[1].key, "configuration item key at 1 should be one.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration.mappings[2].index, "configuration mapping index at 2 should be 1.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("", configuration.mappings[3].key, "configuration mapping key at 3 should be empty.");
}

void test_configuration_load(){
	strncpy(configuration_filename, "test_configuration.ini", 32);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_load(), "Configuration should have been loaded.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(8, configuration.num_items, "Number of configuration items should have been eight.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("one", configuration.items[0].key, "Configuration one should have been in first configuration slot.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_INT, configuration.items[0].val_type, "first configuration item type should be int.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_STR, configuration.items[4].val_type, "fifth configuration item type should be str.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_FLOAT, configuration.items[6].val_type, "seventh configuration item type should be float.");

	// test load using indexes
	reset_configuration();
	snprintf(configuration.mappings[0].key, CONFIGURATION_KEY_MAX, "%s", "three");
	configuration.mappings[0].index = 0;
	snprintf(configuration.mappings[1].key, CONFIGURATION_KEY_MAX, "%s", "two");
	configuration.mappings[1].index = 1;
	snprintf(configuration.mappings[2].key, CONFIGURATION_KEY_MAX, "%s", "one");
	configuration.mappings[2].index = 2;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_load(), "Configuration should have been loaded.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(8, configuration.num_items, "Number of configuration should have been eight.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(8, configuration.num_items, "Number of configuration should have been eight.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("two", configuration.items[1].key, "Configuration two should have been at index 1.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("one", configuration.items[2].key, "Configuration one should have been at index 2.");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("three", configuration.items[0].key, "Configuration three should have been at index 0.");
}

void test_configuration_save(){
	strncpy(configuration_filename, "test_configuration_saved.ini", 32);
	strncpy(configuration.items[0].key, "test1", 32);
	configuration.items[0].val.int_value = 1;
	strncpy(configuration.items[1].key, "test2", 32);
	configuration.items[1].val.int_value = 2;
	strncpy(configuration.items[2].key, "test3", 32);
	configuration.items[2].val.int_value = 3;
	strncpy(configuration.items[3].key, "test4", 32);
	configuration.items[3].val.int_value = 4;
	strncpy(configuration.items[4].key, "teststr1", 32);
	configuration.items[4].val_type = CONFIGURATION_VAL_STR;
	snprintf(&configuration.items[4].val.str_value[0], CONFIGURATION_VAL_STR_LEN, "%s", "str1");
	strncpy(configuration.items[5].key, "teststr2", 32);
	configuration.items[5].val_type = CONFIGURATION_VAL_STR;
	snprintf(&configuration.items[5].val.str_value[0], CONFIGURATION_VAL_STR_LEN, "%s", "str2");
	strncpy(configuration.items[6].key, "testfloat1", 32);
	configuration.items[6].val_type = CONFIGURATION_VAL_FLOAT;
	configuration.items[6].val.float_value = 1.234f;
	strncpy(configuration.items[7].key, "testfloat2", 32);
	configuration.items[7].val_type = CONFIGURATION_VAL_FLOAT;
	configuration.items[7].val.float_value = 56.789f;
	configuration.num_items = 8;
	configuration_save();
	configuration_load();

	TEST_ASSERT_EQUAL_INT_MESSAGE(8, configuration.num_items, "eight configurations should have been loaded.");
	int matched = (strncmp(configuration.items[0].key, "test1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "test1 should have been in slot 0.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration.items[0].val.int_value, "test1 should have had value 1.");

	matched = (strncmp(configuration.items[1].key, "test2", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "test2 should have been in slot 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(2, configuration.items[1].val.int_value, "test2 should have had value 2.");

	matched = (strncmp(configuration.items[2].key, "test3", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "test3 should have been in slot 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(3, configuration.items[2].val.int_value, "test3 should have had value 3.");

	matched = (strncmp(configuration.items[3].key, "test4", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "test4 should have been in slot 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(4, configuration.items[3].val.int_value, "test4 should have had value 4.");

	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_STR, configuration.items[4].val_type, "fifth configuration item type should be str.");

	matched = (strncmp(configuration.items[6].key, "testfloat1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "testfloat1 should have been in slot 6.");
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 1.234f, configuration.items[6].val.float_value, "testfloat1 should have had value 1.234.");
}

void test_configuration_set_by_index_int_value(){
	configuration_set_by_index_int_value(0, 1);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration.items[0].val.int_value, "configuration item at index 0 should have been set to 1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_INT, configuration.items[0].val_type, "configuration item at index 0 should have type INT.");
}

void test_configuration_set_int_value(){
	configuration_set_int_value("test1", 1);
	int matched = (strncmp(configuration.items[0].key, "test1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "test1 should have been in first configuration slot.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration.items[0].val.int_value, "test1 should have had value 1.");

	// update value
	configuration_set_int_value("test1", 0);
	matched = (strncmp(configuration.items[0].key, "test1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "test1 should have been in first configuration slot.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, configuration.items[0].val.int_value, "test1 should have had value 0.");
	matched = (strncmp(configuration.items[1].key, "test1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, matched, "test1 should NOT have been in second configuration slot.");

	configuration_set_int_value("test2", 1);
	matched = (strncmp(configuration.items[1].key, "test2", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "test2 should have been in second configuration slot.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration.items[1].val.int_value, "test2 should have had value 1.");
}

void test_configuration_get_by_index_int_value(){
	configuration.items[0].val.int_value = 1;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_by_index_int_value(0), "should have got 1 from index 0.");
}

void test_configuration_get_int_value(){
	reset_configuration();
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, configuration_get_int_value("test1"), "test1 should NOT be configured.");
	strncpy(configuration.items[0].key, "test1", 32);
	configuration.items[0].val.int_value = 1;
	configuration.num_items = 1;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_int_value("test1"), "test1 should be configured.");
	strncpy(configuration.items[1].key, "test2", 32);
	configuration.items[1].val.int_value = 1;
	configuration.num_items = 2;
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, configuration_get_int_value("test2"), "test2 should be configurationed.");
}

void test_configuration_set_by_index_float_value(){
	configuration_set_by_index_float_value(0, 0.1f);
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.1f, configuration.items[0].val.float_value, "configuration item at index 0 should have been set to 0.1.");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_FLOAT, configuration.items[0].val_type, "configuration item at index 0 should have type FLOAT.");
}

void test_configuration_set_float_value(){
	configuration_set_float_value("testfloat1", 1.234f);
	int matched = (strncmp(configuration.items[0].key, "testfloat1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "testfloat1 should have been in first configuration slot.");
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 1.234f, configuration.items[0].val.float_value, "testfloat1 should have had value 1.234");

	// update value
	configuration_set_float_value("testfloat1", 56.789f);
	matched = (strncmp(configuration.items[0].key, "testfloat1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "testfloat1 should have been in first configuration slot.");
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 56.789, configuration.items[0].val.float_value, "testfloat1 should have had value 56.789.");
	matched = (strncmp(configuration.items[1].key, "testfloat1", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, matched, "testfloat1 should NOT have been in second configuration slot.");

	configuration_set_float_value("testfloat2", 12.345f);
	matched = (strncmp(configuration.items[1].key, "testfloat2", 32) == 0);
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, matched, "testfloat2 should have been in second configuration slot.");
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 12.345f, configuration.items[1].val.float_value, "testfloat2 should have had value 12.345.");
}

void test_configuration_get_by_index_float_value(){
	configuration.items[0].val.float_value = 0.1f;
	TEST_ASSERT_EQUAL_FLOAT_MESSAGE(0.1f, configuration_get_by_index_float_value(0), "should have got 0.1 from index 0.");
}

void test_configuration_get_float_value(){
	reset_configuration();
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, configuration_get_float_value("testfloat1"), "testfloat1 should NOT be configured.");
	strncpy(configuration.items[0].key, "testfloat1", 32);
	configuration.items[0].val_type = CONFIGURATION_VAL_FLOAT;
	configuration.items[0].val.float_value = 1.234f;
	configuration.num_items = 1;
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 1.234f, configuration_get_float_value("testfloat1"), "testfloat1 should be configured.");
	strncpy(configuration.items[1].key, "testfloat2", 32);
	configuration.items[1].val_type = CONFIGURATION_VAL_FLOAT;
	configuration.items[1].val.float_value = 12.345f;
	configuration.num_items = 2;
	TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.001f, 12.345f, configuration_get_float_value("testfloat2"), "testfloat2 should be configurationed.");
}

void test_configuration_set_by_index_str_value(){
	configuration_set_by_index_str_value(0, "test");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("test", configuration.items[0].val.str_value, "configuration item at index 0 should have been set to \"test\".");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_STR, configuration.items[0].val_type, "configuration item at index 0 should have type STR.");
}

void test_configuration_set_str_value(){
	reset_configuration();
	configuration_set_str_value("test1", "str1");
	TEST_ASSERT_EQUAL_INT_MESSAGE(CONFIGURATION_VAL_STR, configuration.items[0].val_type, "val type should be str");
}

void test_configuration_get_by_index_str_value(){
	snprintf(configuration.items[0].val.str_value, CONFIGURATION_VAL_STR_LEN, "%s", "test");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("test", configuration_get_by_index_str_value(0), "should have got \"test\" from index 0.");
}

void test_configuration_get_str_value(){
	reset_configuration();
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("", configuration_get_str_value("test1"), CONFIGURATION_VAL_STR_LEN), "test1 should NOT be configured.");
	snprintf(&configuration.items[0].key[0], 32, "%s", "test1");
	configuration.items[0].val_type = CONFIGURATION_VAL_STR;
	snprintf(&configuration.items[0].val.str_value[0], CONFIGURATION_VAL_STR_LEN, "%s", "str1");
	configuration.loaded = 1;
	configuration.num_items = 1;
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("str1", configuration_get_str_value("test1"), CONFIGURATION_VAL_STR_LEN), "val should be str1");
}

void test_configuration_get_error(){
        TEST_ASSERT_EQUAL_INT_MESSAGE(0, strncmp("", configuration_get_error(), 1), "Should not be any error message.");
	configuration_set_by_index_float_value(CONFIGURATION_ITEMS_MAX + 1, 0.1f);
        TEST_ASSERT_LESS_THAN_INT_MESSAGE(0, strncmp("", configuration_get_error(), 1), "Should have an error message.");
}

int main(){
	UNITY_BEGIN();
	RUN_TEST(test_configuration_init);
	RUN_TEST(test_configuration_init_indexes);
	RUN_TEST(test_configuration_load);
	RUN_TEST(test_configuration_save);
	RUN_TEST(test_configuration_set_by_index_int_value);
	RUN_TEST(test_configuration_set_int_value);
	RUN_TEST(test_configuration_get_by_index_int_value);
	RUN_TEST(test_configuration_get_int_value);
	RUN_TEST(test_configuration_set_by_index_float_value);
	RUN_TEST(test_configuration_set_float_value);
	RUN_TEST(test_configuration_get_by_index_float_value);
	RUN_TEST(test_configuration_get_float_value);
	RUN_TEST(test_configuration_set_by_index_str_value);
	RUN_TEST(test_configuration_set_str_value);
	RUN_TEST(test_configuration_get_by_index_str_value);
	RUN_TEST(test_configuration_get_str_value);
	RUN_TEST(test_configuration_get_error);
	return UNITY_END();
}
