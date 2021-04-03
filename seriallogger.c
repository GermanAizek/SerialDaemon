/* seriallogger.c:  A serial device logging utility.
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

#include "config.h"
#include "seriallogger.h"
#include "util.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#else
#include <types.h>
#endif // HAVE_SYS_TYPES_H

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#else
#include <stat.h>
#endif // HAVE_SYS_STAT_H

#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <glob.h>
#include <syslog.h>
#include <signal.h>



char *package_string = PACKAGE_STRING;

int pid_file; /* File descriptor for pid file. */
char *pid_file_path = "/var/run/seriallogger.pid";

/* Definition of long options for program arguments to
 * be passed to getopt_long. */
struct option long_options[] = {
	{"verbose",      no_argument,       0, 'v'},
	{"device",       required_argument, 0, 'd'},
	{"baudrate",     required_argument, 0, 'b'},
	{"parity",       required_argument, 0, 'p'},
	{"data",         required_argument, 0, 't'},
	{"stop",         required_argument, 0, 's'},
	{"buffer",       required_argument, 0, 'u'},
	{"log",          required_argument, 0, 'l'},
	{"max_log_size", required_argument, 0, 'm'},
	{"archive",      required_argument, 0, 'a'},
	{"max_logs",     required_argument, 0, 'n'},
	{"pid_file",     required_argument, 0, 'i'},
	{"help",         no_argument,       0, 'h'},
	{0,              0,                 0, 0}};



/* help
 *
 * Prints help message.
 */
void help(void) {
	printf("%s - Copyright (C) 2008 Heath Caldwell <hncaldwell@csupomona.edu>\n\n", package_string);
	printf("Options (defaults in parentheses):\n");
	printf("\t-d <device> --device=<device>          Device file for serial port (/dev/ttyS0)\n");
	printf("\t-b <speed>, --baudrate=<speed>          Bits per second to use for the serial connection (9600)\n");
	printf("\t-p <parity>, --parity=<parity>          Parity:  'e' for even, 'o' for odd, 'n' for none (n)\n");
	printf("\t-t <bits>, --data=<bits>                Number of data bits per character:  5, 6, 7 or 8 (8)\n");
	printf("\t-s <times>, --stop=<times>              Number of stop bit-times:  1 or 2 (1)\n");
	printf("\t-u <chars>, --buffer=<chars>            Max number of characters to read at a time (256)\n");
	printf("\t-l <path>, --log=<path>                 Location of log file (/var/log/seriallogger.log)\n");
	printf("\t-m <bytes>, --max_log_size=<bytes>      Size log file should be before rotation, in bytes (409600)\n");
	printf("\t-a <directory>, --archive=<directory>   Location to store archived logs (/var/log/archive)\n");
	printf("\t-n <num>, --max_logs=<num>              Maximum umber of rotated logs to keep archived (2)\n");
	printf("\t-i <path>, --pid_file=<path>            Location of pid file (/var/run/seriallogger.pid)\n");
	printf("\t-h, --help                              Print this message\n");
}



