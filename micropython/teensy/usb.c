#include <string.h>

#include "Arduino.h"

#include "usb.h"
#include "usb_serial.h"
#include "teensy_hal.h"

// Added to core/usb_dev.c
void usb_dev_set_interrupt_char(int ch, void *data);


void usb_init0(void) {
    MP_STATE_PORT(mp_const_vcp_interrupt) = mp_obj_new_exception(&mp_type_KeyboardInterrupt);
    usb_dev_set_interrupt_char(-1, MP_STATE_PORT(mp_const_vcp_interrupt));
}

bool usb_vcp_is_connected(void)
{
  return usb_configuration && (usb_cdc_line_rtsdtr & (USB_SERIAL_DTR | USB_SERIAL_RTS));
}

bool usb_vcp_is_enabled(void)
{
    // We only support CDC mode, so we're always enabled.
    return true;
}

void usb_vcp_set_interrupt_char(int c) {
    if (c != -1) {
        mp_obj_exception_clear_traceback(MP_STATE_PORT(mp_const_vcp_interrupt));
    }
    usb_dev_set_interrupt_char(c, MP_STATE_PORT(mp_const_vcp_interrupt));
}

int usb_vcp_rx_num(void) {
    return usb_serial_available();
}

int usb_vcp_recv_byte(uint8_t *ptr)
{
    int ch = usb_serial_getchar();
    if (ch < 0) {
        return 0;
    }
    *ptr = ch;
    return 1;
}

// This is the only routine which should call usb_serial_write
mp_uint_t usb_serial_tx(const void *buf_in, mp_uint_t size) {
    mp_uint_t remaining = size;
    const uint8_t *buf = buf_in;

    while (remaining > 0) {
        int bytes_to_write = usb_serial_write_buffer_free();
        if (bytes_to_write > 0) {
            if (bytes_to_write > remaining) {
                bytes_to_write = remaining;
            }
            int rc = usb_serial_write(buf, bytes_to_write);
            if (rc < 0) {
                return 0;
            }

            remaining -= bytes_to_write;
            buf += bytes_to_write;
        } else {
            __WFI();
        }
    }
    return size;
}

void usb_vcp_send_str(const char* str)
{
    usb_vcp_send_strn(str, strlen(str));
}

void usb_vcp_send_strn(const char* str, int len)
{
    usb_serial_tx(str, len);
}

void usb_vcp_send_strn_cooked(const char *str, int len)
{
    const char *segment = NULL;
    int seg_len = 0;

    for (const char *top = str + len; str < top; str++) {
        if (*str == '\n') {
            if (segment) {
                usb_serial_tx(segment, seg_len);
                segment = NULL;
                seg_len = 0;
            }
            usb_serial_tx("\r\n", 2);
        } else {
            if (!segment) {
                segment = str;
                seg_len = 0;
            }
            seg_len++;
        }
    }
    if (segment) {
        usb_serial_tx(segment, seg_len);
    }
}

/******************************************************************************/
// Micro Python bindings for USB VCP

/// \moduleref pyb
/// \class USB_VCP - USB virtual comm port
///
/// The USB_VCP class allows creation of an object representing the USB
/// virtual comm port.  It can be used to read and write data over USB to
/// the connected host.

typedef struct _pyb_usb_vcp_obj_t {
    mp_obj_base_t base;
} pyb_usb_vcp_obj_t;

STATIC const pyb_usb_vcp_obj_t pyb_usb_vcp_obj = {{&pyb_usb_vcp_type}};

STATIC void pyb_usb_vcp_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_print_str(print, "USB_VCP()");
}

/// \classmethod \constructor()
/// Create a new USB_VCP object.
STATIC mp_obj_t pyb_usb_vcp_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // TODO raise exception if USB is not configured for VCP

    // return the USB VCP object
    return (mp_obj_t)&pyb_usb_vcp_obj;
}

STATIC mp_obj_t pyb_usb_vcp_setinterrupt(mp_obj_t self_in, mp_obj_t int_chr_in) {
    usb_vcp_set_interrupt_char(mp_obj_get_int(int_chr_in));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pyb_usb_vcp_setinterrupt_obj, pyb_usb_vcp_setinterrupt);

