#include "max30102.h"
#include <math.h>
#include "stm32l4xx_hal.h"

I2C_HandleTypeDef *i2c_h;

uint8_t _temperature_res, _pressure_oversampling,  _mode;
int16_t t2, t3, p2, p3, p4, p5, p6, p7, p8, p9;
uint16_t t1, p1;
int32_t t_fine;
uint8_t _temperature_res, _pressure_oversampling, _huminidity_oversampling,  _mode, h1, h3;
int8_t h6;
int16_t t2, t3, p2, p3, p4, p5, p6, p7, p8, p9, h2, h4, h5;
uint16_t t1, p1;
int32_t t_fine;

uint8_t BMP280_Read8(uint8_t addr)
{
  uint8_t tmp = 0;

  HAL_I2C_Mem_Read(i2c_h, BMP280_I2CADDR, addr, 1, &tmp, 1, 10);

  return tmp;
}



uint16_t BMP280_Read16(uint8_t addr)
{

	uint8_t tmp[2];

	HAL_I2C_Mem_Read(i2c_h, BMP280_I2CADDR, addr, 1, tmp, 2, 10);

	return ((tmp[0] << 8) | tmp[1]);
}

uint16_t BMP280_Read16LE(uint8_t addr)
{
	uint16_t tmp;

	tmp = BMP280_Read16(addr);
	return (tmp >> 8) | (tmp << 8);
}

void BMP280_Write8(uint8_t address, uint8_t data)
{
	HAL_I2C_Mem_Write(i2c_h, BMP280_I2CADDR, address, 1, &data, 1, 10);
}

uint32_t BMP280_Read24(uint8_t addr)
{
	uint8_t tmp[3];

	HAL_I2C_Mem_Read(i2c_h, BMP280_I2CADDR, addr, 1, tmp, 3, 10);

	return ((tmp[0] << 16) | tmp[1] << 8 | tmp[2]);
}


void BMP280_SetConfig(uint8_t standby_time, uint8_t filter)
{
	BMP280_Write8(BME280_CONFIG, (((standby_time & 0x7) << 5) | ((filter & 0x7) << 2)) & 0xFC);
}

void BMP280_Init(I2C_HandleTypeDef *i2c_handler, uint8_t temperature_resolution, uint8_t pressure_oversampling, uint8_t mode)
{
	i2c_h = i2c_handler;

	if (mode > BMP280_NORMALMODE)
	    mode = BMP280_NORMALMODE;
	_mode = mode;
	if(mode == BMP280_FORCEDMODE)
		mode = BMP280_SLEEPMODE;



	if (temperature_resolution > BMP280_TEMPERATURE_20BIT)
		temperature_resolution = BMP280_TEMPERATURE_20BIT;
	_temperature_res = temperature_resolution;

	if (pressure_oversampling > BMP280_ULTRAHIGHRES)
		pressure_oversampling = BMP280_ULTRAHIGHRES;
	_pressure_oversampling = pressure_oversampling;

	while(BMP280_Read8(BMP280_CHIPID) != 0x58);

	/* read calibration data */
	t1 = BMP280_Read16LE(BMP280_DIG_T1);
	t2 = BMP280_Read16LE(BMP280_DIG_T2);
	t3 = BMP280_Read16LE(BMP280_DIG_T3);

	p1 = BMP280_Read16LE(BMP280_DIG_P1);
	p2 = BMP280_Read16LE(BMP280_DIG_P2);
	p3 = BMP280_Read16LE(BMP280_DIG_P3);
	p4 = BMP280_Read16LE(BMP280_DIG_P4);
	p5 = BMP280_Read16LE(BMP280_DIG_P5);
	p6 = BMP280_Read16LE(BMP280_DIG_P6);
	p7 = BMP280_Read16LE(BMP280_DIG_P7);
	p8 = BMP280_Read16LE(BMP280_DIG_P8);
	p9 = BMP280_Read16LE(BMP280_DIG_P9);

	BMP280_Write8(BMP280_CONTROL, ((temperature_resolution<<5) | (pressure_oversampling<<2) | mode));
}

