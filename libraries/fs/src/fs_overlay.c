#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os.h>
#include <nitro/mi.h>

#include <nitro/fs.h>
#include <nitro/math/dgt.h>

#include "../include/util.h"
#include "../include/rom.h"

typedef void (*MWI_DESTRUCTOR_FUNC)(void *p_this);

typedef struct MWiDestructorChain {
  struct MWiDestructorChain *next;
  MWI_DESTRUCTOR_FUNC dtor;
  void *obj;
} MWiDestructorChain;

typedef enum { OVERLAYPROC_ARM9, OVERLAYPROC_ARM7 } OVERLAYPROC;

#define FS_OVERLAY_FLAG_COMP 0x0001
#define FS_OVERLAY_FLAG_AUTH 0x0002
#define FS_OVERLAY_DIGEST_SIZE MATH_SHA1_DIGEST_SIZE

extern u8 SDK_OVERLAYTABLE_DIGEST[];
#ifdef SDK_TWL
extern u8 SDK_LTDOVERLAYTABLE_DIGEST[];
#else
#define SDK_LTDOVERLAYTABLE_DIGEST NULL
#endif // SDK_TWL

extern u8 SDK_OVERLAY_DIGEST[];
extern u8 SDK_OVERLAY_DIGEST_END[];

extern u8 SDK_OVERLAY_NUMBER[];
#ifdef SDK_TWL
extern u8 SDK_LTDOVERLAY_NUMBER[];
#else
#define SDK_LTDOVERLAY_NUMBER 0
#endif // SDK_TWL
#define FS_OVERLAY_NTR_TOTAL (u32) SDK_OVERLAY_NUMBER
#define FS_OVERLAY_TWL_TOTAL (u32) SDK_LTDOVERLAY_NUMBER

static const u8 fsi_def_digest_key[64] = {
    0x21, 0x06, 0xc0, 0xde, 0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d, 0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08, 0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf, 0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c, 0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23, 0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe, 0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37, 0x87, 0x46, 0x58, 0x24,
};

extern MWiDestructorChain *__global_destructor_chain;

typedef struct FSOverlaySource {

  FSArchive *arc;

  CARDRomRegion ovt9;
  CARDRomRegion ovt7;

  const void *digest_key_ptr;
  u32 digest_key_len;
} FSOverlaySource;

static FSOverlaySource FSiOverlayContext;

#if defined(SDK_FINALROM)

#define FSi_RegisterOverlayToDebugger(ovi) (void)0
#define FSi_UnregisterOverlayToDebugger(ovi) (void)0

#else // defined (SDK_FINALROM)

int _ISDbgLib_RegistOverlayInfo(OVERLAYPROC nProc, u32 nAddrRAM, u32 nAddrROM,
                                u32 nSize);
int _ISTDbgLib_RegistOverlayInfoByAddr(OVERLAYPROC nProc, u32 nAddrRAM,
                                       u32 nSlot, u32 nAddrROM, u32 nSize);
int _ISDbgLib_UnregistOverlayInfo(OVERLAYPROC nProc, u32 nAddrRAM, u32 nSize);
int _ISTDbgLib_UnregistOverlayInfoByAddr(OVERLAYPROC nProc, u32 nAddrRAM,
                                         u32 nSize);

static int DUMMY_REGISTER(OVERLAYPROC nProc, u32 nAddrRAM, u32 nAddrROM,
                          u32 nSize) {
  (void)nProc;
  (void)nAddrRAM;
  (void)nAddrROM;
  (void)nSize;
  return 0;
}
static int DUMMY_UNREGISTER(OVERLAYPROC nProc, u32 nAddrRAM, u32 nSize) {
  (void)nProc;
  (void)nAddrRAM;
  (void)nSize;
  return 0;
}

#define FS_REGISTER_OVERLAY_INFO(nProc, nAddrRAM, nAddrROM, nSize)             \
  ((OS_GetConsoleType() & OS_CONSOLE_TWLDEBUGGER)                              \
       ? _ISTDbgLib_RegistOverlayInfoByAddr(nProc, nAddrRAM, 0, nAddrROM,      \
                                            nSize)                             \
   : (OS_GetConsoleType() & OS_CONSOLE_ISDEBUGGER)                             \
       ? _ISDbgLib_RegistOverlayInfo(nProc, nAddrRAM, nAddrROM, nSize)         \
       : DUMMY_REGISTER(nProc, nAddrRAM, nAddrROM, nSize))
