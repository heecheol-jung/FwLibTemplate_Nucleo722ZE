// Firmware library timer delay

#ifndef FL_TIMER_DELAY_H
#define FL_TIMER_DELAY_H

extern TIM_HandleTypeDef htim2;

__STATIC_INLINE void fl_timer2_delay_us(volatile uint32_t microseconds)
{
  uint32_t clk_cycle_start = __HAL_TIM_GET_COUNTER(&htim2);

    /* Go to number of cycles for system */
    // Timer2 clock : 180 MHz(APB1 timer clock)
    //                1 us = 108000000 / 1000000 = 108 clocks
    // microsecond *= (1080000000 / 1000000) -> theoretical formula
    microseconds *= 108;

    /* Delay till end */
    while ((uint32_t)(__HAL_TIM_GET_COUNTER(&htim2) - clk_cycle_start) < microseconds)
    {
      __ASM volatile ("NOP");
    }
}

#endif
