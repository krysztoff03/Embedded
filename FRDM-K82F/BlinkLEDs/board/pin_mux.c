/* This is a template file for pins configuration created by New Kinetis SDK 2.x Project Wizard. Enjoy! */

#include "fsl_device_registers.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "fsl_gpio.h"

#define BLUE_LED 10U
#define RED_LED 8U
#define GREEN_LED 9U

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Initialize all pins used in this example
 */
void BOARD_InitPins(void)
{
	// Enable PortC clock
	CLOCK_EnableClock(kCLOCK_PortC);

	// Configuration for RGB LED
	// Blue LED, PTC10
	PORT_SetPinMux(PORTC, BLUE_LED, kPORT_MuxAsGpio);
	// Red LED, PTC8
	PORT_SetPinMux(PORTC, RED_LED, kPORT_MuxAsGpio);
	// Green LED, PTC9
	PORT_SetPinMux(PORTC, GREEN_LED, kPORT_MuxAsGpio);


}
