#include <string.h>
#include "fl_util.h"

#define CRC_SICK

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

  /* aaaack but it's fast and const should make it shared text page. */
static const unsigned char _pr2six[256] =
{
  /* ASCII table */
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
  64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
  64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

static const char _basis_64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

FL_DECLARE(int) fl_base64_encode_len(int len)
{
  return ((len + 2) / 3 * 4) + 1;
}

FL_DECLARE(int) fl_base64_encode(char* dst, const char* src, int src_len)
{
  int i;
  char* p;

  p = dst;
  for (i = 0; i < src_len - 2; i += 3)
  {
    *p++ = _basis_64[(src[i] >> 2) & 0x3F];
    *p++ = _basis_64[((src[i] & 0x3) << 4) |
      ((int)(src[i + 1] & 0xF0) >> 4)];
    *p++ = _basis_64[((src[i + 1] & 0xF) << 2) |
      ((int)(src[i + 2] & 0xC0) >> 6)];
    *p++ = _basis_64[src[i + 2] & 0x3F];
  }

  if (i < src_len)
  {
    *p++ = _basis_64[(src[i] >> 2) & 0x3F];
    if (i == (src_len - 1))
    {
      *p++ = _basis_64[((src[i] & 0x3) << 4)];
      *p++ = '=';
    }
    else
    {
      *p++ = _basis_64[((src[i] & 0x3) << 4) |
        ((int)(src[i + 1] & 0xF0) >> 4)];
      *p++ = _basis_64[((src[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }

  *p++ = '\0';
  return p - dst;
}

FL_DECLARE(int) fl_base64_decode_len(const char* src)
{
  int nbytesdecoded;
  register const unsigned char* bufin;
  register int nprbytes;

  bufin = (const unsigned char*)src;
  while (_pr2six[*(bufin++)] <= 63);

  nprbytes = (bufin - (const unsigned char*)src) - 1;
  nbytesdecoded = ((nprbytes + 3) / 4) * 3;

  return nbytesdecoded + 1;
}

FL_DECLARE(int) fl_base64_decode(char* dst, const char* src)
{
  int nbytesdecoded;
  register const unsigned char* bufin;
  register unsigned char* bufout;
  register int nprbytes;

  bufin = (const unsigned char*)src;
  while (_pr2six[*(bufin++)] <= 63);
  nprbytes = (bufin - (const unsigned char*)src) - 1;
  nbytesdecoded = ((nprbytes + 3) / 4) * 3;

  bufout = (unsigned char*)dst;
  bufin = (const unsigned char*)src;

  while (nprbytes > 4)
  {
    *(bufout++) =
      (unsigned char)(_pr2six[*bufin] << 2 | _pr2six[bufin[1]] >> 4);
    *(bufout++) =
      (unsigned char)(_pr2six[bufin[1]] << 4 | _pr2six[bufin[2]] >> 2);
    *(bufout++) =
      (unsigned char)(_pr2six[bufin[2]] << 6 | _pr2six[bufin[3]]);
    bufin += 4;
    nprbytes -= 4;
  }

  /* Note: (nprbytes == 1) would be an error, so just ingore that case */
  if (nprbytes > 1)
  {
    *(bufout++) =
      (unsigned char)(_pr2six[*bufin] << 2 | _pr2six[bufin[1]] >> 4);
  }
  if (nprbytes > 2)
  {
    *(bufout++) =
      (unsigned char)(_pr2six[bufin[1]] << 4 | _pr2six[bufin[2]] >> 2);
  }
  if (nprbytes > 3)
  {
    *(bufout++) =
      (unsigned char)(_pr2six[bufin[2]] << 6 | _pr2six[bufin[3]]);
  }

  *(bufout++) = '\0';
  nbytesdecoded -= (4 - nprbytes) & 3;

  return nbytesdecoded;
}


#if defined(CRC_SICK)
// SICK(LIDAR sensor) CRC
FL_DECLARE(uint16_t) fl_crc_16(const unsigned char* input_str, size_t num_bytes)
{
  uint16_t crc;
  uint16_t short_c;
  uint16_t short_p;
  const unsigned char* ptr;
  size_t a;

#if FL_BYTE_ORDER == FL_BYTE_ORDER_BIG_ENDIAN
  uint16_t low_byte;
  uint16_t high_byte;
#endif

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

#if FL_BYTE_ORDER == FL_BYTE_ORDER_BIG_ENDIAN
  low_byte = (crc & 0xFF00) >> 8;
  high_byte = (crc & 0x00FF) << 8;
  crc = low_byte | high_byte;
#endif

  return crc;
}

#else

static fl_bool_t    crc_tab16_init = FW_LIB_FALSE;
static uint16_t         crc_tab16[256];

static void             init_crc16_tab(void);

FL_DECLARE(uint16_t) fl_crc_16(const unsigned char* input_str, size_t num_bytes)
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

  crc_tab16_init = FL_TRUE;

}  /* init_crc16_tab */
#endif