#define FS_UNREGISTER_OVERLAY_INFO(nProc, nAddrRAM, nSize)                     \
  ((OS_GetConsoleType() & OS_CONSOLE_TWLDEBUGGER)                              \
       ? _ISTDbgLib_UnregistOverlayInfoByAddr(nProc, nAddrRAM, nSize)          \
   : (OS_GetConsoleType() & OS_CONSOLE_ISDEBUGGER)                             \
       ? _ISDbgLib_UnregistOverlayInfo(nProc, nAddrRAM, nSize)                 \
       : DUMMY_UNREGISTER(nProc, nAddrRAM, nSize))

SDK_INLINE void FSi_RegisterOverlayToDebugger(const FSOverlayInfo *ovi) {
  const OVERLAYPROC proc =
      (ovi->target == MI_PROCESSOR_ARM9) ? OVERLAYPROC_ARM9 : OVERLAYPROC_ARM7;
  const u32 address = (u32)FS_GetOverlayAddress(ovi);
  const u32 offset = ovi->file_pos.offset;
  const u32 length = FS_GetOverlayImageSize(ovi);
  (void)FS_REGISTER_OVERLAY_INFO(proc, address, offset, length);
}

SDK_INLINE void FSi_UnregisterOverlayToDebugger(const FSOverlayInfo *ovi) {
  const OVERLAYPROC proc =
      (ovi->target == MI_PROCESSOR_ARM9) ? OVERLAYPROC_ARM9 : OVERLAYPROC_ARM7;
  const u32 address = (u32)FS_GetOverlayAddress(ovi);
  const u32 length = FS_GetOverlayImageSize(ovi);
  (void)FS_UNREGISTER_OVERLAY_INFO(proc, address, length);
}
#endif // defined (SDK_FINALROM)

void __register_global_object(void *obj, MWI_DESTRUCTOR_FUNC dtor,
                              MWiDestructorChain *chain);

void FSi_InitOverlay(void) {

  if (OS_GetBootType() == OS_BOOTTYPE_DOWNLOAD_MB) {
    FSiOverlayContext.ovt9.offset = (u32)~0;
    FSiOverlayContext.ovt9.length = 0;
    FSiOverlayContext.ovt7.offset = (u32)~0;
    FSiOverlayContext.ovt7.length = 0;
  } else {
    FSiOverlayContext.ovt9.offset = 0;
    FSiOverlayContext.ovt9.length = 0;
    FSiOverlayContext.ovt7.offset = 0;
    FSiOverlayContext.ovt7.length = 0;
  }
  FSiOverlayContext.digest_key_ptr = fsi_def_digest_key;
  FSiOverlayContext.digest_key_len = sizeof(fsi_def_digest_key);
  FSiOverlayContext.arc = FS_FindArchive("rom", 3);
}

static u32 FSi_GetOverlayBinarySize(const FSOverlayInfo *p_ovi) {
  u32 size = (((p_ovi->header.flag & FS_OVERLAY_FLAG_COMP) != 0)
                  ? p_ovi->header.compressed
                  : p_ovi->header.ram_size);
  return size;
}

void FS_ClearOverlayImage(FSOverlayInfo *p_ovi) {
  u8 *const addr = FS_GetOverlayAddress(p_ovi);
  const u32 image_size = FS_GetOverlayImageSize(p_ovi);
  const u32 total_size = FS_GetOverlayTotalSize(p_ovi);
#if defined(SDK_ARM9)
  IC_InvalidateRange(addr, total_size);
  DC_InvalidateRange(addr, total_size);
#endif
  MI_CpuFill8(addr + image_size, 0, total_size - image_size);
}

#if defined(FS_IMPLEMENT)

FSFileID FS_GetOverlayFileID(const FSOverlayInfo *p_ovi) {
  FSFileID ret;
  ret.arc = FSiOverlayContext.arc;
  ret.file_id = p_ovi->header.file_id;
  return ret;
}