int main(int argc, char **argv)
{
	char *device = "/dev/ttyS0";
	unsigned int baudrate = 9600;
	char parity = 'n';
	unsigned int data = 8;
	unsigned int stop = 1;
	char *log_path = "/var/log/seriallogger.log";
	char *log_archive_dir = "/var/log/archive";
	unsigned int max_log_size = 409600;
	unsigned int max_logs = 2;
	unsigned int buffer_size = 256;

	int serial; /* File descriptor for the serial device. */
	int log;    /* File descriptor for the current log file. */
	char *buffer;
	char *new_line; /* To point to the first newline in buffer. */
	int log_size;
	int readchars, writtenchars;

	int fork_result;
	char *pid_string;
	int i;
	int c, option_index;
	opterr = 0;
	while((c = getopt_long(argc, argv, "d:b:p:t:s:u:l:m:a:n:i:h", long_options, &option_index)) != -1) {
		switch(c) {
			case 'd': device = optarg; break;
			case 'b': baudrate = atoi(optarg); break;
			case 'p': parity = optarg[0]; break;
			case 't': data = atoi(optarg); break;
			case 's': stop = atoi(optarg); break;
			case 'u': buffer_size = atoi(optarg); break;
			case 'l': log_path = optarg; break;
			case 'm': max_log_size = atoi(optarg); break;
			case 'a': log_archive_dir = optarg; break;
			case 'n': max_logs = atoi(optarg); break;
			case 'i': pid_file_path = optarg; break;
			case 'h': help(); return 0;
			case '?': /* Fall through. */
			default:
				help(); return 1;
		}
	}

	fork_result = fork();
	if(fork_result < 0) {
		fprintf(stderr, "Error forking daemon.\n");
		return 1;
	} else if(fork_result > 0) {
		/* Exit parent. */
		return 0;
	}

	setsid();

	/* Open and lock pid file to make sure that only one instance is running. */
	pid_file = open(pid_file_path, O_RDWR | O_CREAT, 0640);
	if(pid_file < 0) {
		fprintf(stderr, "Error opening pid file (%s).\n", pid_file_path);
		return 1;
	}
	if(lockf(pid_file, F_TLOCK, 0) < 0) {
		fprintf(stderr, "Error obtaining lock on pid file (%s).  Already running?\n", pid_file_path);
		return 1;
	}

	/* Write this process' pid to file. */
	pid_string = calloc(digits(getpid(), 10)+2, sizeof(char));
	sprintf(pid_string, "%d\n", getpid());
	write(pid_file, pid_string, strlen(pid_string));
	free(pid_string);

	/* Close all file descriptors (except pid_file), including stdout, etc. */
	for(i=getdtablesize(); i>=0; i--) {
		if(i != pid_file) close(i);
	}
	/* Open stdin, stdout, strerr to /dev/null. */
	i = open("/dev/null", O_RDWR);
	dup(i);
	dup(i);

	atexit(cleanup);

	openlog("seriallogger", LOG_PID, LOG_DAEMON);

	syslog(
			LOG_INFO, 
			"Starting with settings: d:%s, b:%d, p:%c, t:%d, s:%d, u:%d, l:%s, a:%s, m:%d, n:%d, i:%s",
			device, baudrate, parity, data, stop, buffer_size,
			log_path, log_archive_dir, max_log_size, max_logs, pid_file_path
	);

	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);

	serial = setup_serial(device, baudrate, parity, data, stop, buffer_size);

	/* Open the log file. */
	log = rotate_log(0, log_path, 0, 0);

	buffer = calloc(sizeof(char), buffer_size);
	while(1) {
		readchars = read(serial, buffer, buffer_size);

		if(readchars < 0) {
			syslog(LOG_ERR, "Error reading serial device.");
			free(buffer);
			exit(1);
		}

		if(readchars > 0) {
			log_size = get_log_size(log);
			if(log_size + readchars > max_log_size) {
				new_line = 0;
				for(i=0; i<readchars; i++) {
					if(buffer[i] == '\n') {
						new_line = buffer+i;
						break;
					}
				}

				if(new_line) {
					/* Finish the line and then rotate the log. */
					writtenchars = write(log, buffer, new_line - buffer + 1);
					if(writtenchars < 0) {
						syslog(LOG_ERR, "Failed to write to log.");
						exit(1);
					}

					log = rotate_log(log, log_path, log_archive_dir, max_logs);

					/* Set buffer and readchars to what is left. */
					readchars -= new_line - buffer + 1;
					for(i=0; i < readchars; i++)
						buffer[i] = new_line[i+1];
				} else if(log_size > max_log_size + 2*buffer_size) {
					log = rotate_log(log, log_path, log_archive_dir, max_logs);
				}
			}

			writtenchars = write(log, buffer, readchars);
			if(writtenchars < 0) {
				syslog(LOG_ERR, "Failed to write to log.");
				exit(1);
			}
		}
	}
	free(buffer);

	close(log);
	close(serial);

	exit(0);
}

/* signal_handler
 *
 * Function to handle received signals.
 *
 * Arguments:  signal:  Received signal
 */
void
signal_handler(int signal)
{
	switch(signal) {
		case SIGHUP: return;
		case SIGTERM: exit(0);
	}
}

/* cleanup
 *
 * Cleans up stuff at exit.
 */
void
cleanup(void)
{
	syslog(LOG_INFO, "Exiting.");
	close(pid_file);
	unlink(pid_file_path);
	closelog();
}

