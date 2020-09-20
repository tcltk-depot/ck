/*
 * unistd.h --
 *
 *      Macros, CONSTants and prototypes for Posix conformance.
 *
 * Copyright 1989 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * @(#) unistd.h 1.5 94/12/17 16:26:27
 */

#ifndef _UNISTD
#define _UNISTD

#include <sys/types.h>
#ifndef _TCL
#   include "tcl.h"
#endif

#ifndef NULL
#define NULL    0
#endif

/*
 * Strict POSIX stuff goes here.  Extensions go down below, in the
 * ifndef _POSIX_SOURCE section.
 */

extern void _exit(int status);
extern int access(CONST char *path, int mode);
extern int chdir(CONST char *path);
extern int chown(CONST char *path, uid_t owner, gid_t group);
extern int close(int fd);
extern int dup(int oldfd);
extern int dup2(int oldfd, int newfd);
extern int execl(CONST char *path, ...);
extern int execle(CONST char *path, ...);
extern int execlp(CONST char *file, ...);
extern int execv(CONST char *path, char **argv);
extern int execve(CONST char *path, char **argv, char **envp);
extern int execvp(CONST char *file, char **argv);
extern pid_t fork(void);
extern char *getcwd(char *buf, size_t size);
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern int getgroups(int bufSize, int *buffer);
extern pid_t getpid(void);
extern uid_t getuid(void);
extern int isatty(int fd);
extern long lseek(int fd, long offset, int whence);
extern int pipe(int *fildes);
extern int read(int fd, char *buf, size_t size);
extern int setgid(gid_t group);
extern int setuid(uid_t user);
extern unsigned sleep(unsigned seconds);
extern char *ttyname(int fd);
extern int unlink(CONST char *path);
extern int write(int fd, CONST char *buf, size_t size);

#ifndef	_POSIX_SOURCE
extern char *crypt(CONST char *, CONST char *);
extern int fchown(int fd, uid_t owner, gid_t group);
extern int flock(int fd, int operation);
extern int ftruncate(int fd, unsigned long length);
extern int readlink(CONST char *path, char *buf, int bufsize);
extern int setegid(gid_t group);
extern int seteuid(uid_t user);
extern int setreuid(int ruid, int euid);
extern int symlink(CONST char *, CONST char *);
extern int ttyslot(void);
extern int truncate(CONST char *path, unsigned long length);
extern int vfork(void);
#endif /* _POSIX_SOURCE */

#endif /* _UNISTD */

