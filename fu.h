#pragma once

#ifndef SKIP_DEFAULT_INCLUDES_FU
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#endif

/* not sure why this isn't NDEBUG */
#ifdef RELEASE
#define DEBUG_FU 0
#else
#define DEBUG_FU 1
#endif

// make these macros functions

#define debugf(fmt, ...) \
    do { if (DEBUG_FU) fprintf(stderr, "D %s:%d: " fmt "\n", __FILE__, \
            __LINE__, __VA_ARGS__); } while (0)

#define debug(msg) \
    do { if (DEBUG_FU) fprintf(stderr, "D %s:%d:%s\n", __FILE__, \
            __LINE__, msg); } while (0)

#define errorf(fmt, ...) \
        do { fprintf(stderr, "E %s:%d: " fmt "\n", __FILE__, \
                            __LINE__, __VA_ARGS__); } while (0)

#define error(msg) \
        do { fprintf(stderr, "E %s:%d:%s\n",  __FILE__, \
                            __LINE__, msg); } while (0)

#define infof(fmt, ...) \
        do { fprintf(stderr, "I %s:%d: " fmt "\n", __FILE__, \
                            __LINE__, __VA_ARGS__); } while (0)

#define info(msg) \
        do { fprintf(stderr, "I %s:%d:%s\n", __FILE__, \
                            __LINE__, msg); } while (0)


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef uint32_t b32;

/* timespec stuff is included for crude performance timing */

void
debug_timespec(const struct timespec *ts, const char *label)
{
    assert(label != NULL);

    debugf("%s: %ld.%09ld", label, ts->tv_sec, ts->tv_nsec);
}

// return c = a - b
void
sub_timespec(const struct timespec *a, const struct timespec *b, struct timespec *c) {
    c->tv_sec = a->tv_sec - b->tv_sec;
    if (a->tv_nsec < b->tv_nsec) {
        c->tv_sec--;
        c->tv_nsec = (a->tv_nsec + 1000000000) - b->tv_nsec;
    } else {
        c->tv_nsec = a->tv_nsec - b->tv_nsec;
    }
}

void
incr_ns_timespec(struct timespec *ts, long ns) {
    ts->tv_nsec += ns;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_nsec -= 1000000000;
        ts->tv_sec += 1;
    }
}

void
elapsed_debug(struct timespec *last, struct timespec *elapsed, char *label)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (last->tv_sec > 0 && elapsed) {
        sub_timespec(&now, last, elapsed);
        debugf("%s: elapsed %ld.%09ld", label ? label : "no label", elapsed->tv_sec, elapsed->tv_nsec);
    }

    *last = now;
}

b32
gt_timespec(struct timespec *a, struct timespec *b)
{
    return a->tv_sec > b->tv_sec || (a->tv_sec == b->tv_sec && a->tv_nsec > b->tv_nsec);
}

int
errno_return(const char *msg)
{
    errorf("%s: %s", msg, strerror(errno));
    return -errno;
}

int
errno_error(int result, const char *msg)
{
    if (result == -1) {
        errorf("%s: %s", msg, strerror(errno));
        return 1;
    }

    return 0;
}

void
exit_errno(int result, const char *msg)
{
    if (result == -1) exit(errno_return(msg));
}

