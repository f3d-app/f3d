#ifndef __DMON_H__
#define __DMON_H__

//
// Copyright 2023 Sepehr Taghdisian (septag@github). All rights reserved.
// License: https://github.com/septag/dmon#license-bsd-2-clause
//
//  Portable directory monitoring library
//  watches directories for file or directory changes.
//
// Usage:
//      define DMON_IMPL and include this file to use it:
//          #define DMON_IMPL
//          #include "dmon.h"
//
//      dmon_init():
//          Call this once at the start of your program.
//          This will start a low-priority monitoring thread
//      dmon_deinit():
//          Call this when your work with dmon is finished, usually on program terminate
//          This will free resources and stop the monitoring thread
//      dmon_watch:
//          Watch for directories
//          You can watch multiple directories by calling this function multiple times
//              rootdir: root directory to monitor
//              watch_cb: callback function to receive events.
//                        NOTE that this function is called from another thread, so you should
//                        beware of data races in your application when accessing data within this
//                        callback
//              flags: watch flags, see dmon_watch_flags_t
//              user_data: user pointer that is passed to callback function
//          Returns the Id of the watched directory after successful call, or returns Id=0 if error
//      dmon_unwatch:
//          Remove the directory from watch list
//
//      see test.c for the basic example
//
// Configuration:
//      You can customize some low-level functionality like malloc and logging by overriding macros:
//
//      DMON_MALLOC, DMON_FREE, DMON_REALLOC:
//          define these macros to override memory allocations
//          default is 'malloc', 'free' and 'realloc'
//      DMON_ASSERT:
//          define this to provide your own assert
//          default is 'assert'
//      DMON_LOG_ERROR:
//          define this to provide your own logging mechanism
//          default implementation logs to stdout and breaks the program
//      DMON_LOG_DEBUG
//          define this to provide your own extra debug logging mechanism
//          default implementation logs to stdout in DEBUG and does nothing in other builds
//      DMON_API_DECL, DMON_API_IMPL
//          define these to provide your own API declarations. (for example: static)
//          default is nothing (which is extern in C language )
//      DMON_MAX_PATH
//          Maximum size of path characters
//          default is 260 characters
//      DMON_MAX_WATCHES
//          Maximum number of watch directories
//          default is 64
//      DMON_SLEEP_INTERVAL
//          Number of milliseconds to pause between polling for file changes
//          default is 10 ms
//
// TODO:
//      - Use FSEventStreamSetDispatchQueue instead of FSEventStreamScheduleWithRunLoop on MacOS
//      - DMON_WATCHFLAGS_FOLLOW_SYMLINKS does not resolve files
//      - implement DMON_WATCHFLAGS_OUTOFSCOPE_LINKS
//
// History:
//      1.0.0       First version. working Win32/Linux backends
//      1.1.0       MacOS backend
//      1.1.1       Minor fixes, eliminate gcc/clang warnings with -Wall
//      1.1.2       Eliminate some win32 dead code
//      1.1.3       Fixed select not resetting causing high cpu usage on linux
//      1.2.1       inotify (linux) fixes and improvements, added extra functionality header for linux
//                  to manually add/remove directories manually to the watch handle, in case of large file sets
//      1.2.2       Name refactoring
//      1.3.0       Fixing bugs and proper watch/unwatch handles with freelists. Lower memory consumption, especially on Windows backend
//      1.3.1       Fix in MacOS event grouping
//      1.3.2       Fixes and improvements for Windows backend
//      1.3.3       Fixed thread sanitizer issues with Linux backend
//      1.3.4       Fixed thread sanitizer issues with MacOS backend
//      1.3.5       Got rid of volatile for quit variable
//      1.3.6       Fix deadlock when watch/unwatch API is called from the OnChange callback
//      1.3.7       Fix deadlock caused by constantly locking the mutex in the thread loop (recent change)
//      1.3.8       Fix a cpp compatiblity compiler bug after recent changes
//      

#include <stdbool.h>
#include <stdint.h>

#ifndef DMON_API_DECL
#   define DMON_API_DECL
#endif

#ifndef DMON_API_IMPL
#   define DMON_API_IMPL
#endif

typedef struct { uint32_t id; } dmon_watch_id;

// Pass these flags to `dmon_watch`
typedef enum dmon_watch_flags_t {
    DMON_WATCHFLAGS_RECURSIVE = 0x1,            // monitor all child directories
    DMON_WATCHFLAGS_FOLLOW_SYMLINKS = 0x2,      // resolve symlinks (linux only)
    DMON_WATCHFLAGS_OUTOFSCOPE_LINKS = 0x4      // TODO: not implemented yet
} dmon_watch_flags;

// Action is what operation performed on the file. this value is provided by watch callback
typedef enum dmon_action_t {
    DMON_ACTION_CREATE = 1,
    DMON_ACTION_DELETE,
    DMON_ACTION_MODIFY,
    DMON_ACTION_MOVE
} dmon_action;

#ifdef __cplusplus
extern "C" {
#endif

DMON_API_DECL void dmon_init(void);
DMON_API_DECL void dmon_deinit(void);

DMON_API_DECL  dmon_watch_id dmon_watch(const char* rootdir,
                         void (*watch_cb)(dmon_watch_id watch_id, dmon_action action,
                                          const char* rootdir, const char* filepath,
                                          const char* oldfilepath, void* user),
                         uint32_t flags, void* user_data);
DMON_API_DECL void dmon_unwatch(dmon_watch_id id);

#ifdef __cplusplus
}
#endif

#ifdef DMON_IMPL

#define DMON_OS_WINDOWS 0
#define DMON_OS_MACOS 0
#define DMON_OS_LINUX 0

#if defined(_WIN32) || defined(_WIN64)
#    undef DMON_OS_WINDOWS
#    define DMON_OS_WINDOWS 1
#elif defined(__linux__)
#    undef DMON_OS_LINUX
#    define DMON_OS_LINUX 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#    undef DMON_OS_MACOS
#    define DMON_OS_MACOS __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#else
#    define DMON_OS 0
#    error "unsupported platform"
#endif

#if DMON_OS_WINDOWS
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    include <intrin.h>
#    ifdef _MSC_VER
#        pragma intrinsic(_InterlockedExchange)
#    endif
#elif DMON_OS_LINUX
#    ifndef __USE_MISC
#        define __USE_MISC
#    endif
#    include <dirent.h>
#    include <errno.h>
#    include <fcntl.h>
#    include <linux/limits.h>
#    include <pthread.h>
#    include <sys/inotify.h>
#    include <sys/stat.h>
#    include <sys/time.h>
#    include <time.h>
#    include <unistd.h>
#    include <stdlib.h>
#elif DMON_OS_MACOS
#   include <pthread.h>
#   include <CoreServices/CoreServices.h>
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <dispatch/dispatch.h>
#endif

#ifndef DMON_MALLOC
#   include <stdlib.h>
#   define DMON_MALLOC(size)        malloc(size)
#   define DMON_FREE(ptr)           free(ptr)
#   define DMON_REALLOC(ptr, size)  realloc(ptr, size)
#endif

#ifndef DMON_ASSERT
#   include <assert.h>
#   define DMON_ASSERT(e)   assert(e)
#endif

#ifndef DMON_LOG_ERROR
#   include <stdio.h>
#   define DMON_LOG_ERROR(s)    do { puts(s); DMON_ASSERT(0); } while(0)
#endif

#ifndef DMON_LOG_DEBUG
#   ifndef NDEBUG
#       include <stdio.h>
#       define DMON_LOG_DEBUG(s)    do { puts(s); } while(0)
#   else
#       define DMON_LOG_DEBUG(s)
#   endif
#endif

#ifndef DMON_MAX_WATCHES
#   define DMON_MAX_WATCHES 64
#endif

#ifndef DMON_MAX_PATH
#   define DMON_MAX_PATH 260
#endif

#define _DMON_UNUSED(x) (void)(x)

#ifndef _DMON_PRIVATE
#   if defined(__GNUC__) || defined(__clang__)
#       define _DMON_PRIVATE __attribute__((unused)) static
#   else
#       define _DMON_PRIVATE static
#   endif
#endif

#ifndef DMON_SLEEP_INTERVAL
#   define DMON_SLEEP_INTERVAL 10
#endif

#include <string.h>

