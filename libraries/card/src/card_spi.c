#include <nitro.h>

#include "../include/card_common.h"
#include "../include/card_spi.h"

#define MCCNT0_SPI_CLK_MASK 0x0003 /* Baud rate settings mask */
#define MCCNT0_SPI_CLK_4M 0x0000   /* Baud rate 4.19 MHz */
#define MCCNT0_SPI_CLK_2M 0x0001   /* Baud rate 2.10 MHz */
#define MCCNT0_SPI_CLK_1M 0x0002   /* Baud rate 1.05 MHz */
#define MCCNT0_SPI_CLK_524K 0x0003 /* Baud rate 524 kHz */
#define MCCNT0_SPI_BUSY 0x0080     /* SPI busy flag */
#define MMCNT0_SEL_MASK 0x2000     /* CARD ROM / SPI selection mask */
#define MMCNT0_SEL_CARD 0x0000     /* CARD ROM selection */
#define MMCNT0_SEL_SPI 0x2000      /* CARD SPI selection */
#define MCCNT0_INT_MASK 0x4000     /* Transfer completion interrupt mask */
#define MCCNT0_INT_ON 0x4000       /* Enable transfer completion interrupts */
#define MCCNT0_INT_OFF 0x0000      /* Disable transfer completion interrupts */
#define MCCNT0_MASTER_MASK 0x8000  /* CARD master mask */
#define MCCNT0_MASTER_ON 0x8000    /* CARD master ON */
#define MCCNT0_MASTER_OFF 0x0000   /* CARD master OFF */

#define CARD_BACKUP_TYPE_VENDER_IRC (0xFF) // IRC vendor type
#define IRC_BACKUP_WAIT 50 // Interval at which IRC SPI sends commands

typedef struct { /* SPI internal management parameter */
  u32 rest_comm; /* Total remaining number of bytes to send */
  u32 src;       /* Transfer source */
  u32 dst;       /* Transfer destination */
  BOOL cmp;      /* Comparison result */
} CARDiParam;

static CARDiParam cardi_param;

static BOOL CARDi_CommandCheckBusy(void);
static void CARDi_CommArray(const void *src, void *dst, u32 n,
                            void (*func)(CARDiParam *));
static void CARDi_CommReadCore(CARDiParam *p);
static void CARDi_CommWriteCore(CARDiParam *p);
static void CARDi_CommVerifyCore(CARDiParam *p);

SDK_INLINE void CARDi_CommArrayRead(void *dst, u32 len) {
  CARDi_CommArray(NULL, dst, len, CARDi_CommReadCore);
}

SDK_INLINE void CARDi_CommArrayWrite(const void *src, u32 len) {
  CARDi_CommArray(src, NULL, len, CARDi_CommWriteCore);
}

SDK_INLINE void CARDi_CommArrayVerify(const void *src, u32 len) {
  CARDi_CommArray(src, NULL, len, CARDi_CommVerifyCore);
}

SDK_INLINE void CARDi_EnableSpi(u32 cont) {

  const u16 ctrl = (u16)(MCCNT0_MASTER_ON | MCCNT0_INT_OFF | MMCNT0_SEL_SPI |
                         MCCNT0_SPI_CLK_4M | cont);
  reg_MI_MCCNT0 = ctrl;
}

SDK_INLINE void CARDi_WaitBusy(void) {
  while ((reg_MI_MCCNT0 & MCCNT0_SPI_BUSY) != 0) {
  }
}

static void CARDi_CommandBegin(int len) { cardi_param.rest_comm = (u32)len; }

static void CARDi_CommandEnd(u32 force_wait, u32 timeout) {
  if (force_wait + timeout > 0) {

    if (force_wait > 0) {
      OS_Sleep(force_wait);
    }
    if (timeout > 0) {

      int rest = (int)(timeout - force_wait);
      while (!CARDi_CommandCheckBusy() && (rest > 0)) {
        int interval = (rest < 5) ? rest : 5;
        OS_Sleep((u32)interval);
        rest -= interval;
      }
    }

    if (!CARDi_CommandCheckBusy()) {
      cardi_common.cmd->result = CARD_RESULT_TIMEOUT;
    }
  }
}

