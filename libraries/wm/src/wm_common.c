#include "wm_common.h"

WMErrCode WM_CheckInitialized(void) { return WMi_CheckInitialized(); }

#if 0

NWMRetCode NWM_CheckInitialized(void)
{
#ifdef SDK_TWL
	if(OS_IsRunOnTwl()) //Check the old wireless library state only when running on a TWL system
	{
		return NWMi_CheckInitialized();
	}
#endif

	return NWM_RETCODE_ILLEGAL_STATE;
}
#endif