#ifndef _DMON_LOG_ERRORF
#   define _DMON_LOG_ERRORF(str, ...) do { char msg[512]; snprintf(msg, sizeof(msg), str, __VA_ARGS__); DMON_LOG_ERROR(msg); } while(0);
#endif

#ifndef _DMON_LOG_DEBUGF
#   define _DMON_LOG_DEBUGF(str, ...) do { char msg[512]; snprintf(msg, sizeof(msg), str, __VA_ARGS__); DMON_LOG_DEBUG(msg); } while(0);
#endif

#ifndef _dmon_min
#   define _dmon_min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef _dmon_max
#   define _dmon_max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef _dmon_swap
#   define _dmon_swap(a, b, _type)  \
        do {                        \
            _type tmp = a;          \
            a = b;                  \
            b = tmp;                \
        } while (0)
#endif

#ifndef _dmon_make_id
#   ifdef __cplusplus
#       define _dmon_make_id(id)    {id}
#   else
#       define _dmon_make_id(id)    (dmon_watch_id) {id}
#   endif
#endif  // _dmon_make_id

_DMON_PRIVATE bool _dmon_isrange(char ch, char from, char to)
{
    return (uint8_t)(ch - from) <= (uint8_t)(to - from);
}

_DMON_PRIVATE bool _dmon_isupperchar(char ch)
{
    return _dmon_isrange(ch, 'A', 'Z');
}

_DMON_PRIVATE char _dmon_tolowerchar(char ch)
{
    return ch + (_dmon_isupperchar(ch) ? 0x20 : 0);
}

_DMON_PRIVATE char* _dmon_tolower(char* dst, int dst_sz, const char* str)
{
    int offset = 0;
    int dst_max = dst_sz - 1;
    while (*str && offset < dst_max) {
        dst[offset++] = _dmon_tolowerchar(*str);
        ++str;
    }
    dst[offset] = '\0';
    return dst;
}

_DMON_PRIVATE char* _dmon_strcpy(char* dst, int dst_sz, const char* src)
{
    DMON_ASSERT(dst);
    DMON_ASSERT(src);

    const int32_t len = (int32_t)strlen(src);
    const int32_t _max = dst_sz - 1;
    const int32_t num = (len < _max ? len : _max);
    memcpy(dst, src, num);
    dst[num] = '\0';

    return dst;
}

_DMON_PRIVATE char* _dmon_unixpath(char* dst, int size, const char* path)
{
    size_t len = strlen(path), i;
    len = _dmon_min(len, (size_t)size - 1);

    for (i = 0; i < len; i++) {
        if (path[i] != '\\')
            dst[i] = path[i];
        else
            dst[i] = '/';
    }
    dst[len] = '\0';
    return dst;
}

#if DMON_OS_LINUX || DMON_OS_MACOS
_DMON_PRIVATE char* _dmon_strcat(char* dst, int dst_sz, const char* src)
{
    int len = (int)strlen(dst);
    return _dmon_strcpy(dst + len, dst_sz - len, src);
}
#endif // DMON_OS_LINUX || DMON_OS_MACOS

// stretchy buffer: https://github.com/nothings/stb/blob/master/stretchy_buffer.h
#define stb_sb_free(a)         ((a) ? DMON_FREE(stb__sbraw(a)),0 : 0)
#define stb_sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define stb_sb_pop(a)          (stb__sbn(a)--)
#define stb_sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])
#define stb_sb_reset(a)        ((a) ? (stb__sbn(a) = 0) : 0)

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      (*((void **)&(a)) = stb__sbgrowf((a), (n), sizeof(*(a))))

static void * stb__sbgrowf(void *arr, int increment, int itemsize)
{
    int dbl_cur = arr ? 2*stb__sbm(arr) : 0;
    int min_needed = stb_sb_count(arr) + increment;
    int m = dbl_cur > min_needed ? dbl_cur : min_needed;
    int *p = (int *) DMON_REALLOC(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int)*2);
    if (p) {
        if (!arr)
            p[1] = 0;
        p[0] = m;
        return p+2;
    } else {
        return (void *) (2*sizeof(int)); // try to force a NULL pointer exception later
    }
}

// watcher callback (same as dmon.h's declaration)
typedef void (_dmon_watch_cb)(dmon_watch_id, dmon_action, const char*, const char*, const char*, void*);

#if DMON_OS_WINDOWS
// ---------------------------------------------------------------------------------------------------------------------
// @Windows
// IOCP
#ifdef UNICODE
#   define _DMON_WINAPI_STR(name, size) wchar_t _##name[size]; MultiByteToWideChar(CP_UTF8, 0, name, -1, _##name, size)
#else
#   define _DMON_WINAPI_STR(name, size) const char* _##name = name
#endif

typedef struct dmon__win32_event {
    char filepath[DMON_MAX_PATH];
    DWORD action;
    dmon_watch_id watch_id;
    bool skip;
} dmon__win32_event;

typedef struct dmon__watch_state {
    dmon_watch_id id;
    OVERLAPPED overlapped;
    HANDLE dir_handle;
    uint8_t buffer[64512]; // http://msdn.microsoft.com/en-us/library/windows/desktop/aa365465(v=vs.85).aspx
    DWORD notify_filter;
    _dmon_watch_cb* watch_cb;
    uint32_t watch_flags;
    void* user_data;
    char rootdir[DMON_MAX_PATH];
    char old_filepath[DMON_MAX_PATH];
} dmon__watch_state;

typedef struct dmon__state {
    int num_watches;
    dmon__watch_state* watches[DMON_MAX_WATCHES];
	int freelist[DMON_MAX_WATCHES];
    HANDLE thread_handle;
    CRITICAL_SECTION mutex;
    dmon__win32_event* events;
    uint32_t quit;
} dmon__state;

static bool _dmon_init;
static dmon__state _dmon;

_DMON_PRIVATE bool _dmon_refresh_watch(dmon__watch_state* watch)
{
    return ReadDirectoryChangesW(watch->dir_handle, watch->buffer, sizeof(watch->buffer),
                                 (watch->watch_flags & DMON_WATCHFLAGS_RECURSIVE) ? TRUE : FALSE,
                                 watch->notify_filter, NULL, &watch->overlapped, NULL) != 0;
}

_DMON_PRIVATE void _dmon_unwatch(dmon__watch_state* watch)
{
    CancelIo(watch->dir_handle);
    CloseHandle(watch->overlapped.hEvent);
    CloseHandle(watch->dir_handle);
}

_DMON_PRIVATE void _dmon_win32_process_events(void)
{
    int i, c;
    for (i = 0, c = stb_sb_count(_dmon.events); i < c; i++) {
        dmon__win32_event* ev = &_dmon.events[i];
        if (ev->skip) {
            continue;
        }

        if (ev->action == FILE_ACTION_MODIFIED || ev->action == FILE_ACTION_ADDED) {
            // remove duplicate modifies on a single file
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__win32_event* check_ev = &_dmon.events[j];
                if (check_ev->action == FILE_ACTION_MODIFIED &&
                    strcmp(ev->filepath, check_ev->filepath) == 0) {
                    check_ev->skip = true;
                }
            }
        }
    }

    // trigger user callbacks
    for (i = 0, c = stb_sb_count(_dmon.events); i < c; i++) {
        dmon__win32_event* ev = &_dmon.events[i];
        if (ev->skip) {
            continue;
        }
        dmon__watch_state* watch = _dmon.watches[ev->watch_id.id - 1];

        if(watch == NULL || watch->watch_cb == NULL) {
            continue;
        }

        switch (ev->action) {
        case FILE_ACTION_ADDED:
            watch->watch_cb(ev->watch_id, DMON_ACTION_CREATE, watch->rootdir, ev->filepath, NULL,
                            watch->user_data);
            break;
        case FILE_ACTION_MODIFIED:
            watch->watch_cb(ev->watch_id, DMON_ACTION_MODIFY, watch->rootdir, ev->filepath, NULL,
                            watch->user_data);
            break;
        case FILE_ACTION_RENAMED_OLD_NAME: {
            // find the first occurrence of the NEW_NAME
            // this is somewhat API flaw that we have no reference for relating old and new files
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__win32_event* check_ev = &_dmon.events[j];
                if (check_ev->action == FILE_ACTION_RENAMED_NEW_NAME) {
                    watch->watch_cb(check_ev->watch_id, DMON_ACTION_MOVE, watch->rootdir,
                                    check_ev->filepath, ev->filepath, watch->user_data);
                    break;
                }
            }
        } break;
        case FILE_ACTION_REMOVED:
            watch->watch_cb(ev->watch_id, DMON_ACTION_DELETE, watch->rootdir, ev->filepath, NULL,
                            watch->user_data);
            break;
        }
    }
    stb_sb_reset(_dmon.events);
}

