#ifndef NITRO_FS_ROM_H_
#define NITRO_FS_ROM_H_

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/card/hash.h>
#include <nitro/fs/file.h>
#include <nitro/fs/archive.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(FS_IMPLEMENT)

void FSi_InitRomArchive(u32 default_dma_no);

void FSi_EndRomArchive(void);

BOOL FSi_MountSRLFile(FSArchive *arc, FSFile *file, CARDRomHashContext *hash);

void FSi_ConvertPathToFATFS(char *dst, const char *src, BOOL ignorePermission);

FSResult FSi_ConvertError(u32 error);

BOOL FSi_MountFATFS(u32 index, const char *arcname, const char *drivename);

void FSi_MountDefaultArchives(void);

#else /* FS_IMPLEMENT */

void FSi_ReadRomDirect(const void *src, void *dst, u32 len);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_FS_ROM_H_ */
