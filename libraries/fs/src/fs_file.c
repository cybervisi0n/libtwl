#include <nitro/types.h>
#include <nitro/misc.h>
#include <nitro/mi.h>
#include <nitro/os.h>
#include <nitro/pxi.h>
#include <nitro/std/string.h>
#include <nitro/std/unicode.h>
#include <nitro/math/math.h>

#include <nitro/fs.h>

#include "../include/rom.h"
#include "../include/util.h"
#include "../include/command.h"

#define FS_DEBUG_TRACE(...) (void)0

#if defined(FS_IMPLEMENT)

static BOOL FSi_IsValidTransferRegion(const void *buffer, s32 length) {
  BOOL retval = FALSE;
  if (buffer == NULL) {
    OS_TWarning("specified transfer buffer is NULL.\n");
  } else if (((u32)buffer >= HW_IOREG) && ((u32)buffer < HW_IOREG_END)) {
    OS_TWarning("specified transfer buffer is in I/O register %08X. (seems to "
                "be dangerous)\n",
                buffer);
  } else if (length < 0) {
    OS_TWarning("specified transfer size is minus. (%d)\n", length);
  } else {
#if !defined(SDK_TWL)
    s32 mainmem_size = HW_MAIN_MEM_EX_SIZE;
#else
    s32 mainmem_size =
        OS_IsRunOnTwl() ? HW_TWL_MAIN_MEM_EX_SIZE : HW_MAIN_MEM_EX_SIZE;
#endif
    if (length > mainmem_size) {
      OS_TWarning("specified transfer size is over mainmemory-size. (%d)\n",
                  length);
    } else {
      retval = TRUE;
    }
  }
  return retval;
}

int FSi_DecrementSjisPosition(const char *str, int pos) {

  int prev = --pos;

  for (; (prev > 0) && STD_IsSjisLeadByte(str[prev - 1]); --prev) {
  }

  return pos - ((pos - prev) & 1);
}

int FSi_IncrementSjisPositionToSlash(const char *str, int pos) {
  while (str[pos] && !FSi_IsSlash((u8)str[pos])) {
    pos = FSi_IncrementSjisPosition(str, pos);
  }
  return pos;
}

int FSi_DecrementSjisPositionToSlash(const char *str, int pos) {
  for (;;) {
    pos = FSi_DecrementSjisPosition(str, pos);
    if ((pos < 0) || FSi_IsSlash((u8)str[pos])) {
      break;
    }
  }
  return pos;
}

int FSi_TrimSjisTrailingSlash(char *str) {
  int length = STD_GetStringLength(str);
  int lastpos = FSi_DecrementSjisPosition(str, length);
  if ((lastpos >= 0) && FSi_IsSlash((u8)str[lastpos])) {
    length = lastpos;
    str[length] = '\0';
  }
  return length;
}

int FSi_DecrementUnicodePosition(const u16 *str, int pos) {

  int prev = --pos;

  if ((pos > 0) && ((str[pos - 1] >= 0xD800) && (str[pos - 1] <= 0xDC00)) &&
      ((str[pos - 0] >= 0xDC00) && (str[pos - 0] <= 0xE000))) {
    --pos;
  }
  return pos;
}

int FSi_DecrementUnicodePositionToSlash(const u16 *str, int pos) {
  for (;;) {
    pos = FSi_DecrementUnicodePosition(str, pos);
    if ((pos < 0) || FSi_IsUnicodeSlash(str[pos])) {
      break;
    }
  }
  return pos;
}

void FS_InitFile(FSFile *file) {
  SDK_NULL_ASSERT(file);
  {
    file->arc = NULL;
    file->userdata = NULL;
    file->next = NULL;
    OS_InitThreadQueue(file->queue);
    file->stat = 0;
    file->stat |= (FS_COMMAND_INVALID << FS_FILE_STATUS_CMD_SHIFT);
    file->argument = NULL;
    file->error = FS_RESULT_SUCCESS;
  }
}

void FS_CancelFile(FSFile *file) {
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FS_IsAvailable());
  {
    OSIntrMode bak_psr = OS_DisableInterrupts();
    if (FS_IsBusy(file)) {
      file->stat |= FS_FILE_STATUS_CANCEL;
      file->arc->flag |= FS_ARCHIVE_FLAG_CANCELING;
    }
    (void)OS_RestoreInterrupts(bak_psr);
  }
}

BOOL FS_CreateFile(const char *path, u32 permit) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSFile file[1];
      FSArgumentForCreateFile arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      arg->permit = permit;
      retval = FSi_SendCommand(file, FS_COMMAND_CREATEFILE, TRUE);
    }
  }
  return retval;
}

BOOL FS_DeleteFile(const char *path) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSFile file[1];
      FSArgumentForDeleteFile arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      retval = FSi_SendCommand(file, FS_COMMAND_DELETEFILE, TRUE);
    }
  }
  return retval;
}

BOOL FS_RenameFile(const char *src, const char *dst) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s(%s->%s)\n", __FUNCTION__, src, dst);
  SDK_NULL_ASSERT(src);
  SDK_NULL_ASSERT(dst);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath_src[FS_ARCHIVE_FULLPATH_MAX + 1];
    char relpath_dst[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid_src = 0;
    u32 baseid_dst = 0;
    FSArchive *arc_src = FS_NormalizePath(src, &baseid_src, relpath_src);
    FSArchive *arc_dst = FS_NormalizePath(dst, &baseid_dst, relpath_dst);
    if (arc_src != arc_dst) {
      OS_TWarning("cannot rename between defferent archives.\n");
    } else {
      FSFile file[1];
      FSArgumentForRenameFile arg[1];
      FS_InitFile(file);
      file->arc = arc_src;
      file->argument = arg;
      arg->baseid_src = baseid_src;
      arg->relpath_src = relpath_src;
      arg->baseid_dst = baseid_dst;
      arg->relpath_dst = relpath_dst;
      retval = FSi_SendCommand(file, FS_COMMAND_RENAMEFILE, TRUE);
    }
  }
  return retval;
}

