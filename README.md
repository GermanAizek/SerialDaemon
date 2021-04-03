# SerialDaemon

Serial Daemon is a simple utility to log data that comes in on a serial port. It runs as a daemon that opens a serial device and reads whatever data comes in, writing that data to a log file. It will also rotate the log into an archive directory.

    Options (defaults in parentheses):
        -d <device>, --device=<device>          Device file for serial port (/dev/ttyS0)
        -b <speed>, --baudrate=<speed>          Bits per second to use for the serial connection (9600)
        -p <parity>, --parity=<parity>          Parity:  'e' for even, 'o' for odd, 'n' for none (n)
        -t <bits>, --data=<bits>                Number of data bits per character:  5, 6, 7 or 8 (8)
        -s <times>, --stop=<times>              Number of stop bit-times:  1 or 2 (1)
        -u <chars>, --buffer=<chars>            Max number of characters to read at a time (256)
        -l <path>, --log=<path>                 Location of log file (/var/log/seriallogger.log)
        -m <bytes>, --max_log_size=<bytes>      Size log file should be before rotation, in bytes (409600)
        -a <directory>, --archive=<directory>   Location to store archived logs (/var/log/archive)
        -n <num>, --max_logs=<num>              Maximum umber of rotated logs to keep archived (2)
	    -i <path>, --pid_file=<path>            Location of pid file (/var/run/seriallogger.pid)
        -h, --help                              Print this message

This program was originally written to run on the service processor of Sun hardware which lacks a console history. It should be useful, however, for logging any kind of data that comes in through a serial port.

# Running on an SP
## Disclaimer

Please be very careful. These instructions are intended be helpful, but there is the risk of rendering the SP inoperoperable if something goes wrong. Heed the warning that is printed when you log in with the sunservice account seriously.

## x4100 (ppc SP)

If you intend to run this on the SP of a Sun x4100 or similar and want to have it log the machine's console messages to /dev/shm/seriallogger.log, rotating the log in /dev/shm once it reaches 1048576 bytes, keeping only the 4 most recent archived logs, and use /var/lock/LCK..ttyS1 as its pid file (this will cause the SP to think that the serial console is already in use if you try something like "start SP/console"), you can do something like this (you should place seriallogger in /conf so that it will stay there across boots):

```
/conf/seriallogger -d /dev/ttyS1 -l /dev/shm/seriallogger.log -a /dev/shm -m 1048576 -n 4 -i /var/lock/LCK..ttyS1
```

In order to have it start up when the SP boots, you might also want to add something like the following to /conf/crontab. You should probably run it at least once first before rebooting, though, just to check that it works and gain some confidence that it won't break startup (if that happens, your SP will likely no longer be usable).

```
@reboot root /conf/seriallogger -d /dev/ttyS1 -l /dev/shm/seriallogger.log -a /dev/shm -m 1048576 -n 4 -i /var/lock/LCK..ttyS1
```

## x4150 (arm SP)

If you intend to run this on the SP of a Sun x4150, you can run seriallogger with something like this:

```
/conf/seriallogger -d /dev/ttyS0 -l /dev/shm/seriallogger.log -a /dev/shm -m 1048576 -n 4 -i /var/lock/LCK..ttyS0
```

To make it start at boot, you can add a line like the following to /conf/interfaces (this is probably not the best way to do this, but it is the only thing that we could think of). You should probably run it at least once first before rebooting, though, just to check that it works and gain some confidence that it won't break startup (if that happens, your SP will likely no longer be usable).

```
up /conf/seriallogger -d /dev/ttyS0 -l /dev/shm/seriallogger.log -a /dev/shm -m 1048576 -n 4 -i /var/lock/LCK..ttyS0
```

The latest firmware for the x4150 SP disables the sunservice account. Here is how we re-enabled it:

1. Downgrade to the previous firmware version where the sunservice account worked (if it has already been upgraded).
2. Log in with the sunservice account and edit /conf/interfaces, adding the following line to the end:
3. ```up /usr/local/bin/sunserviceacct enable```
4. Upgrade to the latest firmware, making sure to retain the configuration.
5. Once it has rebooted from the upgrade, reset the SP again.

You should then be able to log in with the sunservice account.

# Building for other architectures

crosstool is an excellent tool for building cross-toolchains that can be used to cross-compile things (like seriallogger).


Copyright © 2009 Heath Caldwell <hncaldwell@csupomona.edu> 
