#if !defined(LAYOUT_CAT1C_H)
#define LAYOUT_CAT1C_H

#define STACK_SIZE                      0x1000
#define RAMVECTORS_ALIGNMENT            128

#define SRAM_START_RESERVE              0
#define SRAM_PRIVATE_FOR_SROM           0x800 /* 2K Private SRAM for SROM (e.g. API processing). Reserved at the beginning */

/* RAM */
#define SRAM_BASE_ADDRESS               CY_SRAM_BASE  /* SRAM START */
#define TOTAL_RAM                       CY_SRAM_SIZE
#define CM0PLUS_SRAM_RESERVE            0x00020000  /* 16K cm0plus */
#define CM7_SRAM_NON_CACHE_RESERVE      0x00020000  /* 128K  :non-cacheable sram size */

/* FLASH */
#define CODE_FLASH_BASE_ADDRESS         CY_FLASH_LG_SBM_BASE  /* FLASH START */
#define TOTAL_FLASH                     (CY_FLASH_LG_SBM_SIZE + CY_FLASH_SM_SBM_SIZE)
#define CM0PLUS_CODE_FLASH_RESERVE      0x00020000     /* 512K CM0P FLASH SIZE */

/* In our code example, User app will run on either CM7_0 core or CM7_1 core. So we are giving the same FLASH/RAM base address and size for both the CM7_0/CM7_1 cores */
/* CM7_1 RAM and FLASH */
#if defined (__CM7_1_PRESENT)
    #define CM7_1_SRAM_RESERVE          0x000A0000 - CM0PLUS_SRAM_RESERVE - CM7_SRAM_NON_CACHE_RESERVE   /* (SRAM0/384KB) -> cm7_1 */
    #define CM7_1_CODE_FLASH_RESERVE    0x00020000                          /* 512K CM7_0 FLASH SIZE */
#else
    #define CM7_1_SRAM_RESERVE          0x000A0000 - CM0PLUS_SRAM_RESERVE - CM7_SRAM_NON_CACHE_RESERVE   /* (SRAM0/384KB) -> cm7_0 */
    #define CM7_1_CODE_FLASH_RESERVE    0x00020000                          /* 512K CM7_0 FLASH SIZE */
#endif

/* SRAM reservations */
#define BASE_SRAM_CM0P                  SRAM_BASE_ADDRESS + SRAM_START_RESERVE + SRAM_PRIVATE_FOR_SROM
#define SIZE_SRAM_CM0P                  CM0PLUS_SRAM_RESERVE - SRAM_START_RESERVE - SRAM_PRIVATE_FOR_SROM
#define BASE_SRAM_CM7_0                 SRAM_BASE_ADDRESS + CM0PLUS_SRAM_RESERVE
#define SIZE_SRAM_CM7_0                 CM7_1_SRAM_RESERVE
#define BASE_SRAM_CM7_1                 SRAM_BASE_ADDRESS + CM0PLUS_SRAM_RESERVE

#define BASE_SRAM_NON_CACHE             BASE_SRAM_CM7_1 + CM7_1_SRAM_RESERVE
#define SIZE_SRAM_NON_CACHE             CM7_SRAM_NON_CACHE_RESERVE

/* Code flash reservations */
#define BASE_CODE_FLASH_CM0P            CODE_FLASH_BASE_ADDRESS
#define SIZE_CODE_FLASH_CM0P            CM0PLUS_CODE_FLASH_RESERVE
#define BASE_CODE_FLASH_CM7_0           CODE_FLASH_BASE_ADDRESS + CM0PLUS_CODE_FLASH_RESERVE
#define SIZE_CODE_FLASH_CM7_0           CM7_1_CODE_FLASH_RESERVE
#define BASE_CODE_FLASH_CM7_1           CODE_FLASH_BASE_ADDRESS + CM0PLUS_CODE_FLASH_RESERVE

#endif /* LAYOUT_CAT1C_H */


/* [] END OF FILE */
