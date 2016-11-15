#ifndef _LINUX_RATELIMIT_H
#define _LINUX_RATELIMIT_H

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

#define DEFAULT_RATELIMIT_INTERVAL    (1024)
#define DEFAULT_RATELIMIT_BURST        10000
#define SECOND_TO_USECONDS             1000000

struct ratelimit_state {
	 /* protect the state */
#ifdef USE_SPINLOCK
	pthread_spinlock_t lock;
#else
	pthread_mutex_t lock;
#endif

    int         interval;
    int         burst;
    int         printed;
    int         missed;
    uint64_t    begin;
};

static inline void ratelimit_state_init(struct ratelimit_state *rs,
                    int interval, int burst) {
    memset(rs, 0, sizeof(*rs));
#ifdef USE_SPINLOCK
	pthread_spin_init(&rs->lock, PTHREAD_PROCESS_PRIVATE);
#else
	pthread_mutex_init(&rs->lock, NULL);
#endif
    rs->interval   = interval;
    rs->burst       = burst;
}

static inline void ratelimit_default_init(struct ratelimit_state *rs) {
    return ratelimit_state_init(rs, DEFAULT_RATELIMIT_INTERVAL,
                    DEFAULT_RATELIMIT_BURST);
}

static inline void ratelimit_state_exit(struct ratelimit_state *rs) {
#ifdef USE_SPINLOCK
	pthread_spin_destroy(&rs->lock);
#else
	pthread_mutex_destroy(&rs->lock);
#endif
    if (rs->missed) {
        rs->missed = 0;
    }
}

/*
 * ratelimit - rate limiting
 * @rs: ratelimit_state data
 * @func: name of calling function
 *
 * This enforces a rate limit: not more than @rs->burst callbacks
 * in every @rs->interval
 *
 * RETURNS:
 * 0 means callbacks will be suppressed.
 * 1 means go ahead and do it.
 */
static inline int ratelimit(struct ratelimit_state *rs) {
    int ret;

    if (!rs->interval)
        return 1;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t now_usec = tv.tv_sec * SECOND_TO_USECONDS + tv.tv_usec;

    /*
     * If we contend on this state's lock then almost
     * by definition we are too busy to print a message,
     * in addition to the one that will be printed by
     * the entity that is holding the lock already:
     */
#ifdef USE_SPINLOCK
	pthread_spin_lock(&rs->lock);
#else
	pthread_mutex_lock(&rs->lock);
#endif

    if (!rs->begin)
        rs->begin = now_usec;

    if (now_usec - rs->begin > rs->interval) {
        rs->begin   = now_usec;
        rs->printed = 0;
        rs->missed  = 0;
    }
    if (rs->burst && rs->burst > rs->printed) {
        rs->printed++;
        ret = 1;
    } else {
        rs->missed++;
        ret = 0;
    }

#ifdef USE_SPINLOCK
	pthread_spin_unlock(&rs->lock);
#else
	pthread_mutex_unlock(&rs->lock);
#endif

    return ret;
}

#endif