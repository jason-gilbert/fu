#pragma once

// batch logs.
// no allocs on adding log msg.
// delay formatting until written
// explicitly write logs to output, for example all logs for a request after the response is sent

#define MAX_ENTRIES_LOG 64
#define MAX_MSG_LOG 1024
#define MAX_LABEL_LOG 64

// file and line number?
// should there be an integer, float/double, or number value stored for logging values
// that could be searched?
typedef struct {
    struct timespec timestamp;
    s32 error;
    // short descriptor like a variable name, etc. keeps it separate from the
    // value which may make it more searchable and avoids having to format the
    // msg when adding an entry.
    char label[MAX_LABEL_LOG];
    char msg[MAX_MSG_LOG];

    char * file;
    char * function;
    s64 line;
} log_entry_t;

typedef struct {
    // what size should this be?  doesn't really matter I guess
    size_t size;
    log_entry_t entries[MAX_ENTRIES_LOG];
} log_t;

void
erase_log(log_t * log)
{
    memset(log, 0, sizeof(log_t));
}

// get a timestamp using the same clock as the log.  Useful when logging
// elapsed times
void
timestamp_log(log_t * log, struct timespec * ts)
{
    (void)log; // ignore for now, but possibly store clock_id in log_t
    clock_gettime(CLOCK_REALTIME, ts);
}

// file and function aren't copied as it's assumed they're using the macros
void
_log(log_t * log, const void * msg, size_t size_msg, const char * label, s32 error, const char * file, const char * function, s64 line)
{
    if (log->size < MAX_ENTRIES_LOG) {
        log_entry_t * entry = &log->entries[log->size];

        timestamp_log(log, &entry->timestamp);

        copy_cstr(entry->msg, MAX_MSG_LOG, msg, size_msg);

        if (label) {
            copy_cstr(entry->label, MAX_LABEL_LOG, label, strlen(label));
        }

        entry->error = error;

        entry->file = (char *)file;
        entry->function = (char *)function;
        entry->line = line;
    }

    log->size++;
}

#define error_log(log, msg, label, error) \
    _log(log, msg, strlen(msg), label, error, __FILE__, __func__, __LINE__);

#define info_log(log, msg, label) \
    _log(log, msg, strlen(msg), label, 0, __FILE__, __func__, __LINE__);

#define log_errno(log, label) \
    _log_errno(log, label, __FILE__, __func__, __LINE__);

void
_log_errno(log_t * log, const char * label, const char * file, const char * function, s64 line)
{
    char s[256];
    strerror_r(errno, s, 256);

    _log(log, s, strlen(s), label, errno, file, function, line);
}

void
log_s64(log_t * log, const s64 value, const char * label)
{
    char s[256];
    // XXX: seems kind of sucky to use snprintf for number conversion
    int size = snprintf(s, 256, "%ld", value);
    _log(log, s, size, label, 0, NULL, NULL, 0);
}

void
log_u64(log_t * log, const u64 value, const char * label)
{
    char s[256];
    // XXX: seems kind of sucky to use snprintf for number conversion
    int size = snprintf(s, 256, "%lu", value);
    _log(log, s, size, label, 0, NULL, NULL, 0);
}

// log an elapsed time since the provided start time and now
#define elapsed_log(log, start, label) \
    _elapsed_log(log, start, label, __FILE__, __func__, __LINE__);

void
_elapsed_log(log_t * log, const struct timespec start, const char * label, const char * file, const char * function, s64 line)
{
    struct timespec now;
    timestamp_log(log, &now);

    struct timespec elapsed;

    sub_timespec(&now, &start, &elapsed);

    char msg[2048];
    size_t size = snprintf(msg, 2048, "elapsed %lld.%09ld", (long long)elapsed.tv_sec, elapsed.tv_nsec);

    _log(log, msg, size, label, 0, file, function, line);
}

// TODO(jason): I'm a bit skeptical about the resolution of the timestamps and
// if it's useful/necessary to theoretically have nanosecond precision
void
write_log(log_t * log, int fd)
{
    const int max_msg = 2048;

    char msg[max_msg];

    // TODO(jason): change this to write everything into a single buffer and write once
    for (size_t i = 0; i < log->size; i++) {
        log_entry_t * entry = &log->entries[i];
        size_t size;
        if (entry->label[0]) {
            size = snprintf(msg, max_msg, "%.3d %lld.%.9ld %s:%s:%zd %s: %s\n",
                    entry->error, (long long)entry->timestamp.tv_sec, entry->timestamp.tv_nsec, entry->file, entry->function, entry->line, entry->label, entry->msg);
        } else {
            size = snprintf(msg, max_msg, "%.3d %lld.%.9ld %s:%s:%zd %s\n",
                    entry->error, (long long)entry->timestamp.tv_sec, entry->timestamp.tv_nsec, entry->file, entry->function, entry->line, entry->msg);
        }
        ssize_t written = write(fd, msg, size);
        assert(written != -1);
    }
}

