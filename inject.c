// Petit programme pour court-circuiter les appels aux fonctions de la libc

#define _GNU_SOURCE

#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef pid_t (*real_fork_t)(void);
typedef pid_t (*real_wait_t)(int *);
typedef pid_t (*real_waitpid_t)(pid_t, int *, int);
typedef ssize_t (*real_read_t)(int, void *, size_t);
typedef ssize_t (*real_write_t)(int, const void *, size_t);
typedef size_t (*real_fread_t)(void *, size_t, size_t, FILE*);
typedef size_t (*real_fwrite_t)(const void *, size_t, size_t, FILE*);
typedef ssize_t (*real_copy_file_range_t)(int fd_in, off_t *off_in, int fd_out, off_t *off_out, size_t len, unsigned int flags);

FILE* trololo(void) {
       static FILE *lolo = NULL;
       if(!lolo) lolo = fdopen(3, "w");
       if(!lolo) {
	       perror("lolo");
	       exit(1);
       }
       return lolo;
}

int trace_all() {
	static int z=0;
	static int traceall=0;
	if (!z++)
		traceall = getenv("TRACEALL") != NULL;
	return traceall;
}

pid_t fork(void) {
	static int f=0;
	if (!f++ || trace_all()) {
		char msg[] = "fork()\n";
		((real_fwrite_t)dlsym(RTLD_NEXT, "fwrite"))(msg, sizeof(msg), 1, trololo());
		fflush(trololo());
	}
	return ((real_fork_t)dlsym(RTLD_NEXT, "fork"))();
}

pid_t wait(int *status) {
	static int w=0;
	if (!w++ || trace_all())
		fprintf(trololo(), "wait()\n");
	fflush(trololo());
	return ((real_wait_t)dlsym(RTLD_NEXT, "wait"))(status);
}

pid_t waitpid(pid_t pid, int *status, int options) {
	static int w=0;
	if (!w++ || trace_all())
		fprintf(trololo(), "waitpid()\n");
	fflush(trololo());
	return ((real_waitpid_t)dlsym(RTLD_NEXT, "waitpid"))(pid, status, options);
}

ssize_t read(int fd, void *data, size_t size) {
	static int x=0;
	if (!x++ || trace_all())
		fprintf(trololo(), "read(%ld)\n", size);
	fflush(trololo());
	return ((real_read_t)dlsym(RTLD_NEXT, "read"))(fd, data, size);
}
ssize_t write(int fd, const void *data, size_t size) {
	static int y=0;
	if (!y++ || trace_all())
		fprintf(trololo(), "write(%ld)\n", size);
	fflush(trololo());
	return ((real_write_t)dlsym(RTLD_NEXT, "write"))(fd, data, size);
}
size_t fread(void *data, size_t size, size_t n, FILE *f) {
	static int x=0;
	if (!x++)
		fprintf(trololo(), "fread(%ld)\n", size*n);
	return ((real_fread_t)dlsym(RTLD_NEXT, "fread"))(data, size, n, f);
}
size_t fwrite(const void *data, size_t size, size_t n, FILE *f) {
	static int x=0;
	if (!x++)
		fprintf(trololo(), "fwrite(%ld)\n", size*n);
	return ((real_fwrite_t)dlsym(RTLD_NEXT, "fwrite"))(data, size, n, f);
}