BOOL FS_LoadOverlayInfo(FSOverlayInfo *p_ovi, MIProcessor target,
                        FSOverlayID id) {
  BOOL retval = FALSE;
  const u32 pos = (u32)id * sizeof(FSOverlayInfoHeader);
  const CARDRomRegion *pr = (target == MI_PROCESSOR_ARM9)
                                ? &FSiOverlayContext.ovt9
                                : &FSiOverlayContext.ovt7;
#if !defined(SDK_NITRO)

  if ((id >= FS_OVERLAY_NTR_TOTAL) && !OS_IsRunOnTwl()) {
    OS_TWarning("TWL-overlay is not available on NTR-mode.\n");
  } else
#endif // !defined(SDK_NITRO)
  {
    if (pr->offset) {
      if (pos < pr->length) {
        FSFile file[1];
        FS_InitFile(file);
        MI_CpuCopy8((const void *)(pr->offset + pos), p_ovi,
                    sizeof(FSOverlayInfoHeader));
        {
          {
            p_ovi->target = target;
            if (FS_OpenFileFast(file, FS_GetOverlayFileID(p_ovi))) {
              p_ovi->file_pos.offset = FS_GetFileImageTop(file);
              p_ovi->file_pos.length = FS_GetFileLength(file);
              (void)FS_CloseFile(file);
              retval = TRUE;
            }
          }
        }
      }
    } else {
      pr = (target == MI_PROCESSOR_ARM9)
               ? CARD_GetRomRegionOVT(MI_PROCESSOR_ARM9)
               : CARD_GetRomRegionOVT(MI_PROCESSOR_ARM7);
      if (pos < pr->length) {
        FSFile file[1];
        FS_InitFile(file);
        if (FS_CreateFileFromRom(file, pr->offset + pos,
                                 pr->offset + pr->length)) {
          if (FS_ReadFile(file, p_ovi, sizeof(FSOverlayInfoHeader)) !=
              sizeof(FSOverlayInfoHeader)) {
            (void)FS_CloseFile(file);
          } else {
            (void)FS_CloseFile(file);
            p_ovi->target = target;
            if (FS_OpenFileFast(file, FS_GetOverlayFileID(p_ovi))) {
              p_ovi->file_pos.offset = FS_GetFileImageTop(file);
              p_ovi->file_pos.length = FS_GetFileLength(file);
              (void)FS_CloseFile(file);
              retval = TRUE;
            }
          }
        }
      }
    }
  }
  return retval;
}

BOOL FS_LoadOverlayImageAsync(FSOverlayInfo *p_ovi, FSFile *p_file) {
  BOOL retval = FALSE;
  FS_InitFile(p_file);
  if (FS_OpenFileFast(p_file, FS_GetOverlayFileID(p_ovi))) {
    s32 size = (s32)FSi_GetOverlayBinarySize(p_ovi);
    FS_ClearOverlayImage(p_ovi);
    if (FS_ReadFileAsync(p_file, FS_GetOverlayAddress(p_ovi), size) == size) {
      retval = TRUE;
    } else {
      (void)FS_CloseFile(p_file);
    }
  }
  return retval;
}

BOOL FS_LoadOverlayImage(FSOverlayInfo *p_ovi) {
  BOOL retval = FALSE;
  FSFile p_file[1];
  FS_InitFile(p_file);
  if (FS_OpenFileFast(p_file, FS_GetOverlayFileID(p_ovi))) {
    s32 size = (s32)FSi_GetOverlayBinarySize(p_ovi);
    FS_ClearOverlayImage(p_ovi);
    if (FS_ReadFile(p_file, FS_GetOverlayAddress(p_ovi), size) == size) {
      retval = TRUE;
    }
    (void)FS_CloseFile(p_file);
  }
  return retval;
}

#else

BOOL FS_LoadOverlayInfo(FSOverlayInfo *p_ovi, MIProcessor target,
                        FSOverlayID id) {
  BOOL retval = FALSE;
  const u32 pos = (u32)id * sizeof(FSOverlayInfoHeader);
  const CARDRomRegion *pr;
  pr = (target == MI_PROCESSOR_ARM9) ? &FSiOverlayContext.ovt9
                                     : &FSiOverlayContext.ovt7;
  if (pr->offset) {
    if (pos < pr->length) {
      MI_CpuCopy8((const void *)(pr->offset + pos), p_ovi,
                  sizeof(FSOverlayInfoHeader));
      p_ovi->target = target;
      {
        FSFile file[1];
        FS_InitFile(file);
        if (FS_OpenFileFast(file, FS_GetOverlayFileID(p_ovi))) {
          p_ovi->file_pos.offset = FS_GetFileImageTop(file);
          p_ovi->file_pos.length = FS_GetLength(file);
          (void)FS_CloseFile(file);
          retval = TRUE;
        }
      }
    }
  } else {
    pr = CARD_GetRomRegionOVT(target);
    if (pos < pr->length) {
      FSi_ReadRomDirect((const void *)(pr->offset + pos), p_ovi,
                        sizeof(FSOverlayInfoHeader));
      p_ovi->target = target;
      {
        retval = TRUE;
      }
    }
  }
  return retval;
}

