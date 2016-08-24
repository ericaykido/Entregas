/**
 *	20-UART 
 * Prof. Rafael Corsi
 *
 *    (e.g., HyperTerminal on Microsoft Windows) with these settings:
 *   - 115200 bauds
 *   - 8 bits of data
 *   - No parity
 *   - 1 stop bit
 *   - No flow control
 */

#include "asf.h"
#include "conf_board.h"
#include "conf_clock.h"

/************************************************************************/
/* Configura��es                                                        */
/************************************************************************/

#define STRING_EOL    "\r"
#define STRING_VERSAO "-- "BOARD_NAME" --\r\n" \
					  "-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

#define CONF_UART_BAUDRATE 115200		
#define CONF_UART          CONSOLE_UART

/** 
 * LEDs
 */ 

#define PIN_LED_BLUE	19
#define PIN_LED_RED		20
#define PIN_LED_GREEN	20
#define time			100

/** 
 * Defini��o dos ports
 * Ports referentes a cada pino
 */
#define PORT_LED_BLUE	PIOA
#define PORT_LED_GREEN	PIOA
#define PORT_LED_RED	PIOC


/**
 * Define os IDs dos perif�ricos associados aos pinos
 */
#define ID_LED_BLUE		ID_PIOA
#define ID_LED_GREEN	ID_PIOA
#define ID_LED_RED		ID_PIOC

/**
 *	Define as masks utilziadas
 */
#define MASK_LED_BLUE	(1u << PIN_LED_BLUE)
#define MASK_LED_GREEN	(1u << PIN_LED_GREEN)
#define MASK_LED_RED	(1u << PIN_LED_RED)

void TC0_Handler(Pio port, uint32_t mask)
{
	volatile uint32_t ul_dummy;

	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0,0);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	/** Muda o estado do LED */
	if (pio_get_output_data_status(port, mask) == 0) {
		pio_set(port, mask);
		} else {
		pio_clear(port, mask);
	}
	
}

static void configure_tc(void)
{
	/*
	* Aqui atualizamos o clock da cpu que foi configurado em sysclk init
	*
	* O valor atual est'a em : 120_000_000 Hz (120Mhz)
	*/
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	/*
	*	Ativa o clock do perif�rico TC 0
	* 
	*/
	pmc_enable_periph_clk(ID_TC0);

	/*
	* Configura TC para operar no modo de compara��o e trigger RC
	* devemos nos preocupar com o clock em que o TC ir� operar !
	*
	* Cada TC possui 3 canais, escolher um para utilizar.
	*
	* Configura��es de modo de opera��o :
	*	#define TC_CMR_ABETRG (0x1u << 10) : TIOA or TIOB External Trigger Selection 
	*	#define TC_CMR_CPCTRG (0x1u << 14) : RC Compare Trigger Enable 
	*	#define TC_CMR_WAVE   (0x1u << 15) : Waveform Mode 
	*
	* Configura��es de clock :
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK1 : Clock selected: internal MCK/2 clock signal 
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK2 : Clock selected: internal MCK/8 clock signal 
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK3 : Clock selected: internal MCK/32 clock signal 
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK4 : Clock selected: internal MCK/128 clock signal
	*	#define  TC_CMR_TCCLKS_TIMER_CLOCK5 : Clock selected: internal SLCK clock signal 
	*
	*	MCK		= 120_000_000
	*	SLCK	= 32_768		(rtc)
	*
	* Uma op��o para achar o valor do divisor � utilizar a funcao
	* tc_find_mck_divisor()
	*/
	tc_init(TC0,0,TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK5);
	
	/*
	* Aqui devemos configurar o valor do RC que vai trigar o reinicio da contagem
	* devemos levar em conta a frequ�ncia que queremos que o TC gere as interrup��es
	* e tambem a frequencia com que o TC est� operando.
	*
	* Devemos configurar o RC para o mesmo canal escolhido anteriormente.
	*	
	*   ^ 
	*	|	Contador (incrementado na frequencia escolhida do clock)
	*   |
	*	|	 	Interrupcao	
	*	|------#----------- RC
	*	|	  /
	*	|   /
	*	| /
	*	|-----------------> t
	*
	*
	*/
	tc_write_rc(TC0,0,8192);
	
	/*
	* Devemos configurar o NVIC para receber interrup��es do TC 
	*/
	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	
	/*
	* Op��es poss�veis geradoras de interrup��o :
	* 
	* Essas configura��es est�o definidas no head : tc.h 
	*
	*	#define TC_IER_COVFS (0x1u << 0)	Counter Overflow 
	*	#define TC_IER_LOVRS (0x1u << 1)	Load Overrun 
	*	#define TC_IER_CPAS  (0x1u << 2)	RA Compare 
	*	#define TC_IER_CPBS  (0x1u << 3)	RB Compare 
	*	#define TC_IER_CPCS  (0x1u << 4)	RC Compare 
	*	#define TC_IER_LDRAS (0x1u << 5)	RA Loading 
	*	#define TC_IER_LDRBS (0x1u << 6)	RB Loading 
	*	#define TC_IER_ETRGS (0x1u << 7)	External Trigger 
	*/
	tc_enable_interrupt(TC0,0,TC_IER_CPCS);
	
	tc_start(TC0, 0);
}


