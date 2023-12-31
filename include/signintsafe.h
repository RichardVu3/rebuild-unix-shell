#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h> 

static void sio_reverse(char s[]);

static void sio_ltoa(long v, char s[], int b) ;

static size_t sio_strlen(char s[]);

ssize_t sio_puts(char s[]);

ssize_t sio_putl(long v);

void sio_error(char s[]);

ssize_t Sio_putl(long v);

ssize_t Sio_puts(char s[]);

void Sio_error(char s[]);