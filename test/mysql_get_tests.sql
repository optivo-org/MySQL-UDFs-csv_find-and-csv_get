# 
# MySQL test suite.
# Here we test the integration of the UDF with mysql.
#
--disable_warnings
SET character set utf8;
DROP FUNCTION IF EXISTS csv_get;
CREATE FUNCTION csv_get RETURNS STRING SONAME 'libmysql_udf_csv_binary_search.so';
--enable_warnings

SELECT 	
csv_get('/tmp/mysql-test/test.csv', '00', 0) AS f00_0,
csv_get('/tmp/mysql-test/test.csv', '00', 1) AS f00_1,
csv_get('/tmp/mysql-test/test.csv', '01', 0) AS f01_0,
csv_get('/tmp/mysql-test/test.csv', '01', 1) AS f01_1,
csv_get('/tmp/mysql-test/test.csv', '09', 0) AS f09_0,
csv_get('/tmp/mysql-test/test.csv', '09', 1) AS f09_1,
csv_get('/tmp/mysql-test/test.csv', '18', 0) AS f17_0,
csv_get('/tmp/mysql-test/test.csv', '18', 1) AS f17_1,
csv_get('/tmp/mysql-test/test.csv', '21', 0) AS f21_0,
csv_get('/tmp/mysql-test/test.csv', '21', 1) AS f21_1,
csv_get('/tmp/mysql-test/test.csv', '22', 0) AS f22_0,
csv_get('/tmp/mysql-test/test.csv', '22', 1) AS f22_1,
csv_get('/tmp/mysql-test/test.csv', '22', 2) AS f22_2,
csv_get('/tmp/mysql-test/test.csv', '23', 0) AS f23_1,
csv_get('/tmp/mysql-test/test.csv', '23', 1) AS f23_1,
csv_get('/tmp/mysql-test/test.csv', '23', 10) AS f23_10,
csv_get('/tmp/mysql-test/test.csv', '24', 0) AS f24_0,
csv_get('/tmp/mysql-test/test.csv', '24', 1) AS f24_1;

# Test utf8 ...
SELECT 
csv_get('/tmp/mysql-test/utf8.csv', 'a', 0) AS fa_0,
csv_get('/tmp/mysql-test/utf8.csv', 'a', 1) AS fa_1,
csv_get('/tmp/mysql-test/utf8.csv', 'b', 0) AS fb_0,
csv_get('/tmp/mysql-test/utf8.csv', 'c', 0) AS fc_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ä', 0) AS fae_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ä', 1) AS fae_1,
csv_get('/tmp/mysql-test/utf8.csv', 'ö', 0) AS foe_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ö', 1) AS foe_1,
csv_get('/tmp/mysql-test/utf8.csv', 'ü', 0) AS fue_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ü', 1) AS fue_1,
csv_get('/tmp/mysql-test/utf8.csv', '€', 0) AS feuro_0,
csv_get('/tmp/mysql-test/utf8.csv', '€', 1) AS feuro_1,
csv_get('/tmp/mysql-test/utf8.csv', '文', 0) AS fsign_0,
csv_get('/tmp/mysql-test/utf8.csv', '文', 1) AS fsign_1,
csv_get('/tmp/mysql-test/utf8.csv', 'ł', 0) AS fpolish_0,
csv_get('/tmp/mysql-test/utf8.csv', 'd', 0) AS fd_0;
# Test utf8 with wrong character set. This results in rows containing non ASCII-characters not being found.
SET character set latin1;
DROP FUNCTION csv_get;
CREATE FUNCTION csv_get RETURNS STRING SONAME 'libmysql_udf_csv_binary_search.so';
SELECT 
csv_get('/tmp/mysql-test/utf8.csv', 'a', 0) AS fa_0,
csv_get('/tmp/mysql-test/utf8.csv', 'a', 1) AS fa_1,
csv_get('/tmp/mysql-test/utf8.csv', 'b', 0) AS fb_0,
csv_get('/tmp/mysql-test/utf8.csv', 'c', 0) AS fc_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ä', 0) AS fae_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ö', 0) AS foe_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ü', 0) AS fue_0,
csv_get('/tmp/mysql-test/utf8.csv', '€', 0) AS feuro_0,
csv_get('/tmp/mysql-test/utf8.csv', '文', 0) AS fsign_0,
csv_get('/tmp/mysql-test/utf8.csv', 'ł', 0) AS fpolish_0,
csv_get('/tmp/mysql-test/utf8.csv', 'd', 0) AS fd_0;
