/*
 * path.c - path manipulation
 *
 * Copyright (c) 2017 Huawei.  All Rights Reserved.
 * Author: zhangyi (F) <yi.zhang@huawei.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <string.h>


/*
 * Join a null-terminated base directory path and a subdirectory path or
 * filename into a pathname string.
 *
 * This function remove the duplicate '/' if base directory path end up
 * with '/' and subpath start with another '/', and fill up the missing
 * '/' otherwise. If either input path is the string "." or an empty
 * string, returns the other one; if both input paths are empty string,
 * then returns the string ".".
 *
 * This function not parse the string "./", "../" and duplicate '/' in
 * the middle of input paths, and also not handle the case of NULL input,
 * so need to make sure input correctly.
 *
 * The following list of examples shows the strings returned:
 * 	path	name	join
 * 	/usr	lib	/usr/lib
 * 	/usr	.	/usr
 * 	/usr	..	/usr/..
 * 	.	lib	lib
 * 	..	lib	../lib
 * 	.       .       .
 * 	..      ..      ../..
 * 	...
 *
 * Note: This function will alloc memory, so should free the return value
 *       after use.
 *
 * @path: base directory
 * @name: subdirectory path or file name
 */
char *joinname(const char *path, const char *name)
{
	int lenp = strlen(path);
	int lenn = strlen(name);
	int len;
	int slash = 0;
	char *str;

	if (path[0] == '.' && lenp == 1) {
		path++;
		lenp = 0;
	}
	if (name[0] == '.' && lenn == 1) {
		name++;
		lenn = 0;
	}

	for (; name[0] == '.' && name[1] == '/'; name+=2, lenn-=2);
	for (; path[0] == '.' && path[1] == '/'; path+=2, lenp-=2);
	for (; name[0] == '/'; name++, lenn--);

	len = lenp + lenn + 1;
	if (lenp == 0 && lenn == 0)
		len++;
	else if ((lenp > 0) && (lenn > 0) &&
		 (path[lenp-1] != '/')) {
		slash = 1;
		len++;
	}

	str = malloc(len);
	if (!str)
		goto out;

	if (lenp == 0 && lenn == 0) {
		str[0] = '.';
	} else {
		memcpy(str, path, lenp);
		if (slash) {
			str[lenp] = '/';
			lenp++;
		}
		memcpy(str+lenp, name, lenn);
	}
	str[len-1] = '\0';
out:
	return str;
}

/*
 * Break a null-terminated pathname string into filename or subdirectory
 * path from the specified base directory.
 *
 * If base directory path not found in pathname, returns the origin path;
 * if base directory equal to origin path, returns the string "."; if
 * either the base directory or the origin path is the string ".." or ".",
 * returns the origin path.
 *
 * This function similar to the GNU version of basename, it never modify
 * input strings and do not handle the case of NULL input, so user should
 * make sure input correctly.
 *
 * This function not parse the string "./", "../" and duplicate '/' in the
 * middle of input paths.
 *
 * The following list of examples shows the strings returned:
 * 	path       dir    basename2
 *	/usr/lib   /      /usr/lib
 *	/usr/lib   /usr   lib
 *	/usr       /usr   .
 *	/	   *      /
 *	.          *      .
 *	..         *      ..
 *
 * Note: This function do not alloc memory, so free the return pointer is
 * not necessary and not allowed.
 *
 * @path: pathname need to break
 * @dir: base directory path
 */
char *basename2(const char *path, const char *dir)
{
	static const char dot[] = ".";
	int lend = strlen(dir);
	const char *str;

	for (; path[0] == '.' && path[1] == '/'; path+=2);
	for (; dir[0] == '.' && dir[1] == '/'; dir+=2, lend-=2);

	if (dir[0] == '.' && lend == 1) {
		dir++;
		lend = 0;
	}

	if (lend > 0 && (dir[lend-1] == '/'))
		lend--;

	if (lend > 0 && !strncmp(path, dir, lend)) {
		str = path + lend;
		if (str[0] != '/' && str[0] != '\0')
			goto mismatch;
		for (; *str == '/'; str++);
		if (str[0] == '\0')
			str = dot;
		return (char *)str;
	}
mismatch:
	return (path[0] == '\0') ? (char *)dot : (char *)path;
}
