#include <fl_ds18b20.h>
#include <fl_timer_delay.h>
#include <string.h>

uint8_t       _DS18B20_Start(fl_one_wire *handle, uint8_t* ROM);
void          _DS18B20_StartAll(fl_one_wire *handle);
fl_bool_t _DS18B20_Read(fl_one_wire *handle, uint8_t* ROM, float* destination);
uint8_t       _DS18B20_GetResolution(fl_one_wire *handle, uint8_t* ROM);
uint8_t       _DS18B20_SetResolution(fl_one_wire *handle, uint8_t* ROM, uint8_t resolution);
uint8_t       _DS18B20_Is(uint8_t* ROM);
uint8_t       _DS18B20_SetAlarmHighTemperature(fl_one_wire *handle, uint8_t* ROM, int8_t temp);
uint8_t       _DS18B20_SetAlarmLowTemperature(fl_one_wire *handle, uint8_t* ROM, int8_t temp);
uint8_t       _DS18B20_DisableAlarmTemperature(fl_one_wire *handle, uint8_t* ROM);
uint8_t       _DS18B20_AlarmSearch(fl_one_wire *handle);
uint8_t       _DS18B20_AllDone(fl_one_wire *handle);

FL_DECLARE(fl_bool_t) fl_ds18b20_init(fl_ds18b20_manager* handle, GPIO_TypeDef* hport, uint16_t pin_num)
{
  uint8_t Ds18b20TryToFind=5;

  handle->Ds18b20StartConvert = 0;
  handle->Ds18b20Timeout = 0;
  handle->OneWireDevices = 0;
  handle->TempSensorCount = 0;

  do
  {
    //fl_ow_init(&handle->one_wire, DS18B20_GPIO_Port, DS18B20_Pin);
    fl_ow_init(&handle->one_wire, hport, pin_num);
    handle->TempSensorCount = 0;
    while(HAL_GetTick() < 3000)
      HAL_Delay(100);
    handle->OneWireDevices = fl_ow_first(&handle->one_wire);
    while (handle->OneWireDevices)
    {
      HAL_Delay(100);
      handle->TempSensorCount++;
      fl_ow_get_full_rom(&handle->one_wire, handle->devices[handle->TempSensorCount-1].addr);
      handle->OneWireDevices = fl_ow_next(&handle->one_wire);
    }
    if(handle->TempSensorCount>0)
      break;
    Ds18b20TryToFind--;
  }while(Ds18b20TryToFind>0);
  if(Ds18b20TryToFind==0)
    return FL_FALSE;
  for (uint8_t i = 0; i < handle->TempSensorCount; i++)
  {
    HAL_Delay(50);
    _DS18B20_SetResolution(&handle->one_wire, handle->devices[i].addr, FL_DS18B20_RESOLUTION_10BIT);
    HAL_Delay(50);
    _DS18B20_DisableAlarmTemperature(&handle->one_wire,  handle->devices[i].addr);
  }
  return FL_TRUE;
}

FL_DECLARE(fl_bool_t) fl_ds18b20_manual_convert(fl_ds18b20_manager *handle)
{
  handle->Ds18b20Timeout = FL_DS18B20_CONVERT_TIMEOUT_MS/10;
  _DS18B20_StartAll(&handle->one_wire);
  HAL_Delay(100);
  while (!_DS18B20_AllDone(&handle->one_wire))
  {
    HAL_Delay(10);
    handle->Ds18b20Timeout-=1;
    if(handle->Ds18b20Timeout==0)
      break;
  }
  if(handle->Ds18b20Timeout>0)
  {
    for (uint8_t i = 0; i < handle->TempSensorCount; i++)
    {
      HAL_Delay(100);
      handle->devices[i].data_valid = _DS18B20_Read(&handle->one_wire, handle->devices[i].addr, &handle->devices[i].temperature);
    }
  }
  else
  {
    for (uint8_t i = 0; i < handle->TempSensorCount; i++)
      handle->devices[i].data_valid = FL_FALSE;
  }
  if(handle->Ds18b20Timeout==0)
    return FL_FALSE;
  else
  return FL_TRUE;
}


uint8_t _DS18B20_Start(fl_one_wire *handle, uint8_t *ROM)
{
  /* Check if device is DS18B20 */
  if (!_DS18B20_Is(ROM)) {
    return 0;
  }

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Start temperature conversion */
  fl_ow_write_byte(handle, FL_DS18B20_CMD_CONVERTTEMP);

  return 1;
}

void _DS18B20_StartAll(fl_one_wire *handle)
{
  /* Reset pulse */
  fl_ow_reset(handle);
  /* Skip rom */
  fl_ow_write_byte(handle, FL_OW_CMD_SKIPROM);
  /* Start conversion on all connected devices */
  fl_ow_write_byte(handle, FL_DS18B20_CMD_CONVERTTEMP);
}