_DMON_PRIVATE DWORD WINAPI _dmon_thread(LPVOID arg)
{
    _DMON_UNUSED(arg);
    HANDLE wait_handles[DMON_MAX_WATCHES];
    dmon__watch_state* watch_states[DMON_MAX_WATCHES];

    SYSTEMTIME starttm;
    GetSystemTime(&starttm);
    uint64_t msecs_elapsed = 0;

    while (InterlockedCompareExchange(&_dmon.quit, 0, 0) == 0) {
        Sleep(DMON_SLEEP_INTERVAL);
        if (!TryEnterCriticalSection(&_dmon.mutex)) {
            continue;
        }

        if (_dmon.num_watches == 0) {
            LeaveCriticalSection(&_dmon.mutex);
            continue;
        }

        DWORD active = 0;
        for (unsigned i = 0; i < DMON_MAX_WATCHES; i++) {
            dmon__watch_state* watch = _dmon.watches[i];
            if (watch) {
                watch_states[active] = watch;
                wait_handles[active] = watch->overlapped.hEvent;
                ++active;
            }
        }

        DWORD wait_result = WaitForMultipleObjects(active, wait_handles, FALSE, 10);
        DMON_ASSERT(wait_result != WAIT_FAILED);
        if (wait_result >= WAIT_OBJECT_0 && wait_result < WAIT_OBJECT_0 + active) {
            dmon__watch_state* watch = watch_states[wait_result - WAIT_OBJECT_0];
            DMON_ASSERT(HasOverlappedIoCompleted(&watch->overlapped));

            DWORD bytes;
            if (GetOverlappedResult(watch->dir_handle, &watch->overlapped, &bytes, FALSE)) {
                char filepath[DMON_MAX_PATH];
                PFILE_NOTIFY_INFORMATION notify;
                size_t offset = 0;

                if (bytes == 0) {
                    _dmon_refresh_watch(watch);
                    LeaveCriticalSection(&_dmon.mutex);
                    continue;
                }

                do {
                    notify = (PFILE_NOTIFY_INFORMATION)&watch->buffer[offset];

                    int count = WideCharToMultiByte(CP_UTF8, 0, notify->FileName,
                                                    notify->FileNameLength / sizeof(WCHAR),
                                                    filepath, DMON_MAX_PATH - 1, NULL, NULL);
                    filepath[count] = TEXT('\0');
                    _dmon_unixpath(filepath, sizeof(filepath), filepath);

                    if (stb_sb_count(_dmon.events) == 0) {
                        msecs_elapsed = 0;
                    }
                    dmon__win32_event wev = { { 0 }, notify->Action, watch->id, false };
                    _dmon_strcpy(wev.filepath, sizeof(wev.filepath), filepath);
                    stb_sb_push(_dmon.events, wev);

                    offset += notify->NextEntryOffset;
                } while (notify->NextEntryOffset > 0);

                if (InterlockedCompareExchange(&_dmon.quit, 0, 0) == 0) {
                    _dmon_refresh_watch(watch);
                }
            }
        }    // if (WaitForMultipleObjects)

        SYSTEMTIME tm;
        GetSystemTime(&tm);
        LONG dt =(tm.wSecond - starttm.wSecond) * 1000 + (tm.wMilliseconds - starttm.wMilliseconds);
        starttm = tm;
        msecs_elapsed += dt;
        if (msecs_elapsed > 100 && stb_sb_count(_dmon.events) > 0) {
            _dmon_win32_process_events();
            msecs_elapsed = 0;
        }

        LeaveCriticalSection(&_dmon.mutex);
    }
    return 0;
}


DMON_API_IMPL void dmon_init(void)
{
    DMON_ASSERT(!_dmon_init);
    InitializeCriticalSection(&_dmon.mutex);

    _dmon.thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_dmon_thread, NULL, 0, NULL);
    DMON_ASSERT(_dmon.thread_handle);

	for (int i = 0; i < DMON_MAX_WATCHES; i++)
    {
        _dmon.freelist[i] = DMON_MAX_WATCHES - i - 1;
        _dmon.watches[i] = NULL;
    }

    _dmon_init = true;
}


DMON_API_IMPL void dmon_deinit(void)
{
    DMON_ASSERT(_dmon_init);
    InterlockedExchange(&_dmon.quit, 1);
    if (_dmon.thread_handle != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(_dmon.thread_handle, INFINITE);
        CloseHandle(_dmon.thread_handle);
    }

    for (unsigned i = 0; i < DMON_MAX_WATCHES; i++) {
        if (_dmon.watches[i]) {
            _dmon_unwatch(_dmon.watches[i]);
            DMON_FREE(_dmon.watches[i]);
        }
    }

    DeleteCriticalSection(&_dmon.mutex);
    stb_sb_free(_dmon.events);
    memset(&_dmon, 0x0, sizeof(_dmon));
    _dmon_init = false;
}

DMON_API_IMPL dmon_watch_id dmon_watch(const char* rootdir,
                                       void (*watch_cb)(dmon_watch_id watch_id, dmon_action action,
                                                        const char* dirname, const char* filename,
                                                        const char* oldname, void* user),
                                       uint32_t flags, void* user_data)
{
	DMON_ASSERT(_dmon_init);
    DMON_ASSERT(watch_cb);
    DMON_ASSERT(rootdir && rootdir[0]);

    EnterCriticalSection(&_dmon.mutex);

    DMON_ASSERT(_dmon.num_watches < DMON_MAX_WATCHES);
    if (_dmon.num_watches >= DMON_MAX_WATCHES) {
        DMON_LOG_ERROR("Exceeding maximum number of watches");
        LeaveCriticalSection(&_dmon.mutex);
        return _dmon_make_id(0);
    }

    dmon__watch_state* watch = NULL;
    unsigned id = 0;
    HANDLE hEvent = INVALID_HANDLE_VALUE;
    HANDLE dir_handle = INVALID_HANDLE_VALUE;
    int num_freelist = DMON_MAX_WATCHES - _dmon.num_watches;
    int index = _dmon.freelist[num_freelist - 1];
    size_t rootdir_len;

    {
        _DMON_WINAPI_STR(rootdir, DMON_MAX_PATH);
        dir_handle = CreateFile(_rootdir, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL);
    }

    if(dir_handle == INVALID_HANDLE_VALUE)
    {
        _DMON_LOG_ERRORF("Could not open: %s", rootdir);
        goto fail;
    }

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(hEvent == INVALID_HANDLE_VALUE)
    {
        DMON_LOG_ERROR("CreateEvent() failed");
        goto fail;
    }

    watch = (dmon__watch_state*)DMON_MALLOC(sizeof(dmon__watch_state));
    if (!watch)
    {
        DMON_LOG_ERROR("Out of memory");
        goto fail;
    }
    memset(watch, 0x0, sizeof(dmon__watch_state));

    id = (uint32_t)(index + 1);

    watch->notify_filter = FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_LAST_WRITE |
                           FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
                           FILE_NOTIFY_CHANGE_SIZE;
    watch->overlapped.hEvent = hEvent;
    watch->dir_handle = dir_handle;
    watch->id = _dmon_make_id(id);
    watch->watch_flags = flags;
    watch->watch_cb = watch_cb;
    watch->user_data = user_data;

    _dmon_strcpy(watch->rootdir, sizeof(watch->rootdir) - 1, rootdir);
    _dmon_unixpath(watch->rootdir, sizeof(watch->rootdir), rootdir);
    rootdir_len = strlen(watch->rootdir);
    if (watch->rootdir[rootdir_len - 1] != '/') {
        watch->rootdir[rootdir_len] = '/';
        watch->rootdir[rootdir_len + 1] = '\0';
    }

    if (!_dmon_refresh_watch(watch)) {
        _dmon_unwatch(watch);
        DMON_LOG_ERROR("ReadDirectoryChanges failed");
        goto fail;
    }

    ++_dmon.num_watches;

finish:
    _dmon.watches[index] = watch;
    LeaveCriticalSection(&_dmon.mutex);
    return _dmon_make_id(id);

fail:
    if(hEvent != INVALID_HANDLE_VALUE)
        CloseHandle(hEvent);
    if(dir_handle != INVALID_HANDLE_VALUE)
        CloseHandle(dir_handle);
    if(watch)
    {
        DMON_FREE(watch);
        watch = NULL;
    };
    id = 0;
    goto finish;
}

