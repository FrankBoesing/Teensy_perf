/* This library code is placed under the GPL-v3.0 license
 * Copyright (c) 2020 Frank Bösing
 */

#include "Teensy_perf.h"

#define __DSB() asm volatile ("dsb 0xF":::"memory")
#define __ISB() asm volatile ("isb" );

#if defined(__IMXRT1062__)
#define SCB_CCSIDR_ASSOCIATIVITY_Pos        3U                                            /*!< SCB CCSIDR: Associativity Position */
#define SCB_CCSIDR_ASSOCIATIVITY_Msk       (0x3FFUL << SCB_CCSIDR_ASSOCIATIVITY_Pos)      /*!< SCB CCSIDR: Associativity Mask */
#define SCB_CCSIDR_NUMSETS_Pos             13U                                            /*!< SCB CCSIDR: NumSets Position */
#define SCB_CCSIDR_NUMSETS_Msk             (0x7FFFUL << SCB_CCSIDR_NUMSETS_Pos)           /*!< SCB CCSIDR: NumSets Mask */
#define SCB_DCCISW_WAY_Pos                 30U                                            /*!< SCB DCCISW: Way Position */
#define SCB_DCCISW_WAY_Msk                 (3UL << SCB_DCCISW_WAY_Pos)
#define SCB_DCCISW_SET_Pos                  5U                                            /*!< SCB DCCISW: Set Position */
#define SCB_DCCISW_SET_Msk                 (0x1FFUL << SCB_DCCISW_SET_Pos)                /*!< SCB DCCISW: Set Mask */
/* Cache Size ID Register Macros */
#define CCSIDR_WAYS(x)         (((x) & SCB_CCSIDR_ASSOCIATIVITY_Msk) >> SCB_CCSIDR_ASSOCIATIVITY_Pos)
#define CCSIDR_SETS(x)         (((x) & SCB_CCSIDR_NUMSETS_Msk      ) >> SCB_CCSIDR_NUMSETS_Pos      )
#endif

void emptyfunc() {};

uint32_t __measure(void (*func)(void)) {
  delay(50);

  noInterrupts();

#if defined(__IMXRT1062__)
  // Disable D-Cache
  register uint32_t ccsidr;
  register uint32_t sets;
  register uint32_t ways;

  //Clean & invalidate D-Cache
  ccsidr = SCB_ID_CCSIDR;
  sets = (uint32_t)(CCSIDR_SETS(ccsidr));
  do {
    ways = (uint32_t)(CCSIDR_WAYS(ccsidr));
    do {
      SCB_CACHE_DCCISW = (((sets << SCB_DCCISW_SET_Pos) & SCB_DCCISW_SET_Msk) |
                     ((ways << SCB_DCCISW_WAY_Pos) & SCB_DCCISW_WAY_Msk)  );
    } while (ways-- != 0U);
  } while (sets-- != 0U);

  SCB_CACHE_ICIALLU = 0; // invalidate I-Cache
#endif
  __DSB();
  __ISB();

  register uint32_t cycles_start = ARM_DWT_CYCCNT;
  __DSB();
  __ISB();
  func();
  register uint32_t cycles_stop = ARM_DWT_CYCCNT;
  interrupts();


  return cycles_stop - cycles_start;
}

uint32_t measure(void (*func)(void)) {
  uint32_t cycles_empty = __measure(emptyfunc);
  uint32_t cycles = __measure(func);
  return cycles - cycles_empty;
}