#if !defined(NITRO_MB_CACHE_H_)
#define NITRO_MB_CACHE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define MB_CACHE_INFO_MAX 4

#define MB_CACHE_STATE_EMPTY 0
#define MB_CACHE_STATE_BUSY 1
#define MB_CACHE_STATE_READY 2
#define MB_CACHE_STATE_LOCKED 3

typedef struct {
  u32 src;   /* Logical source address */
  u32 len;   /* Cache length */
  u8 *ptr;   /* Pointer to cache buffer */
  u32 state; /* If 1, ready to use */
} MBiCacheInfo;

typedef struct {
  u32 lifetime;                               /* If hit or timeout, set 0. */
  u32 recent;                                 /* Unused */
  MBiCacheInfo *p_list;                       /* Unused */
  u32 size;                                   /* Unused */
  char arc_name[FS_ARCHIVE_NAME_LEN_MAX + 1]; /* Target archive */
  u32 arc_name_len;                           /* Archive name length */
  FSArchive *arc_pointer;
  u8 reserved[32 - FS_ARCHIVE_NAME_LEN_MAX - 1 - sizeof(u32) -
              sizeof(FSArchive *)];
  MBiCacheInfo list[MB_CACHE_INFO_MAX];
} MBiCacheList;

void MBi_InitCache(MBiCacheList *pl);

void MBi_AttachCacheBuffer(MBiCacheList *pl, u32 src, u32 len, void *ptr,
                           u32 state);

BOOL MBi_ReadFromCache(MBiCacheList *pl, u32 src, void *dst, u32 len);

BOOL MBi_TryLoadCache(MBiCacheList *pl, u32 src, u32 len);

#if defined(__cplusplus)
}
#endif

#endif /* NITRO_MB_CACHE_H_ */