fl_bool_t _DS18B20_Read(fl_one_wire *handle, uint8_t *ROM, float *destination)
{
  uint16_t temperature;
  uint8_t resolution;
  int8_t digit, minus = 0;
  float decimal;
  uint8_t i = 0;
  uint8_t data[9];
  uint8_t crc;

  /* Check if device is DS18B20 */
  if (!_DS18B20_Is(ROM)) {
    return FL_FALSE;
  }

  /* Check if line is released, if it is, then conversion is complete */
  if (!fl_ow_read_bit(handle))
  {
    /* Conversion is not finished yet */
    return FL_FALSE;
  }

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Read scratchpad command by onewire protocol */
  fl_ow_write_byte(handle, FL_OW_CMD_RSCRATCHPAD);

  /* Get data */
  for (i = 0; i < 9; i++)
  {
    /* Read byte by byte */
    data[i] = fl_ow_read_byte(handle);
  }

  /* Calculate CRC */
  crc = fl_ow_crc8(data, 8);

  /* Check if CRC is ok */
  if (crc != data[8])
    /* CRC invalid */
    return FL_FALSE;


  /* First two bytes of scratchpad are temperature values */
  temperature = data[0] | (data[1] << 8);

  /* Reset line */
  fl_ow_reset(handle);

  /* Check if temperature is negative */
  if (temperature & 0x8000)
  {
    /* Two's complement, temperature is negative */
    temperature = ~temperature + 1;
    minus = 1;
  }


  /* Get sensor resolution */
  resolution = ((data[4] & 0x60) >> 5) + 9;


  /* Store temperature integer digits and decimal digits */
  digit = temperature >> 4;
  digit |= ((temperature >> 8) & 0x7) << 4;

  /* Store decimal digits */
  switch (resolution)
  {
    case 9:
      decimal = (temperature >> 3) & 0x01;
      decimal *= (float)FL_DS18B20_DECIMAL_STEPS_9BIT;
    break;
    case 10:
      decimal = (temperature >> 2) & 0x03;
      decimal *= (float)FL_DS18B20_DECIMAL_STEPS_10BIT;
     break;
    case 11:
      decimal = (temperature >> 1) & 0x07;
      decimal *= (float)FL_DS18B20_DECIMAL_STEPS_11BIT;
    break;
    case 12:
      decimal = temperature & 0x0F;
      decimal *= (float)FL_DS18B20_DECIMAL_STEPS_12BIT;
     break;
    default:
      decimal = 0xFF;
      digit = 0;
  }

  /* Check for negative part */
  decimal = digit + decimal;
  if (minus)
    decimal = 0 - decimal;


  /* Set to pointer */
  *destination = decimal;

  /* Return 1, temperature valid */
  return FL_TRUE;
}

uint8_t _DS18B20_GetResolution(fl_one_wire *handle, uint8_t *ROM)
{
  uint8_t conf;

  if (!_DS18B20_Is(ROM))
    return 0;

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Read scratchpad command by onewire protocol */
  fl_ow_write_byte(handle, FL_OW_CMD_RSCRATCHPAD);

  /* Ignore first 4 bytes */
  fl_ow_read_byte(handle);
  fl_ow_read_byte(handle);
  fl_ow_read_byte(handle);
  fl_ow_read_byte(handle);

  /* 5th byte of scratchpad is configuration register */
  conf = fl_ow_read_byte(handle);

  /* Return 9 - 12 value according to number of bits */
  return ((conf & 0x60) >> 5) + 9;
}

