/* *****************************************************************************
 * contask.h v0.0.0000
 * 
 * running task concurrently in multithread
 * 
 * 
 * 
 * *****************************************************************************/
#ifndef __CONTASK_INCLUDED__
#define __CONTASK_INCLUDED__

#include "common.h"

typedef struct {
  void *param;
  void(*job)(void*);
} contask_job;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void contask_init(void);
void contask_term(void);

void contask_pushjob(contask_job);
void contask_tillDone(void);

void contask_lock(void);
void contask_unlock(void);
void contask_wait(void);
void contask_signal(void);
void contask_wakeall(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CONTASK_INCLUDED__