BOOL FS_GetPathInfo(const char *path, FSPathInfo *info) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(path);
  SDK_NULL_ASSERT(info);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSFile file[1];
      FSArgumentForGetPathInfo arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      arg->info = info;
      retval = FSi_SendCommand(file, FS_COMMAND_GETPATHINFO, TRUE);
    }
  }
  return retval;
}

BOOL FS_SetPathInfo(const char *path, const FSPathInfo *info) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(path);
  SDK_NULL_ASSERT(info);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSFile file[1];
      FSArgumentForSetPathInfo arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      arg->info = (FSPathInfo *)
          info; // To clear FATFS_PROPERTY_CTRL_MASK in info->attributes
      retval = FSi_SendCommand(file, FS_COMMAND_SETPATHINFO, TRUE);
    }
  }
  return retval;
}

BOOL FS_CreateDirectory(const char *path, u32 permit) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSFile file[1];
      FSArgumentForCreateDirectory arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      arg->permit = permit;
      retval = FSi_SendCommand(file, FS_COMMAND_CREATEDIRECTORY, TRUE);
    }
  }
  return retval;
}

BOOL FS_DeleteDirectory(const char *path) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSFile file[1];
      FSArgumentForDeleteDirectory arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      retval = FSi_SendCommand(file, FS_COMMAND_DELETEDIRECTORY, TRUE);
    }
  }
  return retval;
}

BOOL FS_RenameDirectory(const char *src, const char *dst) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s(%s->%s)\n", __FUNCTION__, src, dst);
  SDK_NULL_ASSERT(src);
  SDK_NULL_ASSERT(dst);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath_src[FS_ARCHIVE_FULLPATH_MAX + 1];
    char relpath_dst[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid_src = 0;
    u32 baseid_dst = 0;
    FSArchive *arc_src = FS_NormalizePath(src, &baseid_src, relpath_src);
    FSArchive *arc_dst = FS_NormalizePath(dst, &baseid_dst, relpath_dst);
    if (arc_src != arc_dst) {
      OS_TWarning("cannot rename between defferent archives.\n");
    } else {
      FSFile file[1];
      FSArgumentForRenameDirectory arg[1];
      FS_InitFile(file);
      file->arc = arc_src;
      file->argument = arg;
      arg->baseid_src = baseid_src;
      arg->relpath_src = relpath_src;
      arg->baseid_dst = baseid_dst;
      arg->relpath_dst = relpath_dst;
      retval = FSi_SendCommand(file, FS_COMMAND_RENAMEDIRECTORY, TRUE);
    }
  }
  return retval;
}

static BOOL FSi_GetFullPath(char *dst, const char *path) {
  FSArchive *arc = FS_NormalizePath(path, NULL, dst);
  if (arc) {
    const char *arcname = FS_GetArchiveName(arc);
    int m = STD_GetStringLength(arcname);
    int n = STD_GetStringLength(dst);
    (void)STD_MoveMemory(&dst[m + 2], &dst[0], (u32)n + 1);
    (void)STD_MoveMemory(&dst[0], arcname, (u32)m);
    dst[m + 0] = ':';
    dst[m + 1] = '/';
  }
  return (arc != NULL);
}

static BOOL FSi_ComplementDirectory(const char *path, char *autogen) {
  BOOL retval = FALSE;
  int root = 0;

  char *tmppath = autogen;
  if (FSi_GetFullPath(tmppath, path)) {
    int length = STD_GetStringLength(tmppath);
    if (length > 0) {
      int pos = 0;
      FS_DEBUG_TRACE("  trying to complete \"%s\"\n", tmppath);

      length = FSi_TrimSjisTrailingSlash(tmppath);

      length = FSi_DecrementSjisPositionToSlash(tmppath, length);

      for (pos = length; pos >= 0;) {
        FSPathInfo info[1];
        BOOL exists;
        tmppath[pos] = '\0';
        exists = FS_GetPathInfo(tmppath, info);
        FS_DEBUG_TRACE("    - \"%s\" is%s existent (result:%d)\n", tmppath,
                       exists ? "" : " not", FS_GetArchiveResultCode(tmppath));
        tmppath[pos] = '/';

        if (!exists) {
          pos = FSi_DecrementSjisPositionToSlash(tmppath, pos);
        }

        else {

          if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0) {
            pos = -1;
          }

          else {
            ++pos;
          }
          break;
        }
      }

      if (pos >= 0) {
        for (;;) {

          if (pos >= length) {
            retval = TRUE;
            break;
          } else {
            pos = FSi_IncrementSjisPositionToSlash(tmppath, pos);
            tmppath[pos] = '\0';
            if (!FS_CreateDirectory(tmppath, FS_PERMIT_R | FS_PERMIT_W)) {
              break;
            } else {

              if (root == 0) {
                FS_DEBUG_TRACE("    - we have created \"%s\" as root\n",
                               tmppath);
                root = pos;
              }
              tmppath[pos++] = '/';
            }
          }
        }
      }
    }
  }

  autogen[root] = '\0';
  return retval;
}

