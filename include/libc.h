/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */

#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>
#include <semaphore.h>

int waitKey(char *buffer, int timeout);

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

int gotoXY(int x, int y);

int changeColor(int fg, int bg);

int clrscr(char *b);

int get_stats(int pid, struct stats *st);

int threadCreateWithStack(void (*function)(void *arg), int N, void *parameter);

char *memRegGet(int num_pages);

int memRegDel(char *m);

sem_t* semCreate(int initial_value);

int semWait(sem_t* s);

int semSignal(sem_t* s);

int semDestroy(sem_t* s);

#endif /* __LIBC_H__ */
