/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */

#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

int write(int fd, const char *buffer, int size);

int gettime();

void itoa(int value, char *str, int base);

int strlen(const char *a);

char *strcpy(char *dst, const char *src);

char *strcat(char *dst, const char *src);

int getpid();

int fork();

void exit();

void perror();

#endif /* __LIBC_H__ */
