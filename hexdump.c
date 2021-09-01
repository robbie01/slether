#include <stdio.h>

void hexdump(const void *buf, size_t n) {
	FILE *hexdump = popen("hexdump -C", "w");
	fwrite(buf, sizeof(char), n, hexdump);
	pclose(hexdump);
}
