// fw_app.h
// Firmware application.

#ifndef FW_APP_H
#define FW_APP_H

#include "main.h"
#include "usart.h"
#include "gpio.h"

#include "fw_lib_def.h"
#include "fw_lib_queue.h"
#include "fw_lib_message.h"
#include "fw_lib_dio.h"
#include "fw_lib_stm32.h"
#include "fw_lib_dht22.h"

// Parser defines
#define FW_APP_TXT_PARSER           (0)
#define FW_APP_BIN_PARSER           (1)

//#define FW_APP_PARSER               FW_APP_TXT_PARSER
#define FW_APP_PARSER               FW_APP_BIN_PARSER

#define FW_APP_PARSER_CALLBACK      (0) // 0 : No parser callback, 1 : Parser callback
#define FW_APP_PARSER_DEBUG

#if FW_APP_PARSER == FW_APP_TXT_PARSER
#include "fw_lib_txt_message.h"
#include "fw_lib_txt_parser.h"
#else
#include "fw_lib_bin_message.h"
#include "fw_lib_bin_parser.h"
#include "fw_lib_util.h"
#endif

#define FW_APP_HW_MAJOR             (0)
#define FW_APP_HW_MINOR             (0)
#define FW_APP_HW_REVISION          (1)

#define FW_APP_FW_MAJOR             (0)
#define FW_APP_FW_MINOR             (1)
#define FW_APP_FW_REVISION          (1)


#define FW_APP_UART_HANDLE                                UART_HandleTypeDef*
#define FW_APP_GPIO_HANDLE                                GPIO_TypeDef*
#define FW_APP_GPIO_TOGGLE(pin, port)                     HAL_GPIO_TogglePin(port, pin)
#define FW_APP_UART_RCV_IT(handle, buf, count)            HAL_UART_Receive_IT(handle, buf, count)
#define FW_APP_UART_TRANSMIT(handle, buf, count, timeout) HAL_GPIO_Transmit(handle, buf, count, timeout)

// Debug output
#define FW_APP_ENABLE_DEBUG_PRINT
#if defined(FW_APP_ENABLE_DEBUG_PRINT)
#define FW_APP_DEBUG_PRINT(x)       do { printf x; } while (0)
#else
#define FW_APP_DEBUG_PRINT(x)
#endif

#define FW_APP_DEBUG_PACKET_LENGTH  (128)

#define FW_APP_ONE_SEC_INTERVAL     (999) // 1 second

// Digital In/Out
#define FW_APP_MAX_DIN              (2)
#define FW_APP_MAX_DOUT             (2)
#define FW_APP_DIN_MIN_PORT_NUM     (1)
#define FW_APP_DIN_MAX_PORT_NUM     (2)
#define FW_APP_DOUT_MIN_PORT_NUM    (1)
#define FW_APP_DOUT_MAX_PORT_NUM    (2)

// Buttons
#define FW_APP_BTN_COUNT            (1)
#define FW_APP_BTN_STATE_RELEASED   (0)
#define FW_APP_BTN_STATE_PRESSED    (1)
#define FW_APP_BTN_PRESS_CHECK_TIME (10)  // millisecond

#define FW_APP_PROTO_TX_TIMEOUT     (500)

#define FW_APP_DHT22_COUNT          (1)
#define FW_APP_DHT22_MIN_NUM        (1)

FW_LIB_BEGIN_PACK1

typedef struct _fw_app_debug_manager
{
  FW_APP_UART_HANDLE    uart_handle;
  uint8_t               buf[FW_APP_DEBUG_PACKET_LENGTH];
  uint8_t               length;
} fw_app_debug_manager_t;

// Protocol manager
typedef struct _fw_app_proto_manager
{
  // UART handle.
  FW_APP_UART_HANDLE      uart_handle;

  // Buffer for received bytes.
  fw_lib_queue_t          q;
#if FW_APP_PARSER == FW_APP_TXT_PARSER
  fw_lib_txt_parser_t     parser_handle;
  uint8_t                 out_buf[FW_LIB_TXT_MSG_MAX_LENGTH];
#else
  fw_lib_bin_parser_t     parser_handle;
  uint8_t                 out_buf[FW_LIB_BIN_MSG_MAX_LENGTH];
#endif
  uint8_t                 out_length;
  uint8_t                 rx_buf[1];
} fw_app_proto_manager_t;

typedef struct _fw_app_button
{
  uint8_t                 port_id;
  FW_APP_GPIO_HANDLE      port;
  uint16_t                port_pin;
  uint8_t                 prev_physical_status;   // Previous physical button status(PRESSED or RELEASED)
  uint8_t                 prev_logical_status;    // Previous logical button status(PRESSED or RELEASED)
  uint8_t                 current_logical_status; // Current logical button status(PRESSED or RELEASED)
  uint16_t                press_check_time;       // If pressed time > press_check_time -> button pressed
  uint16_t                pressed_count;          // It represents how long a button is pressed. It is used for de-bounce filtering.
} fw_app_button;

typedef struct _fw_app_dht22
{
  fw_lib_stm32_gpio_handle  dht22_gpio;
  fw_lib_dht22              dht22_handle;
} fw_app_dht22_t;
// Firmware application manager.
typedef struct _fw_app
{
  uint32_t                device_id;
#if defined(FW_APP_ENABLE_DEBUG_PRINT)
  fw_app_debug_manager_t  dbg_mgr;
#endif
  // Current tick count.
  volatile uint32_t       tick;

  fw_lib_bool_t           button_initialized;

  // Protocol manager.
  fw_app_proto_manager_t  proto_mgr;

  fw_lib_dio_port_t       dins[FW_APP_MAX_DIN];
  fw_lib_dio_port_t       douts[FW_APP_MAX_DOUT];
  fw_app_button           buttons[FW_APP_BTN_COUNT];
  fw_app_dht22_t          dht22[FW_APP_DHT22_COUNT];
} fw_app_t;

FW_LIB_END_PACK

FW_LIB_BEGIN_DECLS

FW_LIB_DECLARE_DATA extern fw_app_t g_app;

FW_LIB_DECLARE(void) fw_app_init(void);
FW_LIB_DECLARE(void) fw_app_hw_init(void);
FW_LIB_DECLARE(void) fw_app_systick(void);
FW_LIB_DECLARE(fw_lib_dio_port_t*) fw_app_get_dout_port(uint8_t port_id);
FW_LIB_DECLARE(fw_lib_dio_port_t*) fw_app_get_din_port(uint8_t port_id);

FW_LIB_END_DECLS

#endif