BOOL FS_LoadOverlayImage(FSOverlayInfo *p_ovi) {
  BOOL retval = FALSE;
  const CARDRomRegion *fat = CARD_GetRomRegionFAT();
  u32 pos = p_ovi->header.file_id * sizeof(FSArchiveFAT);
  if (pos < fat->length) {
    FSArchiveFAT fat_info;
    FSi_ReadRomDirect((const void *)(fat->offset + pos), &fat_info,
                      sizeof(FSArchiveFAT));
    FS_ClearOverlayImage(p_ovi);
    FSi_ReadRomDirect((const void *)(fat_info.top), p_ovi->header.ram_address,
                      fat_info.bottom - fat_info.top);
    retval = TRUE;
  }
  return retval;
}

#endif /* FS_IMPLEMENT */

static BOOL FSi_CompareDigest(const u8 *spec_digest, void *src, int len,
                              BOOL table_mode) {
  int i;
  u8 digest[FS_OVERLAY_DIGEST_SIZE];
  u8 digest_key[64];

  MI_CpuClear8(digest, sizeof(digest));
  MI_CpuCopy8(FSiOverlayContext.digest_key_ptr, digest_key,
              FSiOverlayContext.digest_key_len);

#ifdef SDK_TWL
  if (!table_mode && OS_IsRunOnTwl()) {
    SVC_CalcHMACSHA1(digest, src, (u32)len, digest_key,
                     FSiOverlayContext.digest_key_len);
  } else
#endif
  {

    int bak_ovt_mode = FALSE;
    if (table_mode) {
      bak_ovt_mode = MATHi_SetOverlayTableMode(TRUE);
    }
    MATH_CalcHMACSHA1(digest, src, (u32)len, digest_key,
                      FSiOverlayContext.digest_key_len);
    if (table_mode) {
      (void)MATHi_SetOverlayTableMode(bak_ovt_mode);
    }
  }
  for (i = 0; i < sizeof(digest); i += sizeof(u32)) {
    if (*(const u32 *)(digest + i) != *(const u32 *)(spec_digest + i)) {
      break;
    }
  }
  return (i == sizeof(digest));
}

void FS_StartOverlay(FSOverlayInfo *p_ovi) {
  u32 rare_size = FSi_GetOverlayBinarySize(p_ovi);

#ifdef SDK_TWL

  {
    extern const u8 SDK_LTDAUTOLOAD_LTDMAIN_START[];
    extern const u8 SDK_LTDAUTOLOAD_LTDMAIN_END[];
    extern const u8 SDK_LTDAUTOLOAD_LTDMAIN_BSS_END[];
    if ((p_ovi->header.ram_address >= SDK_LTDAUTOLOAD_LTDMAIN_START) &&
        (p_ovi->header.ram_address < SDK_LTDAUTOLOAD_LTDMAIN_BSS_END) &&
        OS_IsRunOnTwl()) {
      static BOOL once = FALSE;
      if (!once) {
        OS_TWarning(
            "specified overlay(%d) might have destroyed LTDMAIN segment!\n"
            "(please move LTDMAIN after overlay(%d))",
            p_ovi->header.id, p_ovi->header.id);
        once = TRUE;
      }
    }
  }
#endif

  if (OS_GetBootType() != OS_BOOTTYPE_ROM) {
    BOOL ret = FALSE;

    if ((p_ovi->header.flag & FS_OVERLAY_FLAG_AUTH) != 0) {
      const u32 odt_max = (u32)((SDK_OVERLAY_DIGEST_END - SDK_OVERLAY_DIGEST) /
                                FS_OVERLAY_DIGEST_SIZE);
      if (p_ovi->header.id < odt_max) {
        const u8 *spec_digest =
            (SDK_OVERLAY_DIGEST + FS_OVERLAY_DIGEST_SIZE * p_ovi->header.id);
        ret = FSi_CompareDigest(spec_digest, p_ovi->header.ram_address,
                                (int)rare_size, FALSE);
      }
    }
    if (!ret) {
      MI_CpuClear8(p_ovi->header.ram_address, rare_size);
      OS_TPanic("FS_StartOverlay() failed! (invalid overlay-segment data)");
      return;
    }
  }

  if ((p_ovi->header.flag & FS_OVERLAY_FLAG_COMP) != 0) {
    MIi_UncompressBackward(p_ovi->header.ram_address + rare_size);
  }
#if defined(SDK_ARM9)
  DC_FlushRange(FS_GetOverlayAddress(p_ovi), FS_GetOverlayImageSize(p_ovi));
#endif
  FSi_RegisterOverlayToDebugger(p_ovi);

  {
    FSOverlayInitFunc *p = p_ovi->header.sinit_init;
    FSOverlayInitFunc *q = p_ovi->header.sinit_init_end;
    for (; p < q; ++p) {
      if (*p) {
        (**p)();
      }
    }
  }
}

