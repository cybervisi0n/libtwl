#ifndef _ROM_HEADER_H_
#define _ROM_HEADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>
#include <nitro/fs.h>

typedef struct {

  char title_name[12]; // Soft title name
  u32 game_code;       // Game code

  u16 maker_code;  // Maker code
  u8 machine_code; // Machine code
  u8 rom_type;     // Rom type
  u8 rom_size;     // Rom size

  u8 reserved_A[9]; // System Reserved A ( Set ALL 0 )

  u8 soft_version;              // Soft version
  u8 comp_arm9_boot_area : 1;   // Compress arm9 boot area
  u8 comp_arm7_boot_area : 1;   // Compress arm7 boot area
  u8 inspectCard : 1;           // The inspect card flag
  u8 disableClearMemoryPad : 1; // The IPL memory pad clear/disable flag
  u8 : 0;

  u32 main_rom_offset;      // ROM offset
  void *main_entry_address; // Entry point
  void *main_ram_address;   // RAM address
  u32 main_size;            // Module size

  u32 sub_rom_offset;      // ROM offset
  void *sub_entry_address; // Entry point
  void *sub_ram_address;   // RAM address
  u32 sub_size;            // Module size

  u32 fnt_offset; // ROM offset
  u32 fnt_size;   // Table size

  u32 fat_offset; // ROM offset
  u32 fat_size;   // Table size

  u32 main_ovt_offset; // ROM offset
  u32 main_ovt_size;   // Table size

  u32 sub_ovt_offset; // ROM offset
  u32 sub_ovt_size;   // Table size

  u8 reserved_A2[32];

  u32 own_size;

  u8 reserved_B[60]; // System Reserved B (Set 0)

  u8 nintendo_logo[0x9c];  // NINTENDO logo data
  u16 nintendo_logo_crc16; //            CRC-16

  u16 header_crc16; // ROM header CRC-16

  u8 reserved_C[32]; // Debugger Reserved (Set ALL 0)

} RomHeader;

#define GetRomHeaderAddr() ((RomHeader *)HW_ROM_HEADER_BUF)

#ifdef __cplusplus
}
#endif

#endif // _ROM_HEADER_H_
