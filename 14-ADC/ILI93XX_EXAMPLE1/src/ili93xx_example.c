#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "smc.h"
#include <math.h>

/** Chip select number to be set */
#define ILI93XX_LCD_CS      1

struct ili93xx_opt_t g_ili93xx_display_opt;

#define PIN_PUSHBUTTON_1_MASK	PIO_PB3
#define PIN_PUSHBUTTON_1_PIO	PIOB
#define PIN_PUSHBUTTON_1_ID		ID_PIOB
#define PIN_PUSHBUTTON_1_TYPE	PIO_INPUT
#define PIN_PUSHBUTTON_1_ATTR	PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALL_EDGE

#define PIN_PUSHBUTTON_2_MASK	PIO_PC12
#define PIN_PUSHBUTTON_2_PIO	PIOC
#define PIN_PUSHBUTTON_2_ID		ID_PIOC
#define PIN_PUSHBUTTON_2_TYPE	PIO_INPUT
#define PIN_PUSHBUTTON_2_ATTR	PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALL_EDGE

/************************************************************************/
/* ADC                                                                     */
/************************************************************************/

/** Size of the receive buffer and transmit buffer. */
#define BUFFER_SIZE     (100)
/** Reference voltage for ADC,in mv. */
#define VOLT_REF        (3300)
/* Tracking Time*/
#define TRACKING_TIME    1
/* Transfer Period */
#define TRANSFER_PERIOD  1
/* Startup Time*/
#define STARTUP_TIME ADC_STARTUP_TIME_4

/** The maximal digital value */
#define MAX_DIGITAL     (4095)

/* Redefinir isso */
#define ADC_POT_CHANNEL 5

/** adc buffer */
static int16_t gs_s_adc_values[BUFFER_SIZE] = { 0 };

/************************************************************************/
/* GLOBAL                                                                */
/************************************************************************/
uint32_t adc_value_old;

/************************************************************************/
/* LCD                                                                  */
/************************************************************************/
/** Chip select number to be set */
#define ILI93XX_LCD_CS      1

struct ili93xx_opt_t g_ili93xx_display_opt;

void atualiza_lcd(double valor){
	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(0,90,ILI93XX_LCD_WIDTH,200);
	ili93xx_set_foreground_color(COLOR_BLACK);
	ili93xx_draw_circle(ILI93XX_LCD_WIDTH/2, ILI93XX_LCD_HEIGHT/2, 40);
	double angulo = valor * M_PI/4095;
	double x, y;
	x = ILI93XX_LCD_WIDTH/2 - cos(angulo)*40;
	y = ILI93XX_LCD_HEIGHT/2 - sin(angulo)*40;
	ili93xx_draw_line(ILI93XX_LCD_WIDTH/2, ILI93XX_LCD_HEIGHT/2, x, y);
	char buffer[10];
	snprintf(buffer, 10, "%4.0f Owms", valor);
	ili93xx_draw_string(130, 110, (uint8_t *)buffer);
}

/************************************************************************/
/* prototype                                                            */
/************************************************************************/

void configure_LCD();
void configure_ADC();

/************************************************************************/
/* Configs                                                              */
/************************************************************************/
void configure_LCD(){
	/** Enable peripheral clock */
	pmc_enable_periph_clk(ID_SMC);

	/** Configure SMC interface for Lcd */
	smc_set_setup_timing(SMC, ILI93XX_LCD_CS, SMC_SETUP_NWE_SETUP(2)
	| SMC_SETUP_NCS_WR_SETUP(2)
	| SMC_SETUP_NRD_SETUP(2)
	| SMC_SETUP_NCS_RD_SETUP(2));
	smc_set_pulse_timing(SMC, ILI93XX_LCD_CS, SMC_PULSE_NWE_PULSE(4)
	| SMC_PULSE_NCS_WR_PULSE(4)
	| SMC_PULSE_NRD_PULSE(10)
	| SMC_PULSE_NCS_RD_PULSE(10));
	smc_set_cycle_timing(SMC, ILI93XX_LCD_CS, SMC_CYCLE_NWE_CYCLE(10)
	| SMC_CYCLE_NRD_CYCLE(22));
	#if ((!defined(SAM4S)) && (!defined(SAM4E)))
	smc_set_mode(SMC, ILI93XX_LCD_CS, SMC_MODE_READ_MODE
	| SMC_MODE_WRITE_MODE
	| SMC_MODE_DBW_8_BIT);
	#else
	smc_set_mode(SMC, ILI93XX_LCD_CS, SMC_MODE_READ_MODE
	| SMC_MODE_WRITE_MODE);
	#endif
	/** Initialize display parameter */
	g_ili93xx_display_opt.ul_width = ILI93XX_LCD_WIDTH;
	g_ili93xx_display_opt.ul_height = ILI93XX_LCD_HEIGHT;
	g_ili93xx_display_opt.foreground_color = COLOR_BLACK;
	g_ili93xx_display_opt.background_color = COLOR_WHITE;

	/** Switch off backlight */
	aat31xx_disable_backlight();

	/** Initialize LCD */
	ili93xx_init(&g_ili93xx_display_opt);

	/** Set backlight level */
	aat31xx_set_backlight(AAT31XX_AVG_BACKLIGHT_LEVEL);

	ili93xx_set_foreground_color(COLOR_WHITE);
	ili93xx_draw_filled_rectangle(0, 0, ILI93XX_LCD_WIDTH,
	ILI93XX_LCD_HEIGHT);
	/** Turn on LCD */
	ili93xx_display_on();
	ili93xx_set_cursor_position(0, 0);
}

