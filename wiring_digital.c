/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "nrf.h"
#include "variant.h"
#include "nrf_gpio.h"
#include "wiring_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

const uint32_t g_ADigitalPinMap[] =
{
  // P0
  0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 ,
  8 , 9 , 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 30, 31,

  // P1
  32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47
};

void pinMode( uint32_t ulPin, uint32_t ulMode )
{
  if (ulPin >= PINS_COUNT) {
    return;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  NRF_GPIO_Type * port = nrf_gpio_pin_port_decode(&ulPin);

  // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
  switch ( ulMode )
  {
    case INPUT:
      // Set pin to input mode
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case INPUT_PULLUP:
      // Set pin to input mode with pull-up resistor enabled
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case INPUT_PULLUP_SENSE:
      // Set pin to input mode with pull-up resistor enabled and sense when Low
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Low        << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case INPUT_PULLDOWN:
      // Set pin to input mode with pull-down resistor enabled
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Pulldown    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case INPUT_PULLDOWN_SENSE:
      // Set pin to input mode with pull-down resistor enabled and sense when High
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Pulldown    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_High       << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case INPUT_SENSE_HIGH:
      // Set pin to input mode and sense when High
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_High       << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case INPUT_SENSE_LOW:
      // Set pin to input mode and sense when Low
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Low        << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_S0S1:
      // Set pin to output mode, sink to standard and source to standard
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_H0S1:
      // Set pin to output mode, sink to high drive and source to standard
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_H0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_S0H1:
      // Set pin to output mode, sink to standard and source to high drive
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0H1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_H0H1:
      // Set pin to output mode, sink to high drive and source to high drive
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_H0H1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_D0S1:
      // Set pin to output mode, sink to disconnect and source to standard
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_D0S1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_D0H1:
      // Set pin to output mode, sink to disconnect and source to high drive
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_D0H1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_S0D1:
      // Set pin to output mode, sink to standard and source to disconnect
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    case OUTPUT_H0D1:
      // Set pin to output mode, sink to high drive and source to disconnect
      port->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_DRIVE_H0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                           | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);
    break;

    default:
      // do nothing
    break ;
  }
}

void digitalWrite( uint32_t ulPin, uint32_t ulVal )
{
  if (ulPin >= PINS_COUNT) {
    return;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  NRF_GPIO_Type * port = nrf_gpio_pin_port_decode(&ulPin);

  switch ( ulVal )
  {
    case LOW:
      port->OUTCLR = (1UL << ulPin);
    break ;

    default:
      port->OUTSET = (1UL << ulPin);
    break ;
  }
}

int digitalRead( uint32_t ulPin )
{
  if (ulPin >= PINS_COUNT) {
    return 0;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  NRF_GPIO_Type * port = nrf_gpio_pin_port_decode(&ulPin);
  uint32_t const bm = (1UL << ulPin);

  // Return bit in OUT or IN depending on configured direction
  return (bm  & ((port->DIR & bm) ? port->OUT : port->IN)) ? 1 : 0;
}

void digitalToggle( uint32_t pin )
{
  int state = 1 - digitalRead(pin);
  digitalWrite(pin, state);
}

void ledOn(uint32_t pin)
{
  digitalWrite(pin, LED_STATE_ON);
}

void ledOff(uint32_t pin)
{
  digitalWrite(pin, 1-LED_STATE_ON);
}


#ifdef __cplusplus
}
#endif
