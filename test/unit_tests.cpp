/*
	Copyright (C) 2010 and later optivo GmbH, pr-mysqludf@optivo.de

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


	Unit tests.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mysql.h"
#include "../src/mysql_udf_csv_binary_search.h"

#include "minunit.h"
int tests_run = 0;
int tests_failed = 0;

// Override some global constants.
char *_MUCBS_CSV_PATH = strdup("");
long _MUCBS_MAX_FILE_SIZE = 1024 * 1024 * 1024;

// ---------------------------------------------------------------------------
// Helper functions.
// ---------------------------------------------------------------------------

UDF_INIT *udf_init = new UDF_INIT;
UDF_ARGS *udf_args = new UDF_ARGS;
char *message = new char[255];
char *is_null = new char[255];
char *error = new char[255];
char *buf = new char[255];
char *result = new char[255];
long *length = new long;

void call_csv_find_init(const char *file) {
	udf_args->arg_count = 2;
	udf_args->arg_type = new Item_result;
	udf_args->lengths = (unsigned long *) new unsigned long[2];
	udf_args->args = (char **) new char[2][255];
	udf_args->args[0] = strdup(file);
	udf_args->lengths[0] = strlen(file);
	udf_args->args[1] = strdup("");
	udf_args->lengths[1] = 0;
	my_bool result = csv_find_init(udf_init, udf_args, message);
	char *buf = new char[255];
	sprintf(buf, "Error calling csv_find_init: %s", message);
	mu_assert(buf, result == 0);
}

long call_csv_find(const char *text) {
	if (text == NULL) {
		udf_args->args[1] = NULL;
		udf_args->lengths[1] = 0;
	} else {
		udf_args->args[1] = strdup(text);
		udf_args->lengths[1] = strlen(text);
	}
	return csv_find(udf_init, udf_args, is_null, error);
}

void call_csv_find_deinit() {
	csv_find_deinit(udf_init);
}

void call_csv_get_init(const char *file) {
	udf_args->arg_count = 3;
	udf_args->arg_type = new Item_result;
	udf_args->lengths = (unsigned long *) new unsigned long[3];
	udf_args->args = (char **) new char[3][255];
	udf_args->args[0] = strdup(file);
	udf_args->lengths[0] = strlen(file);
	udf_args->args[1] = strdup("");
	udf_args->lengths[1] = 0;
	udf_args->args[2] = strdup("");
	udf_args->lengths[2] = 0;
	my_bool result = csv_get_init(udf_init, udf_args, message);
	char *buf = new char[255];
	sprintf(buf, "Error calling csv_get_init: %s", message);
	mu_assert(buf, result == 0);
}

char *call_csv_get(const char *text, int col) {
	if (text == NULL) {
		udf_args->args[1] = NULL;
		udf_args->lengths[1] = 0;
	} else {
		udf_args->args[1] = strdup(text);
		udf_args->lengths[1] = strlen(text);
	}
	sprintf(buf, "%d", col);
	udf_args->args[2] = buf;
	udf_args->lengths[2] = strlen(buf);
	char *value = csv_get(udf_init, udf_args, result, length, is_null, error);
	if (value != NULL) {
		strncpy(result, value, *length);
		result[*length] = 0;
		return result;
	} else {
		return NULL;
	}
}

void call_csv_get_deinit() {
	csv_find_deinit(udf_init);
}

// ---------------------------------------------------------------------------
// Test csv_find
// ---------------------------------------------------------------------------
void test_find() {
	call_csv_find_init(strdup("test/test.csv"));
	char *text = new char[2];
	for (int a = 1; a <= 23; a++) {
		sprintf(text, "%02d", a);
		mu_assert("String should have been found.", call_csv_find(text) == 1);
	}
	mu_assert("String '00' should not have been found.", call_csv_find("00") == 0);
	mu_assert("String '24' should not have been found.", call_csv_find("24") == 0);
	mu_assert("String 'a' should not have been found.", call_csv_find("a") == 0);
	mu_assert("NULL should not have been found.", call_csv_find(NULL) == 0);
	call_csv_find_deinit();
}

void test_find_empty() {
	call_csv_find_init(strdup("test/empty.csv"));
	mu_assert("String '1' should not have been found.", call_csv_find("1") == 0);
	call_csv_find_deinit();
}

void test_find_utf8() {
	call_csv_find_init("test/utf8.csv");
	mu_assert("String 'a' should have been found.", call_csv_find("a") == 1);
	mu_assert("String 'b' should have been found.", call_csv_find("b") == 1);
	mu_assert("String 'c' should have been found.", call_csv_find("c") == 1);
	mu_assert("String 'ä' should have been found.", call_csv_find("ä") == 1);
	mu_assert("String 'ö' should have been found.", call_csv_find("ö") == 1);
	mu_assert("String 'ü' should have been found.", call_csv_find("ü") == 1);
	mu_assert("String '€' should have been found.", call_csv_find("€") == 1);
	mu_assert("String '文' should have been found.", call_csv_find("文") == 1);
	mu_assert("String 'ł' should not have been found.", call_csv_find("ł") == 0);
	call_csv_find_deinit();
}

void test_find_performance() {
	const int rows = 1000000;
	// First create a huge csv file ...
	FILE *fd = fopen("test/performance.csv", "w");
	mu_assert("Error opening test/performance.csv for writing.", fd > 0);
	for (int a = 0; a < rows; a++) {
		fprintf(fd, "%07d\n", a);
	}
	fclose(fd);
	call_csv_find_init("test/performance.csv");
	char *text = new char[255];
	clock_t start = clock();
	for (int a = 0; a < rows; a++) {
		sprintf(text, "%07d", a);
		call_csv_find(text);
	}
	clock_t end = clock();
	float elapsed = ((float) end - (float) start) / CLOCKS_PER_SEC;
	int ops_per_sec = rows / elapsed;
	printf("\tTested %d lookups. Elapsed time: %f, Lookups per second: %d\n", rows, elapsed, ops_per_sec);
	// This is just a dump minimum assumption. Performance should be much higher on most machines.
	mu_assert("Performance should be high enough.", ops_per_sec > 300000);
	call_csv_find_deinit();
}

// ---------------------------------------------------------------------------
// Test csv_get
// ---------------------------------------------------------------------------
void test_get() {
	call_csv_get_init("test/test.csv");
	char *text = new char[255];
	char *buf = new char[255];
	for (int a = 1; a <= 20; a++) {
		sprintf(text, "%02d", a);
		char *value = call_csv_get(text, 0);
		sprintf(buf, "Value should be correct for key '%s' and index 0.", text);
		mu_assert(buf, value != NULL && strcmp(value, text) == 0);

		value = call_csv_get(text, 1);
		sprintf(buf, "Value should be correct for key '%s' and index 1.", text);
		sprintf(text, "v%02d", a);
		mu_assert(buf, value != NULL && strcmp(value, text) == 0);
	}
	mu_assert("String at '21':0 should have been found.", strcmp(call_csv_get("21", 0), "21") == 0);
	mu_assert("String at '21':1 should not have been found.", call_csv_get("21", 1) == NULL);
	mu_assert("String at '22':0 should have been found.", strcmp(call_csv_get("22", 0), "22") == 0);
	mu_assert("String at '22':1 should have been found.", strcmp(call_csv_get("22", 1), "") == 0);
	mu_assert("String at '22':2 should have been found.", strcmp(call_csv_get("22", 2), "w22") == 0);
	mu_assert("String at '00':0 should not have been found.", call_csv_get("00", 0) == NULL);
	mu_assert("String at '24':0 should not have been found.", call_csv_get("24", 0) == NULL);
	mu_assert("String at 'a':0 should not have been found.", call_csv_get("a", 1) == NULL);
	mu_assert("String at NULL:0 should not have been found.", call_csv_get(NULL, 1) == NULL);
	call_csv_get_deinit();
}

void test_get_empty() {
	call_csv_get_init(strdup("test/empty.csv"));
	mu_assert("String '1' should not have been found.", call_csv_get("1", 0) == NULL);
	call_csv_get_deinit();
}

void test_get_utf8() {
	call_csv_find_init("test/utf8.csv");
	mu_assert("String 'a' should have been found.", strcmp(call_csv_get("a", 0), "a") == 0);
	mu_assert("String 'b' should have been found.", strcmp(call_csv_get("b", 0), "b") == 0);
	mu_assert("String 'c' should have been found.", strcmp(call_csv_get("c", 0), "c") == 0);
	mu_assert("String 'ä' should have been found.", strcmp(call_csv_get("ä", 0), "ä") == 0);
	mu_assert("String 'ä' should have been found.", strcmp(call_csv_get("ä", 1), "Ä") == 0);
	mu_assert("String 'ö' should have been found.", strcmp(call_csv_get("ö", 0), "ö") == 0);
	mu_assert("String 'ö' should have been found.", strcmp(call_csv_get("ö", 1), "Ö") == 0);
	mu_assert("String 'ü' should have been found.", strcmp(call_csv_get("ü", 0), "ü") == 0);
	mu_assert("String 'ü' should have been found.", strcmp(call_csv_get("ü", 1), "Ü") == 0);
	mu_assert("String '€' should have been found.", strcmp(call_csv_get("€", 0), "€") == 0);
	mu_assert("String '€' should have been found.", strcmp(call_csv_get("€", 1), "€€") == 0);
	mu_assert("String '文' should have been found.", strcmp(call_csv_get("文", 0), "文") == 0);
	mu_assert("String '文' should have been found.", strcmp(call_csv_get("文", 1), "文文") == 0);
	mu_assert("String 'ł' should have been found.", call_csv_get("ł", 0) == NULL);
	call_csv_find_deinit();
}

void test_get_performance() {
	const int rows = 1000000;
	// First create a huge csv file ...
	FILE* fd = fopen("test/performance.csv", "w");
	mu_assert("Error opening test/performance.csv for writing.", fd > 0);
	for (int a = 0; a < rows; a++) {
		fprintf(fd, "%07d\tv%07d\n", a, a);
	}
	fclose(fd);
	call_csv_get_init("test/performance.csv");
	char *text = new char[255];
	clock_t start = clock();
	for (int a = 0; a < rows; a++) {
		sprintf(text, "%07d", a);
		call_csv_get(text, 1);
	}
	clock_t end = clock();
	float elapsed = ((float) end - (float) start) / CLOCKS_PER_SEC;
	int ops_per_sec = rows / elapsed;
	printf("\tTested %d gets. Elapsed time: %f, Gets per second: %d\n", rows, elapsed, ops_per_sec);
	// This is just a dump minimum assumption. Performance should be much higher on most machines.
	mu_assert("Performance should be high enough.", ops_per_sec > 300000);
}

// ---------------------------------------------------------------------------
// Test assertions
// ---------------------------------------------------------------------------
void test_csv_path_should_be_checked() {
	int success = 0;
	try {
		_MUCBS_CSV_PATH = strdup("/tmp");
		call_csv_get_init("test/test.csv");
		success = 1;
	} catch (...) {
	}
	_MUCBS_CSV_PATH = strdup("");
	mu_assert("Invalid path should have raised an error.", success == 0);
	try {
		_MUCBS_CSV_PATH = strdup("/tmp");
		call_csv_get_init("/tmp/../etc");
		success = 1;
	} catch (...) {
	}
	_MUCBS_CSV_PATH = strdup("");
	mu_assert("Invalid relative path should have raised an error.", success == 0);
	try {
		call_csv_get_init("test/not_existing_file.csv");
		success = 1;
	} catch (...) {
	}
	_MUCBS_CSV_PATH = strdup("");
	mu_assert("Not existing path should be rejected.", success == 0);
}

void test_csv_file_size_should_be_checked() {
	int success = 0;
	try {
		_MUCBS_MAX_FILE_SIZE = 10;
		call_csv_get_init("test/test.csv");
		success = 1;
	} catch (...) {
	}
	_MUCBS_MAX_FILE_SIZE = 1024 * 1024 * 1024;
	mu_assert("Too big file should have raised an error.", success == 0);
}

int main(int argc, char **argv) {
	setvbuf(stdout, NULL, _IONBF, 0);
	mu_run_test(test_csv_path_should_be_checked, "test_csv_path_should_be_checked");
	mu_run_test(test_csv_file_size_should_be_checked, "test_csv_file_size_should_be_checked");
	mu_run_test(test_find, "test_find");
	mu_run_test(test_find_empty, "test_find_empty");
	mu_run_test(test_find_utf8, "test_find_utf8");
	mu_run_test(test_find_performance, "test_find_performance");
	mu_run_test(test_get, "test_get");
	mu_run_test(test_get_empty, "test_get_empty");
	mu_run_test(test_get_utf8, "test_get_utf8");
	mu_run_test(test_get_performance, "test_get_performance");
	printf("Tests OK: %d, FAILED: %d\n", tests_run, tests_failed);
	if (tests_failed == 0) {
		printf("\nAll unit tests ran successfully.\n");
	} else {
		printf("\nAt least on unit test FAILED.\n");
	}
	return tests_failed;
}