u8 CARDi_CommandReadStatus(void) {
  const u8 buf = COMM_READ_STATUS;
  u8 dst;
  CARDi_CommandBegin(1 + 1);
  CARDi_CommArrayWrite(&buf, 1);
  CARDi_CommArrayRead(&dst, 1);
  CARDi_CommandEnd(0, 0);
  return dst;
}

static BOOL CARDi_CommandCheckBusy(void) {
  return ((CARDi_CommandReadStatus() & COMM_STATUS_WIP_BIT) == 0);
}

static BOOL CARDi_WaitPrevCommand(void) {
  CARDi_CommandEnd(0, 50);

  if (cardi_common.cmd->result == CARD_RESULT_TIMEOUT) {
    cardi_common.cmd->result = CARD_RESULT_NO_RESPONSE;
    return FALSE;
  }
  return TRUE;
}

SDK_INLINE void CARDi_WaitBusyforIRC(void) {
  u16 tick = OS_GetTickLo();
  while (OS_TicksToMicroSeconds(OS_GetTickLo() - tick) < IRC_BACKUP_WAIT) {
  }
}

static BOOL need_command = TRUE;

void CARDi_CommArray(const void *src, void *dst, u32 len,
                     void (*func)(CARDiParam *)) {
  CARDiParam *const p = &cardi_param;
  CARDiCommandArg *const arg = cardi_common.cmd;

  BOOL isIRC =
      ((u8)((arg->type >> CARD_BACKUP_TYPE_VENDER_SHIFT) &
            CARD_BACKUP_TYPE_VENDER_MASK) == CARD_BACKUP_TYPE_VENDER_IRC)
          ? TRUE
          : FALSE;
  p->src = (u32)src;
  p->dst = (u32)dst;

  CARDi_EnableSpi(CSPI_CONTINUOUS_ON | MCCNT0_SPI_CLK_4M);

  for (; len > 0; --len) {
    if (need_command) {
      if (isIRC) {
        vu16 dummy_read;

        CARDi_WaitBusyforIRC(); // This wait is characteristic of the IRC's
                                // built-in CPU
        CARDi_EnableSpi(
            CSPI_CONTINUOUS_ON |
            MCCNT0_SPI_CLK_1M); // Set to 1 MHz when sending commands to SPI
        CARDi_WaitBusy();
        reg_MI_MCD0 = 0x00; // Swap SSU and backup device
        CARDi_WaitBusy();
        dummy_read = reg_MI_MCD0;
        need_command = FALSE;
        CARDi_WaitBusyforIRC(); // This wait is characteristic of the IRC's
                                // built-in CPU
        CARDi_EnableSpi(CSPI_CONTINUOUS_ON | MCCNT0_SPI_CLK_4M);
      }
    }
    if (!--p->rest_comm) {
      CARDi_EnableSpi(CSPI_CONTINUOUS_OFF | MCCNT0_SPI_CLK_4M);
      need_command = TRUE;
    }
    CARDi_WaitBusy();
    (*func)(p);
  }
  if (!p->rest_comm) {
    reg_MI_MCCNT0 =
        (u16)(MCCNT0_MASTER_OFF | MCCNT0_INT_OFF | MCCNT0_SPI_CLK_4M);
  }
}

void CARDi_CommReadCore(CARDiParam *p) {
  reg_MI_MCD0 = 0;
  CARDi_WaitBusy();
  MI_WriteByte((void *)p->dst, (u8)reg_MI_MCD0);
  ++p->dst;
}

void CARDi_CommWriteCore(CARDiParam *p) {
  vu16 tmp;
  reg_MI_MCD0 = (u16)MI_ReadByte((void *)p->src);
  ++p->src;
  CARDi_WaitBusy();
  tmp = reg_MI_MCD0;
}

