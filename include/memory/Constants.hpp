#ifndef _MEMORY_CONSTANTS_HPP
#define _MEMORY_CONSTANTS_HPP

// General memory map
#define ROM0_OFFSET 0x0000
#define ROM1_OFFSET 0x4000
#define VRAM_OFFSET 0x8000
#define SRAM_OFFSET 0xA000
#define WRAM_OFFSET 0xC000
#define ECHO_OFFSET 0xE000
#define OAM_OFFSET 0xFE00
#define OAM_END_OFFSET 0xFEA0
#define IO_OFFSET 0xFF00
#define HRAM_OFFSET 0xFF80
#define IO_INTERRUPT_ENABLE 0xFFFF

// Memory sections sizes
#define ROM0_SIZE (ROM1_OFFSET - ROM0_OFFSET)
#define ROM1_SIZE (VRAM_OFFSET - ROM1_OFFSET)
#define ROM_SIZE (VRAM_OFFSET - ROM0_OFFSET)
#define ROM_BANK_SIZE ROM0_SIZE
#define VRAM_SIZE (SRAM_OFFSET - VRAM_OFFSET)
#define SRAM_SIZE (WRAM_OFFSET - SRAM_OFFSET)
#define WRAM_SIZE (ECHO_OFFSET - WRAM_OFFSET)
#define ECHO_SIZE (OAM_OFFSET - ECHO_OFFSET)
#define OAM_SIZE (OAM_END_OFFSET - OAM_OFFSET)
#define IO_SIZE (HRAM_OFFSET - IO_OFFSET)
#define HRAM_SIZE (IO_INTERRUPT_ENABLE - HRAM_OFFSET)

// CGB mode parameters (banked VRAM and WRAM)
#define WRAM_BANK_NUM 8
#define WRAM_BANK_SIZE 0x1000
#define VRAM_BANK_NUM 2
#define VRAM_BANK_SIZE 0x2000

// IO Registers (organised by memory mapping)
#define IO_JOYPAD            0xFF00

#define IO_SERIAL_DATA       0xFF01
#define IO_SERIAL_CONTROL    0xFF02

#define IO_TIMER_DIVIDER     0xFF04
#define IO_TIMER_COUNTER     0xFF05
#define IO_TIMER_MODULO      0xFF06
#define IO_TIMER_CONTROL     0xFF07

#define IO_INTERRUPT_REQUEST 0xFF0F

#define IO_CH1_SWEEP         0xFF10
#define IO_CH1_PATTERN       0xFF11
#define IO_CH1_ENVELOPE      0xFF12
#define IO_CH1_FREQLOW       0xFF13
#define IO_CH1_CONTROL       0xFF14

#define IO_CH2_PATTERN       0xFF16
#define IO_CH2_ENVELOPE      0xFF17
#define IO_CH2_FREQLOW       0xFF18
#define IO_CH2_CONTROL       0xFF19

#define IO_CH3_ENABLE        0xFF1A
#define IO_CH3_LENGTH        0xFF1B
#define IO_CH3_LEVEL         0xFF1C
#define IO_CH3_FREQLOW       0xFF1D
#define IO_CH3_CONTROL       0xFF1E

#define IO_CH4_LENGTH        0xFF20
#define IO_CH4_ENVELOPE      0xFF21
#define IO_CH4_COUNTER       0xFF22
#define IO_CH4_CONTROL       0xFF23

#define IO_AUDIO_LEVELS      0xFF24
#define IO_AUDIO_OUTPUT      0xFF25
#define IO_AUDIO_ENABLE      0xFF26

#define IO_WAVEPATTERN_START 0xFF30
#define IO_WAVEPATTERN_END   0xFF3F

#define IO_LCD_CONTROL       0xFF40
#define IO_LCD_STATUS        0xFF41
#define IO_SCROLL_Y          0xFF42
#define IO_SCROLL_X          0xFF43
#define IO_COORD_Y           0xFF44
#define IO_COORD_COMPARE     0xFF45

#define IO_OAM_DMA           0xFF46

#define IO_BG_PALETTE        0xFF47
#define IO_OBJ0_PALETTE      0xFF48
#define IO_OBJ1_PALETTE      0xFF49
#define IO_WINDOW_Y          0xFF4A
#define IO_WINDOW_X          0xFF4B

#define IO_KEY0              0xFF4C
#define IO_KEY1              0xFF4D

#define IO_VRAM_BANK         0xFF4F

#define IO_BOOTROM_UNMAP     0xFF50

#define IO_HDMA_SOURCEHIGH   0xFF51
#define IO_HDMA_SOURCELOW    0xFF52
#define IO_HDMA_DESTHIGH     0xFF53
#define IO_HDMA_DESTLOW      0xFF54
#define IO_HDMA_SETTINGS     0xFF55

#define IO_INFRARED          0xFF56

#define IO_BGPALETTE_INDEX   0xFF68
#define IO_BGPALETTE_DATA    0xFF69
#define IO_OBJPALETTE_INDEX  0xFF6A
#define IO_OBJPALETTE_DATA   0xFF6B
#define IO_OBJPRIORITY       0xFF6C

#define IO_WRAM_BANK         0xFF70

#define IO_UNDOCUMENTED_FF72 0xFF72
#define IO_UNDOCUMENTED_FF73 0xFF73
#define IO_UNDOCUMENTED_FF74 0xFF74
#define IO_UNDOCUMENTED_FF75 0xFF75
#define IO_PCM12             0xFF76
#define IO_PCM34             0xFF77

#define IO_WAVEPATTERN_SIZE IO_WAVEPATTERN_END - IO_WAVEPATTERN_START + 1


// Bus definitions

#define EXTERNAL_BUS_LOW_START  0x0000
#define EXTERNAL_BUS_LOW_END    0x7FFF
#define EXTERNAL_BUS_HIGH_START 0xA000
#define EXTERNAL_BUS_HIGH_END   0xFDFF

#define VIDEO_BUS_START         0x8000
#define VIDEO_BUS_END           0x9FFF

#endif
