#ifndef NITRO_CTRDG_TASK_H_
#define NITRO_CTRDG_TASK_H_

#include <nitro.h>

#define CTRDG_TASK_PRIORITY_DEFAULT                                            \
  20 // Enables the creation of multiple threads between the task and main
     // threads, with lower priorities than that of the main thread.

#define CTRDG_TASK_STACK_SIZE 1024

struct CTRDGTaskInfo_tag;

typedef u32 (*CTRDG_TASK_FUNC)(struct CTRDGTaskInfo_tag *);

typedef struct CTRDGTaskInfo_tag {

  CTRDG_TASK_FUNC task;     /* Task function */
  CTRDG_TASK_FUNC callback; /* Callback */
  u32 result;               /* The task function's return value. */
  u8 *data;    /* The data to be written; only program commands can be used. */
  u8 *adr;     /* Address of the data to be read/written. */
  u32 offset;  /* The offset, in bytes, within the sector */
  u32 size;    /* Size */
  u8 *dst;     /* Address of the work region where the read data is stored */
  u16 sec_num; /* Sector number */
  u8 busy;     /* If now processing */
  u8 param[1]; /* User-defined argument and return-value */
} CTRDGTaskInfo;

typedef struct {
  OSThread th[1];               /* Thread context */
  CTRDGTaskInfo *volatile list; /* Waiting task list */
  CTRDGTaskInfo end_task;       /* Task structure for end-command */
} CTRDGiTaskWork;

void CTRDGi_InitTaskThread(void *p_work);

BOOL CTRDGi_IsTaskAvailable(void);

void CTRDGi_InitTaskInfo(CTRDGTaskInfo *pt);

BOOL CTRDGi_IsTaskBusy(volatile const CTRDGTaskInfo *pt);

void CTRDGi_SetTask(CTRDGTaskInfo *pt, CTRDG_TASK_FUNC task,
                    CTRDG_TASK_FUNC callback);

void CTRDGi_EndTaskThread(CTRDG_TASK_FUNC callback);

void CTRDG_SetTaskThreadPriority(u32 priority);

#endif /* NITRO_CTRDG_TASK_H_ */
