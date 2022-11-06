#include "dev_lcd.h"
#include "ascii_font.h"
static void dev_lcd_clear(uint16_t color);
static void dev_lcd_write_data(uint8_t data)
{
	for(uint8_t i=0;i<8;i++)
	{
		DEV_TFT_SCLK_CLR();
		if(data & 0x80)
		{
			DEV_TFT_SDIN_SET();
		}
		else
		{
			DEV_TFT_SDIN_CLR();
		}
		DEV_TFT_SCLK_SET();
		data <<= 1;
	}
}

static void dev_lcd_write_data_8bit(uint8_t data)
{
	DEV_TFT_DC_SET();
	dev_lcd_write_data(data);
}

static void dev_lcd_write_data_16bit(uint16_t data)
{
	DEV_TFT_DC_SET();
	dev_lcd_write_data(data>>8);
	dev_lcd_write_data(data);
}

static void dev_lcd_write_reg(uint8_t data)
{
	DEV_TFT_DC_CLR();
	dev_lcd_write_data(data);
}

static void dev_lcd_write_reg_and_data(uint16_t reg,uint16_t data)
{
	dev_lcd_write_reg(reg);
	dev_lcd_write_data_16bit(data);
}


static void dev_lcd_set_show_address(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend)
{
	dev_lcd_write_reg(0x2A);
	dev_lcd_write_data_8bit(xsta>>8);
	dev_lcd_write_data_8bit(xsta);
	dev_lcd_write_data_8bit(xend>>8);
	dev_lcd_write_data_8bit(xend);
	
	dev_lcd_write_reg(0x2B);
	dev_lcd_write_data_8bit(ysta>>8);
	dev_lcd_write_data_8bit(ysta);
	dev_lcd_write_data_8bit(yend>>8);
	dev_lcd_write_data_8bit(yend);
	
	dev_lcd_write_reg(0x2C);
}

static void dev_lcd_config_reg(void)
{
	dev_lcd_write_reg(0x36); 
	dev_lcd_write_data_8bit(0x70);

	dev_lcd_write_reg(0x3A); 
	dev_lcd_write_data_8bit(0x05);

	dev_lcd_write_reg(0xB2);
	dev_lcd_write_data_8bit(0x0C);
	dev_lcd_write_data_8bit(0x0C);
	dev_lcd_write_data_8bit(0x00);
	dev_lcd_write_data_8bit(0x33);
	dev_lcd_write_data_8bit(0x33);

	dev_lcd_write_reg(0xB7); 
	dev_lcd_write_data_8bit(0x35);  

	dev_lcd_write_reg(0xBB);
	dev_lcd_write_data_8bit(0x19);

	dev_lcd_write_reg(0xC0);
	dev_lcd_write_data_8bit(0x2C);

	dev_lcd_write_reg(0xC2);
	dev_lcd_write_data_8bit(0x01);

	dev_lcd_write_reg(0xC3);
	dev_lcd_write_data_8bit(0x12);   

	dev_lcd_write_reg(0xC4);
	dev_lcd_write_data_8bit(0x20);  

	dev_lcd_write_reg(0xC6); 
	dev_lcd_write_data_8bit(0x0F);    

	dev_lcd_write_reg(0xD0); 
	dev_lcd_write_data_8bit(0xA4);
	dev_lcd_write_data_8bit(0xA1);

	dev_lcd_write_reg(0xE0);
	dev_lcd_write_data_8bit(0xD0);
	dev_lcd_write_data_8bit(0x04);
	dev_lcd_write_data_8bit(0x0D);
	dev_lcd_write_data_8bit(0x11);
	dev_lcd_write_data_8bit(0x13);
	dev_lcd_write_data_8bit(0x2B);
	dev_lcd_write_data_8bit(0x3F);
	dev_lcd_write_data_8bit(0x54);
	dev_lcd_write_data_8bit(0x4C);
	dev_lcd_write_data_8bit(0x18);
	dev_lcd_write_data_8bit(0x0D);
	dev_lcd_write_data_8bit(0x0B);
	dev_lcd_write_data_8bit(0x1F);
	dev_lcd_write_data_8bit(0x23);

	dev_lcd_write_reg(0xE1);
	dev_lcd_write_data_8bit(0xD0);
	dev_lcd_write_data_8bit(0x04);
	dev_lcd_write_data_8bit(0x0C);
	dev_lcd_write_data_8bit(0x11);
	dev_lcd_write_data_8bit(0x13);
	dev_lcd_write_data_8bit(0x2C);
	dev_lcd_write_data_8bit(0x3F);
	dev_lcd_write_data_8bit(0x44);
	dev_lcd_write_data_8bit(0x51);
	dev_lcd_write_data_8bit(0x2F);
	dev_lcd_write_data_8bit(0x1F);
	dev_lcd_write_data_8bit(0x1F);
	dev_lcd_write_data_8bit(0x20);
	dev_lcd_write_data_8bit(0x23);

	dev_lcd_write_reg(0x21); 

	dev_lcd_write_reg(0x11); 
	/*delay 120ms*/ 

	dev_lcd_write_reg(0x29); 
}



void dev_lcd_hardware_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7,GPIO_PIN_SET);
	
	DEV_TFT_RST_CLR();
	HAL_Delay(20);
	DEV_TFT_RST_SET();
	HAL_Delay(20);
	DEV_TFT_BLK_SET();
	
	dev_lcd_config_reg();
	
	//dev_lcd_clear(WHITE);
}


static void dev_lcd_clear(uint16_t color)
{
	uint16_t i,j;
	dev_lcd_set_show_address(0,0,DEVICE_TFT_WIGHT-1,DEVICE_TFT_HIGHT-1);
	for(i=0;i<DEVICE_TFT_WIGHT;i++)
	{
		for(j=0;j<DEVICE_TFT_HIGHT;j++)
		{
			dev_lcd_write_data_16bit(color);
		}
	}
}

