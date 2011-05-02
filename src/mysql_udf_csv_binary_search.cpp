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


 MySQL UDF: Searching on sorted csv files.

 Prerequisites:
 --------------
 Line and column delimiter are fixed ('\n' and '\t' by default).
 See _MUCBS_LINE_DELIMITER and _MUCBS_COLUMN_DELIMITER.

 csv files must be in ascending lexicographic binary UTF8 order.
 This means that for example 'ä' (0xc3 0xb6) must be placed
 after 'z' (0x7a).
 To sort a file you could use:
 LC_ALL=C sort < input.csv > ordered.csv

 csv files must not contain row or column delimiter within columns.
 No escaping of any kind is done. If you need it, implement it in your
 query like this:
 SELECT * FROM table WHERE csv_find('my.csv', REPLACE(my_column, '\n', '\\n'))
 assuming that all '\n' are escaped as '\\n' in your csv.

 csv files must be smaller than _MUCBS_MAX_FILE_SIZE.

 Only csv files beneath _MUCBS_CSV_PATH (default: /tmp/) are accepted. This
 adds a minimum of security.
 */

#if defined(_WIN32)
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include "mysql.h"
#include "mysql_udf_csv_binary_search.h"

// We define this 'constants' as variables here to override
// them in tests.
int _MUCBS_LINE_DELIMITER = '\n';
int _MUCBS_COLUMN_DELIMITER = '\t';
// Only allow files up to this size.
long _MUCBS_MAX_FILE_SIZE = 1024 * 1024 * 1024;
// Only files beneath this path are accepted.
char *_MUCBS_CSV_PATH = strdup("/");

#define ERR_ARG "Wrong number of arguments."
#define ERR_INVALID_PATH "File is outside restricted path or is not readable by mysqld. Check permissions and apparmor etc."
#define ERR_STAT_FILE "Unable to stat file."
#define ERR_FILE_SIZE "File is too big."
#define ERR_OPEN_FILE "Unable to open file."
#define ERR_MMAP "Cannot mmap file. Out of memory?"
#define ERR_MEM "Out of memory."
#define ERR_OTHER "A c++ exception has been raised."

extern "C" {

/**
 * Hold all data necessary in one struct as required by MySQL UDFs.
 */
struct csv_data {
	char *buf;
	long len;
	int fd;
};

/**
 * Generic UDF init function used by csv_find and csv_get.
 */
my_bool csv_init(UDF_INIT *initid, UDF_ARGS *args, char *message, unsigned int num_args) {
	try {
		if (args->arg_count == num_args) {
			struct csv_data *data;
			if (!(data = (struct csv_data *) malloc(sizeof(struct csv_data)))) {
				strcpy(message, ERR_MEM);
				return 1;
			}
			struct stat sbuf;
			char *filename = realpath(args->args[0], NULL);
			if (filename == NULL || strstr(filename, _MUCBS_CSV_PATH) != filename) {
				strcpy(message, ERR_INVALID_PATH);
				return 1;
			}
			if (stat(filename, &sbuf) == -1) {
				strcpy(message, ERR_STAT_FILE);
				return 1;
			}
			data->len = sbuf.st_size;
			if (data->len > _MUCBS_MAX_FILE_SIZE) {
				strcpy(message, ERR_FILE_SIZE);
				return 1;
			}
			if ((data->fd = open(filename, O_RDONLY)) == -1) {
				strcpy(message, ERR_OPEN_FILE);
				return 1;
			}
			if (data->len == 0) {
				// File is empty but we want to handle this.
				data->buf = new char[1];
			} else {
				if ((data->buf = (char *) mmap((caddr_t) 0, data->len, PROT_READ, MAP_SHARED, data->fd, 0)) == (caddr_t) (-1)) {
					strcpy(message, ERR_MMAP);
					return 1;
				}
			}
			initid->maybe_null = 0;
			initid->ptr = (char *) data;
		} else {
			strcpy(message, ERR_ARG);
			return 1;
		}
		return 0;
	} catch (...) {
		strcpy(message, ERR_OTHER);
		return 1;
	}
}

/**
 * Test whether the given text is found at data[pos].
 *
 * @return matching position or -1 if not found.
 */
long compare_at(char *text, long text_len, char *data, long data_len, long pos) {
	if (pos >= data_len) {
		return 1;
	}
	if (pos + text_len >= data_len) {
		text_len = data_len - pos;
	}
	if (text_len > 0) {
		int cmp = memcmp(text, &data[pos], text_len);
		if (cmp == 0) {
			// Strings seem to be equal, compare real length ...
			long start_pos = pos;
			int c;
			while (pos < data_len && (c = data[pos]) != _MUCBS_LINE_DELIMITER && c != _MUCBS_COLUMN_DELIMITER) {
				pos++;
			}
			if (pos - start_pos > text_len) {
				return -1;
			}
		}
		return cmp;
	} else {
		return 1;
	}
}

/**
 * Main search method. Perform a binary search for text in data.
 *
 * @return matching position or -1 if not found.
 */
long binary_search(char *text, long text_len, char *data, long data_len) {
	long first = 0;
	long last = data_len;
	while (first <= last) {
		long mid = (first + last) / 2;
		if (mid > 0) {
			// Read to the next line ...
			while (mid < data_len && data[mid] != _MUCBS_LINE_DELIMITER) {
				mid++;
			}
			mid++;
		}
		long cmp = compare_at(text, text_len, data, data_len, mid);
		if (cmp > 0) {
			first = (first + last) / 2 + 1;
		} else if (cmp < 0) {
			last = (first + last) / 2 - 1;
		} else {
			return mid;
		}
	}
	return -1;
}

/**
 * UDF init for csv_find.
 */
DLLEXP my_bool csv_find_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	my_bool result = csv_init(initid, args, message, 2);
	args->arg_type[0] = STRING_RESULT;
	args->arg_type[1] = STRING_RESULT;
	return result;
}