BOOL FS_CreateFileAuto(const char *path, u32 permit) {
  BOOL result = FALSE;
  char autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);
  if (FSi_ComplementDirectory(path, autogen)) {
    result = FS_CreateFile(path, permit);
    if (!result) {
      (void)FS_DeleteDirectoryAuto(autogen);
    }
  }
  return result;
}

BOOL FS_DeleteFileAuto(const char *path) {
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);

  return FS_DeleteFile(path);
}

BOOL FS_RenameFileAuto(const char *src, const char *dst) {
  BOOL result = FALSE;
  char autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
  FS_DEBUG_TRACE("%s(%s->%s)\n", __FUNCTION__);
  if (FSi_ComplementDirectory(dst, autogen)) {
    result = FS_RenameFile(src, dst);
    if (!result) {
      (void)FS_DeleteDirectoryAuto(autogen);
    }
  }
  return result;
}

BOOL FS_CreateDirectoryAuto(const char *path, u32 permit) {
  BOOL result = FALSE;
  char autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);
  if (FSi_ComplementDirectory(path, autogen)) {
    result = FS_CreateDirectory(path, permit);
    if (!result) {
      (void)FS_DeleteDirectoryAuto(autogen);
    }
  }
  return result;
}

BOOL FS_DeleteDirectoryAuto(const char *path) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s(%s)\n", __FUNCTION__, path);
  if (path && *path) {
    char tmppath[FS_ARCHIVE_FULLPATH_MAX + 1];
    if (FSi_GetFullPath(tmppath, path)) {
      int pos;
      BOOL mayBeEmpty;
      int length = FSi_TrimSjisTrailingSlash(tmppath);
      FS_DEBUG_TRACE("  trying to force-delete \"%s\"\n", tmppath);
      mayBeEmpty = TRUE;
      for (pos = 0; pos >= 0;) {
        BOOL failure = FALSE;

        tmppath[length + pos] = '\0';
        if (mayBeEmpty &&
            (FS_DeleteDirectory(tmppath) ||
             (FS_GetArchiveResultCode(tmppath) == FS_RESULT_ALREADY_DONE))) {
          FS_DEBUG_TRACE("  -> succeeded to delete \"%s\"\n", tmppath);
          pos = FSi_DecrementSjisPositionToSlash(&tmppath[length], pos);
        } else {

          FSFile dir[1];
          FS_InitFile(dir);
          if (!FS_OpenDirectory(dir, tmppath, FS_FILEMODE_R)) {
            FS_DEBUG_TRACE("  -> failed to delete & open \"%s\"\n", tmppath);
            failure = TRUE;
          } else {
            FSDirectoryEntryInfo info[1];
            tmppath[length + pos] = '/';
            mayBeEmpty = TRUE;
            while (FS_ReadDirectory(dir, info)) {
              (void)STD_CopyString(&tmppath[length + pos + 1], info->longname);

              if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0) {
                if (!FS_DeleteFile(tmppath)) {
                  FS_DEBUG_TRACE("  -> failed to delete file \"%s\"\n",
                                 tmppath);
                  failure = TRUE;
                  break;
                }
                FS_DEBUG_TRACE("  -> succeeded to delete \"%s\"\n", tmppath);
              }

              else if ((STD_CompareString(info->longname, ".") == 0) ||
                       (STD_CompareString(info->longname, "..") == 0)) {
              }

              else if (!FS_DeleteDirectory(tmppath)) {
                pos += 1 + STD_GetStringLength(info->longname);
                mayBeEmpty = FALSE;
                break;
              }
            }
            (void)FS_CloseDirectory(dir);
          }
        }

        if (failure) {
          break;
        }
      }
      retval = (pos < 0);
    }
  }
  return retval;
}

BOOL FS_RenameDirectoryAuto(const char *src, const char *dst) {
  BOOL result = FALSE;
  char autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
  FS_DEBUG_TRACE("%s(%s->%s)\n", __FUNCTION__, src, dst);
  if (FSi_ComplementDirectory(dst, autogen)) {
    result = FS_RenameDirectory(src, dst);
    if (!result) {
      (void)FS_DeleteDirectoryAuto(autogen);
    }
  }
  return result;
}

BOOL FS_GetArchiveResource(const char *path, FSArchiveResource *resource) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArchive *arc = FS_NormalizePath(path, NULL, NULL);
    if (arc) {
      FSFile file[1];
      FSArgumentForGetArchiveResource arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->resource = resource;
      retval = FSi_SendCommand(file, FS_COMMAND_GETARCHIVERESOURCE, TRUE);
    }
  }
  return retval;
}