void dev_lcd_draw_point(uint16_t px,uint16_t py,uint16_t color)
{
	dev_lcd_set_show_address(px,py,px,py);
	dev_lcd_write_data_16bit(color);
}

void dev_lcd_draw_window(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{
	uint16_t i,j;
	dev_lcd_set_show_address(xsta,ysta,xend,yend);
	for(i=ysta;i<yend;i++)
	{
		for(j=xsta;j<xend;j++)
		{
			dev_lcd_write_data_16bit(color);
		}
	}
}

void dev_lcd_draw_line(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x = xend - xsta;
	delta_y = yend - ysta;
	uRow=xsta; 
	uCol=ysta;
	if(delta_x > 0){
		incx = 1;
	}
	else if(delta_x==0){
		incx = 0;
	}
	else{
		incx=-1;
		delta_x=-delta_x;
	}
	if(delta_y > 0){
		incy = 1;
	}
	else if(delta_y==0){
		incy = 0;
	}
	else{
		incy=-1;
		delta_y=-delta_y;
	}
	if(delta_x > delta_y){
		distance = delta_x;
	}
	else{
		distance = delta_y;
	}
	for(uint16_t i=0;i<(distance+1);i++)
	{
		dev_lcd_draw_point(uRow,uCol,color);
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) { 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) { 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}
}


static uint8_t dev_lcd_get_ascii_char_index(char chr)
{
	char ascii_array[] = DEV_LCD_DEFAULT_ASCII_FONT;
	for(uint8_t i=0;i<sizeof(ascii_array);i++)
	{
		if(chr == ascii_array[i])
		{
			return i;
		}
	}
	return 0;
}

void dev_lcd_draw_char(uint16_t px,uint16_t py,char chr,uint8_t type,uint16_t bkcolor,uint16_t frcolor)
{
	uint8_t temp;
	uint8_t ascii_index = dev_lcd_get_ascii_char_index(chr);
	uint16_t ucPage,ucColumn;
	if(type == ASCII_FONT_TYPE_1608)
	{
		dev_lcd_set_show_address(px,py,px+8-1,py+16-1);
		for(ucPage = 0;ucPage<16;ucPage++)
		{
			temp = ucAscii_1608[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
				
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);	
			
        temp >>= 1;					
      }
		}
	}
	else if(type == ASCII_FONT_TYPE_2412)
	{
		dev_lcd_set_show_address(px,py,px+12-1,py+24-1);
		for(ucPage = 0;ucPage<48;ucPage++)
		{
			temp = ucAscii_2412[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
			ucPage++;
			temp = ucAscii_2412[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<4;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
		}
	}
	else if(type == ASCII_FONT_TYPE_3216)
	{
		dev_lcd_set_show_address(px,py,px+16-1,py+32-1);
		for(ucPage = 0;ucPage<64;ucPage++)
		{
			temp = ucAscii_3216[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
			ucPage++;
			temp = ucAscii_3216[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
		}
	}
	else if(type == ASCII_FONT_TYPE_6432)
	{
		dev_lcd_set_show_address(px,py,px+32-1,py+64-1);
		for(ucPage = 0;ucPage<256;ucPage++)
		{
			temp = ucAscii_6432[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
			ucPage++;
			temp = ucAscii_6432[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
			ucPage++;
			temp = ucAscii_6432[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
			ucPage++;
			temp = ucAscii_6432[ascii_index][ucPage];
			for(ucColumn=0;ucColumn<8;ucColumn++)
      {
        if(temp&0x01)
          dev_lcd_write_data_16bit(frcolor);			
        else
          dev_lcd_write_data_16bit(bkcolor);								
        temp >>= 1;					
      }
		}
	}
}

void dev_lcd_draw_string(uint16_t px,uint16_t py,char *string,uint8_t type,uint16_t bkcolor,uint16_t frcolor)
{
	  while(*string!='\0')
    {   
				if(type == ASCII_FONT_TYPE_1608)
				{
					if((px + 8) > DEVICE_TFT_WIGHT)
					{
						px = 0;
						py += 16;
					}
					if((py + 16) > DEVICE_TFT_HIGHT)
					{
						px = 0;
						py = 0;
					}
					dev_lcd_draw_char(px,py,*string,type,bkcolor,frcolor);
					string++;
					px += 8;
				}
				else if(type == ASCII_FONT_TYPE_2412)
				{
					if((px + 12) > DEVICE_TFT_WIGHT)
					{
						px = 0;
						py += 24;
					}
					if((py + 24) > DEVICE_TFT_HIGHT)
					{
						px = 0;
						py = 0;
					}
					dev_lcd_draw_char(px,py,*string,type,bkcolor,frcolor);
					string++;
					px += 12;
				}
				else if(type == ASCII_FONT_TYPE_3216)
				{
					if((px + 16) > DEVICE_TFT_WIGHT)
					{
						px = 0;
						py += 32;
					}
					if((py + 32) > DEVICE_TFT_HIGHT)
					{
						px = 0;
						py = 0;
					}
					dev_lcd_draw_char(px,py,*string,type,bkcolor,frcolor);
					string++;
					px += 16;
				}
				else if(type == ASCII_FONT_TYPE_6432)
				{
					if((px + 32) > DEVICE_TFT_WIGHT)
					{
						px = 0;
						py += 64;
					}
					if((py + 64) > DEVICE_TFT_HIGHT)
					{
						px = 0;
						py = 0;
					}
					dev_lcd_draw_char(px,py,*string,type,bkcolor,frcolor);
					string++;
					px += 32;
				}
    }
}

/**/