void configure_ADC(void){
	
	/* Enable peripheral clock. */
	pmc_enable_periph_clk(ID_ADC);
	/* Initialize ADC. */
	/*
	* Formula: ADCClock = MCK / ( (PRESCAL+1) * 2 )
	* For example, MCK = 64MHZ, PRESCAL = 4, then:
	* ADCClock = 64 / ((4+1) * 2) = 6.4MHz;
	*/
	/* Formula:
	*     Startup  Time = startup value / ADCClock
	*     Startup time = 64 / 6.4MHz = 10 us
	*/
	adc_init(ADC, sysclk_get_cpu_hz(), 6400000, STARTUP_TIME);
	/* Formula:
	*     Transfer Time = (TRANSFER * 2 + 3) / ADCClock
	*     Tracking Time = (TRACKTIM + 1) / ADCClock
	*     Settling Time = settling value / ADCClock
	*
	*     Transfer Time = (1 * 2 + 3) / 6.4MHz = 781 ns
	*     Tracking Time = (1 + 1) / 6.4MHz = 312 ns
	*     Settling Time = 3 / 6.4MHz = 469 ns
	*/
	adc_configure_timing(ADC, TRACKING_TIME	, ADC_SETTLING_TIME_3, TRANSFER_PERIOD);

	adc_configure_trigger(ADC, ADC_TRIG_SW, 0);

	//adc_check(ADC, sysclk_get_cpu_hz());

	/* Enable channel for potentiometer. */
	adc_enable_channel(ADC, ADC_POT_CHANNEL);

	/* Enable the temperature sensor. */
	adc_enable_ts(ADC);

	/* Enable ADC interrupt. */
	NVIC_EnableIRQ(ADC_IRQn);

	/* Start conversion. */
	adc_start(ADC);

	//adc_read_buffer(ADC, gs_s_adc_values, BUFFER_SIZE);

	//adc_get_channel_value(ADC, ADC_POT_CHANNEL);

	/* Enable PDC channel interrupt. */
	adc_enable_interrupt(ADC, ADC_ISR_EOC5);
}

/**
 *  Configure Timer Counter 0 to generate an interrupt every 1s.
 */
static void configure_tc(void)
{
	/*
	* Aqui atualizamos o clock da cpu que foi configurado em sysclk init
	*
	* O valor atual est� em : 120_000_000 Hz (120Mhz)
	*/
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	/*
	*	Ativa o clock do perif�rico TC 0
	*/
	pmc_enable_periph_clk(ID_TC0);
	
	// Configura TC para operar no modo de compara��o e trigger RC
	
	tc_init(TC0,0,TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK5);

	// Valor para o contador de um em um segundo.
	//tc_write_rc(TC0,0,32768);
	tc_write_rc(TC0,0,65536);

	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	
	tc_enable_interrupt(TC0,0,TC_IER_CPCS);
	
	tc_start(TC0, 0);
}


/************************************************************************/
/* Interruptions                                                        */
/************************************************************************/

/**
 * \brief Timmer handler (100ms) starts a new conversion.
 */
void TC0_Handler(void)
{
	volatile uint32_t ul_dummy;

	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC0,0);
	

	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	
	adc_start(ADC);
	
}

/**
 * \brief ADC interrupt handler.
 * Entramos aqui quando a conversao for concluida.
 */
void ADC_Handler(void)
{
	uint32_t resistencia;
	uint32_t status ;

	status = adc_get_status(ADC);
	
	/* Checa se a interrup��o � devido ao canal 5 */
	if ((status & ADC_ISR_EOC5)) {
		resistencia = adc_get_channel_value(ADC, ADC_POT_CHANNEL);
		if ((resistencia > adc_value_old + 2) || (resistencia < adc_value_old - 2))
		{
			atualiza_lcd(resistencia);
		}
		adc_value_old = resistencia;
	}
	
	
}


/**
 * \brief Application entry point for smc_lcd example.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	sysclk_init();
	board_init();

	configure_LCD();
	configure_ADC();
	configure_tc();
	
	ili93xx_set_foreground_color(COLOR_BLACK);
	ili93xx_draw_string(10, 20, (uint8_t *)"14 - ADC");
	
	atualiza_lcd(0.8);
	
	while (1) {
	}
}

