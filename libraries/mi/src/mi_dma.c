#include <nitro.h>
#include "../include/mi_dma.h"

#ifdef SDK_ARM9
#include <nitro/itcm_begin.h>

void MIi_DmaSetParameters(u32 dmaNo, u32 src, u32 dest, u32 ctrl, u32 mode) {
  OSIntrMode enabled;
  vu32 *p;

  if (!(mode & MIi_DMA_MODE_NOINT)) {
    enabled = OS_DisableInterrupts();
  }

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_SAD_WOFFSET);

  if (mode & MIi_DMA_MODE_SRC32) {
    MIiDmaClearSrc *srcp =
        (MIiDmaClearSrc *)((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
    srcp->b32 = src;
    src = (u32)srcp;
  } else if (mode & MIi_DMA_MODE_SRC16) {
    MIiDmaClearSrc *srcp =
        (MIiDmaClearSrc *)((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
    srcp->b16 = (u16)src;
    src = (u32)srcp;
  }

  *p = (vu32)src;
  *(p + 1) = (vu32)dest;
  *(p + 2) = (vu32)ctrl;

  if (mode & MIi_DMA_MODE_WAIT) {

    {
      u32 dummy = reg_MI_DMA0SAD;
    }
    {
      u32 dummy = reg_MI_DMA0SAD;
    }

    if (!(mode & MIi_DMA_MODE_NOCLEAR)) {

      if (dmaNo == MIi_DUMMY_DMA_NO) {
        *p = (vu32)MIi_DUMMY_SRC;
        *(p + 1) = (vu32)MIi_DUMMY_DEST;
        *(p + 2) = (vu32)MIi_DUMMY_CNT;
      }
    }
  }

  if (!(mode & MIi_DMA_MODE_NOINT)) {
    (void)OS_RestoreInterrupts(enabled);
  }

  if (mode & MIi_DMA_MODE_WAIT) {

    {
      u32 dummy = reg_MI_DMA0SAD;
    }
    {
      u32 dummy = reg_MI_DMA0SAD;
    }
  }
}
#include <nitro/itcm_end.h>
#else // ifdef SDK_ARM9
void MIi_DmaSetParameters(u32 dmaNo, u32 src, u32 dest, u32 ctrl, u32 mode) {
  OSIntrMode enabled;
  vu32 *p;

  if (!(mode & MIi_DMA_MODE_NOINT)) {
    enabled = OS_DisableInterrupts();
  }

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_SAD_WOFFSET);

  if (mode & MIi_DMA_MODE_SRC32) {
    MIiDmaClearSrc *srcp =
        (MIiDmaClearSrc *)((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
    srcp->b32 = src;
    src = (u32)srcp;
  } else if (mode & MIi_DMA_MODE_SRC16) {
    MIiDmaClearSrc *srcp =
        (MIiDmaClearSrc *)((u32)MIi_DMA_CLEAR_DATA_BUF + dmaNo * 4);
    srcp->b16 = (u16)src;
    src = (u32)srcp;
  }

  *p = (vu32)src;
  *(p + 1) = (vu32)dest;
  *(p + 2) = (vu32)ctrl;

  if (mode & MIi_DMA_MODE_WAIT) {

    u32 dummy = reg_MI_DMA0SAD;
  }

  if (!(mode & MIi_DMA_MODE_NOINT)) {
    (void)OS_RestoreInterrupts(enabled);
  }
}
#endif

void MIi_DmaFill32(u32 dmaNo, void *dest, u32 data, u32 size, BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size > 0) {
    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR32(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC32);
    } else {
      MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR32(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC32 |
                               MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size,
                   BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY32(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY32(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaSend32(u32 dmaNo, const void *src, volatile void *dest, u32 size,
                   BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, 0);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND32(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND32(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaRecv32(u32 dmaNo, volatile const void *src, void *dest, u32 size,
                   BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, 0);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV32(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaPipe32(u32 dmaNo, volatile const void *src, volatile void *dest,
                   u32 size, BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, 0);
  MIi_WARNING_ADDRINTCM(dest, 0);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE32(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE32(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaFill16(u32 dmaNo, void *dest, u16 data, u32 size, BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size > 0) {
    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR16(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC16);
    } else {
      MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR16(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_SRC16 |
                               MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size,
                   BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY16(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_COPY16(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaSend16(u32 dmaNo, const void *src, volatile void *dest, u32 size,
                   BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, 0);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND16(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_SEND16(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaRecv16(u32 dmaNo, volatile const void *src, void *dest, u32 size,
                   BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, 0);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_RECV16(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaPipe16(u32 dmaNo, volatile const void *src, volatile void *dest,
                   u32 size, BOOL dmaEnable) {
  vu32 *dmaCntp;

  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, 0);
  MIi_WARNING_ADDRINTCM(dest, 0);

  if (size > 0) {

    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    if (dmaEnable) {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE16(size),
                           MIi_DMA_MODE_WAIT);
    } else {
      MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SET_PIPE16(size),
                           MIi_DMA_MODE_WAIT | MIi_DMA_MODE_NOCLEAR);
    }
    MIi_Wait_AfterDMA(dmaCntp);
  }
}

void MIi_DmaFill32Async(u32 dmaNo, void *dest, u32 data, u32 size,
                        MIDmaCallback callback, void *arg, BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR32_IF(size),
                             MIi_DMA_MODE_SRC32);
      } else {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest,
                             MI_CNT_SET_CLEAR32_IF(size),
                             MIi_DMA_MODE_SRC32 | MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR32(size),
                             MIi_DMA_MODE_SRC32);
      } else {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR32(size),
                             MIi_DMA_MODE_SRC32 | MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaCopy32Async(u32 dmaNo, const void *src, void *dest, u32 size,
                        MIDmaCallback callback, void *arg, BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY32_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_COPY32_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY32(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_COPY32(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaSend32Async(u32 dmaNo, const void *src, volatile void *dest,
                        u32 size, MIDmaCallback callback, void *arg,
                        BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND32_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_SEND32_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND32(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_SEND32(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaRecv32Async(u32 dmaNo, volatile const void *src, void *dest,
                        u32 size, MIDmaCallback callback, void *arg,
                        BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_RECV32_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_RECV32(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaPipe32Async(u32 dmaNo, volatile const void *src,
                        volatile void *dest, u32 size, MIDmaCallback callback,
                        void *arg, BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);
  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_ASSERT_DEST_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(src, 0);
  MIi_WARNING_ADDRINTCM(dest, 0);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE32_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_PIPE32_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE32(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_PIPE32(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaFill16Async(u32 dmaNo, void *dest, u16 data, u32 size,
                        MIDmaCallback callback, void *arg, BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(dest, size);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR16_IF(size),
                             MIi_DMA_MODE_SRC16);
      } else {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest,
                             MI_CNT_SET_CLEAR16_IF(size),
                             MIi_DMA_MODE_SRC16 | MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_CLEAR16(size),
                             MIi_DMA_MODE_SRC16);
      } else {
        MIi_DmaSetParameters(dmaNo, data, (u32)dest, MI_CNT_SET_CLEAR16(size),
                             MIi_DMA_MODE_SRC16 | MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaCopy16Async(u32 dmaNo, const void *src, void *dest, u32 size,
                        MIDmaCallback callback, void *arg, BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY16_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_COPY16_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_COPY16(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_COPY16(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaSend16Async(u32 dmaNo, const void *src, volatile void *dest,
                        u32 size, MIDmaCallback callback, void *arg,
                        BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND16_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_SEND16_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_SEND16(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_SEND16(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaRecv16Async(u32 dmaNo, volatile const void *src, void *dest,
                        u32 size, MIDmaCallback callback, void *arg,
                        BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, size);
  MIi_WARNING_ADDRINTCM(dest, size);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_RECV16_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_RECV16(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

void MIi_DmaPipe16Async(u32 dmaNo, volatile const void *src,
                        volatile void *dest, u32 size, MIDmaCallback callback,
                        void *arg, BOOL dmaEnable) {
  MIi_ASSERT_DMANO(dmaNo);
  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);
  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_ASSERT_DEST_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(src, 0);
  MIi_WARNING_ADDRINTCM(dest, 0);

  MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

  if (size == 0) {
    MIi_CallCallback(callback, arg);
  } else {
    MI_WaitDma(dmaNo);

    if (callback) {
      OSi_EnterDmaCallback(dmaNo, callback, arg);
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE16_IF(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_PIPE16_IF(size), MIi_DMA_MODE_NOCLEAR);
      }
    } else {
      if (dmaEnable) {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_PIPE16(size),
                             0);
      } else {
        MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
                             MI_CNT_SET_PIPE16(size), MIi_DMA_MODE_NOCLEAR);
      }
    }
  }
}

BOOL MI_IsDmaBusy(u32 dmaNo) {
  vu32 *dmaCntp = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);

  MIi_ASSERT_DMANO(dmaNo);
  return (BOOL)((*(vu32 *)dmaCntp & REG_MI_DMA0CNT_E_MASK) >>
                REG_MI_DMA0CNT_E_SHIFT);
}

void MI_WaitDma(u32 dmaNo) {
  OSIntrMode enabled = OS_DisableInterrupts();
  vu32 *dmaCntp = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);

  MIi_ASSERT_DMANO(dmaNo);
  while (*dmaCntp & REG_MI_DMA0CNT_E_MASK) {
  }

  if (dmaNo == MIi_DUMMY_DMA_NO) {
    vu32 *p = MI_DMA_REGADDR(dmaNo, MI_DMA_REG_SAD_WOFFSET);
    *p = (vu32)MIi_DUMMY_SRC;
    *(p + 1) = (vu32)MIi_DUMMY_DEST;
    *(p + 2) = (vu32)MIi_DUMMY_CNT;
  }

  (void)OS_RestoreInterrupts(enabled);
}

void MI_StopDma(u32 dmaNo) {
  OSIntrMode enabled = OS_DisableInterrupts();
  vu32 *dmaCntp = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);

  MIi_ASSERT_DMANO(dmaNo);

  *dmaCntp &= ~(MI_DMA_TIMING_MASK | MI_DMA_CONTINUOUS_ON);
  *dmaCntp &= ~MI_DMA_ENABLE;

  {
    u32 dummy = dmaCntp[0];
  }
  {
    u32 dummy = dmaCntp[0];
  }

  if (dmaNo == MIi_DUMMY_DMA_NO) {
    vu32 *p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_SAD_WOFFSET);
    *p = (vu32)MIi_DUMMY_SRC;
    *(p + 1) = (vu32)MIi_DUMMY_DEST;
    *(p + 2) = (vu32)MIi_DUMMY_CNT;
  }

  (void)OS_RestoreInterrupts(enabled);
}

void MI_StopAllDma(void) {
  MI_StopDma(0);
  MI_StopDma(1);
  MI_StopDma(2);
  MI_StopDma(3);
}

void MI_DmaRestart(u32 dmaNo) {
  vu32 *dmaCntp = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);

  MIi_ASSERT_DMANO(dmaNo);
  *dmaCntp |= MI_DMA_ENABLE;
}

#ifdef SDK_ARM9

void MIi_CheckAnotherAutoDMA(u32 dmaNo, u32 dmaType) {
  int n;
  u32 dmaCnt;
  u32 timing;

  for (n = 0; n < MI_DMA_MAX_NUM; n++) {
    if (n == dmaNo) {
      continue;
    }

    dmaCnt = *(REGType32v *)(REG_DMA0CNT_ADDR + n * 12);

    if ((dmaCnt & MI_DMA_ENABLE) == 0) {
      continue;
    }

    timing = dmaCnt & MI_DMA_TIMING_MASK;

    if (timing == dmaType ||
        (timing == MI_DMA_TIMING_V_BLANK && dmaType == MI_DMA_TIMING_H_BLANK) ||
        (timing == MI_DMA_TIMING_H_BLANK && dmaType == MI_DMA_TIMING_V_BLANK)) {
      continue;
    }

    if (timing == MI_DMA_TIMING_DISP || timing == MI_DMA_TIMING_DISP_MMEM ||
        timing == MI_DMA_TIMING_CARD || timing == MI_DMA_TIMING_CARTRIDGE ||
        timing == MI_DMA_TIMING_GXFIFO || timing == MI_DMA_TIMING_V_BLANK ||
        timing == MI_DMA_TIMING_H_BLANK) {
      OS_TPanic("cannot start auto DMA at the same time.");
    }
  }
}

#endif // ifdef SDK_ARM9

void MIi_CheckDma0SourceAddress(u32 dmaNo, u32 src, u32 size, u32 dir) {
#ifdef SDK_TWL

  if (OS_IsRunOnTwl() && SCFG_IsDmacFixed()) {
    return;
  }
#endif

  if (dmaNo == 0) {
    u32 addStart;
    u32 addEnd;

    addStart = src & 0xff000000;

    switch (dir) {
    case MI_DMA_SRC_INC:
      addEnd = src + size;
      break;
    case MI_DMA_SRC_DEC:
      addEnd = src - size;
      break;
    default:
      addEnd = src;
      break;
    }
    addEnd &= 0xff000000;

    if (addStart == 0x04000000 || addStart >= 0x08000000 ||
        addEnd == 0x04000000 || addEnd >= 0x08000000) {
      OS_TPanic("illegal DMA0 source address.");
    }
  }
}

#if defined(SDK_ARM9) && defined(SDK_DEBUG)
void MIi_CheckAddressInTCM(u32 addr, u32 size) {
  u32 itcm = HW_ITCM; // ITCM fixed to HW_ITCM (=0x01ff8000)
  u32 dtcm = OS_GetDTCMAddress();

  SDK_WARNING(itcm >= addr + size || addr >= itcm + HW_ITCM_SIZE,
              "intend to do DMA in ITCM area (%x)", addr);
  SDK_WARNING(dtcm >= addr + size || addr >= dtcm + HW_DTCM_SIZE,
              "intend to do DMA in DTCM area (%x)", addr);
}
#endif

void MIi_SetDmaSrc16(u32 dmaNo, void *src) {
  vu32 *p;

  MIi_ASSERT_SRC_ALIGN2(src);
  MIi_WARNING_ADDRINTCM(src, 0); // size is unknown

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_SAD_WOFFSET);
  *p = (vu32)src;
}
void MIi_SetDmaSrc32(u32 dmaNo, void *src) {
  vu32 *p;

  MIi_ASSERT_SRC_ALIGN4(src);
  MIi_WARNING_ADDRINTCM(src, 0); // size is unknown

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_SAD_WOFFSET);
  *p = (vu32)src;
}
void MIi_SetDmaDest16(u32 dmaNo, void *dest) {
  vu32 *p;

  MIi_ASSERT_SRC_ALIGN2(dest);
  MIi_WARNING_ADDRINTCM(dest, 0); // size is unknown

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_DAD_WOFFSET);
  *p = (vu32)dest;
}
void MIi_SetDmaDest32(u32 dmaNo, void *dest) {
  vu32 *p;

  MIi_ASSERT_SRC_ALIGN4(dest);
  MIi_WARNING_ADDRINTCM(dest, 0); // size is unknown

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_DAD_WOFFSET);
  *p = (vu32)dest;
}

void MIi_SetDmaSize16(u32 dmaNo, u32 size) {
  vu32 *p;

  MIi_ASSERT_MUL2(size);
  MIi_ASSERT_SIZE(dmaNo, size / 2);

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);
  *p = (*p & ~REG_MI_DMA0CNT_WORDCNT_MASK) | (size / 2);
}

void MIi_SetDmaSize32(u32 dmaNo, u32 size) {
  vu32 *p;

  MIi_ASSERT_MUL4(size);
  MIi_ASSERT_SIZE(dmaNo, size / 4);

  p = (vu32 *)MI_DMA_REGADDR(dmaNo, MI_DMA_REG_CNT_WOFFSET);
  *p = (*p & ~REG_MI_DMA0CNT_WORDCNT_MASK) | (size / 4);
}