DMON_API_IMPL void dmon_unwatch(dmon_watch_id id)
{
    EnterCriticalSection(&_dmon.mutex);

	DMON_ASSERT(_dmon_init);
    DMON_ASSERT(id.id > 0);
    int index = id.id - 1;
    DMON_ASSERT(index < DMON_MAX_WATCHES);
    DMON_ASSERT(_dmon.num_watches > 0);

    dmon__watch_state* watch = _dmon.watches[index];
    DMON_ASSERT(watch);

    if (watch) {
        _dmon_unwatch(watch);
        DMON_FREE(watch);
        _dmon.watches[index] = NULL;

        --_dmon.num_watches;
        int num_freelist = DMON_MAX_WATCHES - _dmon.num_watches;
        _dmon.freelist[num_freelist - 1] = index;
    }

    LeaveCriticalSection(&_dmon.mutex);
}

#elif DMON_OS_LINUX
// ---------------------------------------------------------------------------------------------------------------------
// @Linux
// inotify linux backend
#define _DMON_TEMP_BUFFSIZE ((sizeof(struct inotify_event) + PATH_MAX) * 1024)

typedef struct dmon__watch_subdir {
    char rootdir[DMON_MAX_PATH];
} dmon__watch_subdir;

typedef struct dmon__inotify_event {
    char filepath[DMON_MAX_PATH];
    uint32_t mask;
    uint32_t cookie;
    dmon_watch_id watch_id;
    bool skip;
} dmon__inotify_event;

typedef struct dmon__watch_state {
    dmon_watch_id id;
    int fd;
    uint32_t watch_flags;
    _dmon_watch_cb* watch_cb;
    void* user_data;
    char rootdir[DMON_MAX_PATH];
    dmon__watch_subdir* subdirs;
    int* wds;
} dmon__watch_state;

typedef struct dmon__state {
    dmon__watch_state* watches[DMON_MAX_WATCHES];
   	int freelist[DMON_MAX_WATCHES];
    dmon__inotify_event* events;
    int num_watches;
    pthread_t thread_handle;
    pthread_mutex_t mutex;
    bool quit;
} dmon__state;

static bool _dmon_init;
static dmon__state _dmon;

_DMON_PRIVATE void _dmon_watch_recursive(const char* dirname, int fd, uint32_t mask,
                                         bool followlinks, dmon__watch_state* watch)
{
    struct dirent* entry;
    DIR* dir = opendir(dirname);
    DMON_ASSERT(dir);

    char watchdir[DMON_MAX_PATH];

    while ((entry = readdir(dir)) != NULL) {
        bool entry_valid = false;
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0) {
                _dmon_strcpy(watchdir, sizeof(watchdir), dirname);
                _dmon_strcat(watchdir, sizeof(watchdir), entry->d_name);
                entry_valid = true;
            }
        } else if (followlinks && entry->d_type == DT_LNK) {
            char linkpath[PATH_MAX];
            _dmon_strcpy(watchdir, sizeof(watchdir), dirname);
            _dmon_strcat(watchdir, sizeof(watchdir), entry->d_name);
            char* r = realpath(watchdir, linkpath);
            _DMON_UNUSED(r);
            DMON_ASSERT(r);
            _dmon_strcpy(watchdir, sizeof(watchdir), linkpath);
            entry_valid = true;
        }

        // add sub-directory to watch dirs
        if (entry_valid) {
            int watchdir_len = (int)strlen(watchdir);
            if (watchdir[watchdir_len - 1] != '/') {
                watchdir[watchdir_len] = '/';
                watchdir[watchdir_len + 1] = '\0';
            }
            int wd = inotify_add_watch(fd, watchdir, mask);
            _DMON_UNUSED(wd);
            DMON_ASSERT(wd != -1);

            dmon__watch_subdir subdir;
            _dmon_strcpy(subdir.rootdir, sizeof(subdir.rootdir), watchdir);
            if (strstr(subdir.rootdir, watch->rootdir) == subdir.rootdir) {
                _dmon_strcpy(subdir.rootdir, sizeof(subdir.rootdir), watchdir + strlen(watch->rootdir));
            }

            stb_sb_push(watch->subdirs, subdir);
            stb_sb_push(watch->wds, wd);

            // recurse
            _dmon_watch_recursive(watchdir, fd, mask, followlinks, watch);
        }
    }
    closedir(dir);
}

_DMON_PRIVATE const char* _dmon_find_subdir(const dmon__watch_state* watch, int wd)
{
    const int* wds = watch->wds;
    int i, c;
    for (i = 0, c = stb_sb_count(wds); i < c; i++) {
        if (wd == wds[i]) {
            return watch->subdirs[i].rootdir;
        }
    }

    return NULL;
}

_DMON_PRIVATE void _dmon_gather_recursive(dmon__watch_state* watch, const char* dirname)
{
    struct dirent* entry;
    DIR* dir = opendir(dirname);
    DMON_ASSERT(dir);

    char newdir[DMON_MAX_PATH];
    while ((entry = readdir(dir)) != NULL) {
        bool entry_valid = false;
        bool is_dir = false;
        if (strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, ".") != 0) {
            _dmon_strcpy(newdir, sizeof(newdir), dirname);
            _dmon_strcat(newdir, sizeof(newdir), entry->d_name);
            is_dir = (entry->d_type == DT_DIR);
            entry_valid = true;
        }

        // add sub-directory to watch dirs
        if (entry_valid) {
            dmon__watch_subdir subdir;
            _dmon_strcpy(subdir.rootdir, sizeof(subdir.rootdir), newdir);
            if (strstr(subdir.rootdir, watch->rootdir) == subdir.rootdir) {
                _dmon_strcpy(subdir.rootdir, sizeof(subdir.rootdir), newdir + strlen(watch->rootdir));
            }

            dmon__inotify_event dev = { { 0 }, IN_CREATE|(is_dir ? IN_ISDIR : 0U), 0, watch->id, false };
            _dmon_strcpy(dev.filepath, sizeof(dev.filepath), subdir.rootdir);
            stb_sb_push(_dmon.events, dev);
        }
    }
    closedir(dir);
}

