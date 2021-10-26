#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFAULT_TTY
#define DEFAULT_TTY "/dev/dty00"
#endif

#define END	0300
#define ESC	0333
#define ESC_END	0334
#define ESC_ESC	0335

#define BUFLEN	4096

void hexdump(const void *buf, size_t n);

int serial_init(const char* dev) {
	int sp;
	if (!dev)
		sp = open(DEFAULT_TTY, O_RDWR | O_NONBLOCK);
	else if (dev[0] == '/')
		sp = open(dev, O_RDWR | O_NONBLOCK);
	else {
		int devfd = open("/dev", O_RDONLY | O_DIRECTORY);
		sp = openat(devfd, dev, O_RDWR | O_NONBLOCK);
		close(devfd);
	}

	if (sp < 0) /* epic fail */
		err(EXIT_FAILURE, "serial_init: open");

	struct termios t;
	if (tcgetattr(sp, &t) < 0) /* functions as tty check too */
		err(EXIT_FAILURE, "serial_init: tcgetattr");

	cfmakeraw(&t);
	t.c_cflag |=   CS8 /* | CRTSCTS */; /* ensure 8 bits */
	t.c_cflag &= ~(CSTOPB | PARENB);    /* ensure N-1    */
	cfsetspeed(&t, B115200); /* whole lotta nonstandard
	                            (sorry your standards suck at serial) */

	if (tcsetattr(sp, TCSANOW, &t) < 0) /* edge case, probably */
		err(EXIT_FAILURE, "serial_init: tcsetattr");

	return sp;
}

void serial_proc(int sp, int tap) {
	static unsigned char readbuf[BUFLEN];
	static int i = 0;

	ssize_t n;
	while (i < BUFLEN && (n = read(sp, readbuf+i, BUFLEN-i)) > 0) i += n;

	if (n < 0 && errno != EAGAIN)
		err(EXIT_FAILURE, "serial_proc: read");

	int j = 0;
	while (readbuf[j] != END && j < (i-1)) ++j;
	if (readbuf[j] != END) { 
		if (j+1 >= BUFLEN) { /* buffer overflow */
			/* 4k should be enough to prevent this from happening
			   unless an END gets lost in transmission */
			fprintf(stderr, "serial_proc: !!WARNING!! BUFFER OVERFLOW\n");
			i = 0; /* toss it and let layers 3+ do their job */
		}
		return;
	}

	if (j == 0) {
		fprintf(stderr, "serial_proc: received empty frame on serial\n");
		goto movebuf; /* Bro This Is Evil! You Aren't
		                 Supposed To Do That! */
	}

	fprintf(stderr, "serial_proc: received frame on serial, %d bytes\n", j+1);

	unsigned char writebuf[BUFLEN];
	int k = 0;

	for (int r = 0; r < j; ++r) {
		if (readbuf[r] == ESC) {
			switch (readbuf[r+1]) {
			case ESC_ESC:
				writebuf[k++] = ESC;
				++r;
				break;
			case ESC_END:
				writebuf[k++] = END;
				++r;
				break;
			}
		} else writebuf[k++] = readbuf[r];
	}

	hexdump(writebuf, k);
	if (write(tap, writebuf, k) < 0)
		err(EXIT_FAILURE, "serial_proc: write");
	fprintf(stderr, "serial_proc: sent frame on TAP, %d bytes\n", k);

movebuf:
	memmove(readbuf, readbuf+j+1, BUFLEN-j-1);
	i -= j+1;
}

void tap_proc(int sp, int tap) {
	unsigned char readbuf[BUFLEN];
	ssize_t n = read(tap, readbuf, BUFLEN);
	fprintf(stderr, "tap_proc: received frame on TAP, %zd bytes\n", n);
	hexdump(readbuf, n);
	unsigned char writebuf[BUFLEN];
	int k = 0;
	writebuf[k++] = END; /* send initial end as per RFC1055 */
	for (int i = 0; i < n; ++i) {
		switch (readbuf[i]) {
		case ESC:
			writebuf[k++] = ESC;
			writebuf[k++] = ESC_ESC;
			break;
		case END:
			writebuf[k++] = ESC;
			writebuf[k++] = ESC_END;
			break;
		default:
			writebuf[k++] = readbuf[i];
			break;
		}
	}
	writebuf[k++] = END;
	if (write(sp, writebuf, k) < 0)
		err(EXIT_FAILURE, "tap_proc: write");
	fprintf(stderr, "tap_proc: sent frame on serial, %d bytes+empty\n", k-1);
}

int main(void) {
	int sp = serial_init(NULL);
	int tap = open("/dev/tap", O_RDWR | O_NONBLOCK); /* future tap_init() */
	int nfds = (sp > tap ? sp : tap) + 1;
	fd_set readfds, writefds, exceptfds;

	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(sp, &readfds);
		FD_SET(tap, &readfds);

		if (select(nfds, &readfds, &writefds, &exceptfds, NULL) < 0)
			err(EXIT_FAILURE, "select");

		if (FD_ISSET(sp, &readfds)) serial_proc(sp, tap);
		if (FD_ISSET(tap, &readfds)) tap_proc(sp, tap);
	}

	return 0;
}
