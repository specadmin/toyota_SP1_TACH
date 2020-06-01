//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <avr/wdt.h>
#include "lib/avr-misc/avr-misc.h"
#include "lib/avr-debug/debug.h"
#include "config.h"
//-----------------------------------------------------------------------------
#define MIN_SPEED           1       // km/h
#define MAX_SPEED           180     // km/h
#define SPEED_STEP          1       // km/h
//-----------------------------------------------------------------------------
#define MIN_RPM             800
#define MAX_RPM             8000
#define RPM_STEP            100
//-----------------------------------------------------------------------------
#define PULSES_PER_KM       2560    // periods
#define ROTATES_PER_PULSE   20      // ? (not sure)
#define KM                  1000    // meters
#define H                   3600    // seconds
#define M                   60      // seconds
#define T0_PRESCALER        1024    // timer 0 prescaler
#define T1_PRESCALER        256     // timer 1 prescaler
#define SK                  (F_CPU * H / PULSES_PER_KM / T1_PRESCALER / 2)
#define TK                  (F_CPU * ROTATES_PER_PULSE / T0_PRESCALER / 2)
//-----------------------------------------------------------------------------
#define speed_timer_enabled()   TCCR1B
#define tacho_timer_enabled()   TCCR0B
//-----------------------------------------------------------------------------
WORD speed;
WORD tacho;
bool speed_enabled;
bool tacho_enabled;
//-----------------------------------------------------------------------------
__inline void enable_speed_timer()
{
    TCCR1B = 0x0C;              // CTC mode 256 prescaler
    set_bit(TCCR1A, COM1B0);    // enable pin output
}
//-----------------------------------------------------------------------------
__inline void disable_speed_timer()
{
    TCCR1B = 0;
    clr_bit(TCCR1A, COM1B0);    // disable pin output
}
//-----------------------------------------------------------------------------
__inline void enable_tacho_timer()
{
    TCCR0A = 0x02;              // CTC mode
    TCCR0B = 0x05;              // 1024 prescaler
    set_bit(TCCR0A, COM0B0);    // enable pin output
}
//-----------------------------------------------------------------------------
__inline void disable_tacho_timer()
{
    TCCR0B = 0;
    clr_bit(TCCR0A, COM0B0);    // disable pin output
}
//-----------------------------------------------------------------------------
__inline void set_speed(WORD speed)
{
    OCR1A =  SK / speed - 1;
    if(TCNT1 > OCR1A)
    {
        TCNT1 = 0;
    }
    DVAR(speed);
}
//-----------------------------------------------------------------------------
__inline void set_tacho(WORD RPM)
{
    OCR0A =  TK / RPM - 1;
    if(OCR0A > TCNT0)
    {
        TCNT0 = 0;
    }
}
//-----------------------------------------------------------------------------
__inline bool speed_button_pushed()
{
    if(!test_bit(SPEED_BUTTON_PIN, SPEED_BUTTON_BIT))
    {
        mdelay(4);     // anti-jitter delay
        if(!test_bit(SPEED_BUTTON_PIN, SPEED_BUTTON_BIT))
        {
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
__inline bool tacho_button_pushed()
{
    if(!test_bit(TACHO_BUTTON_PIN, TACHO_BUTTON_BIT))
    {
        mdelay(4);     // anti-jitter delay
        if(!test_bit(TACHO_BUTTON_PIN, TACHO_BUTTON_BIT))
        {
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
int main()
{
    DEBUG_INIT();

    // configure outputs
    set_bits(DDRB,  2);
    set_bits(DDRD,  5);

    // configure pull-downs
    set_bits(DDRC,  3);   // for tacho transistor
    set_bits(DDRD,  6);   // for tacho encoder

    // configure pull-ups for encoders
    set_bits(DDRD, 0, 1);
    set_bits(PORTD, 0, 1);

    // configure pull-ups for speed encoder
    set_bit(SPEED_BUTTON_PORT, SPEED_BUTTON_BIT);
    set_bit(SPEED_A_PORT, SPEED_A_BIT);
    set_bit(SPEED_B_PORT, SPEED_B_BIT);

    // configure pull-ups for tacho encoder
    set_bit(TACHO_BUTTON_PORT, TACHO_BUTTON_BIT);
    set_bit(TACHO_A_PORT, TACHO_A_BIT);
    set_bit(TACHO_B_PORT, TACHO_B_BIT);

    // configure pin-change interrupts
    set_bit(PCMSK2, SPEED_ENCODER_INT);
    set_bit(PCMSK0, TACHO_ENCODER_INT);
    PCICR = 0x05;
    PCIFR = 0x05;

    set_speed(speed = 20);
    set_tacho(tacho = 800);

    enable_speed_timer();
    enable_tacho_timer();

    enable_interrupts();

    while(1)
    {
        if(speed_button_pushed())
        {
            if(speed_timer_enabled())
            {
                disable_speed_timer();
            }
            else
            {
                enable_speed_timer();
            }
            while(speed_button_pushed());
        }
        if(tacho_button_pushed())
        {
            if(tacho_timer_enabled())
            {
                disable_tacho_timer();
            }
            else
            {
                enable_tacho_timer();
            }
            while(tacho_button_pushed());
        }
    }
    return 0;
}
//-----------------------------------------------------------------------------
ISR(PCINT0_vect)
{
    if(!tacho_timer_enabled())
    {
        return;
    }
    mdelay(4);     // anti-jitter delay
    set_bit(PCIFR, PCIF0);   // clear pending interrupt flag
    if(test_bit(TACHO_A_PIN, TACHO_A_BIT) == test_bit(TACHO_B_PIN, TACHO_B_BIT))
    {
        if(tacho < MAX_RPM)
        {
            set_tacho(tacho += RPM_STEP);
        }
    }
    else
    {
        if(tacho > MIN_RPM)
        {
            set_tacho(tacho -= RPM_STEP);
        }
    }
    DVAR(tacho);
}
//-----------------------------------------------------------------------------
ISR(PCINT2_vect)
{
    if(!speed_timer_enabled())
    {
        return;
    }
    mdelay(4);     // anti-jitter delay
    set_bit(PCIFR, PCIF0);   // clear pending interrupt flag
    if(test_bit(SPEED_A_PIN, SPEED_A_BIT) == test_bit(SPEED_B_PIN, SPEED_B_BIT))
    {
        if(speed < MAX_SPEED)
        {
            set_speed(speed += SPEED_STEP);
        }
    }
    else
    {
        if(speed > MIN_SPEED)
        {
            set_speed(speed -= SPEED_STEP);
        }
    }
}
//-----------------------------------------------------------------------------
