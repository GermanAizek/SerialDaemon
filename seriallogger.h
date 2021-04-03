/* seriallogger.h:  Function prototypes.
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

#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

unsigned int baudbits(unsigned int baudrate);

int setup_serial(
	const char *device,
	unsigned int baudrate, char parity, unsigned int data, unsigned int stop,
	unsigned int buffer_size);

int get_log_size(int log);

int rotate_log(int log, const char *log_path, const char *log_archive_dir, int max_logs);

const char *log_basename(const char *path);

void cleanup(void);

void signal_handler(int signal);

#endif // SERIALLOGGER_H
