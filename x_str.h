
#ifndef X_STR_H_
#define X_STR_H_

/*	User should ensure, that "from" string ends with '\0' character. */

void x_sprinft_prog(char * where, const char * from);

uint8_t x_strlen(char * str);

uint8_t x_strlen_prog(const char * str);

void x_memcpy(char * to, char * fr, uint8_t bytes_nr);

void x_memmove(char * to, char * from, uint8_t bytes_wr);

char * x_strchr(char * where, char what);

void x_strcat(char * to, const char * from);

void x_strcpy(char * to, const char * from);

#endif /* X_STR_H_ */
