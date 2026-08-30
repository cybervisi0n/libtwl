#ifndef NITRO_FS_ARCHIVE_H_
#define NITRO_FS_ARCHIVE_H_

#include <nitro/fs/types.h>
#include <nitro/fs/romfat.h>
#include <nitro/os/common/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FSArchiveInterface {

  FSResult (*ReadFile)(struct FSArchive *, struct FSFile *, void *buffer,
                       u32 *length);
  FSResult (*WriteFile)(struct FSArchive *, struct FSFile *, const void *buffer,
                        u32 *length);
  FSResult (*SeekDirectory)(struct FSArchive *, struct FSFile *, u32 id,
                            u32 position);
  FSResult (*ReadDirectory)(struct FSArchive *, struct FSFile *,
                            FSDirectoryEntryInfo *info);
  FSResult (*FindPath)(struct FSArchive *, u32 base_dir_id, const char *path,
                       u32 *target_id, BOOL target_is_directory);
  FSResult (*GetPath)(struct FSArchive *, struct FSFile *, BOOL is_directory,
                      char *buffer, u32 *length);
  FSResult (*OpenFileFast)(struct FSArchive *, struct FSFile *, u32 id,
                           u32 mode);
  FSResult (*OpenFileDirect)(struct FSArchive *, struct FSFile *, u32 top,
                             u32 bottom, u32 *id);
  FSResult (*CloseFile)(struct FSArchive *, struct FSFile *);
  void (*Activate)(struct FSArchive *);
  void (*Idle)(struct FSArchive *);
  void (*Suspend)(struct FSArchive *);
  void (*Resume)(struct FSArchive *);

  FSResult (*OpenFile)(struct FSArchive *, struct FSFile *, u32 base_dir_id,
                       const char *path, u32 mode);
  FSResult (*SeekFile)(struct FSArchive *, struct FSFile *, int *offset,
                       FSSeekFileMode from);
  FSResult (*GetFileLength)(struct FSArchive *, struct FSFile *, u32 *length);
  FSResult (*GetFilePosition)(struct FSArchive *, struct FSFile *,
                              u32 *position);

  void (*Mount)(struct FSArchive *);
  void (*Unmount)(struct FSArchive *);
  FSResult (*GetArchiveCaps)(struct FSArchive *, u32 *caps);
  FSResult (*CreateFile)(struct FSArchive *, u32 baseid, const char *relpath,
                         u32 permit);
  FSResult (*DeleteFile)(struct FSArchive *, u32 baseid, const char *relpath);
  FSResult (*RenameFile)(struct FSArchive *, u32 baseid_src,
                         const char *relpath_src, u32 baseid_dst,
                         const char *relpath_dst);
  FSResult (*GetPathInfo)(struct FSArchive *, u32 baseid, const char *relpath,
                          FSPathInfo *info);
  FSResult (*SetPathInfo)(struct FSArchive *, u32 baseid, const char *relpath,
                          FSPathInfo *info);
  FSResult (*CreateDirectory)(struct FSArchive *, u32 baseid,
                              const char *relpath, u32 permit);
  FSResult (*DeleteDirectory)(struct FSArchive *, u32 baseid,
                              const char *relpath);
  FSResult (*RenameDirectory)(struct FSArchive *, u32 baseid,
                              const char *relpath_src, u32 baseid_dst,
                              const char *relpath_dst);
  FSResult (*GetArchiveResource)(struct FSArchive *,
                                 FSArchiveResource *resource);
  void *unused_29;
  FSResult (*FlushFile)(struct FSArchive *, struct FSFile *);
  FSResult (*SetFileLength)(struct FSArchive *, struct FSFile *, u32 length);
  FSResult (*OpenDirectory)(struct FSArchive *, struct FSFile *,
                            u32 base_dir_id, const char *path, u32 mode);
  FSResult (*CloseDirectory)(struct FSArchive *, struct FSFile *);
  FSResult (*SetSeekCache)(struct FSArchive *, struct FSFile *, void *buf,
                           u32 buf_size);

  u8 reserved[116];
} FSArchiveInterface;

SDK_COMPILER_ASSERT(sizeof(FSArchiveInterface) == 256);

typedef struct FSArchive {

  union {
    char ptr[FS_ARCHIVE_NAME_LEN_MAX + 1];
    u32 pack;
  } name;
  struct FSArchive *next;         // Archive registration list
  struct FSFile *list;            // Process wait command list
  OSThreadQueue queue;            // General-purpose queue to wait for events
  u32 flag;                       // Internal status flags (FS_ARCHIVE_FLAG_*)
  FSCommandType command;          // The most recent command
  FSResult result;                // The most recent processing result
  void *userdata;                 // User-defined pointer
  const FSArchiveInterface *vtbl; // Command interface

  union {

    u8 reserved2[52];

    struct FS_ROMFAT_CONTEXT_DEFINITION();
  };
} FSArchive;

SDK_COMPILER_ASSERT(sizeof(FSArchive) == 92);

FSArchive *FS_FindArchive(const char *name, int name_len);

FSArchive *FS_NormalizePath(const char *path, u32 *baseid, char *relpath);

const char *FS_GetCurrentDirectory(void);

SDK_INLINE FSCommandType FS_GetLastArchiveCommand(const FSArchive *arc) {
  return arc->command;
}

FSResult FS_GetArchiveResultCode(const void *path_or_archive);

void FSi_EndArchive(void);

void FS_InitArchive(FSArchive *arc);

const char *FS_GetArchiveName(const FSArchive *arc);

SDK_INLINE BOOL FS_IsArchiveLoaded(volatile const FSArchive *arc) {
  return ((arc->flag & FS_ARCHIVE_FLAG_LOADED) != 0);
}

SDK_INLINE BOOL FS_IsArchiveSuspended(volatile const FSArchive *arc) {
  return ((arc->flag & FS_ARCHIVE_FLAG_SUSPEND) != 0);
}

SDK_INLINE void *FS_GetArchiveUserData(const FSArchive *arc) {
  return arc->userdata;
}

BOOL FS_RegisterArchiveName(FSArchive *arc, const char *name, u32 name_len);

void FS_ReleaseArchiveName(FSArchive *arc);

BOOL FS_MountArchive(FSArchive *arc, void *userdata,
                     const FSArchiveInterface *vtbl, u32 reserved);

BOOL FS_UnmountArchive(FSArchive *arc);

BOOL FS_SuspendArchive(FSArchive *arc);

BOOL FS_ResumeArchive(FSArchive *arc);

void FS_NotifyArchiveAsyncEnd(FSArchive *arc, FSResult ret);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_FS_ARCHIVE_H_ */
