#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/binary_info.h"
#include "test.pio.h"

const uint PIN_LED = 25;
const uint PIN_NRST = 5;

void blink(int count)
{
    for (int i = count * 2 - 1; i > 0; i--)
    {
        gpio_put(PIN_LED, i % 2);
        sleep_ms(100);
    }
    gpio_put(PIN_LED, 0);
}

void wait_reset()
{
    while (!gpio_get(PIN_NRST));
}

void initialiseIO()
{
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_init(PIN_NRST);
    for (int i = 0; i < 8; i++)
    {
        gpio_set_function(PIN_D0 + i, GPIO_FUNC_PIO0);
    }
}

extern void analyse();

static inline bool isRead(uint32_t data)
{
    return !!(data & (1u << 3));
}

static inline void out(PIO pio, uint sm, u_int32_t value) {
    pio_sm_put(pio, 0, value | 0xFF00);
}

int main()
{
    bi_decl(bi_program_description("Acorn Atom PL8 Interface Test"));
    bi_decl(bi_1pin_with_name(PIN_LED, "On-board LED"));

    stdio_init_all();

    initialiseIO();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &test_program);
    printf("Loaded program at %d\n", offset);
    test_program_init(pio, 0, offset);
    pio_sm_set_enabled(pio, 0, true);

    int count = 0;

    uint store[16];
    for (int i = 0; i< 16; i++) {
        store[i] = i * 10;
    }

    for (;;)
    {
            u_int32_t reg = pio_sm_get_blocking(pio, 0);
            uint address = (reg >> 12) & 0xF;
            if (isRead(reg)) {
                out(pio,0,store[address]);
            } else {
                uint data = (reg >> (16 + 4)) & 0xFF;
                store[address] = data;
                if (data == 255 && address == 15) {
                    multicore_launch_core1(analyse);
                }
            }
    }
}
