/*

MIT License

Copyright (c) 2023 Derek Fountain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ssd1306.h"

#define OLED_I2C    (i2c0)
#define OLED_FREQ   400000
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C
#define OLED_SCK    4
#define OLED_SDA    5

/* https://lastminuteengineers.com/rotary-encoder-arduino-tutorial/ is useful */

/* Keep these in a block of 3, in this order */
#define ENC_A	 6     // Marked CLK on some devices
#define ENC_B	 7     // Marked DT on some devices
#define ENC_SW	 8     // Marked SW or Switch on some devices

/* Something to show on the screen */
uint8_t previous_value = 255;
uint8_t value          = 0;

void encoder_callback( uint gpio, uint32_t events ) 
{
  uint32_t gpio_state = (gpio_get_all() >> ENC_A) & 0x0003;
  uint8_t  enc_value  = (gpio_state & 0x03);
	
  static bool counterclockwise_fall = 0;
  static bool clockwise_fall        = 0;
	
  if( gpio == ENC_A ) 
  {
    if( (!clockwise_fall) && (enc_value == 0x02) )
      clockwise_fall = 1; 

    if( (counterclockwise_fall) && (enc_value == 0x00) )
    {
      /* Counter clockwise event */
      clockwise_fall        = 0;
      counterclockwise_fall = 0;

      /* Do application action here */
      value--;
      printf("CCW\n");
    }
  }	
  else if( gpio == ENC_B )
  {
    if( (!counterclockwise_fall) && (enc_value == 0x01) )
      counterclockwise_fall = 1;

    if( (clockwise_fall) && (enc_value == 0x00) )
    {
      /* Clockwise event */
      clockwise_fall        = 0;
      counterclockwise_fall = 0;

      /* Do application action here */
      value++;
      printf("CW\n");
    }    
  }
  else if( gpio == ENC_SW )
  {
    /*
     * Switch event, set to interrupt on falling edge, so this is a click down.
     * Debounce is left as an exercise for the reader :)
     */
    value = 0;
    printf("Reset\n");
  }
}


void main( void )
{
  stdio_init_all();

  /*
   * First, set up the screen. It's an I2C device.
   */
  i2c_init(OLED_I2C, OLED_FREQ);
  gpio_set_function( OLED_SCK, GPIO_FUNC_I2C ); gpio_pull_up( OLED_SCK );
  gpio_set_function( OLED_SDA, GPIO_FUNC_I2C ); gpio_pull_up( OLED_SDA );

  ssd1306_t display;
  display.external_vcc=false;

  ssd1306_init( &display, OLED_WIDTH, OLED_HEIGHT, OLED_ADDR, OLED_I2C );
  ssd1306_clear( &display );


  /* Rotary encoder, 3 GPIOs */
  gpio_init( ENC_SW ); gpio_set_dir( ENC_SW, GPIO_IN ); // gpio_disable_pulls(ENC_SW);
  gpio_init( ENC_A );  gpio_set_dir( ENC_A, GPIO_IN );  // gpio_disable_pulls(ENC_A);
  gpio_init( ENC_B );  gpio_set_dir( ENC_B, GPIO_IN );  // gpio_disable_pulls(ENC_B);

  /* Set the handler for all 3 GPIOs */
  gpio_set_irq_enabled_with_callback( ENC_SW, GPIO_IRQ_EDGE_FALL, true, &encoder_callback );
  gpio_set_irq_enabled( ENC_A, GPIO_IRQ_EDGE_FALL, true );
  gpio_set_irq_enabled( ENC_B, GPIO_IRQ_EDGE_FALL, true );

  /* Loop, not doing much for this example. Just update the display if value changes */
  while( true ) 
  {
    if( value != previous_value )
    {
      uint8_t value_str[4];
      snprintf( value_str, 4, "%d", value );

      ssd1306_clear(&display);
      ssd1306_draw_string(&display, 10, 10, 2, value_str);
      ssd1306_show(&display);

      previous_value = value;
    }
    sleep_ms(5);
  }
}