_DMON_PRIVATE void _dmon_inotify_process_events(void)
{
    int i, c;
    for (i = 0, c = stb_sb_count(_dmon.events); i < c; i++) {
        dmon__inotify_event* ev = &_dmon.events[i];
        if (ev->skip) {
            continue;
        }

        // remove redundant modify events on a single file
        if (ev->mask & IN_MODIFY) {
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__inotify_event* check_ev = &_dmon.events[j];
                if ((check_ev->mask & IN_MODIFY) && strcmp(ev->filepath, check_ev->filepath) == 0) {
                    ev->skip = true;
                    break;
                } else if ((ev->mask & IN_ISDIR) && (check_ev->mask & (IN_ISDIR|IN_MODIFY))) {
                    // in some cases, particularly when created files under sub directories
                    // there can be two modify events for a single subdir one with trailing slash and one without
                    // remove trailing slash from both cases and test
                    int l1 = (int)strlen(ev->filepath);
                    int l2 = (int)strlen(check_ev->filepath);
                    if (ev->filepath[l1-1] == '/')          ev->filepath[l1-1] = '\0';
                    if (check_ev->filepath[l2-1] == '/')    check_ev->filepath[l2-1] = '\0';
                    if (strcmp(ev->filepath, check_ev->filepath) == 0) {
                        ev->skip = true;
                        break;
                    }
                }
            }
        } else if (ev->mask & IN_CREATE) {
            int j;
            bool loop_break = false;
            for (j = i + 1; j < c && !loop_break; j++) {
                dmon__inotify_event* check_ev = &_dmon.events[j];
                if ((check_ev->mask & IN_MOVED_FROM) && strcmp(ev->filepath, check_ev->filepath) == 0) {
                    // there is a case where some programs (like gedit):
                    // when we save, it creates a temp file, and moves it to the file being modified
                    // search for these cases and remove all of them
                    int k;
                    for (k = j + 1; k < c; k++) {
                        dmon__inotify_event* third_ev = &_dmon.events[k];
                        if (third_ev->mask & IN_MOVED_TO && check_ev->cookie == third_ev->cookie) {
                            third_ev->mask = IN_MODIFY;    // change to modified
                            ev->skip = check_ev->skip = true;
                            loop_break = true;
                            break;
                        }
                    }
                } else if ((check_ev->mask & IN_MODIFY) && strcmp(ev->filepath, check_ev->filepath) == 0) {
                    // Another case is that file is copied. CREATE and MODIFY happens sequentially
                    // so we ignore MODIFY event
                    check_ev->skip = true;
                }
            }
        } else if (ev->mask & IN_MOVED_FROM) {
            bool move_valid = false;
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__inotify_event* check_ev = &_dmon.events[j];
                if (check_ev->mask & IN_MOVED_TO && ev->cookie == check_ev->cookie) {
                    move_valid = true;
                    break;
                }
            }

            // in some environments like nautilus file explorer:
            // when a file is deleted, it is moved to recycle bin
            // so if the destination of the move is not valid, it's probably DELETE
            if (!move_valid) {
                ev->mask = IN_DELETE;
            }
        } else if (ev->mask & IN_MOVED_TO) {
            bool move_valid = false;
            int j;
            for (j = 0; j < i; j++) {
                dmon__inotify_event* check_ev = &_dmon.events[j];
                if (check_ev->mask & IN_MOVED_FROM && ev->cookie == check_ev->cookie) {
                    move_valid = true;
                    break;
                }
            }

            // in some environments like nautilus file explorer:
            // when a file is deleted, it is moved to recycle bin, on undo it is moved back it
            // so if the destination of the move is not valid, it's probably CREATE
            if (!move_valid) {
                ev->mask = IN_CREATE;
            }
        } else if (ev->mask & IN_DELETE) {
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__inotify_event* check_ev = &_dmon.events[j];
                // if the file is DELETED and then MODIFIED after, just ignore the modify event
                if ((check_ev->mask & IN_MODIFY) && strcmp(ev->filepath, check_ev->filepath) == 0) {
                    check_ev->skip = true;
                    break;
                }
            }
        }
    }

    // trigger user callbacks
    for (i = 0; i < stb_sb_count(_dmon.events); i++) {
        dmon__inotify_event* ev = &_dmon.events[i];
        if (ev->skip) {
            continue;
        }
        dmon__watch_state* watch = _dmon.watches[ev->watch_id.id - 1];

        if(watch == NULL || watch->watch_cb == NULL) {
            continue;
        }

        if (ev->mask & IN_CREATE) {
            if (ev->mask & IN_ISDIR) {
                if (watch->watch_flags & DMON_WATCHFLAGS_RECURSIVE) {
                    char watchdir[DMON_MAX_PATH];
                    _dmon_strcpy(watchdir, sizeof(watchdir), watch->rootdir);
                    _dmon_strcat(watchdir, sizeof(watchdir), ev->filepath);
                    _dmon_strcat(watchdir, sizeof(watchdir), "/");
                    uint32_t mask = IN_MOVED_TO | IN_CREATE | IN_MOVED_FROM | IN_DELETE | IN_MODIFY;
                    int wd = inotify_add_watch(watch->fd, watchdir, mask);
                    _DMON_UNUSED(wd);
                    DMON_ASSERT(wd != -1);

                    dmon__watch_subdir subdir;
                    _dmon_strcpy(subdir.rootdir, sizeof(subdir.rootdir), watchdir);
                    if (strstr(subdir.rootdir, watch->rootdir) == subdir.rootdir) {
                        _dmon_strcpy(subdir.rootdir, sizeof(subdir.rootdir), watchdir + strlen(watch->rootdir));
                    }

                    stb_sb_push(watch->subdirs, subdir);
                    stb_sb_push(watch->wds, wd);

                    // some directories may be already created, for instance, with the command: mkdir -p
                    // so we will enumerate them manually and add them to the events
                    _dmon_gather_recursive(watch, watchdir);
                    ev = &_dmon.events[i]; // gotta refresh the pointer because it may be relocated
                }
            }
            watch->watch_cb(ev->watch_id, DMON_ACTION_CREATE, watch->rootdir, ev->filepath, NULL, watch->user_data);
        }
        else if (ev->mask & IN_MODIFY) {
            watch->watch_cb(ev->watch_id, DMON_ACTION_MODIFY, watch->rootdir, ev->filepath, NULL, watch->user_data);
        }
        else if (ev->mask & IN_MOVED_FROM) {
            int j;
            for (j = i + 1; j < stb_sb_count(_dmon.events); j++) {
                dmon__inotify_event* check_ev = &_dmon.events[j];
                if (check_ev->mask & IN_MOVED_TO && ev->cookie == check_ev->cookie) {
                    watch->watch_cb(check_ev->watch_id, DMON_ACTION_MOVE, watch->rootdir,
                                    check_ev->filepath, ev->filepath, watch->user_data);
                    break;
                }
            }
        }
        else if (ev->mask & IN_DELETE) {
            watch->watch_cb(ev->watch_id, DMON_ACTION_DELETE, watch->rootdir, ev->filepath, NULL, watch->user_data);
        }
    }

    stb_sb_reset(_dmon.events);
}

static void* _dmon_thread(void* arg)
{
    _DMON_UNUSED(arg);

    static uint8_t buff[_DMON_TEMP_BUFFSIZE];
    struct timespec req = { (time_t)DMON_SLEEP_INTERVAL / 1000, (long)(DMON_SLEEP_INTERVAL * 1000000) };
    struct timespec rem = { 0, 0 };
    struct timeval timeout;
    uint64_t usecs_elapsed = 0;

    struct timeval starttm;
    gettimeofday(&starttm, 0);

    while (__sync_bool_compare_and_swap(&_dmon.quit, false, false)) {
        nanosleep(&req, &rem);
        if (pthread_mutex_trylock(&_dmon.mutex) != 0) {
            continue;
        }

        if (_dmon.num_watches == 0) {
            pthread_mutex_unlock(&_dmon.mutex);
            continue;
        }

        // Create read FD set
        fd_set rfds;
        FD_ZERO(&rfds);
        {
            int i;
            for (i = 0; i < _dmon.num_watches; i++) {
                dmon__watch_state* watch = _dmon.watches[i];
                FD_SET(watch->fd, &rfds);
            }
        }

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        if (select(FD_SETSIZE, &rfds, NULL, NULL, &timeout)) {
            int i;
            for (i = 0; i < _dmon.num_watches; i++) {
                dmon__watch_state* watch = _dmon.watches[i];
                if (FD_ISSET(watch->fd, &rfds)) {
                    ssize_t offset = 0;
                    ssize_t len = read(watch->fd, buff, _DMON_TEMP_BUFFSIZE);
                    if (len <= 0) {
                        continue;
                    }

                    while (offset < len) {
                        struct inotify_event* iev = (struct inotify_event*)&buff[offset];

                        const char *subdir = _dmon_find_subdir(watch, iev->wd);
                        if (subdir) {
                            char filepath[DMON_MAX_PATH];
                            _dmon_strcpy(filepath, sizeof(filepath), subdir);
                            _dmon_strcat(filepath, sizeof(filepath), iev->name);

                            if (stb_sb_count(_dmon.events) == 0) {
                                usecs_elapsed = 0;
                            }
                            dmon__inotify_event dev = { { 0 }, iev->mask, iev->cookie, watch->id, false };
                            _dmon_strcpy(dev.filepath, sizeof(dev.filepath), filepath);
                            stb_sb_push(_dmon.events, dev);
                        }

                        offset += sizeof(struct inotify_event) + iev->len;
                    }
                }
            }
        }

        struct timeval tm;
        gettimeofday(&tm, 0);
        long dt = (tm.tv_sec - starttm.tv_sec) * 1000000 + tm.tv_usec - starttm.tv_usec;
        starttm = tm;
        usecs_elapsed += dt;
        if (usecs_elapsed > 100000 && stb_sb_count(_dmon.events) > 0) {
            _dmon_inotify_process_events();
            usecs_elapsed = 0;
        }

        pthread_mutex_unlock(&_dmon.mutex);
    }
    return 0x0;
}

