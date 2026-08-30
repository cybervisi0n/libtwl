#include <nitro/os.h>
#include <nitro/std/string.h>
#ifdef SDK_TWL
#include <twl/os/common/systemWork.h>
#endif

#define OSi_INIVALUE 0xffff

#ifndef OS_NO_ARGUMENT

#include <nitro/version_begin.h>

static OSArgumentBuffer OSi_ArgumentBuffer = {
    OS_ARGUMENT_ID_STRING,
    OS_ARGUMENT_BUFFER_SIZE,
    {'\0', '\0'},
};
#include <nitro/version_end.h>

const char *sCurrentArgBuffer = &OSi_ArgumentBuffer.buffer[0];

static const char *match_opt(int optchar, const char *optstring);
#endif // ifndef OS_NO_ARGUMENT

#ifdef SDK_TWL
static u32 sBufferState = OS_DELIVER_ARG_BUF_INVALID;
static OSDeliverArgInfo *sBufferPtr = NULL;
static u8 *sPtr;
static OSTitleId sEncodedTitleId = 0;
#endif

static int OSi_GetArgc_core(const char *p);
static const char *OSi_GetArgv_core(const char *p, int n);

static int OSi_GetArgc_core(const char *p) {
  int n = 0;
  for (; *p; p++, n++) {
    while (*p) {
      p++;
    }
  }
  return n;
}

#ifndef OS_NO_ARGC_AND_ARGV
int OS_GetArgc(void) {
  return OSi_GetArgc_core((const char *)sCurrentArgBuffer);
}
#endif

static const char *OSi_GetArgv_core(const char *p, int n) {
  SDK_ASSERT(n >= 0);
  for (; *p && n > 0; p++, n--) {
    while (*p) {
      p++;
    }
  }
  return (*p) ? p : NULL;
}

#ifndef OS_NO_ARGC_AND_ARGV
const char *OS_GetArgv(int n) {
  return OSi_GetArgv_core((const char *)sCurrentArgBuffer, n);
}
#endif

#ifndef OS_NO_ARGUMENT
const char *OSi_OptArg = NULL;
int OSi_OptInd = 1;
int OSi_OptOpt = 0;

int OS_GetOpt(const char *optstring) {
  static BOOL end_of_option = FALSE;
  int optchar;
  const char *arg;
  const char *opt;
  const char *optarg;

  OSi_OptArg = NULL;
  OSi_OptOpt = 0;

  if (optstring == NULL) {
    OSi_OptInd = 1;
    end_of_option = FALSE;
    return 0;
  }

  arg = OS_GetArgv(OSi_OptInd);

  if (arg == NULL) {
    return -1;
  }

  if (optstring[0] == '-') // Minus Mode
  {
    OSi_OptInd++; // Consume arguments

    if (end_of_option || arg[0] != '-') {
      OSi_OptArg = arg; // Normal arguments are also set to OptArg.
      return 1;
    }

    optchar = arg[1];

    if (optchar == '-') // End of options when '--' is encountered
    {
      end_of_option = TRUE; // Normal arguments from here on
      return OS_GetOpt(optstring);
    }
  } else // normal mode
  {

    if (end_of_option || arg[0] != '-') {
      return -1; // OptArg remains NULL
    }

    OSi_OptInd++; // Consume arguments

    optchar = arg[1];

    if (optchar == '-') // End of options when '--' is encountered
    {
      end_of_option = TRUE; // Normal arguments from here on
      return -1;
    }
  }

  opt = match_opt(optchar, optstring);

  if (opt == NULL) {
    OSi_OptOpt = optchar; // Unknown option
    return '?';
  }

  if (opt[1] == ':') // OptArg search specification?
  {
    optarg = OS_GetArgv(OSi_OptInd);

    if (optarg == NULL || optarg[0] == '-') {
      if (opt[2] != ':') // '::' Or not?
      {
        OSi_OptOpt = optchar; // OptArg not present.
        return '?';
      }

    } else {
      OSi_OptArg = optarg;
      OSi_OptInd++; // Consume arguments
    }
  }
  return optchar;
}

static const char *match_opt(int optchar, const char *optstring) {
  if (optstring[0] == '-' || optstring[0] == '+') {
    optstring++;
  }

  if (optchar != ':') {
    while (*optstring) {
      if (optchar == *optstring) {
        return optstring;
      }
      optstring++;
    }
  }
  return NULL;
}
#endif // ifndef OS_NO_ARGUMENT

