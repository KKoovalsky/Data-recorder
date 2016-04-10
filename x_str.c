#include "general.h"
#include "x_str.h"

void x_sprinft_prog(char * to, const char * from) {
	char byte;
	while((byte = pgm_read_byte(from++))) {
		*to = byte;
		to++;
	}
	*to = '\0';
}

uint8_t x_strlen(char * str) {
	uint8_t size = 0;
	while(*str++)
		size++;
	return size;
}

uint8_t x_strlen_prog(const char * str) {
	uint8_t size = 0;
	while(pgm_read_byte(str++))
		size++;
	return size;
}

void x_memcpy(char * to, char * fr, uint8_t bytes_nr) {
	for(uint8_t i = 0; i < bytes_nr; i++) {
		*to = *fr++;
		to++;
	}
}

void x_memmove(char * to, char * from, uint8_t bytes_wr) {
	char * temp = (char *) malloc (sizeof(char) * bytes_wr);
	for(uint8_t i = 0; i < bytes_wr; i ++)
		temp[i] = *from++;
	for(uint8_t i = 0; i < bytes_wr; i ++) {
		*to = temp[i];
		to++;
	}
	free(temp);
}

char * x_strchr(char * where, char what) {
	char * temp = where;
	while(*temp) {
		if(*temp == what) return temp;
		temp++;
	}
	return NULL;
}

void x_strcat(char * to, const char * from) {
	while(*to++) {}
	to -= 1;
	while(*from) {
		*to = *from++;
		to++;
	}
	*to = '\0';
}

void x_strcpy(char * to, const char * from) {
	while(*from) {
		*to = *from++;
		to++;
	}
	*to = '\0';
}