u32 FSi_GetSpaceToCreateDirectoryEntries(const char *path,
                                         u32 bytesPerCluster) {
  static const u32 bytesPerEntry = 32UL;
  static const u32 longnamePerEntry = 13UL;

  const char *root = STD_SearchString(path, ":");
  const char *current = (root != NULL) ? (root + 1) : path;
  u32 totalBytes = 0;
  u32 restBytesInCluster = 0;
  current += (*current == '/');
  while (*current) {
    BOOL isShortName = FALSE;
    u32 entries = 0;

    u32 len = (u32)FSi_IncrementSjisPositionToSlash(current, 0);

#if 0


        {
            static const char  *alnum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            static const char  *special = "!#$%&'()*+-<>?@^_`{}~";
            if ((len <= 8 + 1 + 3) && STD_SearchChar(alnum, current[0]))
            {
                u32     namelen = 0;
                u32     extlen = 0;
                u32     scanned = 0;
                for (; namelen < len; ++namelen)
                {
                    char    c = current[scanned + namelen];
                    if (!STD_SearchChar(alnum, c) && !STD_SearchChar(special, c))
                    {
                        break;
                    }
                }
                scanned += namelen;
                if ((scanned < len) && (current[scanned] == '.'))
                {
                    ++scanned;
                    for (; scanned + extlen < len; ++extlen)
                    {
                        char    c = current[scanned + extlen];
                        if (!STD_SearchChar(alnum, c) && !STD_SearchChar(special, c))
                        {
                            break;
                        }
                    }
                    scanned += extlen;
                }
                if ((scanned == len) && (namelen <= 8) && (extlen <= 3))
                {
                    isShortName = TRUE;
                }
            }
        }
#endif

    if (!isShortName) {
      entries += ((len + longnamePerEntry - 1UL) / longnamePerEntry);
    }

    entries += 1;
    current += len;

    {
      int over = (int)(entries * bytesPerEntry - restBytesInCluster);
      if (over > 0) {
        totalBytes += MATH_ROUNDUP(over, bytesPerCluster);
      }
    }

    if (*current != '\0') {
      current += 1;
      totalBytes += bytesPerCluster;
      restBytesInCluster = bytesPerCluster - (2 * bytesPerEntry);
    }
  }
  return totalBytes;
}

BOOL FS_HasEnoughSpaceToCreateFile(FSArchiveResource *resource,
                                   const char *path, u32 size) {
  BOOL retval = FALSE;
  u32 bytesPerCluster = resource->bytesPerSector * resource->sectorsPerCluster;
  if (bytesPerCluster != 0) {
    u32 needbytes =
        (FSi_GetSpaceToCreateDirectoryEntries(path, bytesPerCluster) +
         MATH_ROUNDUP(size, bytesPerCluster));
    u32 needclusters = needbytes / bytesPerCluster;
    if (needclusters <= resource->availableClusters) {
      resource->availableClusters -= needclusters;
      resource->availableSize -= needbytes;
      retval = TRUE;
    }
  }
  return retval;
}

BOOL FS_IsArchiveReady(const char *path) {
  FSArchiveResource resource[1];
  return FS_GetArchiveResource(path, resource);
}

FSResult FS_FlushFile(FSFile *file) {
  FSResult retval = FS_RESULT_ERROR;
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FS_IsFile(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    (void)FSi_SendCommand(file, FS_COMMAND_FLUSHFILE, TRUE);
    retval = FS_GetResultCode(file);
  }
  return retval;
}

FSResult FS_SetFileLength(FSFile *file, u32 length) {
  FSResult retval = FS_RESULT_ERROR;
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FS_IsFile(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArgumentForSetFileLength arg[1];
    file->argument = arg;
    arg->length = length;
    (void)FSi_SendCommand(file, FS_COMMAND_SETFILELENGTH, TRUE);
    retval = FS_GetResultCode(file);
  }
  return retval;
}

BOOL FS_GetPathName(FSFile *file, char *buffer, u32 length) {
  BOOL retval = FALSE;
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file) || FS_IsDir(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArgumentForGetPath arg[1];
    file->argument = arg;
    arg->is_directory = FS_IsDir(file);
    arg->buffer = buffer;
    arg->length = length;
    retval = FSi_SendCommand(file, FS_COMMAND_GETPATH, TRUE);
  }
  return retval;
}

s32 FS_GetPathLength(FSFile *file) {
  s32 retval = -1;
  if (FS_GetPathName(file, NULL, 0)) {
    retval = file->arg.getpath.total_len;
  }
  return retval;
}

BOOL FS_ConvertPathToFileID(FSFileID *p_fileid, const char *path) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(p_fileid);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSFile file[1];
      FSArgumentForFindPath arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      arg->target_is_directory = FALSE;
      if (FSi_SendCommand(file, FS_COMMAND_FINDPATH, TRUE)) {
        p_fileid->arc = arc;
        p_fileid->file_id = arg->target_id;
        retval = TRUE;
      }
    }
  }
  return retval;
}

BOOL FS_OpenFileDirect(FSFile *file, FSArchive *arc, u32 image_top,
                       u32 image_bottom, u32 id) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(file);
  SDK_NULL_ASSERT(arc);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(!FS_IsFile(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArgumentForOpenFileDirect arg[1];
    file->arc = arc;
    file->argument = arg;
    arg->id = id;
    arg->top = image_top;
    arg->bottom = image_bottom;
    arg->mode = 0;
    retval = FSi_SendCommand(file, FS_COMMAND_OPENFILEDIRECT, TRUE);
  }
  return retval;
}

BOOL FS_OpenFileFast(FSFile *file, FSFileID id) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(!FS_IsFile(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  if (id.arc) {
    FSArgumentForOpenFileFast arg[1];
    file->arc = id.arc;
    file->argument = arg;
    arg->id = id.file_id;
    arg->mode = 0;
    retval = FSi_SendCommand(file, FS_COMMAND_OPENFILEFAST, TRUE);
  }
  return retval;
}

BOOL FS_OpenFileEx(FSFile *file, const char *path, u32 mode) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

  if (((mode & FS_FILEMODE_L) != 0) &&
      ((mode & FS_FILEMODE_RW) == FS_FILEMODE_W)) {
    OS_TWarning(
        "\"FS_FILEMODE_WL\" seems useless.\n"
        "(this means creating empty file and prohibiting any modifications)");
  }
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSArgumentForOpenFile arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      arg->mode = mode;
      if (FSi_SendCommand(file, FS_COMMAND_OPENFILE, TRUE)) {
        retval = TRUE;
      } else {
        file->arc = NULL;
      }
    }
  }
  return retval;
}

