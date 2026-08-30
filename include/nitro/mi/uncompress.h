#ifndef NITRO_MI_UNCOMPRESS_H_
#define NITRO_MI_UNCOMPRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

typedef enum {
  MI_COMPRESSION_LZ = 0x10,      // LZ77
  MI_COMPRESSION_HUFFMAN = 0x20, // Huffman
  MI_COMPRESSION_RL = 0x30,      // Run Length
  MI_COMPRESSION_DIFF = 0x80,    // Differential filter

  MI_COMPRESSION_TYPE_MASK = 0xf0,
  MI_COMPRESSION_TYPE_EX_MASK = 0xff
} MICompressionType;

typedef struct {
  u32 compParam : 4;
  u32 compType : 4;
  u32 destSize : 24;

} MICompressionHeader;

typedef struct {
  u16 srcNum;             // Source data / Number of bytes
  u16 srcBitNum : 8;      // Number of bits per one source data element
  u16 destBitNum : 8;     // Number of bits per one destination data element
  u32 destOffset : 31;    // Number to add to source data
  u32 destOffset0_on : 1; // Flag for whether to add an offset to 0 data.
} MIUnpackBitsParam;

void MI_UnpackBits(const void *srcp, void *destp, MIUnpackBitsParam *paramp);

void MI_UncompressLZ8(const void *srcp, void *destp);

void MI_UncompressLZ16(const void *srcp, void *destp);

void MI_UncompressHuffman(const void *srcp, void *destp);

void MI_UncompressRL8(const void *srcp, void *destp);

void MI_UncompressRL16(const void *srcp, void *destp);

void MI_UncompressRL32(register const void *srcp, register void *destp);

void MI_UnfilterDiff8(const void *srcp, void *destp);

void MI_UnfilterDiff16(const void *srcp, void *destp);

void MI_UnfilterDiff32(register const void *srcp, register void *destp);

void MI_FilterDiff8(register const void *srcp, register void *destp,
                    register u32 size, register BOOL bitsize);

void MI_FilterDiff16(register const void *srcp, register void *destp,
                     register u32 size, register BOOL bitsize);

void MI_FilterDiff32(register const void *srcp, register void *destp,
                     register u32 size, register BOOL bitsize);

static inline u32 MI_GetUncompressedSize(const void *srcp) {
  return (*(u32 *)srcp >> 8);
}

static inline MICompressionType MI_GetCompressionType(const void *srcp) {
  return (MICompressionType)(*(u32 *)srcp & MI_COMPRESSION_TYPE_MASK);
}

void MIi_UncompressBackward(void *bottom);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