#ifndef OS_NO_ARGUMENT
void OS_ConvertToArguments(const char *str, char cs, char *buffer,
                           u32 bufSize) {
  char *p = buffer;
  char *pEnd = buffer + bufSize;
  BOOL isQuoted = FALSE;

  while (1) {

    while (*str == cs && p < pEnd) {
      str++;
    }

    while (*str && p < pEnd) {

      if (*str == '\"') {
        isQuoted = (isQuoted == FALSE);
        str++;
        continue;
      }

      else if (*str == cs && isQuoted == FALSE) {
        break;
      }

      *p++ = *str++;
    }

    if (p >= pEnd) {
      *(pEnd - 2) = '\0';
      *(pEnd - 1) = '\0';
      break;
    }

    if (*str == '\0') {
      while ((p + 1) >= pEnd) {
        p--;
      }
      *p++ = '\0';
      *p = '\0';
      break;
    }

    if (*str == cs) {
      *p++ = '\0';
    }
  }

  if (p < pEnd) {
    *p++ = '\0';
  }
}
#endif // ifndef OS_NO_ARGUMENT

#ifndef OS_NO_ARGUMENT
void OS_SetArgumentBuffer(const char *buffer) { sCurrentArgBuffer = buffer; }
#endif // ifndef OS_NO_ARGUMENT

#ifndef OS_NO_ARGUMENT
const char *OS_GetArgumentBuffer(void) { return sCurrentArgBuffer; }
#endif // ifndef OS_NO_ARGUMENT

#ifdef SDK_TWL

void OS_InitDeliverArgInfo(OSDeliverArgInfo *info, int binSize) {
  SDK_ASSERT(2 <= (OS_DELIVER_ARG_BUFFER_SIZE - binSize) && binSize >= 0);
  SDK_ASSERT(info != NULL);

  sBufferPtr = info;

  MI_CpuClear8(sBufferPtr, HW_PARAM_DELIVER_ARG_SIZE);

  sBufferState = OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE;
  sBufferPtr->header.argBufferSize =
      (u16)(OS_DELIVER_ARG_BUFFER_SIZE - binSize);
  sBufferPtr->header.binarySize = 0;

  sPtr = &sBufferPtr->buf[0];
  *sPtr = '\0';
  *(sPtr + 1) = '\0';
}

int OS_SetStringToDeliverArg(const char *str) {
  int length;

  if (!(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE)) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  length = STD_StrLen(str);

  if (length >
      sBufferPtr->header.argBufferSize - (sPtr - &sBufferPtr->buf[0]) - 2) {
    return OS_DELIVER_ARG_OVER_SIZE;
  }

  (void)STD_StrCpy((char *)sPtr, str);
  sPtr += length;
  *sPtr++ = '\0';
  *sPtr = '\0';

  return OS_DELIVER_ARG_SUCCESS;
}

int OS_SetBinaryToDeliverArg(const void *bin, int size) {
  u8 *dest = &sBufferPtr->buf[sBufferPtr->header.argBufferSize +
                              sBufferPtr->header.binarySize];

  if (!(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE)) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  if (size > OS_DELIVER_ARG_BUFFER_SIZE - sBufferPtr->header.argBufferSize -
                 sBufferPtr->header.binarySize) {
    return OS_DELIVER_ARG_OVER_SIZE;
  }

  MI_CpuCopy8(bin, dest, (u32)size);
  sBufferPtr->header.binarySize += (u16)size;

  return OS_DELIVER_ARG_SUCCESS;
}

int OS_ConvertStringToDeliverArg(const char *str, char cs) {
  int length;
  const char *endp;

  if (!(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE)) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  while (1) {

    while (*str == cs) {
      str++;
    }

    if (!*str) {
      break;
    }

    endp = str;
    while (*endp != cs && *endp != '\0') {
      endp++;
    }
    length = endp - str;

    if (length >
        sBufferPtr->header.argBufferSize - (sPtr - &sBufferPtr->buf[0]) - 2) {
      return OS_DELIVER_ARG_OVER_SIZE;
    }

    while (str != endp) {
      *sPtr++ = *(u8 *)str++;
    }
    *sPtr++ = '\0';
    *sPtr = '\0';
  }

  return OS_DELIVER_ARG_SUCCESS;
}