_DMON_PRIVATE void _dmon_unwatch(dmon__watch_state* watch)
{
    close(watch->fd);
    stb_sb_free(watch->subdirs);
    stb_sb_free(watch->wds);
}

DMON_API_IMPL void dmon_init(void)
{
    DMON_ASSERT(!_dmon_init);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_dmon.mutex, &attr);

    int r = pthread_create(&_dmon.thread_handle, NULL, _dmon_thread, NULL);
    _DMON_UNUSED(r);
    DMON_ASSERT(r == 0 && "pthread_create failed");

    for (int i = 0; i < DMON_MAX_WATCHES; i++)
        _dmon.freelist[i] = DMON_MAX_WATCHES - i - 1;

    _dmon_init = true;
}

DMON_API_IMPL void dmon_deinit(void)
{
    DMON_ASSERT(_dmon_init);
    _DMON_UNUSED(__sync_lock_test_and_set(&_dmon.quit, true));
    pthread_join(_dmon.thread_handle, NULL);

    {
        int i;
        for (i = 0; i < _dmon.num_watches; i++) {
            if (_dmon.watches[i]) {
                _dmon_unwatch(_dmon.watches[i]);
                DMON_FREE(_dmon.watches[i]);
            }
        }
    }

    pthread_mutex_destroy(&_dmon.mutex);
    stb_sb_free(_dmon.events);
    memset(&_dmon, 0x0, sizeof(_dmon));
    _dmon_init = false;
}

DMON_API_IMPL dmon_watch_id dmon_watch(const char* rootdir,
                                       void (*watch_cb)(dmon_watch_id watch_id, dmon_action action,
                                                        const char* dirname, const char* filename,
                                                        const char* oldname, void* user),
                                       uint32_t flags, void* user_data)
{
	DMON_ASSERT(_dmon_init);
    DMON_ASSERT(watch_cb);
    DMON_ASSERT(rootdir && rootdir[0]);

    pthread_mutex_lock(&_dmon.mutex);

    DMON_ASSERT(_dmon.num_watches < DMON_MAX_WATCHES);
    if (_dmon.num_watches >= DMON_MAX_WATCHES) {
        DMON_LOG_ERROR("Exceeding maximum number of watches");
        pthread_mutex_unlock(&_dmon.mutex);
        return _dmon_make_id(0);
    }

    int num_freelist = DMON_MAX_WATCHES - _dmon.num_watches;
    int index = _dmon.freelist[num_freelist - 1];
    uint32_t id = (uint32_t)(index + 1);

    if (_dmon.watches[index] == NULL) {
        dmon__watch_state* state =  (dmon__watch_state*)DMON_MALLOC(sizeof(dmon__watch_state));
        DMON_ASSERT(state);
        if (state == NULL) {
            pthread_mutex_unlock(&_dmon.mutex);
            return _dmon_make_id(0);
        }
        memset(state, 0x0, sizeof(dmon__watch_state));
        _dmon.watches[index] = state;
    }

    ++_dmon.num_watches;

    dmon__watch_state* watch = _dmon.watches[index];
    DMON_ASSERT(watch);
    watch->id = _dmon_make_id(id);
    watch->watch_flags = flags;
    watch->watch_cb = watch_cb;
    watch->user_data = user_data;

    struct stat root_st;
    if (stat(rootdir, &root_st) != 0 || !S_ISDIR(root_st.st_mode) || (root_st.st_mode & S_IRUSR) != S_IRUSR) {
        _DMON_LOG_ERRORF("Could not open/read directory: %s", rootdir);
        pthread_mutex_unlock(&_dmon.mutex);
        return _dmon_make_id(0);
    }

    if (S_ISLNK(root_st.st_mode)) {
        if (flags & DMON_WATCHFLAGS_FOLLOW_SYMLINKS) {
            char linkpath[PATH_MAX];
            char* r = realpath(rootdir, linkpath);
            _DMON_UNUSED(r);
            DMON_ASSERT(r);

            _dmon_strcpy(watch->rootdir, sizeof(watch->rootdir) - 1, linkpath);
        } else {
            _DMON_LOG_ERRORF("symlinks are unsupported: %s. use DMON_WATCHFLAGS_FOLLOW_SYMLINKS",
                             rootdir);
            pthread_mutex_unlock(&_dmon.mutex);
            return _dmon_make_id(0);
        }
    } else {
        _dmon_strcpy(watch->rootdir, sizeof(watch->rootdir) - 1, rootdir);
    }

    // add trailing slash
    int rootdir_len = (int)strlen(watch->rootdir);
    if (watch->rootdir[rootdir_len - 1] != '/') {
        watch->rootdir[rootdir_len] = '/';
        watch->rootdir[rootdir_len + 1] = '\0';
    }

    watch->fd = inotify_init();
    if (watch->fd < -1) {
        DMON_LOG_ERROR("could not create inotify instance");
        pthread_mutex_unlock(&_dmon.mutex);
        return _dmon_make_id(0);
    }

    uint32_t inotify_mask = IN_MOVED_TO | IN_CREATE | IN_MOVED_FROM | IN_DELETE | IN_MODIFY;
    int wd = inotify_add_watch(watch->fd, watch->rootdir, inotify_mask);
    if (wd < 0) {
       _DMON_LOG_ERRORF("Error watching directory '%s'. (inotify_add_watch:err=%d)", watch->rootdir, errno);
        pthread_mutex_unlock(&_dmon.mutex);
        return _dmon_make_id(0);
    }
    dmon__watch_subdir subdir;
    _dmon_strcpy(subdir.rootdir, sizeof(subdir.rootdir), "");   // root dir is just a dummy entry
    stb_sb_push(watch->subdirs, subdir);
    stb_sb_push(watch->wds, wd);

    // recursive mode: enumerate all child directories and add them to watch
    if (flags & DMON_WATCHFLAGS_RECURSIVE) {
        _dmon_watch_recursive(watch->rootdir, watch->fd, inotify_mask,
                              (flags & DMON_WATCHFLAGS_FOLLOW_SYMLINKS) ? true : false, watch);
    }


    pthread_mutex_unlock(&_dmon.mutex);
    return _dmon_make_id(id);
}

DMON_API_IMPL void dmon_unwatch(dmon_watch_id id)
{
	DMON_ASSERT(_dmon_init);
    DMON_ASSERT(id.id > 0);
    int index = id.id - 1;
    DMON_ASSERT(index < DMON_MAX_WATCHES);
    DMON_ASSERT(_dmon.watches[index]);
    DMON_ASSERT(_dmon.num_watches > 0);

    if (_dmon.watches[index]) {
        pthread_mutex_lock(&_dmon.mutex);

        _dmon_unwatch(_dmon.watches[index]);
        DMON_FREE(_dmon.watches[index]);
        _dmon.watches[index] = NULL;

        --_dmon.num_watches;
        int num_freelist = DMON_MAX_WATCHES - _dmon.num_watches;
        _dmon.freelist[num_freelist - 1] = index;

        pthread_mutex_unlock(&_dmon.mutex);
    }
}
#elif DMON_OS_MACOS
// ---------------------------------------------------------------------------------------------------------------------
// @MacOS
// FSEvents MacOS backend
typedef struct dmon__fsevent_event {
    char filepath[DMON_MAX_PATH];
    uint64_t event_id;
    long event_flags;
    dmon_watch_id watch_id;
    bool skip;
    bool move_valid;
} dmon__fsevent_event;

typedef struct dmon__watch_state {
    dmon_watch_id id;
    uint32_t watch_flags;
    FSEventStreamRef fsev_stream_ref;
    _dmon_watch_cb* watch_cb;
    void* user_data;
    char rootdir[DMON_MAX_PATH];
    char rootdir_unmod[DMON_MAX_PATH];
    bool init;
} dmon__watch_state;

