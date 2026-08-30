#include "mb_private.h"

#define MY_ROUND(n, a) (((u32)(n) + (a) - 1) & ~((a) - 1))

#define MB_PARENT_WORK_SIZE_MIN                                                \
  (32 + sizeof(MBiParam) + 32 + sizeof(MB_CommPWork) + 32 + WM_SYSTEM_BUF_SIZE)
#define MB_CHILD_WORK_SIZE_MIN                                                 \
  (32 + sizeof(MBiParam) + 32 + sizeof(MB_CommCWork) + 32 + WM_SYSTEM_BUF_SIZE)

SDK_COMPILER_ASSERT(MB_PARENT_WORK_SIZE_MIN <= MB_SYSTEM_BUF_SIZE);
SDK_COMPILER_ASSERT(MB_CHILD_WORK_SIZE_MIN <= MB_CHILD_SYSTEM_BUF_SIZE);

void MBi_BlockHeaderBegin(u8 type, u32 *sendbuf) {
  u8 *p = (u8 *)sendbuf;
  p[0] = type; /* type is common to the parent and child. */
}

int MBi_BlockHeaderEnd(int body_len, u16 pollbmp, u32 *sendbuf) {

  DC_FlushRange(sendbuf, MY_ROUND(body_len, 32));
  DC_WaitWriteBufferEmpty();

  MB_DEBUG_OUTPUT("SEND (BMP:%04x)", pollbmp);
  MB_COMM_TYPE_OUTPUT(((u8 *)sendbuf)[0]);

  return MBi_SendMP(sendbuf, body_len, pollbmp);
}

int MB_GetParentSystemBufSize(void) { return MB_PARENT_WORK_SIZE_MIN; }

int MB_GetChildSystemBufSize(void) { return MB_CHILD_WORK_SIZE_MIN; }

u16 MBi_calc_cksum(const u16 *buf, int length) {
  u32 sum;
  int nwords = length >> 1;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;

  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (u16)(sum ^ 0xffff);
}

#ifdef PRINT_DEBUG

void MBi_DebugPrint(const char *file, int line, const char *func,
                    const char *fmt, ...) {
  va_list vlist;

  OS_TPrintf("func: %s [%s:%d]:\n", func, file, line);

  va_start(vlist, fmt);
  OS_TVPrintf(fmt, vlist);
  va_end(vlist);

  OS_TPrintf("\n");
}

void MBi_comm_type_output(u16 type) {
  enum { MB_TYPE_STRING_NUM = 12 };
  static const char *const mb_type_string[MB_TYPE_STRING_NUM] = {
      "MB_COMM_TYPE_DUMMY", //      0

      "MB_COMM_TYPE_PARENT_SENDSTART",   //      1
      "MB_COMM_TYPE_PARENT_KICKREQ",     //      2
      "MB_COMM_TYPE_PARENT_DL_FILEINFO", //      3
      "MB_COMM_TYPE_PARENT_DATA",        //      4
      "MB_COMM_TYPE_PARENT_BOOTREQ",     //      5
      "MB_COMM_TYPE_PARENT_MEMBER_FULL", //      6

      "MB_COMM_TYPE_CHILD_FILEREQ",          //      7
      "MB_COMM_TYPE_CHILD_ACCEPT_FILEINFO",  //      8
      "MB_COMM_TYPE_CHILD_CONTINUE",         //      9
      "MB_COMM_TYPE_CHILD_STOPREQ",          //      10
      "MB_COMM_TYPE_CHILD_BOOTREQ_ACCEPTED", //      11
  };
  if (type >= MB_TYPE_STRING_NUM) {
    MB_OUTPUT("TYPE: unknown\n");
    return;
  }
  MB_OUTPUT("TYPE: %s\n", mb_type_string[type]);
}