/**
 * UDF csv_find.
 *
 * @return 1 if found, 0 otherwise.
 */
DLLEXP long csv_find(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
	try {
		if (args->args[1] == NULL) {
			return 0;
		}
		struct csv_data *data = (struct csv_data *) initid->ptr;
		if (binary_search(args->args[1], args->lengths[1], data->buf, data->len) >= 0) {
			return 1;
		} else {
			return 0;
		}
	} catch (...) {
		strcpy(error, ERR_OTHER);
		return 1;
	}
}

/**
 * UDF deinit for csv_find.
 */
DLLEXP void csv_find_deinit(UDF_INIT *initid) {
	try {
		struct csv_data *data = (struct csv_data *) initid->ptr;
		if (data->buf) {
			munmap(data->buf, data->len);
		}
		if (data->fd) {
			close(data->fd);
		}
		free(initid->ptr);
	} catch (...) {}
}

/**
 * UDF init for csv_get.
 */
DLLEXP my_bool csv_get_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	my_bool result = csv_init(initid, args, message, 3);
	args->arg_type[0] = STRING_RESULT;
	args->arg_type[1] = STRING_RESULT;
	args->arg_type[2] = STRING_RESULT;
	return result;
}

/**
 * UDF csv_get.
 *
 * @return column value searched for or NULL if not found or any of the arguments is NULL.
 */
DLLEXP char *csv_get(UDF_INIT *initid, UDF_ARGS *args, char *result, long *length, char *is_null, char *error) {
	try {
		if (args->args[1] == NULL || args->args[2] == NULL) {
			return NULL;
		}
		struct csv_data *data = (struct csv_data *) initid->ptr;
		long pos = binary_search(args->args[1], args->lengths[1], data->buf, data->len);
		if (pos >= 0) {
			// Row found, now search for the right column ...
			int dest_col = atoi(args->args[2]);
			int col = 0;
			int c;
			while (col != dest_col && pos < data->len && (c = data->buf[pos]) != _MUCBS_LINE_DELIMITER) {
				if (c == _MUCBS_COLUMN_DELIMITER) {
					col++;
				}
				pos++;
			}
			if (col == dest_col) {
				// Column found ...
				result = data->buf + pos;
				long start_pos = pos;
				while (pos < data->len && (c = data->buf[pos]) != _MUCBS_LINE_DELIMITER && c != _MUCBS_COLUMN_DELIMITER) {
					pos++;
				}
				*length = pos - start_pos;
				return result;
			} else {
				return NULL;
			}
		} else {
			return NULL;
		}
	} catch (...) {
		strcpy(error, ERR_OTHER);
		return NULL;
	}
}

/**
 * UDF deinit for csv_get.
 */
DLLEXP void csv_get_deinit(UDF_INIT *initid) {
	csv_find_deinit(initid);
}

}
