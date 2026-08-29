/* *****************************************************************************
 * contask.h v0.0.0000
 * 
 * running task concurrently in multithread
 * 
 * 
 * 
 * *****************************************************************************/
#include "util/contask.h"
#include "array/darray.h"

#define JOBS_CONTAINER 4096
#define LIMIT_WORKERS 8

static bool worker_busy[LIMIT_WORKERS];
#ifdef _WIN32
static HANDLE worker[LIMIT_WORKERS];
static CRITICAL_SECTION mtx;
static CONDITION_VARIABLE cnd;
static DWORD WINAPI contask_working(void*);
#else
#  include <pthread.h>
static pthread_t worker[LIMIT_WORKERS];
static pthread_mutex_t mtx;
static pthread_cond_t cnd;
static void *contask_working(void*);
#endif


static struct {
  contask_job *jobs;
  iter head, tail;
  bool close;
} contask__queue = {0};

void contask_init(void) {
  iter i;
  contask__queue.close = false;
  contask__queue.head = contask__queue.tail = 0;
  contask__queue.jobs = CAST(contask_job*)malloc(sizeof(contask_job) * JOBS_CONTAINER);
#ifdef _WIN32
  InitializeCriticalSection(&mtx);
  InitializeConditionVariable(&cnd);
  for (i = 0; i < LIMIT_WORKERS; ++i) {
    worker[i] = CreateThread(NULL, 0, contask_working, CAST(void*)worker_busy + i, 0, NULL);
    ASSERT(worker[i] != NULL && "fail to make thread.\n");
  }
#else
  pthread_mutex_init(&mtx, NULL);
  pthread_cond_init(&cnd, NULL);
  for (i = 0; i < LIMIT_WORKERS; ++i) {
    IS_ERROR (pthread_create(worker + i, NULL, contask_working, CAST(void*)worker_busy+i) != 0)
      ASSERT(false && "fail to make thread.\n");
  }
    
#endif
}
void contask_term(void) {
  iter i;
  // closing
  contask_lock();
  contask__queue.close = true;
  contask_unlock();
  contask_wakeall();
#ifdef _WIN32
  for (i = 0; i < LIMIT_WORKERS; ++i) {
    WaitForSingleObject(worker[i], INFINITE);
    CloseHandle(worker[i]);
  }
  DeleteCriticalSection(&mtx);
  cnd = 0;
#else
  for (i = 0; i < LIMIT_WORKERS; ++i) {
    pthread_join(worker[i], NULL);
  }
  pthread_mutex_destroy(&mtx);
  pthread_cond_destroy(&cnd);
#endif
  free(contask__queue.jobs);
  memset(&contask__queue, 0, sizeof(contask__queue));
}

inline void contask_pushjob(contask_job j) {
  contask_lock();
  while (contask__queue.tail >= JOBS_CONTAINER) {
    contask_signal();
    contask_wait();
  }
  contask__queue.jobs[contask__queue.tail++] = j;
  contask_unlock();
  contask_signal();
}
inline void contask_tillDone(void) {
  contask_lock();
  while (contask__queue.head < contask__queue.tail)
    contask_wait();
  contask_unlock();
}


inline void contask_lock(void) {
#ifdef _WIN32
  EnterCriticalSection(&mtx);
#else
  pthread_mutex_lock(&mtx);
#endif
}
inline void contask_unlock(void) {
#ifdef _WIN32
  LeaveCriticalSection(&mtx);
#else
  pthread_mutex_unlock(&mtx);
#endif
}

inline void contask_wait(void) {
#ifdef _WIN32
  SleepConditionVariableCS(&cnd, &mtx, INFINITE);
#else
  pthread_cond_wait(&cnd, &mtx);
#endif
}
inline void contask_signal(void) {
#ifdef _WIN32
  WakeConditionVariable(&cnd);
#else
  pthread_cond_signal(&cnd);
#endif
}
inline void contask_wakeall(void) {
#ifdef _WIN32
  WakeAllConditionVariable(&cnd);
#else
  pthread_cond_broadcast(&cnd);
#endif
}

#ifdef _WIN32
DWORD WINAPI
#else
void *
#endif
contask_working(void *p) {
  bool *busy = CAST(bool*)p;
  contask_job cj;
  for (;;) {
    contask_lock();
    if (contask__queue.close) {
      contask_unlock();
      break;
    } else if (contask__queue.head < contask__queue.tail) {
      cj = contask__queue.jobs[contask__queue.head++];
      *busy = true;
      contask_unlock();
      // running work
      cj.job(cj.param);
      contask_wakeall();
    } else if (
        (contask__queue.head > (JOBS_CONTAINER >> 2)) ||
        ((contask__queue.head > 0) && (contask__queue.tail >= JOBS_CONTAINER))
      ) {
      // optimize while doing nothing
      if (contask__queue.head < contask__queue.tail)
        memmove(contask__queue.jobs, contask__queue.jobs + contask__queue.head, sizeof(contask_job) * (contask__queue.tail - contask__queue.head));
      contask__queue.head -= contask__queue.head;
      contask__queue.tail -= contask__queue.head;
      *busy = true;
      contask_unlock();
      contask_wakeall();
    } else {
      // doing nothing
      *busy = false;
      contask_wait();
      contask_unlock();
    }
  }
#ifdef _WIN32
  return 0;
#else
  return NULL;
#endif
}