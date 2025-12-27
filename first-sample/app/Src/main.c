#include "config.h"
#include <stdio.h>
#include "driver_systick.h"
#include "led.h"
#include "button.h"

int main(void)
 {
    config_drivers();
    config_bsp();

    printf("Init board...\n\r");

    uint64_t start_time = ticks_get();

    while (1)
    {   
        // blinky
        if((ticks_get() - start_time) >= 500)
        {
            led_toggle();
            start_time = ticks_get();
        }

        if(button_get_state())
        {
            printf("Button pressed!\n");
            while(!button_get_state());
        }
    }
}