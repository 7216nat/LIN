#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define NUM_LEDS 8
#define LEN 88

char *off = "0x000000";
char *rojo = "0xFF0000";
char *amarillo = "0xFFFF00";
char *azul = "0x00FFFF";
char *verde = "0x00FF00";
const char *file = "/dev/usb/blinkstick0";

void navidad();
void clear();

int main(int argc, char *argv[]){
	int ret = 0;
	if (argc != 2){
		perror("error argumento.");
		exit(EXIT_FAILURE);
	}
	if (strcmp(argv[1], "navidad") == 0)
		navidad();
	else{
		printf("Usage: %s command.\n", argv[0]);
		printf("./%s navidad --> luces de navidad", argv[0]);
		return -1;
	}
	return ret;	
}


void format(char *c1, char *c2, char *c3, char *c4, char *c5, char *c6, char *c7, char *c8){
	char kbuf[LEN];
	int fd = open(file, O_WRONLY);
	if (fd < 0){
		perror("Error al abrir fichero");
		exit(EXIT_FAILURE);
	}
	sprintf(kbuf, "0:%s,1:%s,2:%s,3:%s,4:%s,5:%s,6:%s,7:%s", c1,c2,c3,c4,c5,c6,c7,c8);
	write(fd, kbuf, LEN);
	close(fd);
}


void clear(){
	int fd = open(file, O_WRONLY);
	if (fd < 0){
		perror("Error al abrir fichero");
		exit(EXIT_FAILURE);
	}
	write(fd, "", 0);
	close(fd);
}
void navidad(){
	int num = 20;
	for (int i = 0; i < num; i++){
		format(rojo, off, amarillo, off, rojo, off, amarillo, off);
		sleep(1);
		clear();
		sleep(1);
		format(off, verde, off, azul, off, verde, off, azul);
		sleep(1);
		clear();
		sleep(1);
	}
	
	clear();
}
