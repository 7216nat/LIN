#include <linux/errno.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <stdio.h>
#define __NR_LEDCTL 332
long ledctl(unsigned int leds) {
	return (long) syscall(__NR_LEDCTL, leds);
}

int main(int argc, char *argv[]) {
	if (argc != 2){
		perror("Usage: %s <0xNUM>\n", argv[0]);
		return -EXIT_FAILURE;
	}
	unsigned int leds;
	if (sscanf(argv[1], "0x%x", leds) == 1){
		if (ledctl(leds) < 0){
			perror("ledctl syscall error.\n");
			return -EXIT_FAILURE;
		}
	} else { 
		perror("Usage: %s <0xNUM>\n", argv[0]);
		return -EINVAL;
	}
	return 0;
}