BOOL FS_CloseFile(FSFile *file) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    retval = FSi_SendCommand(file, FS_COMMAND_CLOSEFILE, TRUE);
  }
  return retval;
}

u32 FS_GetSeekCacheSize(const char *path) {
  u32 retval = 0;

  FSPathInfo info;
  if (FS_GetPathInfo(path, &info) &&
      ((info.attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0)) {

    FSArchiveResource resource;
    if (FS_GetArchiveResource(path, &resource)) {

      u32 bytesPerCluster =
          resource.sectorsPerCluster * resource.bytesPerSector;
      if (bytesPerCluster != 0) {
        static const u32 fatBits = 32;
        retval =
            (u32)((info.filesize + bytesPerCluster - 1) / bytesPerCluster) *
            ((fatBits + 4) / 8);

        retval += (u32)(HW_CACHE_LINE_SIZE * 2);
      }
    }
  }
  return retval;
}

BOOL FS_SetSeekCache(FSFile *file, void *buf, u32 buf_size) {
  FSArgumentForSetSeekCache arg[1];
  BOOL retval = FALSE;
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file));

  file->argument = arg;
  arg->buf = buf;
  arg->buf_size = buf_size;
  retval = FSi_SendCommand(file, FS_COMMAND_SETSEEKCACHE, TRUE);

  return retval;
}

u32 FS_GetFileLength(FSFile *file) {
  u32 retval = 0;
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file));

  if (!FSi_GetFileLengthIfProc(file, &retval)) {
    FSArgumentForGetFileLength arg[1];
    file->argument = arg;
    arg->length = 0;
    if (FSi_SendCommand(file, FS_COMMAND_GETFILELENGTH, TRUE)) {
      retval = arg->length;
    }
  }
  return retval;
}

u32 FS_GetFilePosition(FSFile *file) {
  u32 retval = 0;
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file));

  if (!FSi_GetFilePositionIfProc(file, &retval)) {
    FSArgumentForGetFilePosition arg[1];
    file->argument = arg;
    arg->position = 0;
    if (FSi_SendCommand(file, FS_COMMAND_GETFILEPOSITION, TRUE)) {
      retval = arg->position;
    }
  }
  return retval;
}

BOOL FS_SeekFile(FSFile *file, s32 offset, FSSeekFileMode origin) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file));

  if (!(retval = FSi_SeekFileIfProc(file, offset, origin))) {
    FSArgumentForSeekFile arg[1];
    file->argument = arg;
    arg->offset = (int)offset;
    arg->from = origin;
    retval = FSi_SendCommand(file, FS_COMMAND_SEEKFILE, TRUE);
  }
  return retval;
}

s32 FS_ReadFile(FSFile *file, void *buffer, s32 length) {
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArgumentForReadFile arg[1];
    file->argument = arg;
    arg->buffer = buffer;
    arg->length = (u32)length;
    if (FSi_SendCommand(file, FS_COMMAND_READFILE, TRUE)) {
      length = (s32)arg->length;
    } else {
      if ((file->error == FS_RESULT_INVALID_PARAMETER) ||
          (file->error == FS_RESULT_ERROR)) {
        length = -1; // If not read at all
      } else {
        length = (s32)arg->length; // If reading was tried, a value higher than
                                   // -1 is entered
      }
    }
  }
  return length;
}

s32 FS_ReadFileAsync(FSFile *file, void *buffer, s32 length) {
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));

  {
    u32 end, pos;
    if (FSi_GetFilePositionIfProc(file, &pos) &&
        FSi_GetFileLengthIfProc(file, &end) && (pos + length > end)) {
      length = (s32)(end - pos);
    }
  }
  {
    FSArgumentForReadFile *arg = (FSArgumentForReadFile *)file->reserved2;
    file->argument = arg;
    arg->buffer = buffer;
    arg->length = (u32)length;
    (void)FSi_SendCommand(file, FS_COMMAND_READFILE, FALSE);
  }
  return length;
}

s32 FS_WriteFile(FSFile *file, const void *buffer, s32 length) {
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArgumentForWriteFile arg[1];
    file->argument = arg;
    arg->buffer = buffer;
    arg->length = (u32)length;
    if (FSi_SendCommand(file, FS_COMMAND_WRITEFILE, TRUE)) {
      length = (s32)arg->length;
    } else {
      if (file->error == FS_RESULT_INVALID_PARAMETER) {
        length = -1; // If not written at all
      } else {
        length = (s32)arg->length; // If writing was tried, a value higher than
                                   // -1 is entered
      }
    }
  }
  return length;
}

s32 FS_WriteFileAsync(FSFile *file, const void *buffer, s32 length) {
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));

  {
    u32 end, pos;
    if (FSi_GetFilePositionIfProc(file, &pos) &&
        FSi_GetFileLengthIfProc(file, &end) && (pos + length > end)) {
      length = (s32)(end - pos);
    }
  }
  {
    FSArgumentForWriteFile *arg = (FSArgumentForWriteFile *)file->reserved2;
    file->argument = arg;
    arg->buffer = buffer;
    arg->length = (u32)length;
    (void)FSi_SendCommand(file, FS_COMMAND_WRITEFILE, FALSE);
  }
  return length;
}

