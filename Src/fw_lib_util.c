#include "fw_lib_def.h"
#include "fw_lib_util.h"

#define CRC_SICK

// https://github.com/lammertb/libcrc
/*
 * #define CRC_POLY_xxxx
 *
 * The constants of the form CRC_POLY_xxxx define the polynomials for some well
 * known CRC calculations.
 */

#define   CRC_POLY_16       (0xA001)
#define   CRC_POLY_32       (0xEDB88320ul)
#define   CRC_POLY_64       (0x42F0E1EBA9EA3693ull)
#define   CRC_POLY_CCITT    (0x1021)
#define   CRC_POLY_DNP      (0xA6BC)
#define   CRC_POLY_KERMIT   (0x8408)
#define   CRC_POLY_SICK     (0x8005)

 /*
  * #define CRC_START_xxxx
  *
  * The constants of the form CRC_START_xxxx define the values that are used for
  * initialization of a CRC value for common used calculation methods.
  */

#define   CRC_START_8           (0x00)
#define   CRC_START_16          (0x0000)
#define   CRC_START_MODBUS      (0xFFFF)
#define   CRC_START_XMODEM      (0x0000)
#define   CRC_START_CCITT_1D0F  (0x1D0F)
#define   CRC_START_CCITT_FFFF  (0xFFFF)
#define   CRC_START_KERMIT      (0x0000)
#define   CRC_START_SICK        (0x0000)
#define   CRC_START_DNP         (0x0000)
#define   CRC_START_32          (0xFFFFFFFFul)
#define   CRC_START_64_ECMA     (0x0000000000000000ull)
#define   CRC_START_64_WE       (0xFFFFFFFFFFFFFFFFull)

#if defined(CRC_SICK)
  // SICK(LIDAR sensor) CRC
FW_LIB_DECLARE(uint16_t) fw_lib_crc_16(const unsigned char* input_str, size_t num_bytes)
{
  uint16_t crc;
  uint16_t low_byte;
  uint16_t high_byte;
  uint16_t short_c;
  uint16_t short_p;
  const unsigned char* ptr;
  size_t a;

  crc = CRC_START_SICK;
  ptr = input_str;
  short_p = 0;

  if (ptr != NULL)
  {
    for (a = 0; a < num_bytes; a++)
    {
      short_c = 0x00FF & (uint16_t)*ptr;

      if (crc & 0x8000)
      {
        crc = (crc << 1) ^ CRC_POLY_SICK;
      }
      else
      {
        crc = crc << 1;
      }

      crc ^= (short_c | short_p);
      short_p = short_c << 8;

      ptr++;
    }
  }

  low_byte = (crc & 0xFF00) >> 8;
  high_byte = (crc & 0x00FF) << 8;
  crc = low_byte | high_byte;

  return crc;
}

#else

static fw_lib_bool_t    crc_tab16_init = FW_LIB_FALSE;
static uint16_t         crc_tab16[256];

static void             init_crc16_tab(void);

FW_LIB_DECLARE(uint16_t) fw_lib_crc_16(const unsigned char* input_str, size_t num_bytes)
{
  uint16_t crc;
  const unsigned char* ptr;
  size_t a;

  if (!crc_tab16_init)
  {
    init_crc16_tab();
  }

  crc = CRC_START_16;
  ptr = input_str;

  if (ptr != NULL)
  {
    for (a = 0; a < num_bytes; a++)
    {
      crc = (crc >> 8) ^ crc_tab16[(crc ^ (uint16_t)*ptr++) & 0x00FF];
    }
  }

  return crc;
}

/*
 * static void init_crc16_tab( void );
 *
 * For optimal performance uses the CRC16 routine a lookup table with values
 * that can be used directly in the XOR arithmetic in the algorithm. This
 * lookup table is calculated by the init_crc16_tab() routine, the first time
 * the CRC function is called.
 */
static void init_crc16_tab(void)
{
  uint16_t i;
  uint16_t j;
  uint16_t crc;
  uint16_t c;

  for (i = 0; i < 256; i++)
  {
    crc = 0;
    c = i;

    for (j = 0; j < 8; j++)
    {
      if ((crc ^ c) & 0x0001)
      {
        crc = (crc >> 1) ^ CRC_POLY_16;
      }
      else
      {
        crc = crc >> 1;
      }

      c = c >> 1;
    }

    crc_tab16[i] = crc;
  }

  crc_tab16_init = FW_LIB_TRUE;

}  /* init_crc16_tab */
#endif