/* rotate_log
 *
 * Rotates the archived logs, moves the current log into the archive, and
 * creates a new log file.
 *
 * Arguments:   log:             File descriptor for the current log file
 *              log_path:        Path to log file
 *              lag_archive_dir: Directory to place archived logs into,
 *                               set NULL to just create the log file
 *              max_logs:        Maximum number of archived logs to keep
 * Returns:  File descriptor for new log file
 */
int
rotate_log(int log, const char *log_path, const char *log_archive_dir, int max_logs)
{
	int i;
	int new_arclog_file;
	char *new_arclog_name;
	char *temp;
	char *cwd = 0;
	glob_t globbuf;
	char *arcglob;

	if(log_archive_dir) {
		if(!(cwd = getcwd(cwd, 0))) {
			syslog(LOG_ERR, "Failed to get current working directory.");
			exit(1);
		}

		if(chdir(log_archive_dir) < 0) {
			syslog(LOG_ERR, "Failed to set current working directory.");
			exit(1);
		}

		arcglob = calloc(strlen(log_basename(log_path)) + 3, sizeof(char));
		sprintf(arcglob, "%s-*", log_basename(log_path));
		glob(arcglob, 0, NULL, &globbuf);

		if(globbuf.gl_pathc < max_logs) {
			new_arclog_name = calloc(strlen(log_basename(log_path)) + 2 + digits(max_logs, 10), sizeof(char));
			temp = calloc(7 + digits(digits(max_logs, 10), 10), sizeof(char));
			sprintf(temp, "%%s-%%0%dd", digits(max_logs, 10));
			sprintf(new_arclog_name, temp, log_basename(log_path), globbuf.gl_pathc);
			free(temp);

			new_arclog_file = open(new_arclog_name, O_WRONLY | O_CREAT, 0664);

			if(new_arclog_file < 0) {
				syslog(LOG_ERR, "Failed to create archived log file.");
				exit(1);
			}

			close(new_arclog_file);
			free(new_arclog_name);

			/* glob again now that the new file is created. */
			glob(arcglob, 0, NULL, &globbuf);
		}

		/* Rotate archived logs. */
		for(i=globbuf.gl_pathc-2; i>=0; i--) {
			if(rename(globbuf.gl_pathv[i], globbuf.gl_pathv[i+1]) < 0) {
				syslog(LOG_ERR, "Failed to move archived log file (%s to %s).", globbuf.gl_pathv[i], globbuf.gl_pathv[i+1]);
				exit(1);
			}
		}

		if(close(log) < 0) {
			syslog(LOG_ERR, "Failed to close log file.");
			exit(1);
		}

		/* Move log file to the archive at the zeroth place. */
		if(move_file(log_path, globbuf.gl_pathv[0]) < 0) {
			syslog(LOG_ERR, "Failed to move log file (%s to %s).", globbuf.gl_pathv[i], globbuf.gl_pathv[i+1]);
			exit(1);
		}

		globfree(&globbuf);
		free(arcglob);

		if(chdir(cwd) < 0) {
			syslog(LOG_ERR, "Failed to set current working directory.");
			exit(1);
		}

		free(cwd);
	}

	/* Create the new log. */
	log = open(
		log_path,
		O_WRONLY | O_CREAT | O_APPEND,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if(log < 0) {
		syslog(LOG_ERR, "Failed to open log file.");
		exit(1);
	}

	return log;
}

/* get_log_size
 *
 * Gets the size of the log file, in bytes.
 *
 * Arguments:   log:  File descriptor for the current log file
 * Returns:  The size of the file, in bytes.
 *           Returns 0 on error.
 */
int
get_log_size(int log)
{
	struct stat log_stat;

	if(fstat(log, &log_stat)) {
		syslog(LOG_WARNING, "Error getting log file status.");
		return 0;
	}

	return log_stat.st_size;
}

/* setup_serial
 *
 * Sets up the serial connection.
 *
 * Arguments:   device:      Path to device file for the serial port
 *              baudrate:    Baud rate to use for the connection (bits per second as an integer number)
 *              parity:      Use even ('e'), odd ('o'), or no ('n') parity bit
 *              data:        The number of data bits per character (5, 6, 7 or 8)
 *              stop:        The number of stop bit-times (1 or 2)
 *              buffer_size: Size of the buffer that will be used reads from the connection
 * Returns:  File descriptor for the serial connection
 */
int
setup_serial(
		const char *device,
		unsigned int baudrate,
		char parity,
		unsigned int data,
		unsigned int stop,
		unsigned int buffer_size)
{
	int serial; /* File descriptor for the serial device. */
	struct termios set_termios, real_termios;

	serial = open(device, O_RDONLY | O_NOCTTY);
	if(serial < 0) {
		syslog(LOG_ERR, "Error opening device '%s'.", device);
		exit(1);
	}

	tcgetattr(serial, &set_termios);

	set_termios.c_cflag = 0;
	set_termios.c_cflag |= CLOCAL | CREAD;

	// Set speed.
	cfsetispeed(&set_termios, baudbits(baudrate));
	cfsetospeed(&set_termios, baudbits(baudrate));

	// Set parity.
	if(parity == 'e') set_termios.c_cflag |= PARENB;
	if(parity == 'o') set_termios.c_cflag |= PARENB | PARODD;

	// Set data bits per character.
	set_termios.c_cflag |=
		(data == 5) ? CS5 :
		(data == 6) ? CS6 :
		(data == 7) ? CS7 : CS8;

	// Set stop bit-times.
	if(stop == 2) set_termios.c_cflag |= CSTOPB;

	set_termios.c_lflag = 0;

	memset(set_termios.c_cc, 0, sizeof(set_termios.c_cc));
	set_termios.c_cc[VTIME] = 10;  // Wait at most one second for data.
	set_termios.c_cc[VMIN] = buffer_size;

	set_termios.c_iflag = 0;
	if(parity == 'e' || parity == 'o') set_termios.c_cflag |= INPCK | ISTRIP;

	set_termios.c_oflag = 0;

	tcflush(serial, TCIFLUSH);
	tcsetattr(serial, TCSANOW, &set_termios);

	tcgetattr(serial, &real_termios);
	if(
		set_termios.c_cflag != real_termios.c_cflag ||
		set_termios.c_lflag != real_termios.c_lflag) {

		syslog(LOG_ERR, "Unable to set up serial device.");
		exit(1);
	}

	return serial;
}

/* baudbits
 *
 * Returns the bits to use in a call to cfsetispeed or cfsetospeed in order
 * to set the baud rate for the given rate.  If the given rate is not valid,
 * it is rounded down to the next valid rate.
 *
 * Arguments:   baudrate: Baud rate
 * Returns:  Bits to use in a call to cfsetispeed or cfsetospeed
 */
unsigned int
baudbits(unsigned int baudrate)
{
	if(baudrate >= 4000000) return B4000000;
	else if(baudrate >= 3500000) return B3500000;
	else if(baudrate >= 3000000) return B3000000;
	else if(baudrate >= 2500000) return B2500000;
	else if(baudrate >= 2000000) return B2000000;
	else if(baudrate >= 1500000) return B1500000;
	else if(baudrate >= 1152000) return B1152000;
	else if(baudrate >= 1000000) return B1000000;
	else if(baudrate >= 921600) return B921600;
	else if(baudrate >= 576000) return B576000;
	else if(baudrate >= 500000) return B500000;
	else if(baudrate >= 460800) return B460800;
	else if(baudrate >= 230400) return B230400;
	else if(baudrate >= 115200) return B115200;
	else if(baudrate >= 57600) return B57600;
	else if(baudrate >= 38400) return B38400;
	else if(baudrate >= 19200) return B19200;
	else if(baudrate >= 9600) return B9600;
	else if(baudrate >= 4800) return B4800;
	else if(baudrate >= 2400) return B2400;
	else if(baudrate >= 1800) return B1800;
	else if(baudrate >= 1200) return B1200;
	else if(baudrate >= 600) return B600;
	else if(baudrate >= 300) return B300;
	else if(baudrate >= 200) return B200;
	else if(baudrate >= 150) return B150;
	else if(baudrate >= 134) return B134;
	else if(baudrate >= 110) return B110;
	else if(baudrate >= 75) return B75;
	else if(baudrate >= 50) return B50;

	return 0;
}

/* log_basename
 *
 * Returns the base filename of the given path.
 *
 * Arguments:   path:  File path
 * Returns:  Pointer within path where the basename starts
 */
const char *
log_basename(const char *path)
{
	int i;

	if(!path) return 0;

	for(i=strlen(path)-1; i>=0; i--) {
		if(path[i] == '/') return (path+i+1);
	}

	return path;
}

