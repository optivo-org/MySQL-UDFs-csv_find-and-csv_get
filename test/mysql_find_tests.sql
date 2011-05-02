# 
# MySQL test suite.
# Here we test the integration of the UDF with mysql.
#
--disable_warnings
SET character set utf8;
DROP FUNCTION IF EXISTS csv_find;
CREATE FUNCTION csv_find RETURNS INTEGER SONAME 'libmysql_udf_csv_binary_search.so';
--enable_warnings

SELECT 	
csv_find('/tmp/mysql-test/test.csv', '00') AS f00,
csv_find('/tmp/mysql-test/test.csv', '01') AS f01,
csv_find('/tmp/mysql-test/test.csv', '02') AS f02,
csv_find('/tmp/mysql-test/test.csv', '03') AS f03,
csv_find('/tmp/mysql-test/test.csv', '04') AS f04,
csv_find('/tmp/mysql-test/test.csv', '05') AS f05,
csv_find('/tmp/mysql-test/test.csv', '06') AS f06,
csv_find('/tmp/mysql-test/test.csv', '07') AS f07,
csv_find('/tmp/mysql-test/test.csv', '08') AS f08,
csv_find('/tmp/mysql-test/test.csv', '09') AS f09,
csv_find('/tmp/mysql-test/test.csv', '10') AS f10,
csv_find('/tmp/mysql-test/test.csv', '11') AS f11,
csv_find('/tmp/mysql-test/test.csv', '12') AS f12,
csv_find('/tmp/mysql-test/test.csv', '13') AS f13,
csv_find('/tmp/mysql-test/test.csv', '14') AS f14,
csv_find('/tmp/mysql-test/test.csv', '15') AS f15,
csv_find('/tmp/mysql-test/test.csv', '16') AS f16,
csv_find('/tmp/mysql-test/test.csv', '17') AS f17,
csv_find('/tmp/mysql-test/test.csv', '18') AS f18,
csv_find('/tmp/mysql-test/test.csv', '19') AS f19,
csv_find('/tmp/mysql-test/test.csv', '20') AS f20,
csv_find('/tmp/mysql-test/test.csv', '21') AS f21,
csv_find('/tmp/mysql-test/test.csv', '22') AS f22,
csv_find('/tmp/mysql-test/test.csv', '23') AS f23,
csv_find('/tmp/mysql-test/test.csv', '24') AS f24;

# Test utf8 ...
SELECT 
csv_find('/tmp/mysql-test/utf8.csv', 'a') AS fa,
csv_find('/tmp/mysql-test/utf8.csv', 'b') AS fb,
csv_find('/tmp/mysql-test/utf8.csv', 'c') AS fc,
csv_find('/tmp/mysql-test/utf8.csv', 'ä') AS fae,
csv_find('/tmp/mysql-test/utf8.csv', 'ö') AS foe,
csv_find('/tmp/mysql-test/utf8.csv', 'ü') AS fue,
csv_find('/tmp/mysql-test/utf8.csv', '€') AS feuro,
csv_find('/tmp/mysql-test/utf8.csv', '文') AS fsign,
csv_find('/tmp/mysql-test/utf8.csv', 'ł') AS fpolish,
csv_find('/tmp/mysql-test/utf8.csv', 'd') AS fd;

# Test utf8 with wrong character set. This results in rows containing non ASCII-characters not being found.
SET character set latin1;
DROP FUNCTION csv_find;
CREATE FUNCTION csv_find RETURNS INTEGER SONAME 'libmysql_udf_csv_binary_search.so';
SELECT 
csv_find('/tmp/mysql-test/utf8.csv', 'a') AS fa,
csv_find('/tmp/mysql-test/utf8.csv', 'b') AS fb,
csv_find('/tmp/mysql-test/utf8.csv', 'c') AS fc,
csv_find('/tmp/mysql-test/utf8.csv', 'ä') AS fae,
csv_find('/tmp/mysql-test/utf8.csv', 'ö') AS foe,
csv_find('/tmp/mysql-test/utf8.csv', 'ü') AS fue,
csv_find('/tmp/mysql-test/utf8.csv', '€') AS feuro,
csv_find('/tmp/mysql-test/utf8.csv', '文') AS fsign,
csv_find('/tmp/mysql-test/utf8.csv', 'ł') AS fpolish,
csv_find('/tmp/mysql-test/utf8.csv', 'd') AS fd;