typedef struct dmon__state {
    dmon__watch_state* watches[DMON_MAX_WATCHES];
   	int freelist[DMON_MAX_WATCHES];
    dmon__fsevent_event* events;
    int num_watches;
    pthread_t thread_handle;
    dispatch_semaphore_t thread_sem;
    pthread_mutex_t mutex;
    CFRunLoopRef cf_loop_ref;
    CFAllocatorRef cf_alloc_ref;
    bool quit;
} dmon__state;

union dmon__cast_userdata {
    void* ptr;
    uint32_t id;
};

static bool _dmon_init;
static dmon__state _dmon;

_DMON_PRIVATE void* _dmon_cf_malloc(CFIndex size, CFOptionFlags hints, void* info)
{
    _DMON_UNUSED(hints);
    _DMON_UNUSED(info);
    return DMON_MALLOC(size);
}

_DMON_PRIVATE void _dmon_cf_free(void* ptr, void* info)
{
    _DMON_UNUSED(info);
    DMON_FREE(ptr);
}

_DMON_PRIVATE void* _dmon_cf_realloc(void* ptr, CFIndex newsize, CFOptionFlags hints, void* info)
{
    _DMON_UNUSED(hints);
    _DMON_UNUSED(info);
    return DMON_REALLOC(ptr, (size_t)newsize);
}

_DMON_PRIVATE void _dmon_fsevent_process_events(void)
{
    int i, c;
    for (i = 0, c = stb_sb_count(_dmon.events); i < c; i++) {
        dmon__fsevent_event* ev = &_dmon.events[i];
        if (ev->skip) {
            continue;
        }

        // remove redundant modify events on a single file
        if (ev->event_flags & kFSEventStreamEventFlagItemModified) {
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__fsevent_event* check_ev = &_dmon.events[j];
                if ((check_ev->event_flags & kFSEventStreamEventFlagItemModified) &&
                    strcmp(ev->filepath, check_ev->filepath) == 0) {
                    ev->skip = true;
                    break;
                }
            }
        } else if ((ev->event_flags & kFSEventStreamEventFlagItemRenamed) && !ev->move_valid) {
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__fsevent_event* check_ev = &_dmon.events[j];
                if ((check_ev->event_flags & kFSEventStreamEventFlagItemRenamed) &&
                    check_ev->event_id == (ev->event_id + 1)) {
                    ev->move_valid = check_ev->move_valid = true;
                    break;
                }
            }

            // in some environments like finder file explorer:
            // when a file is deleted, it is moved to recycle bin
            // so if the destination of the move is not valid, it's probably DELETE or CREATE
            // decide CREATE if file exists
            if (!ev->move_valid) {
                ev->event_flags &= ~kFSEventStreamEventFlagItemRenamed;

                char abs_filepath[DMON_MAX_PATH];
                dmon__watch_state* watch = _dmon.watches[ev->watch_id.id-1];
                _dmon_strcpy(abs_filepath, sizeof(abs_filepath), watch->rootdir);
                _dmon_strcat(abs_filepath, sizeof(abs_filepath), ev->filepath);

                struct stat root_st;
                if (stat(abs_filepath, &root_st) != 0) {
                    ev->event_flags |= kFSEventStreamEventFlagItemRemoved;
                } else {
                    ev->event_flags |= kFSEventStreamEventFlagItemCreated;
                }
            }
        }
    }

    // trigger user callbacks
    for (i = 0, c = stb_sb_count(_dmon.events); i < c; i++) {
        dmon__fsevent_event* ev = &_dmon.events[i];
        if (ev->skip) {
            continue;
        }
        dmon__watch_state* watch = _dmon.watches[ev->watch_id.id - 1];

        if(watch == NULL || watch->watch_cb == NULL) {
            continue;
        }

        if (ev->event_flags & kFSEventStreamEventFlagItemCreated) {
            watch->watch_cb(ev->watch_id, DMON_ACTION_CREATE, watch->rootdir_unmod, ev->filepath, NULL,
                            watch->user_data);
        }

        if (ev->event_flags & kFSEventStreamEventFlagItemModified) {
            watch->watch_cb(ev->watch_id, DMON_ACTION_MODIFY, watch->rootdir_unmod, ev->filepath, NULL, watch->user_data);
        } else if (ev->event_flags & kFSEventStreamEventFlagItemRenamed) {
            int j;
            for (j = i + 1; j < c; j++) {
                dmon__fsevent_event* check_ev = &_dmon.events[j];
                if (check_ev->event_flags & kFSEventStreamEventFlagItemRenamed) {
                    watch->watch_cb(check_ev->watch_id, DMON_ACTION_MOVE, watch->rootdir_unmod,
                                    check_ev->filepath, ev->filepath, watch->user_data);
                    break;
                }
            }
        } else if (ev->event_flags & kFSEventStreamEventFlagItemRemoved) {
            watch->watch_cb(ev->watch_id, DMON_ACTION_DELETE, watch->rootdir_unmod, ev->filepath, NULL,
                            watch->user_data);
        }
    }

    stb_sb_reset(_dmon.events);
}

_DMON_PRIVATE void* _dmon_thread(void* arg)
{
    _DMON_UNUSED(arg);

    struct timespec req = { (time_t)DMON_SLEEP_INTERVAL / 1000, (long)(DMON_SLEEP_INTERVAL * 1000000) };
    struct timespec rem = { 0, 0 };

    _dmon.cf_loop_ref = CFRunLoopGetCurrent();
    dispatch_semaphore_signal(_dmon.thread_sem);

    while (__sync_bool_compare_and_swap(&_dmon.quit, false, false)) {
        int i;
        nanosleep(&req, &rem);
        if (pthread_mutex_trylock(&_dmon.mutex) != 0) {
            continue;
        }

        if (_dmon.num_watches == 0) {
            pthread_mutex_unlock(&_dmon.mutex);
            continue;
        }

        for (i = 0; i < _dmon.num_watches; i++) {
            dmon__watch_state* watch = _dmon.watches[i];
            if (!watch->init) {
                DMON_ASSERT(watch->fsev_stream_ref);
                FSEventStreamScheduleWithRunLoop(watch->fsev_stream_ref, _dmon.cf_loop_ref, kCFRunLoopDefaultMode);
                FSEventStreamStart(watch->fsev_stream_ref);

                watch->init = true;
            }
        }

        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.5, kCFRunLoopRunTimedOut);
        _dmon_fsevent_process_events();

        pthread_mutex_unlock(&_dmon.mutex);
    }

    CFRunLoopStop(_dmon.cf_loop_ref);
    _dmon.cf_loop_ref = NULL;
    return 0x0;
}

_DMON_PRIVATE void _dmon_unwatch(dmon__watch_state* watch)
{
    if (watch->fsev_stream_ref) {
        FSEventStreamStop(watch->fsev_stream_ref);
        FSEventStreamInvalidate(watch->fsev_stream_ref);
        FSEventStreamRelease(watch->fsev_stream_ref);
        watch->fsev_stream_ref = NULL;
    }
}

DMON_API_IMPL void dmon_init(void)
{
    DMON_ASSERT(!_dmon_init);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_dmon.mutex, &attr);

    CFAllocatorContext cf_alloc_ctx = { 0 };
    cf_alloc_ctx.allocate = _dmon_cf_malloc;
    cf_alloc_ctx.deallocate = _dmon_cf_free;
    cf_alloc_ctx.reallocate = _dmon_cf_realloc;
    _dmon.cf_alloc_ref = CFAllocatorCreate(NULL, &cf_alloc_ctx);

    _dmon.thread_sem = dispatch_semaphore_create(0);
    DMON_ASSERT(_dmon.thread_sem);

    int r = pthread_create(&_dmon.thread_handle, NULL, _dmon_thread, NULL);
    _DMON_UNUSED(r);
    DMON_ASSERT(r == 0 && "pthread_create failed");

    // wait for thread to initialize loop object
    dispatch_semaphore_wait(_dmon.thread_sem, DISPATCH_TIME_FOREVER);

    for (int i = 0; i < DMON_MAX_WATCHES; i++)
        _dmon.freelist[i] = DMON_MAX_WATCHES - i - 1;

    _dmon_init = true;
}

