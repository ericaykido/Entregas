/**
 * IMT - Rafael Corsi
 * 
 * PIO - 07
 *  Configura o PIO do SAM4S (Banco A, pino 19) para operar em
 *  modo de output. Esse pino está conectado a um LED, que em 
 *  lógica alta apaga e lógica baixa acende.
*/

#include <asf.h>
#include "Driver/pio_maua.h"
#include "Driver/pmc_maua.h"

/*
 * Prototypes
 */

/** 
 * Definição dos pinos
 * Pinos do uC referente aos LEDS.
 *
 * O número referente ao pino (PIOAxx), refere-se ao
 * bit que deve ser configurado no registrador para alterar
 * o estado desse bit específico.
 *
 * exe : O pino PIOA_19 é configurado nos registradores pelo bit
 * 19. O registrador PIO_SODR configura se os pinos serão nível alto.
 * Nesse caso o bit 19 desse registrador é referente ao pino PIOA_19
 *
 * ----------------------------------
 * | BIT 19  | BIT 18  | ... |BIT 0 |
 * ----------------------------------
 * | PIOA_19 | PIOA_18 | ... |PIOA_0|
 * ----------------------------------
 */
#define PIN_LED_BLUE 19
#define PIN_LED_RED 20
#define PIN_LED_GREEN 20
#define PIN_BUTTON 3
#define time 100
/** 
 * Definição dos ports
 * Ports referentes a cada pino
 */
#define PORT_LED_BLUE PIOA
#define PORT_LED_GREEN PIOA
#define PORT_LED_RED PIOC
#define PORT_BUTTON PIOB


/**
 * Main function
 * 1. configura o clock do sistema
 * 2. desabilita wathdog
 * 3. ativa o clock para o PIOA
 * 4. ativa o controle do pino ao PIO
 * 5. desabilita a proteção contra gravações do registradores
 * 6. ativa a o pino como modo output
 * 7. coloca o HIGH no pino
 */

int main (void)
{

	/**
	* Inicializando o clock do uP
	*/
	sysclk_init();
	
	/** 
	*  Desabilitando o WathDog do uP
	*/
	WDT->WDT_MR = WDT_MR_WDDIS;
		

	_pmc_enable_clock_periferico(ID_PIOA);
	_pmc_enable_clock_periferico(ID_PIOC);
	_pmc_enable_clock_periferico(ID_PIOB);
	
	_pio_set_output(PIOA, PIN_LED_BLUE,1,0);
	_pio_set_output(PIOA, PIN_LED_GREEN,1,0);
	_pio_set_output(PIOC, PIN_LED_RED,1,0);
	 //31.6.1 PIO Enable Register
	// 1: Enables the PIO to control the corresponding pin (disables peripheral control of the pin).	
	
	//PIOC->PIO_OER |=  (1 << PIN_LED_RED );

	// 31.6.10 PIO Set Output Data Register
	// 1: Sets the data to be driven on the I/O line.
	//PIOA->PIO_SODR = (0 << PIN_LED_BLUE );
	PIOC->PIO_SODR = (1 << PIN_LED_RED);

	config_pin_input(PIOB, 1 << PIN_BUTTON);

	/**
	*	Loop infinito
	*/
	
		while(1){

            /*
             * Utilize a função delay_ms para fazer o led piscar na frequência
             * escolhida por você.
             */
			/* 
			Estamos setando os dois pinos set e clear em nível alto, com o delay de 1000 segundos.
			*/

				
			//if (((PIOB->PIO_PDSR >> 3) & 1 ) == 0) {
			if (_pio_get_output_data_status(PIOB, PIN_BUTTON) == 0) {
				_pio_set(PIOC, (1 << PIN_LED_RED ));
				
				delay_ms(time);
				_pio_clear(PIOA, (1 << PIN_LED_BLUE));
				
				delay_ms(time);
			    _pio_clear(PIOA, (1 << PIN_LED_GREEN ));
				
				delay_ms(time);
				_pio_clear(PIOC, (1 << PIN_LED_RED ));
				
				delay_ms(time);
				_pio_set(PIOA, (1 << PIN_LED_BLUE ));
				
				delay_ms(time);
				_pio_set(PIOA, (1 << PIN_LED_GREEN ));
				
				delay_ms(time);
			} else {
				_pio_clear(PIOC, (1 << PIN_LED_RED ));
				_pio_set(PIOA, (1 << PIN_LED_BLUE ));
				_pio_set(PIOA, (1 << PIN_LED_GREEN ));
				
			}
	}
}