STATIC mp_obj_t pyb_usb_vcp_isconnected(mp_obj_t self_in) {
    return mp_obj_new_bool(usb_configuration != 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_usb_vcp_isconnected_obj, pyb_usb_vcp_isconnected);

/// \method any()
/// Return `True` if any characters waiting, else `False`.
STATIC mp_obj_t pyb_usb_vcp_any(mp_obj_t self_in) {
    return MP_OBJ_NEW_SMALL_INT(usb_serial_available());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_usb_vcp_any_obj, pyb_usb_vcp_any);

/// \method send(data, *, timeout=5000)
/// Send data over the USB VCP:
///
///   - `data` is the data to send (an integer to send, or a buffer object).
///   - `timeout` is the timeout in milliseconds to wait for the send.
///
/// Return value: number of bytes sent.
STATIC const mp_arg_t pyb_usb_vcp_send_args[] = {
    { MP_QSTR_data,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
};
#define PYB_USB_VCP_SEND_NUM_ARGS MP_ARRAY_SIZE(pyb_usb_vcp_send_args)

STATIC mp_obj_t pyb_usb_vcp_send(mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // parse args
    mp_arg_val_t vals[PYB_USB_VCP_SEND_NUM_ARGS];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, PYB_USB_VCP_SEND_NUM_ARGS, pyb_usb_vcp_send_args, vals);

    // TODO: timeout is currently ignored

    // get the buffer to send from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    pyb_buf_get_for_send(vals[0].u_obj, &bufinfo, data);

    // send the data
    return mp_obj_new_int(usb_serial_tx(bufinfo.buf, bufinfo.len));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_usb_vcp_send_obj, 1, pyb_usb_vcp_send);

/// \method recv(data, *, timeout=5000)
///
/// Receive data on the bus:
///
///   - `data` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: if `data` is an integer then a new buffer of the bytes received,
/// otherwise the number of bytes read into `data` is returned.
STATIC mp_obj_t pyb_usb_vcp_recv(mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // parse args
    mp_arg_val_t vals[PYB_USB_VCP_SEND_NUM_ARGS];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, PYB_USB_VCP_SEND_NUM_ARGS, pyb_usb_vcp_send_args, vals);

    // TODO: timeout is currently ignored

    // get the buffer to receive into
    vstr_t vstr;
    mp_obj_t o_ret = pyb_buf_get_for_recv(vals[0].u_obj, &vstr);

    // receive the data
    int bytes_read = usb_serial_read(vstr.buf, vstr.len);

    // return the received data
    if (o_ret != MP_OBJ_NULL) {
        return mp_obj_new_int(bytes_read); // number of bytes read into given buffer
    }

    vstr.len = bytes_read; // set actual number of bytes read
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr); // create a new buffer
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_usb_vcp_recv_obj, 1, pyb_usb_vcp_recv);

mp_obj_t pyb_usb_vcp___exit__(mp_uint_t n_args, const mp_obj_t *args) {
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_usb_vcp___exit___obj, 4, 4, pyb_usb_vcp___exit__);

STATIC const mp_map_elem_t pyb_usb_vcp_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_setinterrupt), (mp_obj_t)&pyb_usb_vcp_setinterrupt_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_isconnected), (mp_obj_t)&pyb_usb_vcp_isconnected_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_any), (mp_obj_t)&pyb_usb_vcp_any_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&pyb_usb_vcp_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recv), (mp_obj_t)&pyb_usb_vcp_recv_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&mp_stream_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readinto), (mp_obj_t)&mp_stream_readinto_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readline), (mp_obj_t)&mp_stream_unbuffered_readline_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_readlines), (mp_obj_t)&mp_stream_unbuffered_readlines_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&mp_stream_write_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_close), (mp_obj_t)&mp_identity_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR___del__), (mp_obj_t)&mp_identity_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR___enter__), (mp_obj_t)&mp_identity_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR___exit__), (mp_obj_t)&pyb_usb_vcp___exit___obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_usb_vcp_locals_dict, pyb_usb_vcp_locals_dict_table);

STATIC mp_uint_t pyb_usb_vcp_read(mp_obj_t self_in, void *buf, mp_uint_t size, int *errcode) {
    int bytes_read = usb_serial_read(buf, size);
    if (bytes_read == 0) {
        // return EAGAIN error to indicate non-blocking
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }
    return bytes_read;
}

STATIC mp_uint_t pyb_usb_vcp_write(mp_obj_t self_in, const void *buf, mp_uint_t size, int *errcode) {
    int ret = usb_serial_tx(buf, size);
    if (ret == 0) {
        // return EAGAIN error to indicate non-blocking
        *errcode = MP_EAGAIN;
        return MP_STREAM_ERROR;
    }
    return ret;
}

STATIC mp_uint_t pyb_usb_vcp_ioctl(mp_obj_t self_in, mp_uint_t request, mp_uint_t arg, int *errcode) {
    mp_uint_t ret;
    if (request == MP_STREAM_POLL) {
        mp_uint_t flags = arg;
        ret = 0;
        if ((flags & MP_STREAM_POLL_RD) && usb_serial_available() > 0) {
            ret |= MP_STREAM_POLL_RD;
        }
        if ((flags & MP_STREAM_POLL_WR) && usb_serial_write_buffer_free() > 0) {
            ret |= MP_STREAM_POLL_WR;
        }
    } else {
        *errcode = MP_EINVAL;
        ret = MP_STREAM_ERROR;
    }
    return ret;
}

STATIC const mp_stream_p_t pyb_usb_vcp_stream_p = {
    .read = pyb_usb_vcp_read,
    .write = pyb_usb_vcp_write,
    .ioctl = pyb_usb_vcp_ioctl,
};

const mp_obj_type_t pyb_usb_vcp_type = {
    { &mp_type_type },
    .name = MP_QSTR_USB_VCP,
    .print = pyb_usb_vcp_print,
    .make_new = pyb_usb_vcp_make_new,
    .getiter = mp_identity,
    .iternext = mp_stream_unbuffered_iter,
    .protocol = &pyb_usb_vcp_stream_p,
    .locals_dict = (mp_obj_t)&pyb_usb_vcp_locals_dict,
};

