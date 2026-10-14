/*
 * newlib's system interface for the RedHills Cafe build.
 *
 * The retail image links the Green Hills C library on Nintendo's Cafe port of libsys (reconstructed under
 * source/cafe/libsys) and coreinit's C-runtime services. The RedHills build links newlib and connects it
 * to the same layer:
 * - file descriptors go to the libsys calls (stdout and stderr reach OSConsoleWrite there);
 * - the heap is the game's dlmalloc (source/vendor/dlmalloc.cpp, which defines malloc and its family);
 * - locks are coreinit's __ghs_mtx mutexes, the ones the Green Hills library locks with, created on first
 *   use under coreinit's __ghsLock;
 * - _exit ends the process through coreinit's _Exit, as the retail C library does;
 * - each thread gets its own newlib state on first use. The Cafe crt0 registers the Green Hills
 *   runtime's per-thread hooks with coreinit, which calls them with the thread's runtime-state word when
 *   the thread starts and when it ends; that word holds the thread's state, which the end hook frees.
 */

#include <errno.h>
#include <malloc.h>
#include <reent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/lock.h>
#include <sys/stat.h>

/* source/cafe/libsys */
int write(int fno, const void *buf, int size);
int read(int fno, void *buf, int size);
int open(const char *filename, int flags, ...);
int close(int fno);
long lseek(int fno, long offset, int end);
int unlink(const char *name);
void *__ghs_GetThreadLocalStorageItem(int specifier);

/* coreinit */
void __ghsLock(void);
void __ghsUnlock(void);
void __ghs_mtx_init(_LOCK_T *mutex);
void __ghs_mtx_dst(_LOCK_T *mutex);
void __ghs_mtx_lock(_LOCK_T *mutex);
void __ghs_mtx_unlock(_LOCK_T *mutex);
int OSTryLockMutex(void *mutex);

/* The libsys port's per-thread item that holds the runtime-state word (__ghs_TLS___eh_globals). */
enum { THREAD_RUNTIME_STATE = 5 };

enum { STDIN_FILENO_CAFE = 0, STDERR_FILENO_CAFE = 2 };

static struct _reent **thread_state_word(void)
{
    return (struct _reent **)__ghs_GetThreadLocalStorageItem(THREAD_RUNTIME_STATE);
}

struct _reent *__getreent(void)
{
    struct _reent **word = thread_state_word();
    struct _reent *state = *word;

    if (state == NULL) {
        state = (struct _reent *)malloc(sizeof(struct _reent));
        if (state == NULL)
            return _GLOBAL_REENT;
        _REENT_INIT_PTR(state);
        *word = state;
    }
    return state;
}

void __ghs_cpp_exception_init(struct _reent **word)
{
    *word = NULL;
}

void __ghs_cpp_exception_cleanup(struct _reent **word)
{
    struct _reent *state = *word;

    if (state != NULL && state != _GLOBAL_REENT) {
        _reclaim_reent(state);
        free(state);
    }
    *word = NULL;
}

static void create_lock(_LOCK_T *lock)
{
    __ghsLock();
    if (*lock == __LOCK_INITIALIZER)
        __ghs_mtx_init(lock);
    __ghsUnlock();
}

void __libc_lock_acquire(_LOCK_T *lock)
{
    create_lock(lock);
    __ghs_mtx_lock(lock);
}

int __libc_lock_try_acquire(_LOCK_T *lock)
{
    create_lock(lock);
    return OSTryLockMutex((void *)*lock) ? 0 : 1;
}

void __libc_lock_release(_LOCK_T *lock)
{
    __ghs_mtx_unlock(lock);
}

void __libc_lock_close(_LOCK_T *lock)
{
    if (*lock != __LOCK_INITIALIZER)
        __ghs_mtx_dst(lock);
    *lock = __LOCK_INITIALIZER;
}

/* coreinit mutexes are recursive, so a recursive lock is the same mutex. */
void __libc_lock_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
    __libc_lock_acquire(&lock->lock);
}

int __libc_lock_try_acquire_recursive(_LOCK_RECURSIVE_T *lock)
{
    return __libc_lock_try_acquire(&lock->lock);
}

void __libc_lock_release_recursive(_LOCK_RECURSIVE_T *lock)
{
    __libc_lock_release(&lock->lock);
}

void __libc_lock_close_recursive(_LOCK_RECURSIVE_T *lock)
{
    __libc_lock_close(&lock->lock);
}

void __flockfile(FILE *fp)
{
    __libc_lock_acquire_recursive(&fp->_lock);
}

void __funlockfile(FILE *fp)
{
    __libc_lock_release_recursive(&fp->_lock);
}

void *_malloc_r(struct _reent *reent, size_t size)
{
    return malloc(size);
}

void _free_r(struct _reent *reent, void *block)
{
    free(block);
}

void *_calloc_r(struct _reent *reent, size_t count, size_t size)
{
    return calloc(count, size);
}

void *_realloc_r(struct _reent *reent, void *block, size_t size)
{
    return realloc(block, size);
}

void *_memalign_r(struct _reent *reent, size_t alignment, size_t size)
{
    return memalign(alignment, size);
}

_ssize_t _write_r(struct _reent *reent, int fd, const void *buf, size_t size)
{
    return write(fd, buf, (int)size);
}

_ssize_t _read_r(struct _reent *reent, int fd, void *buf, size_t size)
{
    return read(fd, buf, (int)size);
}

int _open_r(struct _reent *reent, const char *name, int flags, int mode)
{
    return open(name, flags, mode);
}

int _close_r(struct _reent *reent, int fd)
{
    return close(fd);
}

_off_t _lseek_r(struct _reent *reent, int fd, _off_t offset, int whence)
{
    return lseek(fd, offset, whence);
}

int _unlink_r(struct _reent *reent, const char *name)
{
    return unlink(name);
}

/* libsys has no directory removal (the Green Hills remove() only unlinks). */
int _rmdir_r(struct _reent *reent, const char *name)
{
    reent->_errno = ENOSYS;
    return -1;
}

/* The standard streams are the console; libsys has no other file status to report. */
int _isatty_r(struct _reent *reent, int fd)
{
    if (fd >= STDIN_FILENO_CAFE && fd <= STDERR_FILENO_CAFE)
        return 1;
    reent->_errno = ENOTTY;
    return 0;
}

int _fstat_r(struct _reent *reent, int fd, struct stat *status)
{
    if (fd >= STDIN_FILENO_CAFE && fd <= STDERR_FILENO_CAFE) {
        memset(status, 0, sizeof(*status));
        status->st_mode = S_IFCHR;
        return 0;
    }
    reent->_errno = ENOSYS;
    return -1;
}

void _exit(int status)
{
    _Exit(status);
}

/* newlib's powerpc-eabi configuration leaves abort to the system (ABORT_PROVIDED); this is its own
   abort: raise SIGABRT (the libsys port's raise runs the handler), and end the process if that returns. */
void abort(void)
{
    raise(SIGABRT);
    _exit(1);
}

void __assert_func(const char *file, int line, const char *function, const char *expression)
{
    fprintf(stderr, "assertion \"%s\" failed: file \"%s\", line %d%s%s\n", expression, file, line,
            function ? ", function: " : "", function ? function : "");
    abort();
}