DMON_API_IMPL void dmon_deinit(void)
{
    DMON_ASSERT(_dmon_init);
    _DMON_UNUSED(__sync_lock_test_and_set(&_dmon.quit, true));
    pthread_join(_dmon.thread_handle, NULL);

    dispatch_release(_dmon.thread_sem);

    {
        int i;
        for (i = 0; i < _dmon.num_watches; i++) {
            if (_dmon.watches[i]) {
                _dmon_unwatch(_dmon.watches[i]);
                DMON_FREE(_dmon.watches[i]);
            }
        }
    }

    pthread_mutex_destroy(&_dmon.mutex);
    stb_sb_free(_dmon.events);
    if (_dmon.cf_alloc_ref)
        CFRelease(_dmon.cf_alloc_ref);

    memset(&_dmon, 0x0, sizeof(_dmon));
    _dmon_init = false;
}

_DMON_PRIVATE void _dmon_fsevent_callback(ConstFSEventStreamRef stream_ref, void* user_data,
                                          size_t num_events, void* event_paths,
                                          const FSEventStreamEventFlags event_flags[],
                                          const FSEventStreamEventId event_ids[])
{
    _DMON_UNUSED(stream_ref);

    union dmon__cast_userdata _userdata;
    _userdata.ptr = user_data;
    dmon_watch_id watch_id = _dmon_make_id(_userdata.id);
    DMON_ASSERT(watch_id.id > 0);
    dmon__watch_state* watch = _dmon.watches[watch_id.id - 1];
    char abs_filepath[DMON_MAX_PATH];
    char abs_filepath_lower[DMON_MAX_PATH];

    {
        size_t i;
        for (i = 0; i < num_events; i++) {
            const char *filepath = ((const char **) event_paths)[i];
            long flags = (long) event_flags[i];
            uint64_t event_id = (uint64_t) event_ids[i];
            dmon__fsevent_event ev;
            memset(&ev, 0x0, sizeof(ev));

            _dmon_strcpy(abs_filepath, sizeof(abs_filepath), filepath);
            _dmon_unixpath(abs_filepath, sizeof(abs_filepath), abs_filepath);

            // normalize path, so it would be the same on both MacOS file-system types (case/nocase)
            _dmon_tolower(abs_filepath_lower, sizeof(abs_filepath), abs_filepath);
            DMON_ASSERT(strstr(abs_filepath_lower, watch->rootdir) == abs_filepath_lower);

            // strip the root dir from the beginning
            _dmon_strcpy(ev.filepath, sizeof(ev.filepath), abs_filepath + strlen(watch->rootdir));

            ev.event_flags = flags;
            ev.event_id = event_id;
            ev.watch_id = watch_id;
            stb_sb_push(_dmon.events, ev);
        }
    }
}

DMON_API_IMPL dmon_watch_id dmon_watch(const char* rootdir,
                                       void (*watch_cb)(dmon_watch_id watch_id, dmon_action action,
                                                        const char* dirname, const char* filename,
                                                        const char* oldname, void* user),
                                       uint32_t flags, void* user_data)
{
	DMON_ASSERT(_dmon_init);
    DMON_ASSERT(watch_cb);
    DMON_ASSERT(rootdir && rootdir[0]);

    pthread_mutex_lock(&_dmon.mutex);

    DMON_ASSERT(_dmon.num_watches < DMON_MAX_WATCHES);
    if (_dmon.num_watches >= DMON_MAX_WATCHES) {
        DMON_LOG_ERROR("Exceeding maximum number of watches");
        pthread_mutex_unlock(&_dmon.mutex);
        return _dmon_make_id(0);
    }


    int num_freelist = DMON_MAX_WATCHES - _dmon.num_watches;
    int index = _dmon.freelist[num_freelist - 1];
    uint32_t id = (uint32_t)(index + 1);

    if (_dmon.watches[index] == NULL) {
        dmon__watch_state* state =  (dmon__watch_state*)DMON_MALLOC(sizeof(dmon__watch_state));
        DMON_ASSERT(state);
        if (state == NULL) {
            pthread_mutex_unlock(&_dmon.mutex);
            return _dmon_make_id(0);
        }
        memset(state, 0x0, sizeof(dmon__watch_state));
        _dmon.watches[index] = state;
    }

    ++_dmon.num_watches;

    dmon__watch_state* watch = _dmon.watches[id - 1];
    DMON_ASSERT(watch);
    watch->id = _dmon_make_id(id);
    watch->watch_flags = flags;
    watch->watch_cb = watch_cb;
    watch->user_data = user_data;

    struct stat root_st;
    if (stat(rootdir, &root_st) != 0 || !S_ISDIR(root_st.st_mode) ||
        (root_st.st_mode & S_IRUSR) != S_IRUSR) {
        _DMON_LOG_ERRORF("Could not open/read directory: %s", rootdir);
        pthread_mutex_unlock(&_dmon.mutex);
        return _dmon_make_id(0);
    }

    if (S_ISLNK(root_st.st_mode)) {
        if (flags & DMON_WATCHFLAGS_FOLLOW_SYMLINKS) {
            char linkpath[PATH_MAX];
            char* r = realpath(rootdir, linkpath);
            _DMON_UNUSED(r);
            DMON_ASSERT(r);

            _dmon_strcpy(watch->rootdir, sizeof(watch->rootdir) - 1, linkpath);
        } else {
            _DMON_LOG_ERRORF("symlinks are unsupported: %s. use DMON_WATCHFLAGS_FOLLOW_SYMLINKS", rootdir);
            pthread_mutex_unlock(&_dmon.mutex);
            return _dmon_make_id(0);
        }
    } else {
        char rootdir_abspath[DMON_MAX_PATH];
        if (realpath(rootdir, rootdir_abspath) != NULL) {
            _dmon_strcpy(watch->rootdir, sizeof(watch->rootdir) - 1, rootdir_abspath);
        } else {
            _dmon_strcpy(watch->rootdir, sizeof(watch->rootdir) - 1, rootdir);
        }
    }

    _dmon_unixpath(watch->rootdir, sizeof(watch->rootdir), watch->rootdir);

    // add trailing slash
    int rootdir_len = (int)strlen(watch->rootdir);
    if (watch->rootdir[rootdir_len - 1] != '/') {
        watch->rootdir[rootdir_len] = '/';
        watch->rootdir[rootdir_len + 1] = '\0';
    }

    _dmon_strcpy(watch->rootdir_unmod, sizeof(watch->rootdir_unmod), watch->rootdir);
    _dmon_tolower(watch->rootdir, sizeof(watch->rootdir), watch->rootdir);

    // create FS objects
    CFStringRef cf_dir = CFStringCreateWithCString(NULL, watch->rootdir_unmod, kCFStringEncodingUTF8);
    CFArrayRef cf_dirarr = CFArrayCreate(NULL, (const void**)&cf_dir, 1, NULL);

    FSEventStreamContext ctx;
    union dmon__cast_userdata userdata;
    userdata.id = id;
    ctx.version = 0;
    ctx.info = userdata.ptr;
    ctx.retain = NULL;
    ctx.release = NULL;
    ctx.copyDescription = NULL;
    watch->fsev_stream_ref = FSEventStreamCreate(_dmon.cf_alloc_ref, _dmon_fsevent_callback, &ctx,
                                                 cf_dirarr, kFSEventStreamEventIdSinceNow, 0.25,
                                                 kFSEventStreamCreateFlagFileEvents);


    CFRelease(cf_dirarr);
    CFRelease(cf_dir);

    pthread_mutex_unlock(&_dmon.mutex);
    return _dmon_make_id(id);
}

DMON_API_IMPL void dmon_unwatch(dmon_watch_id id)
{
   	DMON_ASSERT(_dmon_init);
    DMON_ASSERT(id.id > 0);
    int index = id.id - 1;
    DMON_ASSERT(index < DMON_MAX_WATCHES);
    DMON_ASSERT(_dmon.watches[index]);
    DMON_ASSERT(_dmon.num_watches > 0);

    if (_dmon.watches[index]) {
        pthread_mutex_lock(&_dmon.mutex);

        _dmon_unwatch(_dmon.watches[index]);
        DMON_FREE(_dmon.watches[index]);
        _dmon.watches[index] = NULL;

        --_dmon.num_watches;
        int num_freelist = DMON_MAX_WATCHES - _dmon.num_watches;
        _dmon.freelist[num_freelist - 1] = index;

        pthread_mutex_unlock(&_dmon.mutex);
    }
}

#endif

#endif    // DMON_IMPL
#endif  // __DMON_H__