float BMP280_ReadTemperature(void)
{
  int32_t var1, var2;

  if(_mode == BMP280_FORCEDMODE)
  {
	  uint8_t mode;
	  uint8_t ctrl = BMP280_Read8(BMP280_CONTROL);
	  ctrl &= ~(0x03);
	  ctrl |= BMP280_FORCEDMODE;
	  BMP280_Write8(BMP280_CONTROL, ctrl);

	  mode = BMP280_Read8(BMP280_CONTROL); 	// Read written mode
	  mode &= 0x03;							// Do not work without it...

	  if(mode == BMP280_FORCEDMODE)
	  {
		  while(1) // Wait for end of conversion
		  {
			  mode = BMP280_Read8(BMP280_CONTROL);
			  mode &= 0x03;
			  if(mode == BMP280_SLEEPMODE)
				  break;
		  }

		  int32_t adc_T = BMP280_Read24(BMP280_TEMPDATA);
		  adc_T >>= 4;

		  var1  = ((((adc_T>>3) - ((int32_t)t1 <<1))) *
				  ((int32_t)t2)) >> 11;

		  var2  = (((((adc_T>>4) - ((int32_t)t1)) *
				  ((adc_T>>4) - ((int32_t)t1))) >> 12) *
				  ((int32_t)t3)) >> 14;

		  t_fine = var1 + var2;

		  float T  = (t_fine * 5 + 128) >> 8;
		  return T/100;
	  }
  }

  return -99;
}


int32_t BMP280_ReadPressure(void)
{
	  int64_t var1, var2, p;

	  // Must be done first to get the t_fine variable set up
	  BMP280_ReadTemperature();

	  int32_t adc_P = BMP280_Read24(BMP280_PRESSUREDATA);
	  adc_P >>= 4;

	  var1 = ((int64_t)t_fine) - 128000;
	  var2 = var1 * var1 * (int64_t)p6;
	  var2 = var2 + ((var1*(int64_t)p5)<<17);
	  var2 = var2 + (((int64_t)p4)<<35);
	  var1 = ((var1 * var1 * (int64_t)p3)>>8) +
	    ((var1 * (int64_t)p2)<<12);
	  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)p1)>>33;

	  if (var1 == 0) {
	    return 0;  // avoid exception caused by division by zero
	  }
	  p = 1048576 - adc_P;
	  p = (((p<<31) - var2)*3125) / var1;
	  var1 = (((int64_t)p9) * (p>>13) * (p>>13)) >> 25;
	  var2 = (((int64_t)p8) * p) >> 19;

	  p = ((p + var1 + var2) >> 8) + (((int64_t)p7)<<4);
	  return (int32_t)p/256;
}


uint8_t BMP280_ReadTemperatureAndPressure(float *temperature, int32_t *pressure)
{
	  int64_t var1, var2, p;

	  // Must be done first to get the t_fine variable set up
	  *temperature = BMP280_ReadTemperature();

	  if(*temperature == -99)
		  return -1;

	  int32_t adc_P = BMP280_Read24(BMP280_PRESSUREDATA);
	  adc_P >>= 4;

	  var1 = ((int64_t)t_fine) - 128000;
	  var2 = var1 * var1 * (int64_t)p6;
	  var2 = var2 + ((var1*(int64_t)p5)<<17);
	  var2 = var2 + (((int64_t)p4)<<35);
	  var1 = ((var1 * var1 * (int64_t)p3)>>8) +
	    ((var1 * (int64_t)p2)<<12);
	  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)p1)>>33;

	  if (var1 == 0) {
	    return 0;  // avoid exception caused by division by zero
	  }
	  p = 1048576 - adc_P;
	  p = (((p<<31) - var2)*3125) / var1;
	  var1 = (((int64_t)p9) * (p>>13) * (p>>13)) >> 25;
	  var2 = (((int64_t)p8) * p) >> 19;

	  p = ((p + var1 + var2) >> 8) + (((int64_t)p7)<<4);
	  *pressure = (int32_t)p/256;

	  return 0;
}

float BMP280_ReadAltitude(float sea_level_pa)
{
	  float altitude;

	  float pressure = BMP280_ReadPressure(); // in Si units for Pascal
	 

	  altitude = 44306 * (1.0 - pow(pressure / sea_level_pa, 0.1903));

	  return altitude;
}
