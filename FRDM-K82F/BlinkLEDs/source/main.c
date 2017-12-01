/**
 * This is template for main module created by New Kinetis SDK 2.x Project Wizard. Enjoy!
 **/

/*******************************************************************************
 * RGB LED Program - Each LED will turn on with a delay between them.
 * The first is the blue one, then the red one and finally and the green one.
 ******************************************************************************/

#include "board.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BLUE_LED 10U
#define RED_LED 8U
#define GREEN_LED 9U

#define CLK_SPEED 150U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief delay a while.
 */
void delay(int time);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

// Delay function
/*
void delay()
{
    volatile uint32_t i = 0;
    for (i = 0; i < 2000000; ++i)
    {
        __asm("NOP");
    }
}
*/

void delay(int time)
{
	uint32_t delayTime = (CLK_SPEED / 15) * time;
	volatile uint32_t counter = 0U;
	for(counter = 0U;counter < delayTime;++counter)
	{
		__asm("NOP");
	}
}


/*!
 * @brief Application entry point.
 */
int main(void) {
	// kGPIO_DigitalOutput use as output pin, 1 = initial value
	gpio_pin_config_t led_config = {
			kGPIO_DigitalOutput,
			0, };

  /* Init board hardware. */
  BOARD_InitPins();
  BOARD_BootClockRUN();
  BOARD_InitDebugConsole();

  // Mux PTC10 as output
  GPIO_PinInit(GPIOC, 10U, &led_config);
  // Mux PTC8 as output
  GPIO_PinInit(GPIOC, 8U, &led_config);
  //Mux PTC9 as output
  GPIO_PinInit(GPIOC, 9U, &led_config);


  /* Add your code here */

  for(;;)
  {
	  // Blue LED on
	  GPIO_ClearPinsOutput(GPIOC, 1 << 10U);
	  delay(100000);
	  //Blue LED off
	  GPIO_SetPinsOutput(GPIOC, 1 << 10U);
	  delay(100000);

	  // Red LED on
	  GPIO_ClearPinsOutput(GPIOC, 1 << 8U);
	  delay(100000);
	  // Red LED off
	  GPIO_SetPinsOutput(GPIOC, 1 << 8U);
	  delay(100000);

	  // Green LED on
	  GPIO_ClearPinsOutput(GPIOC, 1 << 9U);
	  delay(100000);
	  // Green LED off
	  GPIO_SetPinsOutput(GPIOC, 1 << 9U);
	  delay(100000);

  }


}
