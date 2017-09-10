#include "utils.h"

/* Parametrii sintetizatorului */
int M, ampl, threshold;
/* Note corespunzatoare clapelor */
int tones[NUM_KEYS];
/* Canale ADC pentru modulare in amplitudine si frecventa */
int chan[NUM_CHANS], channel;
/* Numarator pentru schimbarea timer2 */
int counter;
/* Moduri de generare a sunetelor si interpretare */
int sound_mode, sing_mode;

/* Notele cantecului si duratele lor */
int song[NUM_NOTES];
int dur[NUM_NOTES];

ISR(ADC_vect)
{
	if(channel == 0)
		M = ADC - POT_EQ;
	else if(channel == 1)
		ampl = (int)((int32_t)255 * ADC / MAX_ADC);
	else if(channel == 2) {
		if(ADC < 512 && sound_mode == 1) {
			sound_mode = 0;
			TCCR1B = 0;
			TIMSK1 = 0;
			OCR1A = 0;
			
			TCCR2A = 0;
			TCCR2B = 0;
			OCR2A = OCR2B = 0;
		}
		else if(ADC >= 512 && sound_mode == 0) {
			sound_mode = 1;
			/* Seteaza timer1 cu CTC, frecventa de 10kHz */
			TCCR1B = (1<<WGM12) | (1<<CS11);
			TIMSK1 |= (1<<OCIE1A);
			OCR1A = 99;
			
			TCCR2A = (3<<WGM20) | (2<<COM2A0) | (2<<COM2B0);
			TCCR2B = (1<<CS20);
			OCR2A = OCR2B = ampl;
		}
	}
	else if(channel == 3) {
		if(ADC < 512)
			sing_mode = 0;
		else
			sing_mode = 1;
	}
	
	channel = (channel + 1) % NUM_CHANS;
	ADMUX = (ADMUX & ~0x1f) | chan[channel];
	ADCSRA |= (1 << ADSC);
}

ISR(TIMER1_COMPA_vect)
{
	if(counter < threshold / 2)
		OCR2A = OCR2B = ampl;
	else if(counter < threshold)
		OCR2A = OCR2B = 0;
	else
		counter = 0;

	++counter;
}

void init_system()
{
	/* Seteaza porturile PD6 si PD7 ca iesiri pentru difuzoare */
	DDRD |= (1<<PD6) | (1<<PD7);
	PORTD = (1<<PD3) | (1<<PD4) | (1<<PD5);
	
	/* Seteaza porturile PB ca iesiri pentru activarea liniei la citire de clape */
	DDRB = 0xff;
	
	/* Seteaza porturile PC ca intrari pentru citirea coloanelor */
	DDRC = 0;
	
	/* Seteaza timer1 cu CTC, frecventa de 10kHz */
	/*TCCR1B = (1<<WGM12) | (1<<CS11);
    TIMSK1 |= (1<<OCIE1A);
    OCR1A = 99;
	
	TCCR2A = (3<<WGM20) | (2<<COM2A0) | (2<<COM2B0);
	TCCR2B = (1<<CS20);
	OCR2A = OCR2B = ampl;*/
	
	/* ADC cu tensiune de referinta AVCC, prescaler 32 si intreruperi */
	ADMUX = 0;
	ADMUX |= (1<<REFS0);
	ADCSRA = 0;
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (5<<ADPS0);
	
	/* Incepe citirea potentiometrilor */
	ADMUX = (ADMUX & ~(0x1f << MUX0)) | chan[0];
	ADCSRA |= (1 << ADSC);
}

void sing(int freq)
{
	if(sound_mode == 0) {
		/* Timer2 cu fastPWM cu TOP la OCR2A, prescaler 1024 si toggle pe OCR2A */
		TCCR2A = (1<<WGM21) | (1<<COM2A0);
		TCCR2B = (7<<CS20);
		OCR2A = F_CPU / 2048 / freq;
	}
	else
		threshold = SAMPLE_RATE / freq;
}

void no_sing()
{
	if(sound_mode == 0) {
		TCCR2A = 0;
		TCCR2B = 0;
		TIMSK2 = 0;
		OCR2A = 0;
		OCR2B = 0;
	}
	else
		threshold = 0;
}

void read_keys()
{
	int i, j;
	
	for(i = 0; i < 6; ++i) {
		PORTB |= (1<<i);
		_delay_ms(2);
		for(j = 0; j < 8; ++j)
			if(PINC & (1<<j)) {
				PORTB &= ~(1<<i);
				sing(tones[8 * i + j] + M);
				return;
			}
		PORTB &= ~(1<<i);
	}
	
	PORTB |= (1<<PB6);
	_delay_ms(2);
	for(i = 0; i < 6; ++i)
		if(PINC & (1<<i)) {
			sing(tones[48 + i] + M);
			PORTB &= ~(1<<PB6);
			return;
		}
	
	no_sing();
	PORTB &= ~(1<<PB6);
}

/* Interpreteaza melodia */
void play_song()
{
	int i;
	
	_delay_ms(100);
	for(i = 0; i < NUM_NOTES; ++i) {
		if(i == NUM_NOTES - 1) {
			no_sing();
			_delay_ms(2);
		}
		sing(tones[song[i]]);
		if(sound_mode == 0)
			_delay_ms(8 * dur[i]);
		else
			_delay_ms(dur[i]);
	}
	no_sing();
}

/* Verifica daca melodia a fost cantata bine */
void check_song()
{
	int current_note = 0, pressed = 0, prev_pressed;
	int i, j;
	
	while(1) {
		if(current_note == NUM_NOTES) {
			for(i = 0; i < 5; ++i) {
				PORTD &= ~((1<<PD3) | (1<<PD4) | (1<<PD5));
				_delay_ms(100);
				PORTD |= (1<<PD3) | (1<<PD4) | (1<<PD5);
				_delay_ms(100);
			}
			return;
		}
		prev_pressed = pressed;
		pressed = 0;
		for(i = 0; i < 6; ++i) {
			PORTB |= (1<<i);
			_delay_ms(2);
			for(j = 0; j < 8; ++j)
				if(PINC & (1<<j)){
					pressed = 1;
					if(song[current_note] == 8 * i + j) {
						sing(tones[song[current_note]]);
						break;
					}
					else {
						sing(50);
						no_sing();
						return;
					}
				}
			PORTB &= ~(1<<i);
			if(pressed)
				break;
		}
		
		PORTB |= (1<<PB6);
		_delay_ms(2);
		for(i = 0; i < 8; ++i)
			if(PINC & (1<<i)) {
				pressed = 1;
				if(song[current_note] == 48 + i) {
					sing(tones[song[current_note]]);
					break;
				}
				else {
					sing(50);
					no_sing();
					return;
				}
			}
		PORTB &= ~(1<<PB6);
		
		if(pressed == 0) {
			no_sing();
			if(prev_pressed == 1)
				++current_note;
		}
	}
}

int main()
{
	/* Initializeaza cananelele ADC */
	chan[0] = PA3;
	chan[1] = PA4;
	chan[2] = PA5;
	chan[3] = PA6;
	/* Initializeaza notele */
	init_tones(tones);
	/* Initializeaza notele cantecului */
	init_song(song, dur);
	/* Initializeaza intreruperile */
	sei();
	init_system();

	while(1) {
		if(sing_mode == 1) {
			play_song();
			check_song();
		}
		else
			read_keys();
	}
	return 0;
}
