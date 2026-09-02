#ifndef DEMO_UTILITY_H_
#define DEMO_UTILITY_H_
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void DEMO_Set3DDefaultMaterial(BOOL bUsediffuseAsVtxCol, BOOL bUseShininessTbl);
void DEMO_Set3DDefaultShininessTable();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DEMO_UTILITY_H_ */
