#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed 
#endif

#define en 0x08
#define rs 0x04
#define lcd_port PORTB

#include <avr/io.h>
#include <util/delay.h>

char sel;
char prev_sel;
char aux_sel;
unsigned int delay_ms = 0;

void lcd_cmd(unsigned int);
void lcd_data(unsigned char);
void lcd_init();
void lcd_clear();
void lcd_print(char *);
void ADC_init();
int ADC_read(unsigned char);
void print_resistance(unsigned long);
void resistance_meter();
void print_capacitance(unsigned long);
void capacitance_meter();
void custom_delay();
unsigned long _micros();
void voltage_meter();

int main(void)
{
  DDRB = 0xff;
  DDRL = 0X00;
  PORTL = 0x0f;
  DDRC = 0xf0;
  PORTC = 0x0f;
  ADC_init();
  lcd_init();
  while(1){
    if ((PINC & 0x01) && (PINC & 0x02)) {
      sel = 0;
    resistance_meter();
  }
  else if (!(PINC & 0x01) && (PINC & 0x02)) {
    sel = 1;
    capacitance_meter();
    
  }
  else if ((PINC & 0x01) && !(PINC & 0x02)){
    sel = 2;
    voltage_meter();
  }
  else {
    sel = 3;
    current_meter();
  }
    _delay_ms(100);
  }
}

void lcd_cmd(unsigned int cmd) {
  lcd_port &= 0x0F;
  lcd_port |= (cmd & 0xF0); //sending higher nibble
  lcd_port &= ~rs; //rs = 0
  lcd_port |= en; //en = 1
  _delay_ms(1);
  lcd_port &= ~en; //en = 0

  _delay_ms(10);

  lcd_port &= 0x0F;
  lcd_port |= ((cmd << 4) & 0xF0); //sending lower nibble
  lcd_port &= ~rs; //rs = 0
  lcd_port |= en; //en = 1
  _delay_ms(1);
  lcd_port &= ~en; //en = 0
}

void lcd_data(unsigned char data) {
  lcd_port &= 0x0F;
  lcd_port |= (data & 0xF0); //sending higher nibble
  lcd_port |= rs; //rs = 1
  lcd_port |= en; //en = 1
  _delay_ms(1);
  lcd_port &= ~en; //en = 0

  _delay_ms(10);

  lcd_port &= 0x0F;
  lcd_port |= ((data << 4) & 0xF0); //sending lower nibble
  lcd_port |= rs;  //rs = 1
  lcd_port |= en; //en = 1
  _delay_ms(1);
  lcd_port &= ~en; //en = 0
}

void lcd_init() {
  lcd_cmd(0x33);
  _delay_ms(10);
  lcd_cmd(0x32);
  _delay_ms(10);
  lcd_cmd(0x28); // for using 2 lines and 5X7 matrix of LCD 4 bit
  _delay_ms(10);
  lcd_cmd(0x0c); // turn display ON; 0x0c for no cursor, 0x0f for cursor blinking
  _delay_ms(10);
  lcd_clear();
}

void lcd_clear() {
  lcd_cmd(0x01); //clear screen
  _delay_ms(10);
  lcd_cmd(0x80); // bring cursor to position 0 of line 1
  _delay_ms(10);
}


void lcd_print(char *display_text) {
  char l = 0;
  while (display_text[l] != '\0') {
    lcd_data(display_text[l]);
    l++;
    _delay_ms(50);
  }
}

void ADC_init() {
  DDRF = 0x00;      //Port F input
  ADCSRA = 0x8F;    //Enable ADC, Division factor-128
  ADMUX = 0x40;     //Ref voltage = 5
}

int ADC_read(unsigned char channel) {

  int Ain, AinLow;

  ADMUX = (0x40 | channel);

  ADCSRA |= (1 << ADSC);
  while ((ADCSRA & (1 << ADIF)) == 0);

  _delay_us(10);
  AinLow = (int) ADCL;
  Ain = (int) ADCH * 256;

  Ain = Ain + AinLow;
  return (Ain);
}

void print_resistance(unsigned long R){
  char R_string[5] = "R = ";
  char value_string[7];
  
  if(sel != prev_sel){
    lcd_clear();
    prev_sel = sel;
  }
    
  lcd_cmd(0x80);
  _delay_ms(10);

  lcd_print(R_string);

  if (R / 1000000 == 0) {
    if ( R / 1000 == 0) {
      value_string[0] = 48 + R / 100;
      value_string[1] = 48 + (R % 100) / 10;
      value_string[2] = 48 + R % 10;
      value_string[3] = 0xf4;
      value_string[4] = ' ';
      value_string[5] = ' ';
      value_string[6] = ' ';
      value_string[7] = '\0';
    }

    else {
      R = R / 100;
      value_string[0] = 48 + R / 1000;
      value_string[1] = 48 + (R % 1000) / 100;
      value_string[2] = 48 + (R % 100) / 10;
      value_string[3] = '.';
      value_string[4] = 48 + R % 10;
      value_string[5] = 'K';
      value_string[6] = 0xf4;
      value_string[7] = '\0';
    }
  }
  else {
    R = R / 100000;
    value_string[0] = 48 + R / 1000;
    value_string[1] = 48 + (R % 1000) / 100;
    value_string[2] = 48 + (R % 100) / 10;
    value_string[3] = '.';
    value_string[4] = 48 + R % 10;
    value_string[5] = 'M';
    value_string[6] = 0xf4;
    value_string[7] = '\0';
  }

  lcd_print(value_string);

}

