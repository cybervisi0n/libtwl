#if !defined(NITRO_FS_FILE_H_)
#define NITRO_FS_FILE_H_

#include <nitro/fs/archive.h>

#ifdef SDK_PORT
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FSFile {

  struct FSFile *next;
  void *userdata;
  struct FSArchive *arc;
  u32 stat;
  void *argument;
  FSResult error;
  OSThreadQueue queue[1];

  union {
    u8 reserved1[16];
    FSROMFATProperty prop;
  };
  union {
    u8 reserved2[24];
    FSROMFATCommandInfo arg;
  };
	#ifdef SDK_PORT
	FILE* pcFilePtr;
	#endif
} FSFile;

SDK_COMPILER_ASSERT(sizeof(FSFile) == 72);

void FS_InitFile(FSFile *file);

SDK_INLINE FSArchive *FS_GetAttachedArchive(const FSFile *file) {
  return file->arc;
}

SDK_INLINE BOOL FS_IsBusy(volatile const FSFile *file) {
  return ((file->stat & FS_FILE_STATUS_BUSY) != 0);
}

SDK_INLINE BOOL FS_IsCanceling(volatile const FSFile *file) {
  return ((file->stat & FS_FILE_STATUS_CANCEL) != 0);
}

SDK_INLINE BOOL FS_IsSucceeded(volatile const FSFile *file) {
  return (file->error == FS_RESULT_SUCCESS);
}

SDK_INLINE BOOL FS_IsFile(volatile const FSFile *file) {
  return ((file->stat & FS_FILE_STATUS_IS_FILE) != 0);
}

SDK_INLINE BOOL FS_IsDir(volatile const FSFile *file) {
  return ((file->stat & FS_FILE_STATUS_IS_DIR) != 0);
}

SDK_INLINE FSResult FS_GetResultCode(volatile const FSFile *file) {
  return file->error;
}

BOOL FS_WaitAsync(FSFile *file);

void FS_CancelFile(FSFile *file);

SDK_INLINE void FS_SetFileHandle(FSFile *file, FSArchive *arc, void *userdata) {
  file->stat |= FS_FILE_STATUS_IS_FILE;
  file->stat &= ~FS_FILE_STATUS_IS_DIR;
  file->arc = arc;
  file->userdata = userdata;
}

SDK_INLINE void FS_SetDirectoryHandle(FSFile *file, FSArchive *arc,
                                      void *userdata) {
  file->stat |= FS_FILE_STATUS_IS_DIR;
  file->stat &= ~FS_FILE_STATUS_IS_FILE;
  file->arc = arc;
  file->userdata = userdata;
}

SDK_INLINE void FS_DetachHandle(FSFile *file) {
  file->userdata = NULL;
  file->stat &= ~(FS_FILE_STATUS_IS_FILE | FS_FILE_STATUS_IS_DIR);
}

SDK_INLINE void *FS_GetFileUserData(const FSFile *file) {
  return file->userdata;
}

BOOL FS_GetPathName(FSFile *file, char *buffer, u32 length);

s32 FS_GetPathLength(FSFile *file);

BOOL FS_CreateFile(const char *path, u32 permit);

BOOL FS_DeleteFile(const char *path);

BOOL FS_RenameFile(const char *src, const char *dst);

BOOL FS_GetPathInfo(const char *path, FSPathInfo *info);

BOOL FS_SetPathInfo(const char *path, const FSPathInfo *info);

BOOL FS_CreateDirectory(const char *path, u32 permit);

BOOL FS_DeleteDirectory(const char *path);

BOOL FS_RenameDirectory(const char *src, const char *dst);

BOOL FS_CreateFileAuto(const char *path, u32 permit);

BOOL FS_DeleteFileAuto(const char *path);

BOOL FS_RenameFileAuto(const char *src, const char *dst);

BOOL FS_CreateDirectoryAuto(const char *path, u32 permit);

BOOL FS_DeleteDirectoryAuto(const char *path);

BOOL FS_RenameDirectoryAuto(const char *src, const char *dst);

BOOL FS_GetArchiveResource(const char *path, FSArchiveResource *resource);

u32 FSi_GetSpaceToCreateDirectoryEntries(const char *path,
                                         const u32 bytesPerCluster);

BOOL FS_HasEnoughSpaceToCreateFile(FSArchiveResource *resource,
                                   const char *path, u32 size);

BOOL FS_IsArchiveReady(const char *path);

BOOL FS_OpenFileEx(FSFile *file, const char *path, u32 mode);

BOOL FS_ConvertPathToFileID(FSFileID *p_fileid, const char *path);

BOOL FS_OpenFileFast(FSFile *file, FSFileID fileid);

BOOL FS_OpenFileDirect(FSFile *file, FSArchive *arc, u32 top, u32 bottom,
                       u32 fileid);

BOOL FS_CloseFile(FSFile *file);

u32 FS_GetFileLength(FSFile *file);

FSResult FS_SetFileLength(FSFile *file, u32 length);

u32 FS_GetFilePosition(FSFile *file);

u32 FS_GetSeekCacheSize(const char *path);

BOOL FS_SetSeekCache(FSFile *file, void *buf, u32 buf_size);

BOOL FS_SeekFile(FSFile *file, s32 offset, FSSeekFileMode origin);

SDK_INLINE BOOL FS_SeekFileToBegin(FSFile *file) {
  return FS_SeekFile(file, 0, FS_SEEK_SET);
}

SDK_INLINE BOOL FS_SeekFileToEnd(FSFile *file) {
  return FS_SeekFile(file, 0, FS_SEEK_END);
}

s32 FS_ReadFile(FSFile *file, void *buffer, s32 length);

s32 FS_ReadFileAsync(FSFile *file, void *buffer, s32 length);

s32 FS_WriteFile(FSFile *file, const void *buffer, s32 length);

s32 FS_WriteFileAsync(FSFile *file, const void *buffer, s32 length);

FSResult FS_FlushFile(FSFile *file);

BOOL FS_OpenDirectory(FSFile *file, const char *path, u32 mode);

BOOL FS_CloseDirectory(FSFile *file);

BOOL FS_ReadDirectory(FSFile *file, FSDirectoryEntryInfo *info);

BOOL FS_TellDir(const FSFile *dir, FSDirPos *pos);

BOOL FS_SeekDir(FSFile *p_dir, const FSDirPos *p_pos);

BOOL FS_RewindDir(FSFile *dir);

BOOL FS_OpenFileExW(FSFile *file, const u16 *path, u32 mode);

BOOL FS_OpenDirectoryW(FSFile *file, const u16 *path, u32 mode);

BOOL FS_ReadDirectoryW(FSFile *file, FSDirectoryEntryInfoW *info);

BOOL FS_OpenFile(FSFile *file, const char *path);

u32 FS_GetLength(FSFile *file);

u32 FS_GetPosition(FSFile *file);

BOOL FS_FindDir(FSFile *dir, const char *path);

BOOL FS_ReadDir(FSFile *dir, FSDirEntry *entry);

typedef FSPathInfo FSFileInfo;
FSResult FS_GetFileInfo(const char *path, FSFileInfo *info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_FS_FILE_H_ */