void CARDi_CommVerifyCore(CARDiParam *p) {
  reg_MI_MCD0 = 0;
  CARDi_WaitBusy();

  if ((u8)reg_MI_MCD0 != MI_ReadByte((void *)p->src)) {
    p->cmp = FALSE;
    if (p->rest_comm > 1) {
      p->rest_comm = 1;
    }
  }
  ++p->src;
}

static void CARDi_WriteEnable(void) {
  static const u8 arg[1] = {
      COMM_WRITE_ENABLE,
  };
  CARDi_CommandBegin(sizeof(arg));
  CARDi_CommArrayWrite(arg, sizeof(arg));
  CARDi_CommandEnd(0, 0);
}

static void CARDi_SendSpiAddressingCommand(u32 addr, u32 mode) {
  const u32 width = cardi_common.cmd->spec.addr_width;
  u32 addr_cmd;
  switch (width) {
  case 1:

    addr_cmd = (u32)(mode | ((addr >> 5) & 0x8) | ((addr & 0xFF) << 8));
    break;
  case 2:
    addr_cmd = (u32)(mode | (addr & 0xFF00) | ((addr & 0xFF) << 16));
    break;
  case 3:
    addr_cmd = (u32)(mode | ((addr >> 8) & 0xFF00) | ((addr & 0xFF00) << 8) |
                     ((addr & 0xFF) << 24));
    break;
  default:
    SDK_ASSERT(FALSE);
    break;
  }
  CARDi_CommArrayWrite(&addr_cmd, (u32)(1 + width));
}

void CARDi_InitStatusRegister(void) {

  const u8 stat = cardi_common.cmd->spec.initial_status;
  if (stat != 0xFF) {
    static BOOL status_checked = FALSE;
    if (!status_checked) {
      if (CARDi_CommandReadStatus() != stat) {
        CARDi_SetWriteProtectCore(stat);
      }
      status_checked = TRUE;
    }
  }
}

void CARDi_ReadBackupCore(u32 src, void *dst, u32 len) {
  if (CARDi_WaitPrevCommand()) {
    CARDiCommandArg *const cmd = cardi_common.cmd;

    CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + len));
    CARDi_SendSpiAddressingCommand(src, COMM_READ_ARRAY);
    CARDi_CommArrayRead(dst, len);
    CARDi_CommandEnd(0, 0);
  }
}

void CARDi_ProgramBackupCore(u32 dst, const void *src, u32 len) {
  if (CARDi_WaitPrevCommand()) {
    CARDiCommandArg *const cmd = cardi_common.cmd;

    const u32 page = cmd->spec.page_size;
    while (len > 0) {
      const u32 mod = (u32)(dst & (page - 1));
      u32 size = page - mod;
      if (size > len) {
        size = len;
      }
      CARDi_WriteEnable();
      CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + size));
      CARDi_SendSpiAddressingCommand(dst, COMM_PROGRAM_PAGE);
      CARDi_CommArrayWrite(src, size);
      CARDi_CommandEnd(cmd->spec.program_page, 0);
      if (cmd->result != CARD_RESULT_SUCCESS) {
        break;
      }
      src = (const void *)((u32)src + size);
      dst += size;
      len -= size;
    }
  }
}

void CARDi_WriteBackupCore(u32 dst, const void *src, u32 len) {
  if (CARDi_WaitPrevCommand()) {
    CARDiCommandArg *const cmd = cardi_common.cmd;

    const u32 page = cmd->spec.page_size;
    while (len > 0) {
      const u32 mod = (u32)(dst & (page - 1));
      u32 size = page - mod;
      if (size > len) {
        size = len;
      }
      CARDi_WriteEnable();
      CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + size));
      CARDi_SendSpiAddressingCommand(dst, COMM_PAGE_WRITE);
      CARDi_CommArrayWrite(src, size);
      CARDi_CommandEnd(cmd->spec.write_page, cmd->spec.write_page_total);
      if (cmd->result != CARD_RESULT_SUCCESS) {
        break;
      }
      src = (const void *)((u32)src + size);
      dst += size;
      len -= size;
    }
  }
}

