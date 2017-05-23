#ifndef __MICROPY_INCLUDED_TEENSY_USB_H__
#define __MICROPY_INCLUDED_TEENSY_USB_H__

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "bufhelper.h"

extern const mp_obj_type_t pyb_usb_vcp_type;

void usb_init0(void);
bool usb_vcp_is_connected(void);
bool usb_vcp_is_enabled(void);
void usb_vcp_set_interrupt_char(int c);
int usb_vcp_rx_num(void);
int usb_vcp_recv_byte(uint8_t *ptr);
void usb_vcp_send_str(const char* str);
void usb_vcp_send_strn(const char* str, int len);
void usb_vcp_send_strn_cooked(const char *str, int len);

#endif  // __MICROPY_INCLUDED_TEENSY_USB_H__
