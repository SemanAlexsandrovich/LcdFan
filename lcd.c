
#include "main.h"
//----------------------------------------
unsigned char portlcd = 0;
//----------------------------------------
void sendhalfbyte(unsigned char c)
{
	c<<=4;
	e1;
	_delay_us(50);
	I2C_SendByteByADDR(portlcd|c,0b01001110);
	e0;
	_delay_us(50);
}
//----------------------------------------
void sendbyte(unsigned char c, unsigned char mode)
{
	if (mode==0) rs0;
	else         rs1;
	unsigned char hc=0;
	hc=c>>4;
	sendhalfbyte(hc); sendhalfbyte(c);
}
//----------------------------------------
void sendcharlcd(unsigned char c)
{
	sendbyte(c,1);
}
//----------------------------------------
void setpos(unsigned char x, unsigned y)
{
	switch(y)
	{
		case 0:
		sendbyte(x|0x80,0);
		break;
		case 1:
		sendbyte((0x40+x)|0x80,0);
		break;
		case 2:
		sendbyte((0x14+x)|0x80,0);
		break;
		case 3:
		sendbyte((0x54+x)|0x80,0);
		break;
	}
}
//----------------------------------------
void LCD_ini(void)
{
	_delay_ms(15);
	sendhalfbyte(0b00000011);
	_delay_ms(4);
	sendhalfbyte(0b00000011);
	_delay_us(100);
	sendhalfbyte(0b00000011);
	_delay_ms(1);
	sendhalfbyte(0b00000010);
	_delay_ms(1);
	sendbyte(0b00101000, 0);
	_delay_ms(1);
	sendbyte(0b00001100, 0);
	_delay_ms(1);
	sendbyte(0b00000110, 0);
	_delay_ms(1);
	setled();
	setwrite();
}
//----------------------------------------
void clearlcd(void)
{
	sendbyte(0b00000001, 0);
	_delay_us(1500);
}
//----------------------------------------
void str_lcd (char str1[])
{
	wchar_t n;
	for(n=0;str1[n]!='\0';n++)
	sendcharlcd(str1[n]);
}
//----------------------------------------