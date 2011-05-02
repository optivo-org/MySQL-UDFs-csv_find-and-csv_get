CC = /usr/bin/g++
CFLAGS = -fPIC -Wall -o3 -I/usr/include/mysql/
LDFLAGS = 
OBJ = libmysql_udf_csv_binary_search.so
MYSQL_PLUGIN_PATH = /usr/lib64/mysql/plugin
MYSQL_TEST_PATH = /tmp/mysql-test

all: sharedlib testbuild

install: sharedlib
	cp $(OBJ) $(MYSQL_PLUGIN_PATH)

sharedlib: 
	@$(CC) $(CFLAGS) -shared src/mysql_udf_csv_binary_search.cpp -o $(OBJ) $(LDFLAGS)

test: unittests mysqltests

testbuild: sharedlib
	@$(CC) $(CFLAGS) -o unit_tests $(OBJ) test/unit_tests.cpp $(LDFLAGS)
	
unittests: testbuild
	@echo "Running unit tests ..."
	@echo
	@LD_LIBRARY_PATH=. ./unit_tests

mysqltests: testbuild
	@echo
	@echo "Running mysql tests (must be root) ..."
	@echo "Creating database 'csv_test' ..."
	@mysql -e "CREATE DATABASE IF NOT EXISTS csv_test DEFAULT CHARACTER SET 'utf8'; USE csv_test; DROP FUNCTION IF EXISTS csv_find; DROP FUNCTION IF EXISTS csv_get"
	@echo "Copying plugin and test data ..."
	@cp $(OBJ) $(MYSQL_PLUGIN_PATH)
	@rm -rf $(MYSQL_TEST_PATH)
	@mkdir $(MYSQL_TEST_PATH)
	@cp test/*.sql test/*.expected test/*.csv $(MYSQL_TEST_PATH)
	@chown -R mysql.mysql $(MYSQL_TEST_PATH)
	@echo -n "Executing tests for 'csv_find' ... "
	@mysqltest -R $(MYSQL_TEST_PATH)/mysql_find_tests.expected csv_test < $(MYSQL_TEST_PATH)/mysql_find_tests.sql 
	@echo -n "Executing tests for 'csv_get' ... "
	@mysqltest -R $(MYSQL_TEST_PATH)/mysql_get_tests.expected csv_test < $(MYSQL_TEST_PATH)/mysql_get_tests.sql 
	@echo "Dropping database 'csv_test' and UDFs ..."
	@mysql -e "USE csv_test; DROP FUNCTION IF EXISTS csv_find; DROP FUNCTION IF EXISTS csv_get; DROP DATABASE IF EXISTS csv_test"

clean:
	rm -f *.o *.so unit_tests test/performance.csv
