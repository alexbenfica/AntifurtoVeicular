

/*Esses defines determinam quais microcontroladores serão utilizados.

Para utilizar o 16F84, deve estar selecionado 14 bits.

Para utilizar o 16C505, deve estar selecionado 12 bits.


*/

#define PIC_505


#ifdef PIC_505


#include <16C505.h>
#use delay(clock=4000000,RESTART_WDT)
#fuses INTRC,NOWDT,WDT





#define RELE PIN_C3
#define LED PIN_B2
#define SENSOR_PORTA PIN_B0
#define SIRENE PIN_C4
#define IGNICAO PIN_B4
#define JP1 PIN_C2
#define JP2 PIN_C1
#define DEBUG PIN_B1
#define CHAVE_SECRETA PIN_C5

#else

#include <16F84.h>
#fuses HS,NOWDT,PUT
#use delay(clock=4000000)

#define RELE PIN_A3
#define LED PIN_B2
#define SENSOR_PORTA PIN_B0
#define SIRENE PIN_A4
#define IGNICAO PIN_B4
#define JP1 PIN_A2
#define JP2 PIN_A1
#define DEBUG PIN_B1

#define CHAVE_SECRETA PIN_A0

#endif