void FS_EndOverlay(FSOverlayInfo *p_ovi) {
  for (;;) {

    MWiDestructorChain *head = NULL, *tail = NULL;
    const u32 region_top = (u32)FS_GetOverlayAddress(p_ovi);
    const u32 region_bottom = region_top + FS_GetOverlayTotalSize(p_ovi);

    {
      OSIntrMode bak_psr = OS_DisableInterrupts();
      MWiDestructorChain *prev = NULL;
      MWiDestructorChain *base = __global_destructor_chain;
      MWiDestructorChain *p = base;
      while (p) {
        MWiDestructorChain *next = p->next;
        const u32 dtor = (u32)p->dtor;
        const u32 obj = (u32)p->obj;
        if (((obj == 0) && (dtor >= region_top) && (dtor < region_bottom)) ||
            ((obj >= region_top) && (obj < region_bottom))) {
          if (!tail) {
            head = p;
          } else {
            tail->next = p;
          }
          if (base == p) {
            base = __global_destructor_chain = next;
          }
          tail = p, p->next = NULL;
          if (prev) {
            prev->next = next;
          }
        } else {
          prev = p;
        }
        p = next;
      }
      (void)OS_RestoreInterrupts(bak_psr);
    }

    if (!head) {
      break;
    }

    do {
      MWiDestructorChain *next = head->next;
      if (head->dtor) {
        (*head->dtor)(head->obj);
      }
      head = next;
    } while (head);
  }

  FSi_UnregisterOverlayToDebugger(p_ovi);
}

BOOL FS_UnloadOverlayImage(FSOverlayInfo *p_ovi) {
  FS_EndOverlay(p_ovi);
  return TRUE;
}

BOOL FS_LoadOverlay(MIProcessor target, FSOverlayID id) {
  BOOL retval = FALSE;
  SDK_ASSERT(FS_IsAvailable());
  {
    FSOverlayInfo ovi;
    if (FS_LoadOverlayInfo(&ovi, target, id)) {
      if (FS_LoadOverlayImage(&ovi)) {
        FS_StartOverlay(&ovi);
        retval = TRUE;
      }
    }
  }
  return retval;
}

BOOL FS_UnloadOverlay(MIProcessor target, FSOverlayID id) {
  BOOL retval = FALSE;
  SDK_ASSERT(FS_IsAvailable());
  {
    FSOverlayInfo ovi;
    if (FS_LoadOverlayInfo(&ovi, target, id)) {
      if (FS_UnloadOverlayImage(&ovi)) {
        retval = TRUE;
      }
    }
  }
  return retval;
}

void FS_AttachOverlayTable(MIProcessor target, const void *ptr, u32 len) {
  if ((ptr != NULL) && (target == MI_PROCESSOR_ARM9)) {

    const int length_ntr =
        (int)(FS_OVERLAY_NTR_TOTAL * sizeof(FSOverlayInfoHeader));
    const int length_twl =
        (int)(FS_OVERLAY_TWL_TOTAL * sizeof(FSOverlayInfoHeader));

    if ((len != length_ntr) && (len != length_ntr + length_twl)) {
      OS_TPanic("specified overlay-digest-table is invalid size!");
    } else {

      u8 *buffer_ntr = (u8 *)ptr;
      if (!FSi_CompareDigest((const u8 *)SDK_OVERLAYTABLE_DIGEST, buffer_ntr,
                             length_ntr, TRUE)) {
        OS_TPanic("specified overlay-digest-table is invalid!");
      }

      else if (length_twl != 0) {
        u8 *buffer_twl = buffer_ntr + length_ntr;
        if (!FSi_CompareDigest((const u8 *)SDK_LTDOVERLAYTABLE_DIGEST,
                               buffer_twl, length_twl, TRUE)) {
          OS_TPanic("specified overlay-digest-table is invalid!");
        }
      }
    }
  }

  {
    CARDRomRegion *const pr = (target == MI_PROCESSOR_ARM9)
                                  ? &FSiOverlayContext.ovt9
                                  : &FSiOverlayContext.ovt7;
    OSIntrMode bak_psr = OS_DisableInterrupts();
    pr->offset = (u32)ptr;
    pr->length = len;
    (void)OS_RestoreInterrupts(bak_psr);
  }
}