/************************************************************************/
/* Configura UART                                                       */
/************************************************************************/


void config_uart(void){
	
	/* configura pinos */
	gpio_configure_group(PINS_UART0_PIO, PINS_UART0, PINS_UART0_FLAGS);
	
	/* ativa clock */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);

	/* Configura��o UART */
	const usart_serial_options_t uart_serial_options = {
		.baudrate   = CONF_UART_BAUDRATE,
		.paritytype = UART_MR_PAR_NO,
		.stopbits   = 0
	};
	
	stdio_serial_init((Usart *)CONF_UART, &uart_serial_options);
}


/************************************************************************/
/* Display Menu                                                         */
/************************************************************************/
static void display_menu(void)
{
	puts(" 1 : exibe novamente esse menu \n\r"
		 " 2 : Ativa o LED  AZUL \n\r"
		 " 3 : Desliga o LED  AZUL \n\r "
		  " 4 : Ativa o LED  VERDE \n\r"
		  " 5 : Desliga o LED  VERDE \n\r "
		   " 6 : Ativa o LED  VERMELHO \n\r"
		   " 7 : Desliga o LED  VERMELHO \n\r "
		 );
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/
int main(void)
{
	uint8_t uc_key;

	/* Initialize the system */
	sysclk_init();
	board_init();

	/* Configure LED 1 */
	pmc_enable_periph_clk(ID_LED_BLUE);
	pio_set_output(PORT_LED_BLUE  , MASK_LED_BLUE	,1,0,0);
	pmc_enable_periph_clk(ID_LED_BLUE);
	pio_set_output(PORT_LED_GREEN  , MASK_LED_GREEN	,1,0,0);
	pmc_enable_periph_clk(ID_LED_BLUE);
	pio_set_output(PORT_LED_RED  , MASK_LED_RED	,1,0,1);
	

	/* Initialize debug console */
	config_uart();
	
	/* frase de boas vindas */
	puts(" ---------------------------- \n\r"
	 	 " Bem vindo terraquio !		\n\r"
		 " ---------------------------- \n\r");
		 
	/* display main menu */
	display_menu();
	
	/**
	* Inicializando o clock do uP
	*/
	sysclk_init();
	
	/** 
	*  Desabilitando o WathDog do uP
	*/
	WDT->WDT_MR = WDT_MR_WDDIS;
		
	/**
	* Ativa clock nos perif�ricos
	*/
	pmc_enable_periph_clk(ID_LED_BLUE);
	pmc_enable_periph_clk(ID_LED_GREEN);
	pmc_enable_periph_clk(ID_LED_RED);
	
	

	while (1) {
			
	usart_serial_getchar((Usart *)CONSOLE_UART, &uc_key);

		switch (uc_key) {
			case '1':
				display_menu();
				break;
			case '2':
				pio_clear(PORT_LED_BLUE, MASK_LED_BLUE);
				puts("Led ON \n\r");
				configure_tc();
				TC0_Handler(PORT_LED_BLUE, MASK_LED_BLUE);
				break;
			case '3' :
				pio_set(PORT_LED_BLUE, MASK_LED_BLUE);
				puts("Led OFF \n\r");
	
				break;
			case '4' :
				pio_clear(PORT_LED_GREEN, MASK_LED_GREEN);
				puts("Led ON \n\r");
				configure_tc();
				//TC0_Handler_green();
			    break;
			case '5' :
				pio_set(PORT_LED_GREEN, MASK_LED_GREEN);
				puts("Led OFF \n\r");
				break;
			case '6' :
				pio_set(PORT_LED_RED, MASK_LED_RED);
				puts("Led OFF \n\r");
				configure_tc();
				//TC0_Handler_red();
				break;
			case '7' :
				pio_clear(PORT_LED_RED, MASK_LED_RED);
				puts("Led ON \n\r");
				
				break;
			default:
				printf("Opcao nao definida: %d \n\r", uc_key);
		}	
	}
}