uint8_t _DS18B20_SetResolution(fl_one_wire *handle, uint8_t *ROM, uint8_t resolution)
{
  uint8_t th, tl, conf;
  if (!_DS18B20_Is(ROM))
    return 0;


  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Read scratchpad command by onewire protocol */
  fl_ow_write_byte(handle, FL_OW_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  fl_ow_read_byte(handle);
  fl_ow_read_byte(handle);

  th = fl_ow_read_byte(handle);
  tl = fl_ow_read_byte(handle);
  conf = fl_ow_read_byte(handle);

  if (resolution == FL_DS18B20_RESOLUTION_9BIT)
  {
    conf &= ~(1 << FL_DS18B20_RESOLUTION_R1);
    conf &= ~(1 << FL_DS18B20_RESOLUTION_R0);
  }
  else if (resolution == FL_DS18B20_RESOLUTION_10BIT)
  {
    conf &= ~(1 << FL_DS18B20_RESOLUTION_R1);
    conf |= 1 << FL_DS18B20_RESOLUTION_R0;
  }
  else if (resolution == FL_DS18B20_RESOLUTION_11BIT)
  {
    conf |= 1 << FL_DS18B20_RESOLUTION_R1;
    conf &= ~(1 << FL_DS18B20_RESOLUTION_R0);
  }
  else if (resolution == FL_DS18B20_RESOLUTION_12BIT)
  {
    conf |= 1 << FL_DS18B20_RESOLUTION_R1;
    conf |= 1 << FL_DS18B20_RESOLUTION_R0;
  }

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  fl_ow_write_byte(handle, FL_OW_CMD_WSCRATCHPAD);

  /* Write bytes */
  fl_ow_write_byte(handle, th);
  fl_ow_write_byte(handle, tl);
  fl_ow_write_byte(handle, conf);

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  fl_ow_write_byte(handle, FL_OW_CMD_CPYSCRATCHPAD);

  return 1;
}

uint8_t _DS18B20_Is(uint8_t *ROM)
{
  /* Checks if first byte is equal to DS18B20's family code */
  if (*ROM == FL_DS18B20_FAMILY_CODE)
    return 1;

  return 0;
}

uint8_t _DS18B20_SetAlarmLowTemperature(fl_one_wire *handle, uint8_t *ROM, int8_t temp)
{
  uint8_t tl, th, conf;
  if (!_DS18B20_Is(ROM))
    return 0;

  if (temp > 125)
    temp = 125;

  if (temp < -55)
    temp = -55;

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Read scratchpad command by handle protocol */
  fl_ow_write_byte(handle, FL_OW_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  fl_ow_read_byte(handle);
  fl_ow_read_byte(handle);

  th = fl_ow_read_byte(handle);
  tl = fl_ow_read_byte(handle);
  conf = fl_ow_read_byte(handle);

  tl = (uint8_t)temp;

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  fl_ow_write_byte(handle, FL_OW_CMD_WSCRATCHPAD);

  /* Write bytes */
  fl_ow_write_byte(handle, th);
  fl_ow_write_byte(handle, tl);
  fl_ow_write_byte(handle, conf);

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  fl_ow_write_byte(handle, FL_OW_CMD_CPYSCRATCHPAD);

  return 1;
}

uint8_t _DS18B20_SetAlarmHighTemperature(fl_one_wire *handle, uint8_t *ROM, int8_t temp)
{
  uint8_t tl, th, conf;
  if (!_DS18B20_Is(ROM))
    return 0;

  if (temp > 125)
    temp = 125;

  if (temp < -55)
    temp = -55;

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Read scratchpad command by onewire protocol */
  fl_ow_write_byte(handle, FL_OW_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  fl_ow_read_byte(handle);
  fl_ow_read_byte(handle);

  th = fl_ow_read_byte(handle);
  tl = fl_ow_read_byte(handle);
  conf = fl_ow_read_byte(handle);

  th = (uint8_t)temp;

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  fl_ow_write_byte(handle, FL_OW_CMD_WSCRATCHPAD);

  /* Write bytes */
  fl_ow_write_byte(handle, th);
  fl_ow_write_byte(handle, tl);
  fl_ow_write_byte(handle, conf);

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  fl_ow_write_byte(handle, FL_OW_CMD_CPYSCRATCHPAD);

  return 1;
}

uint8_t _DS18B20_DisableAlarmTemperature(fl_one_wire *handle, uint8_t *ROM)
{
  uint8_t tl, th, conf;
  if (!_DS18B20_Is(ROM))
    return 0;

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Read scratchpad command by onewire protocol */
  fl_ow_write_byte(handle, FL_OW_CMD_RSCRATCHPAD);

  /* Ignore first 2 bytes */
  fl_ow_read_byte(handle);
  fl_ow_read_byte(handle);

  th = fl_ow_read_byte(handle);
  tl = fl_ow_read_byte(handle);
  conf = fl_ow_read_byte(handle);

  th = 125;
  tl = (uint8_t)-55;

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Write scratchpad command by onewire protocol, only th, tl and conf register can be written */
  fl_ow_write_byte(handle, FL_OW_CMD_WSCRATCHPAD);

  /* Write bytes */
  fl_ow_write_byte(handle, th);
  fl_ow_write_byte(handle, tl);
  fl_ow_write_byte(handle, conf);

  /* Reset line */
  fl_ow_reset(handle);
  /* Select ROM number */
  fl_ow_select_with_pointer(handle, ROM);
  /* Copy scratchpad to EEPROM of DS18B20 */
  fl_ow_write_byte(handle, FL_OW_CMD_CPYSCRATCHPAD);

  return 1;
}

uint8_t _DS18B20_AlarmSearch(fl_one_wire *handle)
{
  /* Start alarm search */
  return fl_ow_search(handle, FL_DS18B20_CMD_ALARMSEARCH);
}

uint8_t _DS18B20_AllDone(fl_one_wire *handle)
{
  /* If read bit is low, then device is not finished yet with calculation temperature */
  return fl_ow_read_bit(handle);
}