void resistance_meter(){
  unsigned long resistance;
  unsigned int ADC_value;

  if ((PINL & 0x01) && (PINL & 0x02)) {
    ADC_value = ADC_read(0);
    resistance = (120.0 * ADC_value) / (1023 - ADC_value);
    aux_sel= 'A';
  }
  else if (!(PINL & 0x01) && (PINL & 0x02)) {
    ADC_value = ADC_read(1);
    resistance = (1000.0 * ADC_value) / (1023 - ADC_value);
    aux_sel= 'B';
  }
  else if ((PINL & 0x01) && !(PINL & 0x02)){
    ADC_value = ADC_read(2);
    resistance = (10000.0 * ADC_value) / (1023 - ADC_value);
    aux_sel= 'C';
  }
  else {
    ADC_value = ADC_read(3);
    resistance = (100000.0 * ADC_value) / (1023 - ADC_value);
    aux_sel= 'D';
  }

  print_resistance(resistance);

}

void capacitance_meter() {
  volatile unsigned long ms = 0, us = 0, capacitance = 0;

  if (PINL & 0x01) {
    PORTC |= 0x80;
    TCCR1B |= (1 << WGM12) | (1 << CS11);        //Count to compare, Clock / 8 (0.5 us)
    TCNT1 = 0;
    OCR1A = 1999;
    while(PINC & 0x04) {
      if (TIFR1 & (1 << TOV1))
      {
        TCNT1 = 0;
        ms++;
        TIFR1 |= (1 << TOV1);
      };
    }
    us = (TCNT1 + 1) / 2;
    PORTC &= ~0x80;
    us = (1000 * ms) + us;
    capacitance = 10 * us;
  }

  else {
    PORTC |= 0x40;
    TCCR1B |= (1 << WGM12) | (1 << CS11);        //Count to compare, Clock / 8 (0.5 us)
    TCNT1 = 0;
    OCR1A = 1999;
    while(PINC & 0x04) {
      if (TIFR1 & (1 << TOV1))
      {
        TCNT1 = 0;
        ms++;
        TIFR1 |= (1 << TOV1);
      }
    }
   us = (TCNT1 + 1) / 2;
    PORTC &= ~0x40;
    us = (1000 * ms) + us;
    capacitance = 10000 * us;
  }

  print_capacitance(capacitance);
  _delay_ms(4000);
}

void print_capacitance(unsigned long C){
  char C_string[5] = "C = ";
  char value_string[7];

  if(sel != prev_sel){
    lcd_clear();
    prev_sel = sel;
  }
    
  lcd_cmd(0x80);
  _delay_ms(10);

  lcd_print(C_string);

  if (C / 1000000 == 0) {    
  
    if ( C / 1000 == 0) {
      value_string[0] = 48 + C / 100;
      value_string[1] = 48 + (C % 100) / 10;
      value_string[2] = 48 + C % 10;
      value_string[3] = 'p';
      value_string[4] = 'F';
      value_string[5] = ' ';
      value_string[6] = ' ';
      value_string[7] = '\0';
    }

    else {
      C = C / 100;
      value_string[0] = 48 + C / 1000;
      value_string[1] = 48 + (C % 1000) / 100;
      value_string[2] = 48 + (C % 100) / 10;
      value_string[3] = '.';
      value_string[4] = 48 + C % 10;
      value_string[5] = 'n';
      value_string[6] = 'F';
      value_string[7] = '\0';
    }
  }
  else {
    C = C / 100000;
    value_string[0] = 48 + C / 1000;
    value_string[1] = 48 + (C % 1000) / 100;
    value_string[2] = 48 + (C % 100) / 10;
    value_string[3] = '.';
    value_string[4] = 48 + C % 10;
    value_string[5] = 'u';
    value_string[6] = 'F';
    value_string[7] = '\0';
  }

  lcd_print(value_string);

}

void voltage_meter(){
  unsigned int ADC_value, voltage;
  char value_string[11];
  
  if(sel != prev_sel){
    lcd_clear();
    prev_sel = sel;
  }
    
  lcd_cmd(0x80);
  _delay_ms(10);
  
  ADC_value = ADC_read(4);
  voltage = (ADC_value * 5500.0)/ 1023;
  value_string[0] = 'V';
    value_string[1] = ' ';
    value_string[2] = '=';
    value_string[3] = ' ';
    value_string[4] = 48 + voltage / 1000;
    value_string[5] = 48 + (voltage % 1000) / 100;
    value_string[6] = '.';
    value_string[7] = 48 + (voltage % 100) / 10;
    value_string[8] = 48 + voltage % 10;
    value_string[9] = 'V';
    value_string[10] = '\0';
    
  lcd_print(value_string);
}

void current_meter(){
  unsigned int ADC_value, current;
  char value_string[11];
  
  if(sel != prev_sel){
    lcd_clear();
    prev_sel = sel;
  }
    
  lcd_cmd(0x80);
  _delay_ms(10);
  
  ADC_value = ADC_read(5);
  
  if(ADC_value >= 512){
    current = 58 * (ADC_value - 512);
  }
  else {
    current = 58 * (512 - ADC_value);
  }
  value_string[0] = 'I';
  value_string[1] = ' ';
  value_string[2] = '=';
  value_string[3] = ' ';
  value_string[4] = 48 + (current / 10000);
  value_string[5] = 48 + (current % 10000) / 1000;
  value_string[6] = '.';
  value_string[7] = 48 + (current % 1000) / 100;
  value_string[8] = 48 + (current % 100) / 10;
  value_string[9] = 'A';
  value_string[10] = '\0';

  lcd_print(value_string);
}

void custom_delay(){
  TCCR1B |= (1 << WGM12) | (1 << CS11);        //Count to compare, Clock / 8 (0.5 us)
  TCNT1 = 0;
  OCR1A = 1999;
  while(!(TIFR1 & (1 << TOV1)));
  TIFR1 |= (1 << TOV1);
  //TCCR1B = 0x00;
}