void MBi_comm_wmevent_output(u16 type, void *arg) {
  enum { MB_CB_STRING_NUM = 43 };
  static const char *const mb_cb_string[MB_CB_STRING_NUM + 2] = {
      "MB_CALLBACK_CHILD_CONNECTED",          //              0
      "MB_CALLBACK_CHILD_DISCONNECTED",       //              1
      "MB_CALLBACK_MP_PARENT_SENT",           //              2
      "MB_CALLBACK_MP_PARENT_RECV",           //              3
      "MB_CALLBACK_PARENT_FOUND",             //              4
      "MB_CALLBACK_PARENT_NOT_FOUND",         //              5
      "MB_CALLBACK_CONNECTED_TO_PARENT",      //              6
      "MB_CALLBACK_DISCONNECTED",             //              7
      "MB_CALLBACK_MP_CHILD_SENT",            //              8
      "MB_CALLBACK_MP_CHILD_RECV",            //              9
      "MB_CALLBACK_DISCONNECTED_FROM_PARENT", //              10
      "MB_CALLBACK_CONNECT_FAILED",           //              11
      "MB_CALLBACK_DCF_CHILD_SENT",           //              12
      "MB_CALLBACK_DCF_CHILD_SENT_ERR",       //              13
      "MB_CALLBACK_DCF_CHILD_RECV",           //              14
      "MB_CALLBACK_DISCONNECT_COMPLETE",      //              15
      "MB_CALLBACK_DISCONNECT_FAILED",        //              16
      "MB_CALLBACK_END_COMPLETE",             //              17
      "MB_CALLBACK_MP_CHILD_SENT_ERR",        //              18
      "MB_CALLBACK_MP_PARENT_SENT_ERR",       //              19
      "MB_CALLBACK_MP_STARTED",               //              20
      "MB_CALLBACK_INIT_COMPLETE",            //              21
      "MB_CALLBACK_END_MP_COMPLETE",          //              22
      "MB_CALLBACK_SET_GAMEINFO_COMPLETE",    //              23
      "MB_CALLBACK_SET_GAMEINFO_FAILED",      //              24
      "MB_CALLBACK_MP_SEND_ENABLE",           //              25
      "MB_CALLBACK_PARENT_STARTED",           //              26
      "MB_CALLBACK_BEACON_LOST",              //              27
      "MB_CALLBACK_BEACON_SENT",              //              28
      "MB_CALLBACK_BEACON_RECV",              //              29
      "MB_CALLBACK_MP_SEND_DISABLE",          //              30
      "MB_CALLBACK_DISASSOCIATE",             //              31
      "MB_CALLBACK_REASSOCIATE",              //              32
      "MB_CALLBACK_AUTHENTICATE",             //              33
      "MB_CALLBACK_SET_LIFETIME",             //              34
      "MB_CALLBACK_DCF_STARTED",              //              35
      "MB_CALLBACK_DCF_SENT",                 //              36
      "MB_CALLBACK_DCF_SENT_ERR",             //              37
      "MB_CALLBACK_DCF_RECV",                 //              38
      "MB_CALLBACK_DCF_END",                  //              39
      "MB_CALLBACK_MPACK_IND",                //              40
      "MB_CALLBACK_MP_CHILD_SENT_TIMEOUT",    //              41
      "MB_CALLBACK_SEND_QUEUE_FULL_ERR",      //              42
      "MB_CALLBACK_API_ERROR",                //              255
      "MB_CALLBACK_ERROR",                    //              256
  };

  if (type == MB_CALLBACK_API_ERROR)
    type = MB_CB_STRING_NUM;
  else if (type == MB_CALLBACK_ERROR)
    type = MB_CB_STRING_NUM + 1;
  else if (type >= MB_CB_STRING_NUM) {
    MB_OUTPUT("EVENTTYPE: unknown\n");
    return;
  }

  MB_OUTPUT("EVENTTYPE:%s\n", mb_cb_string[type]);
  if (arg) {
    MB_OUTPUT("\tAPPID:%04x ERRCODE:%04x\n", ((u16 *)arg)[0], ((u16 *)arg)[1]);
    MB_OUTPUT("\twlCmd:%04x wlResult:%04x\n", ((u16 *)arg)[2], ((u16 *)arg)[3]);
  }
}

#endif