BOOL FS_OpenDirectory(FSFile *file, const char *path, u32 mode) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    char relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc) {
      FSArgumentForOpenDirectory arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = relpath;
      arg->mode = mode;
      if (FSi_SendCommand(file, FS_COMMAND_OPENDIRECTORY, TRUE)) {
        retval = TRUE;
      } else {
        file->arc = NULL;
      }
    }
  }
  return retval;
}

BOOL FS_CloseDirectory(FSFile *file) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsDir(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    if (FSi_SendCommand(file, FS_COMMAND_CLOSEDIRECTORY, TRUE)) {
      retval = TRUE;
    }
  }
  return retval;
}

BOOL FS_ReadDirectory(FSFile *file, FSDirectoryEntryInfo *info) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(file);
  SDK_NULL_ASSERT(info);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsDir(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArgumentForReadDirectory arg[1];
    file->argument = arg;
    arg->info = info;
    MI_CpuFill8(info, 0x00, sizeof(info));
    info->id = FS_INVALID_FILE_ID;
    if (FSi_SendCommand(file, FS_COMMAND_READDIR, TRUE)) {
      retval = TRUE;
    }
  }
  return retval;
}

BOOL FS_SeekDir(FSFile *file, const FSDirPos *pos) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(file);
  SDK_NULL_ASSERT(pos);
  SDK_NULL_ASSERT(pos->arc);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArgumentForSeekDirectory arg[1];
    arg->id = (u32)((pos->own_id << 0) | (pos->index << 16));
    arg->position = pos->pos;
    file->arc = pos->arc;
    file->argument = arg;
    if (FSi_SendCommand(file, FS_COMMAND_SEEKDIR, TRUE)) {
      file->stat |= FS_FILE_STATUS_IS_DIR;
      retval = TRUE;
    }
  }
  return retval;
}

BOOL FS_TellDir(const FSFile *dir, FSDirPos *pos) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(dir);
  SDK_NULL_ASSERT(pos);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsDir(dir));
  {
    *pos = dir->prop.dir.pos;
    retval = TRUE;
  }
  return retval;
}

BOOL FS_RewindDir(FSFile *dir) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(dir);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsDir(dir));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

  {
    FSDirPos pos;
    pos.arc = dir->arc;
    pos.own_id = dir->prop.dir.pos.own_id;
    pos.pos = 0;
    pos.index = 0;
    retval = FS_SeekDir(dir, &pos);
  }
  return retval;
}

enum {
  FS_UNICODE_CONVSRC_ASCII,
  FS_UNICODE_CONVSRC_SHIFT_JIS,
  FS_UNICODE_CONVSRC_UNICODE
};

static int FSi_CopySafeUnicodeString(u16 *dst, int dstlen, const void *srcptr,
                                     int srclen, int srctype,
                                     BOOL *stickyFailure) {
  int srcpos = 0;
  int dstpos = 0;
  if (srctype == FS_UNICODE_CONVSRC_ASCII) {
    const char *src = (const char *)srcptr;
    int n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
    while ((dstpos < n) && src[srcpos]) {
      dst[dstpos++] = (u8)src[srcpos++];
    }
    if ((srcpos < srclen) && src[srcpos]) {
      *stickyFailure = TRUE;
    }
  } else if (srctype == FS_UNICODE_CONVSRC_UNICODE) {
    const u16 *src = (const u16 *)srcptr;
    int n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
    while ((dstpos < n) && src[srcpos]) {
      dst[dstpos++] = src[srcpos++];
    }
    if ((srcpos < srclen) && src[srcpos]) {
      *stickyFailure = TRUE;
    }
  } else if (srctype == FS_UNICODE_CONVSRC_SHIFT_JIS) {
    const char *src = (const char *)srcptr;
    srcpos = srclen;
    dstpos = dstlen - 1;
    (void)FSi_ConvertStringSjisToUnicode(dst, &dstpos, src, &srcpos, NULL);
    if ((srcpos < srclen) && src[srcpos]) {
      *stickyFailure = TRUE;
    }
  }
  dst[dstpos] = L'\0';
  return dstpos;
}