int OS_EncodeDeliverArg(void) {

  if (!(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  sBufferPtr->header.makerCode = OS_GetMakerCode();
  sBufferPtr->header.titleId = OS_GetTitleId();

  sBufferPtr->header.flag =
      OS_DELIVER_ARG_ENCODE_FLAG | OS_DELIVER_ARG_VALID_FLAG;

  sEncodedTitleId = sBufferPtr->header.titleId;

  sBufferPtr->header.crc = 0;
  sBufferPtr->header.crc = SVC_GetCRC16(OSi_INIVALUE, (const void *)sBufferPtr,
                                        sizeof(OSDeliverArgInfo));

  MI_CpuCopy8(sBufferPtr, (void *)HW_PARAM_DELIVER_ARG,
              sizeof(OSDeliverArgInfo));

  MI_CpuClear8(sBufferPtr, sizeof(OSDeliverArgInfo));

  sBufferState = OS_DELIVER_ARG_BUF_INVALID;

  return OS_DELIVER_ARG_SUCCESS;
}

int OS_DecodeDeliverArg(void) {

  if (!OS_IsValidDeliverArg()) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  if (!sBufferPtr) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  MI_CpuCopy8((void *)HW_PARAM_DELIVER_ARG, sBufferPtr,
              sizeof(OSDeliverArgInfo));

  {
    u16 crc = sBufferPtr->header.crc;
    sBufferPtr->header.crc = 0;
    sBufferPtr->header.crc = SVC_GetCRC16(
        OSi_INIVALUE, (const void *)sBufferPtr, sizeof(OSDeliverArgInfo));
    if (crc != sBufferPtr->header.crc) {
      MI_CpuClear8(sBufferPtr, sizeof(OSDeliverArgInfo));
      OS_SetDeliverArgStateInvalid();
      return OS_DELIVER_ARG_BUF_INVALID;
    }
  }

  sBufferPtr->header.flag &= ~OS_DELIVER_ARG_ENCODE_FLAG;

  sBufferState = OS_DELIVER_ARG_BUF_ACCESSIBLE;

  return OS_DELIVER_ARG_SUCCESS;
}

u32 OS_GetDeliverArgState(void) { return sBufferState; }

void OSi_SetDeliverArgState(u32 state) { sBufferState = state; }

void OS_SetDeliverArgStateInvalid(void) {
  sBufferState = OS_DELIVER_ARG_BUF_INVALID;
}

int OS_GetBinarySizeFromDeliverArg(void) {
  if (!(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)) {
    return -1;
  }

  return sBufferPtr->header.binarySize;
}

int OS_GetBinaryFromDeliverArg(void *buffer, int *size, int maxSize) {
  int retval;
  int copySize;

  if (!(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  if (maxSize < sBufferPtr->header.binarySize) {
    retval = OS_DELIVER_ARG_OVER_SIZE;
    copySize = maxSize;
  } else {
    retval = OS_DELIVER_ARG_SUCCESS;
    copySize = sBufferPtr->header.binarySize;
  }

  MI_CpuCopy8(&sBufferPtr->buf[sBufferPtr->header.argBufferSize], buffer,
              (u32)copySize);
  if (size) {
    *size = copySize;
  }

  return retval;
}

OSTitleId OS_GetTitleIdFromDeliverArg(void) {
  return (sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)
             ? sBufferPtr->header.titleId
             : 0;
}

u32 OS_GetGameCodeFromDeliverArg(void) {
  return (sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)
             ? *(u32 *)(&sBufferPtr->header.titleId)
             : 0;
}

u16 OS_GetMakerCodeFromDeliverArg(void) {
  return (u16)((sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)
                   ? sBufferPtr->header.makerCode
                   : 0);
}

BOOL OS_IsValidDeliverArg(void) {

  OSDeliverArgInfo *p = (OSDeliverArgInfo *)HW_PARAM_DELIVER_ARG;

  return (p->header.flag & OS_DELIVER_ARG_VALID_FLAG) ? TRUE : FALSE;
}

BOOL OS_IsDeliverArgEncoded(void) {
  OSDeliverArgInfo *p = (OSDeliverArgInfo *)HW_PARAM_DELIVER_ARG;
  return (p->header.flag &
          (OS_DELIVER_ARG_ENCODE_FLAG | OS_DELIVER_ARG_VALID_FLAG))
             ? TRUE
             : FALSE;
}

OSTitleId OS_GetTitleIdLastEncoded(void) { return sEncodedTitleId; }

int OS_SetSysParamToDeliverArg(u16 param) {
  if (!(sBufferState & OS_DELIVER_ARG_BUF_WRITABLE)) {
    return OS_DELIVER_ARG_NOT_READY;
  }

  sBufferPtr->header.sysParam = param;
  return OS_DELIVER_ARG_SUCCESS;
}

u16 OS_GetSysParamFromDeliverArg(void) {
  if (!(sBufferState & OS_DELIVER_ARG_BUF_ACCESSIBLE)) {
    return 0;
  }

  return sBufferPtr->header.sysParam;
}

int OS_GetDeliverArgc(void) {
  if (sBufferPtr && sBufferState == OS_DELIVER_ARG_BUF_ACCESSIBLE) {
    return OSi_GetArgc_core((const char *)sBufferPtr->buf) + 1;
  }

  return 0;
}

const char *OS_GetDeliverArgv(int n) {
  static const char *procName = "main";
  SDK_ASSERT(n >= 0);

  if (sBufferPtr && sBufferState == OS_DELIVER_ARG_BUF_ACCESSIBLE) {
    return (n == 0) ? procName
                    : OSi_GetArgv_core((const char *)sBufferPtr->buf, n - 1);
  }

  return NULL;
}
#endif // ifdef SDK_TWL
