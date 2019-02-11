#ifndef _BSP_H_
#define _BSP_H_

uint32_t const BSP_TICKS_PER_SEC = static_cast<uint32_t>(100);

void BSP_init(void);
void BSP_ledOff(void);
void BSP_ledOn(void);

#endif  // _BSP_H_
