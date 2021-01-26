/*
 * digital_thermometer.c
 *
 * Created: 15.01.2021 14:28:07
 * 
 */ 

#include <avr/io.h>
#include <util/delay.h>

#include "HD44780.h"	// Biblioteka obsługująca wyświetlacz LCD

#define F_CPU 1000000UL // Taktowanie mikrokontrolera, wewnętrzny oscylator 1000000 (Hz)

#define VREF_DIVIDED 25
#define SR 8

/* Makra do łatwiejszej obsługi diod LED */

#define BLUE_LED_ON		PORTB |= (1 << PB0)
#define BLUE_LED_OFF	PORTB &= ~ (1 << PB0)
#define GREEN_LED_ON	PORTD |= (1 << PD0)
#define GREEN_LED_OFF	PORTD &= ~(1 << PD0)
#define YELLOW_LED_ON	PORTD |= (1 << PD3)
#define YELLOW_LED_OFF	PORTD &= ~(1 << PD3)
#define RED_LED_ON		PORTD |= (1 << PD6)
#define RED_LED_OFF		PORTD &= ~(1 << PD6);

uint16_t adc_value;
uint16_t voltage_value;
int16_t decimal_value;
int16_t temp;
char decimal_buffer[33];

void ADC_init(void); // Inicjalizacja ADC

void get_average(void); // Funkcja odpowiedzialna za obliczanie średniej wartości ADC

void IO_PORTS_init(void); // Funkca odpowiedzialna z inicjalizację portów IO

uint16_t measurement(uint8_t adc_channel); // Funkcja odpiwadająca za pomiar


int main(void)
{
	LCD_Initalize();
	
	ADC_init();

	IO_PORTS_init();
	
	for(int i = 0 ; i<8 ; i++)
	{
		get_average();
		_delay_us(250);
	}
	
	while (1)
	{
		voltage_value = adc_value * VREF_DIVIDED;
		temp = voltage_value / 10 - 500;
		
		decimal_value = temp/10;
		
		/*if(temp%10 >= 5) decimal_value = temp/10 + 1;
		else decimal_value = temp/10;*/
		
		if(decimal_value >= 19 && decimal_value <= 22)
		{
			GREEN_LED_ON;
			YELLOW_LED_OFF;
			RED_LED_OFF;
			BLUE_LED_OFF;
		}
		else if(decimal_value < 19)
		{
			BLUE_LED_ON;
			GREEN_LED_OFF;
			YELLOW_LED_OFF;
			RED_LED_OFF;
		}
		else if(decimal_value >= 23 && decimal_value <= 25)
		{
			YELLOW_LED_ON;
			BLUE_LED_OFF;
			GREEN_LED_OFF;
			RED_LED_OFF;
		}
		else if(decimal_value > 25)
		{
			RED_LED_ON;
			BLUE_LED_OFF;
			GREEN_LED_OFF;
			YELLOW_LED_OFF;
		}
		
		itoa(decimal_value, decimal_buffer, 10);
		LCD_Clear();
		LCD_GoTo(2, 0);
		LCD_WriteText("Temperatura:");
		LCD_GoTo(7, 1);
		LCD_WriteText(decimal_buffer);
		LCD_WriteText("C");
		_delay_ms(400);
		get_average();
	}
}

void ADC_init(void)
{
	ADMUX |= (1 << REFS1) | (1 << REFS0);					// Vref = 2.56V (wewnętrzen źródło odniesienia)
	ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1);	// włączenie ADC, prescaller = 8
}

void get_average(void)
{
	static uint16_t sr[SR];
	static uint8_t idx;
	uint32_t sr1 = 0;
	uint8_t i;
	
	sr[idx] = measurement(PA5);
	idx++;
	if(idx > 7)
	{
		idx=0;
		for(i = 0; i<SR ; i++) sr1 += sr[i];
		
		if(sr1%SR*10/SR >= 5) sr1 = sr1/SR+1;
		else sr1 /= SR;
		
		adc_value = sr1;
	}
	
}

void IO_PORTS_init(void)
{
	DDRA &= ~(1 << PA5) | ~(1 << PA6) | ~(1 << PA4);
	DDRB |= (1 << PB0);
	DDRD |= (1 << PD0) | (1 << PD3) | (1 << PD6);
}

uint16_t measurement(uint8_t adc_channel)
{
	ADMUX |= (ADMUX & 0xF8) | adc_channel;
	
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	
	return ADC;
}
