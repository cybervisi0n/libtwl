#ifndef TWL_CHT_ARM9_PICTOCATCH_H_
#define TWL_CHT_ARM9_PICTOCATCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/wm.h>

BOOL CHT_IsPictochatParent(const WMBssDesc *pWmBssDesc);

int CHT_GetPictochatClientNum(const WMBssDesc *pWmBssDesc);

int CHT_GetPictochatRoomNumber(const WMBssDesc *pWmBssDesc);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CHT_ARM9_PICTOCATCH_H_ */
