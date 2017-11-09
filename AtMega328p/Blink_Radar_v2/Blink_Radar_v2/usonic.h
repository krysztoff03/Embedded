
/*
 * The board used is ATMega 328p (Mega 2560)
 * 
 */

#define US_PORT PORTB
#define US_PIN PINB
#define US_DDR  DDRB
#define US_TRIG PINB6 // Trigger defined on Pin 12
#define US_ECHO PINB7 // Echo defined on Pin 13


//Calculating width of high pulse in micro second.
#define US_TIME_OUT -12
#define US_NO_OBSTACLE -13

//Prototypes
void US_init(); // Initialize the ultrasonic sensor
void US_trig(); // Trigger a pulse

void US_init()
{
	US_DDR |= (1 << US_TRIG);
}

void US_trig()
{
	// Send a 10uS pulse on trigger line
	US_PORT |= (1 << US_TRIG); // High
	_delay_us(15); // Wait 15uS
	US_PORT &= ~(1 << US_TRIG); // Low
}

uint16_t pulse_width()
{
	uint32_t i,result; // Value goes from 0 to 65536
	//Wait for the rising edge
	for(i = 0;i < 600000;i++)
	{
		if(!(US_PIN & (1 << US_ECHO)))
		continue; // Line is still low, so wait
		else
		break; // High edge detected, so break
	}

	if(i == 600000)
	return US_TIME_OUT; // No Reply found from the ultrasonic sensor, time lapsed
	// High Edge found
	
	// Setup Timer1
	TCCR1A = 0x00; 
	TCCR1B = (1 << CS11); // Prescaler = F_CPU / 8
	TCNT1 = 0x00; //Initialize the counter

	// Counting the width of pulse
	
	// Now wait for the falling edge
	for(i = 0;i < 600000;i++)
	{
		if(US_PIN & (1 << US_ECHO))
		{
			if(TCNT1 > 60000) 
			break; 
			else 
			continue;
		}
		else
		break;
	}

	if(i == 600000)
	return US_NO_OBSTACLE; // Indicates time out

	//Falling edge found

	result = TCNT1;

	//Stop Timer1
	TCCR1B = 0x00;

	if(result > 60000)
	return US_NO_OBSTACLE; //No obstacle
	else
	return (result >> 1);
}