void CARDi_VerifyBackupCore(u32 dst, const void *src, u32 len) {
  if (CARDi_WaitPrevCommand()) {
    CARDiCommandArg *const cmd = cardi_common.cmd;

    cardi_param.cmp = TRUE;
    CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + len));
    CARDi_SendSpiAddressingCommand(dst, COMM_READ_ARRAY);
    CARDi_CommArrayVerify(src, len);
    CARDi_CommandEnd(0, 0);
    if ((cmd->result == CARD_RESULT_SUCCESS) && !cardi_param.cmp) {
      cmd->result = CARD_RESULT_FAILURE;
    }
  }
}

void CARDi_EraseBackupSectorCore(u32 dst, u32 len) {
  CARDiCommandArg *const cmd = cardi_common.cmd;
  const u32 sector = cmd->spec.sect_size;

  if (((dst | len) & (sector - 1)) != 0) {
    cmd->result = CARD_RESULT_INVALID_PARAM;
  } else if (CARDi_WaitPrevCommand()) {

    while (len > 0) {
      CARDi_WriteEnable();
      CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + 0));
      CARDi_SendSpiAddressingCommand(dst, COMM_SECTOR_ERASE);
      CARDi_CommandEnd(cmd->spec.erase_sector, cmd->spec.erase_sector_total);
      if (cmd->result != CARD_RESULT_SUCCESS) {
        break;
      }
      dst += sector;
      len -= sector;
    }
  }
}

void CARDi_EraseBackupSubSectorCore(u32 dst, u32 len) {
  CARDiCommandArg *const cmd = cardi_common.cmd;
  const u32 sector = cmd->spec.subsect_size;

  if (((dst | len) & (sector - 1)) != 0) {
    cmd->result = CARD_RESULT_INVALID_PARAM;
  } else if (CARDi_WaitPrevCommand()) {

    while (len > 0) {
      CARDi_WriteEnable();
      CARDi_CommandBegin((int)(1 + cmd->spec.addr_width + 0));
      CARDi_SendSpiAddressingCommand(dst, COMM_SUBSECTOR_ERASE);
      CARDi_CommandEnd(cmd->spec.erase_subsector,
                       cmd->spec.erase_subsector_total);
      if (cmd->result != CARD_RESULT_SUCCESS) {
        break;
      }
      dst += sector;
      len -= sector;
    }
  }
}

void CARDi_EraseChipCore(void) {
  if (CARDi_WaitPrevCommand()) {
    CARDiCommandArg *const cmd = cardi_common.cmd;
    static const u8 arg[1] = {
        COMM_CHIP_ERASE,
    };
    CARDi_WriteEnable();
    CARDi_CommandBegin(sizeof(arg));
    CARDi_CommArrayWrite(arg, sizeof(arg));
    CARDi_CommandEnd(cmd->spec.erase_chip, cmd->spec.erase_chip_total);
  }
}

void CARDi_SetWriteProtectCore(u16 stat) {
  if (CARDi_WaitPrevCommand()) {
    CARDiCommandArg *const cmd = cardi_common.cmd;
    int retry_count = 10;
    u8 arg[2];
    arg[0] = COMM_WRITE_STATUS;
    arg[1] = (u8)stat;
    do {
      CARDi_WriteEnable();
      CARDi_CommandBegin(1 + 1);
      CARDi_CommArrayWrite(&arg, sizeof(arg));
      CARDi_CommandEnd(5, 0);
    } while ((cmd->result == CARD_RESULT_TIMEOUT) && (--retry_count > 0));
  }
}

#if 0









static void CARDi_ReadIdCore(void)
{










    cardi_common.cmd->result = CARD_RESULT_UNSUPPORTED;
}




static void CARDi_ErasePageCore(u32 dst)
{
    CARDi_WriteEnable();
    CARDi_CommandBegin(1 + cardi_common.cmd->spec.addr_width + 0);
    CARDi_SendSpiAddressingCommand(dst, COMM_PAGE_ERASE);
    CARDi_CommandEnd(cardi_common.cmd->spec.erase_page, 0);
}

#endif
