#include "rw_lock.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void __init_rw_lock(struct rw_lock *rw) {
    rw->no_readers = 0;
    rw->no_writers = 0;
    rw->no_writing = 0;
    if (pthread_mutex_init(&rw->lock, NULL)) {
        perror("couldn't create lock\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&rw->cond, NULL)) {
        perror("couldn't create condition variable\n");
        exit(EXIT_FAILURE);
    }
}

void __destroy_rw_lock(struct rw_lock *rw) {
    rw->no_readers = 0;
    rw->no_writers = 0;
    rw->no_writing = 0;
    pthread_mutex_destroy(&rw->lock);
    pthread_cond_destroy(&rw->cond);
}

void __lock_r_rw_lock(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    while (rw->no_writers)
        pthread_cond_wait(&rw->cond, &rw->lock);
    rw->no_readers += 1U;
    pthread_mutex_unlock(&rw->lock);
}

void __unlock_r_rw_lock(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->no_readers -= 1U;
    pthread_cond_broadcast(&rw->cond);
    pthread_mutex_unlock(&rw->lock);
}

void __lock_w_rw_lock(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->no_writers += 1U;
    while (rw->no_writing || rw->no_readers)
        pthread_cond_wait(&rw->cond, &rw->lock);
    rw->no_writing = 1U;
    pthread_mutex_unlock(&rw->lock);
}

void __unlock_w_rw(struct rw_lock *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->no_writing = 0;
    rw->no_writers -= 1U;
    pthread_cond_broadcast(&rw->cond);
    pthread_mutex_unlock(&rw->lock);
}
