/*
 * header file to be used by applications.
 */

int printu(const char *s, ...);
int exit(int code);
void* naive_malloc();
void naive_free(void* va);
int fork();
void yield();
void wait(int pid);
int sem_new(int value);
void sem_V(int sem);
void sem_P(int sem);