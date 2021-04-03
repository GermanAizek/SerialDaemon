/* util.c:  Utility functions.
 *
 * This file is part of seriallogger.
 *
 * Copyright (C) 2008 Heath Caldwell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License , or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * The Free Software Foundation, Inc.
 * 59 Temple Place, Suite 330
 * Boston, MA 02111 USA
 *
 * You may contact the author at:
 * Heath Caldwell <hncaldwell@csupomona.edu>
 */

#include "util.h"
#include "config.h"

#include <fcntl.h>
#include <string.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#else
#include <stat.h>
#endif // HAVE_SYS_STAT_H

#ifdef HAVE_MATH_H
#include <math.h>
#endif // HAVE_MATH_H



/* move_file
 *
 * Moves a file from one location to another.
 *
 * Arguments:  from: Path of file to move
 *             to:   Path to move file to
 * Returns:  0 on success,
 *           -1 on error
 */
int
move_file(const char *from, const char *to)
{
	int f, t;  /* File descriptors. */
	struct stat f_stat;
	char buffer[1024];
	int readchars;
	int writtenchars;

	f = open(from, O_RDONLY);
	if(f < 0) return -1;

	if(fstat(f, &f_stat) < 0) return -1;

	t = open(to, O_WRONLY | O_CREAT, f_stat.st_mode);
	if(t < 0) return -1;

	while((readchars = read(f, buffer, 1024))) {
		if(readchars < 0) return -1;

		writtenchars = write(t, buffer, readchars);
		if(writtenchars != readchars) return -1;
	}

	close(f);
	close(t);

	if(unlink(from) < 0) return -1;

	return 0;
}

/* digits
 *
 * Finds the number of digits of a given integer in a given base.
 *
 * Arguments:   n:    Integer to count digits of
 *              base: Base to count digits in
 * Returns:  Number of digits n has in base base.
 *           If n is negative, returns the number of digits of
 *           the absolute value of n plus 1 (for the minus sign).
 *           If base is not above 1, returns 0.
 */
int
digits(int n, int base)
{
	if(base <= 1) return 0;
	return n ? (floor(log(fabs(n)) / log(base) + 1) + (n < 0 ? 1 : 0)) : 1;
}

