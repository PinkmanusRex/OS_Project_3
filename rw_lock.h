#include <pthread.h>

struct rw_lock {
    unsigned int no_readers;
    unsigned int no_writers;
    unsigned int no_writing;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

void __init_rw_lock(struct rw_lock *rw);

void __destroy_rw_lock(struct rw_lock *rw);

/* a shared read lock */
void __lock_r_rw_lock(struct rw_lock *rw);

/* unregisters this read lock */
void __unlock_r_rw_lock(struct rw_lock *rw);

/* a mutually exclusive write lock */
void __lock_w_rw_lock(struct rw_lock *rw);

void __unlock_w_rw_lock(struct rw_lock *rw);
