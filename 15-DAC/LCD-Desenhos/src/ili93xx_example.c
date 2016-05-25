#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "smc.h"
#include "img.h"

/** Chip select number to be set */
#define ILI93XX_LCD_CS      1

struct ili93xx_opt_t g_ili93xx_display_opt;


//! DAC channel used for test
//#define DACC_CHANNEL        0 // (PB13)
#define DACC_CHANNEL        1 // (PB14)

//! DAC register base for test
#define DACC_BASE           DACC
//! DAC ID for test
#define DACC_ID             ID_DACC

/** Analog control value */
#define DACC_ANALOG_CONTROL (DACC_ACR_IBCTLCH0(0x02) \
| DACC_ACR_IBCTLCH1(0x02) \
| DACC_ACR_IBCTLDACCORE(0x01))


/** The maximal sine wave sample data (no sign) */
#define MAX_DIGITAL   (0x7ff)
/** The maximal (peak-peak) amplitude value */
#define MAX_AMPLITUDE (DACC_MAX_DATA)
/** The minimal (peak-peak) amplitude value */
#define MIN_AMPLITUDE (100)

/** SAMPLES per cycle */
#define SAMPLES (100)

/** Default frequency */
#define DEFAULT_FREQUENCY 1000
/** Min frequency */
#define MIN_FREQUENCY   200
/** Max frequency */
#define MAX_FREQUENCY   3000

/** Invalid value */
#define VAL_INVALID     0xFFFFFFFF


/**
*  Interrupt handler for TC0 interrupt.
*/static void configure_tc(void)
{
	/*
	* Aqui atualizamos o clock da cpu que foi configurado em sysclk init
	*
	* O valor atual está em : 120_000_000 Hz (120Mhz)
	*/
	uint32_t ul_sysclk = sysclk_get_cpu_hz();
	
	/*
	*	Ativa o clock do periférico TC 0
	*/
	pmc_enable_periph_clk(ID_TC0);
	
	// Configura TC para operar no modo de comparação e trigger RC
	
	tc_init(TC0,0,TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK5);

	// Valor para o contador de um em um segundo.
	uint32_t tempo = 32768 * 0.01;
	tc_write_rc(TC0,0,tempo);

	NVIC_EnableIRQ((IRQn_Type) ID_TC0);
	
	tc_enable_interrupt(TC0,0,TC_IER_CPCS);
	
	tc_start(TC0, 0);
}
void TC0_Handler(void){
	volatile uint32_t ul_dummy, status;
	static uint32_t valorDAC = 4095;		valorDAC |= 1<<12;
	//valorDAC++;

	ul_dummy = tc_get_status(TC0,0);
	UNUSED(ul_dummy);		/************************************************************************/
	/* Escreve um novo valor no DAC                                         */
	/************************************************************************/	status = dacc_get_interrupt_status(DACC_BASE);
	dacc_write_conversion_data(DACC_BASE, valorDAC);
}

uint32_t prox_ponto(){
	uint32_t amplitude;
	uint32_t frequencia;
	

}

void configure_LCD(void){
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
};


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

	ili93xx_draw_pixmap(0,
	ILI93XX_LCD_HEIGHT-100-1,
	240-1,
	100-1,
	&image_data_maua[0]);
	
	/************************************************************************/
	/* DAC                                                                  */
	/************************************************************************/
	
	/* Enable clock for DACC */
	sysclk_enable_peripheral_clock(DACC_ID);
	
	/* Reset DACC registers */
	dacc_reset(DACC_BASE);

	/* Half word transfer mode */
	dacc_set_transfer_mode(DACC_BASE, 0);

	dacc_set_timing(DACC_BASE, 0x08, 0, 0x10);

	/* Disable TAG and select output channel DACC_CHANNEL */
	dacc_set_channel_selection(DACC_BASE, DACC_CHANNEL);

	/* Enable output channel DACC_CHANNEL */
	dacc_enable_channel(DACC_BASE, DACC_CHANNEL);

	/* Set up analog current */
	dacc_set_analog_control(DACC_BASE, DACC_ANALOG_CONTROL);

	//configure_tc();
	
	int status;
	
	while (1) {
		status = dacc_get_interrupt_status(DACC_BASE);
		dacc_write_conversion_data(DACC_BASE, 4095/4);
	}
}

