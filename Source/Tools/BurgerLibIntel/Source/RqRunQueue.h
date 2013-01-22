/*******************************

	Run Queue execution handler

*******************************/

#ifndef __RQRUNQUEUE_H__
#define __RQRUNQUEUE_H__

#ifndef __BRTYPES_H__
#include <BRTypes.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Timed code execution handler */

typedef Word (BURGERCALL *RunQueueProc_t)(void **,Word EventNum);	/* Run queue code */
typedef Word (BURGERCALL *RunQueuePollCallback_t)(void **);	/* Poll callback */

typedef struct RunQueue_t {
	struct RunQueue_t **NextQueue;		/* Next time event code routine in list */
	struct RunQueue_t **PrevQueue;		/* Previous entry for linked list */
	RunQueueProc_t Proc;	/* Event procedure */
	Word TimeQuantum;		/* Time in ticks before execution */
	Word ElapsedTime;		/* Fractional time in ticks */
	Word Priority;			/* Execution priority */
	Word IDNum;				/* ID used by application (-1 for master) */
} RunQueue_t;

enum {RQTIME,RQINIT,RQDESTROY,RQSAVE,RQLOAD,RQDEBUG,RQUSER};

extern RunQueue_t ** BURGERCALL MasterRunQueueNew(void);
extern void BURGERCALL MasterRunQueueInit(RunQueue_t **MasterQueueHandle);
extern void BURGERCALL MasterRunQueueDelete(RunQueue_t **MasterQueueHandle);
extern void BURGERCALL MasterRunQueueDestroy(RunQueue_t **MasterQueueHandle);
extern void BURGERCALL MasterRunQueueExecute(RunQueue_t **MasterQueueHandle,Word TimeQuantum);
extern void BURGERCALL MasterRunQueueDump(RunQueue_t **MasterQueueHandle);

extern Word BURGERCALL RunQueueCallProc(RunQueue_t **RunQueueHandle,Word Event);
extern Word BURGERCALL RunQueueDefaultProc(void **RunQueueHandle,Word Event);
extern RunQueue_t ** BURGERCALL RunQueueNew(RunQueue_t **MasterQueueHandle,Word MemSize,
	Word TimeQuantum,Word Priority,Word IDNum,RunQueueProc_t Proc);
extern void BURGERCALL RunQueueDelete(RunQueue_t **RunQueueHandle);
extern Word BURGERCALL RunQueueDeleteProc(void **RunQueueHandle,Word Event);
extern void BURGERCALL RunQueueDeleteDefer(RunQueue_t **RunQueueHandle);
extern void BURGERCALL RunQueueLink(RunQueue_t **MasterQueueHandle,RunQueue_t **RunQueueHandle,Word Priority);
extern void BURGERCALL RunQueueUnlink(RunQueue_t **RunQueueHandle);
extern RunQueue_t ** BURGERCALL RunQueueFindIDNum(RunQueue_t **RunQueueHandle,Word IDNum);
extern Word BURGERCALL RunQueuePoll(RunQueue_t **MasterQueueHandle,RunQueuePollCallback_t CallBack,Word IDNum);

#ifdef __cplusplus
}
#endif


#endif

