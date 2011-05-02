/*
	Copyright (C) 2010 and later optivo GmbH

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
*/
#ifndef MYSQL_UDF_CSV_BINARY_SEARCH_H
#define MYSQL_UDF_CSV_BINARY_SEARCH_H

#if defined(_WIN32)
#define DLLEXP __declspec(dllexport)
#else
#define DLLEXP
#endif

extern "C" {

DLLEXP my_bool csv_find_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
DLLEXP long csv_find(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
DLLEXP void csv_find_deinit(UDF_INIT *initid);

DLLEXP my_bool csv_get_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
DLLEXP char* csv_get(UDF_INIT *initid, UDF_ARGS *args, char *result, long *length, char *is_null, char *error);
DLLEXP void csv_get_deinit(UDF_INIT *initid);

}

#endif