FSArchive *FSi_NormalizePathWtoW(const u16 *path, u32 *baseid, u16 *relpath);
FSArchive *FSi_NormalizePathWtoW(const u16 *path, u32 *baseid, u16 *relpath) {
  FSArchive *arc = NULL;
  int pathlen = 0;
  int pathmax = FS_ARCHIVE_FULLPATH_MAX + 1;
  BOOL stickyFailure = FALSE;

  BOOL absolute = FALSE;
  int arcnameLen;
  for (arcnameLen = 0; arcnameLen < FS_ARCHIVE_NAME_LONG_MAX + 1;
       ++arcnameLen) {
    if (path[arcnameLen] == L'\0') {
      break;
    } else if (FSi_IsUnicodeSlash(path[arcnameLen])) {
      break;
    } else if (path[arcnameLen] == L':') {
      char arcname[FS_ARCHIVE_NAME_LONG_MAX + 1];
      int j;
      for (j = 0; j < arcnameLen; ++j) {
        arcname[j] = (char)path[j];
      }
      arcname[arcnameLen] = '\0';
      arc = FS_FindArchive(arcname, arcnameLen);
      break;
    }
  }
  if (arc) {
    absolute = TRUE;
    *baseid = 0;
  } else {
    arc = FS_NormalizePath("", baseid, NULL);
  }
  if (arc) {

    u32 caps = 0;
    (void)arc->vtbl->GetArchiveCaps(arc, &caps);
    if ((caps & FS_ARCHIVE_CAPS_UNICODE) == 0) {
      arc = NULL;
    } else {

      pathlen += FSi_CopySafeUnicodeString(
          &relpath[pathlen], pathmax - pathlen, FS_GetArchiveName(arc),
          FS_ARCHIVE_NAME_LONG_MAX, FS_UNICODE_CONVSRC_ASCII, &stickyFailure);
      pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                           L":", 1, FS_UNICODE_CONVSRC_UNICODE,
                                           &stickyFailure);

      if (absolute) {
        path += arcnameLen + 1 + FSi_IsUnicodeSlash(path[arcnameLen + 1]);
      }

      else if (FSi_IsUnicodeSlash(*path)) {
        path += 1;
      }

      else {
        pathlen += FSi_CopySafeUnicodeString(
            &relpath[pathlen], pathmax - pathlen, L"/", 1,
            FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
        pathlen += FSi_CopySafeUnicodeString(
            &relpath[pathlen], pathmax - pathlen, FS_GetCurrentDirectory(),
            FS_ENTRY_LONGNAME_MAX, FS_UNICODE_CONVSRC_SHIFT_JIS,
            &stickyFailure);
      }

      pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                           L"/", 1, FS_UNICODE_CONVSRC_UNICODE,
                                           &stickyFailure);
      {

        int curlen = 0;
        while (!stickyFailure) {
          u16 c = path[curlen];
          if ((c != L'\0') && !FSi_IsUnicodeSlash(c)) {
            curlen += 1;
          } else {

            if (curlen == 0) {
            }

            else if ((curlen == 1) && (path[0] == L'.')) {
            }

            else if ((curlen == 2) && (path[0] == '.') && (path[1] == '.')) {
              if ((pathlen > 2) && (relpath[pathlen - 2] != L':')) {
                --pathlen;
                pathlen =
                    FSi_DecrementUnicodePositionToSlash(relpath, pathlen) + 1;
              }
            }

            else {
              pathlen += FSi_CopySafeUnicodeString(
                  &relpath[pathlen], pathmax - pathlen, path, curlen,
                  FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
              if (c != L'\0') {
                pathlen += FSi_CopySafeUnicodeString(
                    &relpath[pathlen], pathmax - pathlen, L"/", 1,
                    FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
              }
            }
            if (c == L'\0') {
              break;
            }
            path += curlen + 1;
            curlen = 0;
          }
        }
      }
      relpath[pathlen] = L'\0';
    }
  }
  return stickyFailure ? NULL : arc;
}

BOOL FS_OpenFileExW(FSFile *file, const u16 *path, u32 mode) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(file);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

  if (((mode & FS_FILEMODE_L) != 0) &&
      ((mode & FS_FILEMODE_RW) == FS_FILEMODE_W)) {
    OS_TWarning(
        "\"FS_FILEMODE_WL\" seems useless.\n"
        "(this means creating empty file and prohibiting any modifications)");
  }
  {
    u16 relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FSi_NormalizePathWtoW(path, &baseid, relpath);

    if (!arc) {
      file->error = FS_RESULT_UNSUPPORTED;
    } else {
      FSArgumentForOpenFile arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = (char *)relpath;
      arg->mode = mode;
      file->stat |= FS_FILE_STATUS_UNICODE_MODE;
      if (FSi_SendCommand(file, FS_COMMAND_OPENFILE, TRUE)) {
        retval = TRUE;
      } else {
        file->arc = NULL;
      }
    }
  }
  return retval;
}

BOOL FS_OpenDirectoryW(FSFile *file, const u16 *path, u32 mode) {
  BOOL retval = FALSE;
  FS_DEBUG_TRACE("%s\n", __FUNCTION__);
  SDK_NULL_ASSERT(path);
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    u16 relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
    u32 baseid = 0;
    FSArchive *arc = FSi_NormalizePathWtoW(path, &baseid, relpath);

    if (!arc) {
      file->error = FS_RESULT_UNSUPPORTED;
    } else {
      FSArgumentForOpenDirectory arg[1];
      FS_InitFile(file);
      file->arc = arc;
      file->argument = arg;
      arg->baseid = baseid;
      arg->relpath = (char *)relpath;
      arg->mode = mode;
      file->stat |= FS_FILE_STATUS_UNICODE_MODE;
      if (FSi_SendCommand(file, FS_COMMAND_OPENDIRECTORY, TRUE)) {
        retval = TRUE;
      } else {
        file->arc = NULL;
      }
    }
  }
  return retval;
}

BOOL FS_ReadDirectoryW(FSFile *file, FSDirectoryEntryInfoW *info) {
  BOOL retval = FALSE;
  SDK_NULL_ASSERT(file);
  SDK_NULL_ASSERT(info);
  SDK_ASSERT(FS_IsAvailable());
  SDK_ASSERT(FS_IsDir(file));
  SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
  {
    FSArchive *arc = file->arc;

    u32 caps = 0;
    (void)arc->vtbl->GetArchiveCaps(arc, &caps);
    if ((caps & FS_ARCHIVE_CAPS_UNICODE) == 0) {
      file->error = FS_RESULT_UNSUPPORTED;
    } else {
      FSArgumentForReadDirectory arg[1];
      file->argument = arg;
      arg->info = (FSDirectoryEntryInfo *)info;
      MI_CpuFill8(info, 0x00, sizeof(info));
      info->id = FS_INVALID_FILE_ID;
      file->stat |= FS_FILE_STATUS_UNICODE_MODE;
      if (FSi_SendCommand(file, FS_COMMAND_READDIR, TRUE)) {
        retval = TRUE;
      }
    }
  }
  return retval;
}

