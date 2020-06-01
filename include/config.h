#ifndef CONFIG_H
#define CONFIG_H
//----------------------------------------------------------------------------
#include <avr/io.h>
//----------------------------------------------------------------------------
#define DEBUG_PORT          PORTB
#define DEBUG_DDR           DDRB
#define DEBUG_BIT           4
#define DEBUG_BAUD_RATE     1152000

#define SPEED_BUTTON_PORT   PORTD
#define SPEED_BUTTON_PIN    PIND
#define SPEED_BUTTON_BIT    2

#define SPEED_A_PORT        PORTD
#define SPEED_A_PIN         PIND
#define SPEED_A_BIT         3

#define SPEED_B_PORT        PORTD
#define SPEED_B_PIN         PIND
#define SPEED_B_BIT         4
#define SPEED_ENCODER_INT   PCINT19

#define TACHO_BUTTON_PORT   PORTD
#define TACHO_BUTTON_PIN    PIND
#define TACHO_BUTTON_BIT    7

#define TACHO_A_PORT        PORTB
#define TACHO_A_PIN         PINB
#define TACHO_A_BIT         0
#define TACHO_ENCODER_INT   PCINT0

#define TACHO_B_PORT        PORTB
#define TACHO_B_PIN         PINB
#define TACHO_B_BIT         1
//----------------------------------------------------------------------------
#endif

