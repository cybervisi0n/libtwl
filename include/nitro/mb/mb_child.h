#ifndef NITRO_MB_MB_CHILD_H_
#define NITRO_MB_MB_CHILD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb/mb.h>

typedef enum {
  MB_COMM_CSTATE_NONE,          // State before initialization
  MB_COMM_CSTATE_INIT_COMPLETE, // State when MB child initialization has
                                // completed.
  MB_COMM_CSTATE_CONNECT, // State when connection to parent has completed.
  MB_COMM_CSTATE_CONNECT_FAILED, // State when connection to parent has failed.
  MB_COMM_CSTATE_DISCONNECTED_BY_PARENT, // State when disconnected by parent
  MB_COMM_CSTATE_REQ_ENABLE,  // State when MP is established and data requests
                              // are possible.
  MB_COMM_CSTATE_REQ_REFUSED, // State when kicked from the parent in response
                              // to a request
  MB_COMM_CSTATE_DLINFO_ACCEPTED, // State when a request has been accepted by a
                                  // parent
  MB_COMM_CSTATE_RECV_PROCEED,    // State when download data receipt has
                                  // commenced.
  MB_COMM_CSTATE_RECV_COMPLETE,   // State when download data receipt has
                                  // completed.
  MB_COMM_CSTATE_BOOTREQ_ACCEPTED, // State when a boot request from a parent
                                   // has been received.
  MB_COMM_CSTATE_BOOT_READY,       // State when communication with a parent is
                                   // disconnected and boot is complete.
  MB_COMM_CSTATE_CANCELED, // State when canceled midway, and disconnect from
                           // parent is complete.
  MB_COMM_CSTATE_CANCELLED = MB_COMM_CSTATE_CANCELED,
  MB_COMM_CSTATE_AUTHENTICATION_FAILED, //
  MB_COMM_CSTATE_MEMBER_FULL, // State when the number of entries to the parent
                              // has exceeded capacity.
  MB_COMM_CSTATE_GAMEINFO_VALIDATED,   // State when parent beacon is received.
  MB_COMM_CSTATE_GAMEINFO_INVALIDATED, // Case in which a parent beacon that has
                                       // already been obtained is in an
                                       // incomplete state.
  MB_COMM_CSTATE_GAMEINFO_LOST, // Notification of a state in which the parent's
                                // beacon has been lost.
  MB_COMM_CSTATE_GAMEINFO_LIST_FULL, // Notification of state in which no more
                                     // parents can be obtained.
  MB_COMM_CSTATE_ERROR, // State in which an error was generated midway through.

  MB_COMM_CSTATE_FAKE_END, // Fake child completion notification (this state
                           // transitioned to only when using mb_fake_child)

  MB_COMM_CSTATE_WM_EVENT = 0x80000000
} MBCommCState;

#define MBCommCStateCallback MBCommCStateCallbackFunc

typedef void (*MBCommCStateCallbackFunc)(u32 status, void *arg);

void MB_SetRejectGgid(u32 ggid, u32 mask, BOOL enable);

#ifdef __cplusplus
}
#endif

#endif // NITRO_MB_MB_CHILD_H_