static void FSi_ConvertToDirEntry(FSDirEntry *entry, FSArchive *arc,
                                  const FSDirectoryEntryInfo *info) {
  entry->name_len = info->longname_length;
  if (entry->name_len > sizeof(entry->name) - 1) {
    entry->name_len = sizeof(entry->name) - 1;
  }
  MI_CpuCopy8(info->longname, entry->name, entry->name_len);
  entry->name[entry->name_len] = '\0';
  if (info->id == FS_INVALID_FILE_ID) {
    entry->is_directory = FALSE;
    entry->file_id.file_id = FS_INVALID_FILE_ID;
    entry->file_id.arc = NULL;
  } else if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0) {
    entry->is_directory = TRUE;
    entry->dir_id.arc = arc;
    entry->dir_id.own_id = (u16)(info->id >> 0);
    entry->dir_id.index = (u16)(info->id >> 16);
    entry->dir_id.pos = 0;
  } else {
    entry->is_directory = FALSE;
    entry->file_id.file_id = info->id;
    entry->file_id.arc = arc;
  }
}

BOOL FS_OpenFile(FSFile *file, const char *path) {
  return FS_OpenFileEx(file, path, FS_FILEMODE_R);
}

u32 FS_GetLength(FSFile *file) { return FS_GetFileLength(file); }

u32 FS_GetPosition(FSFile *file) { return FS_GetFilePosition(file); }

BOOL FS_FindDir(FSFile *dir, const char *path) {
  return FS_OpenDirectory(dir, path, FS_FILEMODE_R);
}

BOOL FS_ReadDir(FSFile *file, FSDirEntry *entry) {
  BOOL retval = FALSE;
  FSDirectoryEntryInfo info[1];
  if (FS_ReadDirectory(file, info)) {
    FSi_ConvertToDirEntry(entry, FS_GetAttachedArchive(file), info);
    retval = TRUE;
  }
  return retval;
}

BOOL FS_ChangeDir(const char *path) { return FS_SetCurrentDirectory(path); }

FSResult FS_GetFileInfo(const char *path, FSFileInfo *info) {
  return FS_GetPathInfo(path, info) ? FS_RESULT_SUCCESS
                                    : FS_GetArchiveResultCode(path);
}

#endif /* FS_IMPLEMENT */

#if defined(SDK_TWL) && defined(SDK_ARM7)
#include <twl/ltdmain_begin.h>
#endif

static const int FSiUnicodeBufferQueueMax = 4;
static OSMessageQueue FSiUnicodeBufferQueue[1];
#ifdef SDK_BUILD_ARM
static OSMessage FSiUnicodeBufferQueueArray[FSiUnicodeBufferQueueMax];
#else
static OSMessage FSiUnicodeBufferQueueArray[4];
#endif
static BOOL FSiUnicodeBufferQueueInitialized = FALSE;
#ifdef SDK_BUILD_ARM
static u16 FSiUnicodeBufferTable[FSiUnicodeBufferQueueMax]
                                [FS_ARCHIVE_FULLPATH_MAX + 1];
#else
static u16 FSiUnicodeBufferTable[4][FS_ARCHIVE_FULLPATH_MAX + 1];
#endif

u16 *FSi_GetUnicodeBuffer(const char *src) {
  u16 *retval = NULL;

  OSIntrMode bak = OS_DisableInterrupts();
  if (!FSiUnicodeBufferQueueInitialized) {
    int i;
    FSiUnicodeBufferQueueInitialized = TRUE;
    OS_InitMessageQueue(FSiUnicodeBufferQueue, FSiUnicodeBufferQueueArray, 4);
    for (i = 0; i < FSiUnicodeBufferQueueMax; ++i) {
      (void)OS_SendMessage(FSiUnicodeBufferQueue, FSiUnicodeBufferTable[i],
                           OS_MESSAGE_BLOCK);
    }
  }
  (void)OS_RestoreInterrupts(bak);

  (void)OS_ReceiveMessage(FSiUnicodeBufferQueue, (OSMessage *)&retval,
                          OS_MESSAGE_BLOCK);
  if (src) {
    int dstlen = FS_ARCHIVE_FULLPATH_MAX;
    (void)FSi_ConvertStringSjisToUnicode(retval, &dstlen, src, NULL, NULL);
    retval[dstlen] = L'\0';
  }
  return retval;
}

void FSi_ReleaseUnicodeBuffer(const void *buf) {
  if (buf) {

    (void)OS_SendMessage(FSiUnicodeBufferQueue, (OSMessage)buf,
                         OS_MESSAGE_BLOCK);
  }
}

SDK_WEAK_SYMBOL
STDResult FSi_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback)
#ifdef SDK_BUILD_ARM
    __attribute__((never_inline))
#endif
{
  return STD_ConvertStringSjisToUnicode(dst, dst_len, src, src_len, callback);
}

SDK_WEAK_SYMBOL
STDResult FSi_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback)
#ifdef SDK_BUILD_ARM
    __attribute__((never_inline))
#endif
{
  return STD_ConvertStringUnicodeToSjis(dst, dst_len, src, src_len, callback);
}

#if defined(SDK_TWL) && defined(SDK_ARM7)
#include <twl/ltdmain_end.h>
#endif
