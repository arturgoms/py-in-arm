/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows 
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Some of this code was taken from teensyduino's analog.c (hence the
 * preservation of the copyright notice above).
 */

#include <stdio.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include "adc.h"
#include "pin.h"

#include "kinetis.h"

typedef struct _pyb_adc_obj_t {
    mp_obj_base_t   base;
    uint8_t         id;
    uint8_t         bits;
    uint8_t         average;
    uint8_t         calibrating;
    uint8_t         use_aref;
    uint8_t         right_shift;
    ADC_TypeDef    *adc;
    volatile bool   busy;
} pyb_adc_obj_t;

typedef struct {
    mp_obj_base_t   base;
    pyb_adc_obj_t  *adc;
    uint8_t         id;

} pyb_adc_channel_obj_t;

typedef ADC_TypeDef *ADC_TypeDefPtr;

STATIC const ADC_TypeDefPtr pyb_ADC[MICROPY_HW_NUM_ADCS] = { ADC0, ADC1 };

#define ADC_CHANNEL_MASK    0x1F
#define ADC_CHANNEL_DIFF    0x20
#define ADC_CHANNEL_MUXSEL  0x40    // for channels 4-7

// the alternate clock is connected to OSCERCLK (16 MHz).
// datasheet says ADC clock should be 2 to 12 MHz for 16 bit mode
// datasheet says ADC clock should be 1 to 18 MHz for 8-12 bit mode

#if F_BUS == 120000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(3) + ADC_CFG1_ADICLK(1) // 7.5 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 15 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 15 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 15 MHz
#elif F_BUS == 108000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(3) + ADC_CFG1_ADICLK(1) // 7 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 14 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 14 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 14 MHz
#elif F_BUS == 96000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 12 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 12 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 12 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 24 MHz
#elif F_BUS == 90000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 11.25 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 11.25 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 11.25 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 22.5 MHz
#elif F_BUS == 80000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 10 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 10 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 10 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 20 MHz			
#elif F_BUS == 72000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 9 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 18 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 18 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 18 MHz
#elif F_BUS == 64000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 8 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 16 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 16 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 16 MHz
#elif F_BUS == 60000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 7.5 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 15 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 15 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 15 MHz
#elif F_BUS == 56000000 || F_BUS == 54000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1) // 7 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 14 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 14 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 14 MHz
#elif F_BUS == 48000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 12 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 12 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 12 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(1) // 24 MHz
#elif F_BUS == 40000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 10 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 10 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 10 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(1) // 20 MHz
#elif F_BUS == 36000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1) // 9 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(1) // 18 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(1) // 18 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(1) // 18 MHz
#elif F_BUS == 24000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(0) // 12 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(0) // 12 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(0) // 12 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 24 MHz
#elif F_BUS == 16000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 8 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 8 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 8 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 16 MHz
#elif F_BUS == 8000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 8 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 8 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 8 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 8 MHz
#elif F_BUS == 4000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 4 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 4 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 4 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 4 MHz
#elif F_BUS == 2000000
  #define ADC_CFG1_16BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 2 MHz
  #define ADC_CFG1_12BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 2 MHz
  #define ADC_CFG1_10BIT  ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 2 MHz
  #define ADC_CFG1_8BIT   ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(0) // 2 MHz
#else
#error "F_BUS must be 120, 108, 96, 90, 80, 72, 64, 60, 56, 54, 48, 40, 36, 24, 4 or 2 MHz"
#endif

const pin_af_obj_t *adc_pin_find_af(const pin_obj_t *pin, uint8_t unit) {
    const pin_af_obj_t *af = pin->af;
    for (mp_uint_t i = 0; i < pin->num_af; i++, af++) {
        if (af->fn == AF_FN_ADC && (af->unit == unit || unit == 0xff)) {
            return af;
        }
    }
    return NULL;
}

STATIC void pyb_adc_channel_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_adc_channel_obj_t *self = self_in;
    uint8_t channel = self->id & ADC_CHANNEL_MASK;
    mp_printf(print, "ADC_Channel(ADC%d, id=%u", self->adc->id, channel);
    if (channel >= 4 && channel <= 7) {
        if (self->id & ADC_CHANNEL_MUXSEL) {
            mp_printf(print, "a");
        } else {
            mp_printf(print, "b");
        }
    }
    mp_printf(print, ", diff=%s)", (self->id & ADC_CHANNEL_DIFF) ? "True" : "False");
}

STATIC void adc_wait_for_cal(pyb_adc_obj_t *self) {
    ADC_TypeDef *adc = self->adc;
    while (adc->SC3 & ADC_SC3_CAL) {
        ; // wait
    }
    mp_uint_t state = disable_irq();
    if (self->calibrating) {
        uint16_t sum = adc->CLPS + adc->CLP4 + adc->CLP3 + adc->CLP2 + adc->CLP1 + adc->CLP0;
        sum = (sum / 2) | 0x8000;
        adc->PG = sum;
        sum = adc->CLMS + adc->CLM4 + adc->CLM3 + adc->CLM2 + adc->CLM1 + adc->CLM0;
        sum = (sum / 2) | 0x8000;
        adc->MG = sum;
        self->calibrating = 0;
    }
    enable_irq(state);
}

STATIC mp_obj_t pyb_adc_channel_read(mp_obj_t self_in) {
    pyb_adc_channel_obj_t *self = self_in;
    pyb_adc_obj_t *adc = self->adc;
    ADC_TypeDef *adc_reg = adc->adc;

    if (adc->calibrating) {
        adc_wait_for_cal(adc);
    }
    mp_uint_t state = disable_irq();
    uint8_t channel = self->id;
startADC:
    if (channel & ADC_CHANNEL_MUXSEL) {
        adc_reg->CFG2 &= ~ADC_CFG2_MUXSEL;
        channel &= 0x3F;    // Note: that channel also includes DIFF bit
    } else {
        adc_reg->CFG2 |= ADC_CFG2_MUXSEL;
    }
    adc_reg->SC1A = channel;
    adc->busy = 1;
    enable_irq(state);
    while (1) {
        state = disable_irq();
        if (adc_reg->SC1A & ADC_SC1_COCO) {
            mp_int_t result = adc_reg->RA;
            adc->busy = false;
            enable_irq(state);
            return mp_obj_new_int(result >> adc->right_shift);
        }
        // Detect if read was used from an interrupt. If so, our read got
        // cancelled, so it must be restarted.
        if (!adc->busy) {
            goto startADC;
        }
        enable_irq(state);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_adc_channel_read_obj, pyb_adc_channel_read);

STATIC const mp_map_elem_t pyb_adc_channel_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&pyb_adc_channel_read_obj },
};
STATIC MP_DEFINE_CONST_DICT(pyb_adc_channel_locals_dict, pyb_adc_channel_locals_dict_table);

const mp_obj_type_t pyb_adc_channel_type = {
    { &mp_type_type },
    .name = MP_QSTR_ADCChannel,
    .print = pyb_adc_channel_print,
    .locals_dict = (mp_obj_t)&pyb_adc_channel_locals_dict,
};

STATIC void pyb_adc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_adc_obj_t *self = self_in;
    mp_printf(print, "ADC(%d, bits=%d, average=%d, use_aref=%s)",
              self->id, self->bits, self->average,
              self->use_aref ? "True" : "False");
}

STATIC void adc_init(pyb_adc_obj_t *self) {
    ADC_TypeDef *adc = self->adc;
    if (self->calibrating) {
        adc->SC3 = 0;   // cancel calibration
    }
    VREF_TRM = 0x60;
    VREF_SC = 0xE1;		// enable 1.2 volt ref

    if (self->bits > 12) {
        // Configure for 16 bit
        adc->CFG1 = ADC_CFG1_16BIT + ADC_CFG1_MODE(3)  + ADC_CFG1_ADLSMP;
        adc->CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(2);
        self->right_shift = 16 - self->bits;
    } else if (self->bits > 10) {
        // Configure for 12 bit
        adc->CFG1 = ADC_CFG1_12BIT + ADC_CFG1_MODE(1)  + ADC_CFG1_ADLSMP;
        adc->CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(2);
        self->right_shift = 12 - self->bits;
    } else if (self->bits > 8) {
        // Configure for 10 bit
        adc->CFG1 = ADC_CFG1_10BIT + ADC_CFG1_MODE(2)  + ADC_CFG1_ADLSMP;
        adc->CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(3);
        self->right_shift = 10 - self->bits;
    } else {
        // Configure for 8 bit
        adc->CFG1 = ADC_CFG1_8BIT + ADC_CFG1_MODE(0);
        adc->CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(3);
        self->right_shift = 8 - self->bits;
    }

    if (self->use_aref) {
        adc->SC2 = ADC_SC2_REFSEL(0); // AREF pin
    } else {
        adc->SC2 = ADC_SC2_REFSEL(1); // 1.2V ref
    }

    if (self->average <= 1) {
        self->average = 1;
        adc->SC3 = ADC_SC3_CAL; // begin calibration
    } else if (self->average <= 4) {
        self->average = 4;
        adc->SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(0);
    } else if (self->average <= 8) {
        self->average = 8;
        adc->SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(1);
    } else if (self->average <= 16) {
        self->average = 16;
        adc->SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(2);
    } else {
        self->average = 32;
        adc->SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(3);
    }
    self->calibrating = 1;
}

STATIC mp_obj_t pyb_adc_init_helper(pyb_adc_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_bits, MP_ARG_INT, {.u_int = 10} },
        { MP_QSTR_average, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_use_aref, MP_ARG_BOOL, {.u_bool = true} },
    };

    // parse args
    struct {
        mp_arg_val_t bits, average, use_aref;
    } args;
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

    if (args.bits.u_int > 16 || args.bits.u_int < 1) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "bits must be between 1 and 16"));
    }
    self->bits = args.bits.u_int;
    self->average = args.average.u_int;
    self->use_aref = args.use_aref.u_bool;

    // Initialize the ADC
    adc_init(self);

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t pyb_adc_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    int adc_id = mp_obj_get_int(args[0]);
    if (adc_id < 0 || adc_id >= MICROPY_HW_NUM_ADCS) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "ADC(%d) does not exist", adc_id));
    }

    pyb_adc_obj_t *self = MP_STATE_PORT(pyb_adc_obj_all)[adc_id];
    if (self == NULL) {
        self = m_new0(pyb_adc_obj_t, 1);
        self->base.type = &pyb_adc_type;
        self->id = adc_id;
        self->bits = 10;
        self->average = 1;
        self->calibrating = 0;
        self->use_aref = true;
        self->busy = false;
        self->adc = pyb_ADC[adc_id];
        MP_STATE_PORT(pyb_adc_obj_all)[adc_id] = self;
    }
    if (n_args > 1 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pyb_adc_init_helper(self, n_args - 1, args + 1, &kw_args);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t pyb_adc_init(mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_adc_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_adc_init_obj, 1, pyb_adc_init);

STATIC mp_obj_t adc_new_channel_obj(pyb_adc_obj_t *adc, uint8_t id, bool diff) {
    pyb_adc_channel_obj_t *channel = m_new0(pyb_adc_channel_obj_t, 1);
    channel->base.type = &pyb_adc_channel_type;
    channel->adc = adc;
    channel->id = id;
    if (diff) {
        channel->id |= ADC_CHANNEL_DIFF;
    }
    return MP_OBJ_FROM_PTR(channel);
}

STATIC mp_obj_t pyb_adc_channel(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_pin, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_diff, MP_ARG_BOOL, {.u_obj = false} },
    };
    mp_arg_check_num(n_args, kw_args->used, 1, MP_OBJ_FUN_ARGS_MAX, true);

        // parse args
    struct {
        mp_arg_val_t id, pin, diff;
    } args;
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, (mp_arg_val_t*)&args);

    pyb_adc_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    if (args.id.u_obj == mp_const_none && args.pin.u_obj == mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Must specify id or pin"));
    }

    mp_int_t id = -1;
    mp_obj_t pin_obj = args.pin.u_obj;

    if (!mp_obj_get_int_maybe(args.id.u_obj, &id) && pin_obj == mp_const_none) {
        // If the user calls channel and uses a non-numeric positional argument
        // then for convenience, we'll allow that to be used as a pin argument
        pin_obj = args.id.u_obj;
    }

    if (id >= 0) {
        // We've been given a channel and adc unit, let's just use it.
        return adc_new_channel_obj(self, id, args.diff.u_bool);
    }

    printf("Using pin lookup\n");

    // We've been given a pin, check to see if our ADC exists on the pin.
    const pin_obj_t *pin = pin_find(pin_obj);
    printf("Found pin %s\n", qstr_str(pin->name));
    const pin_af_obj_t *pin_af = adc_pin_find_af(pin, self->id);
    if (pin_af == NULL) {
        printf("pin doesn't have ADC%d\n", self->id);

        // An ADC channel for the same ADC unit doesn't exist, see if we can
        // find an ADC channel for any ADC unit.
        pin_af = adc_pin_find_af(pin, 0xff);
        if (pin_af == NULL) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "pin has no ADC function"));
        }
        printf("Found ADC%d\n", pin_af->unit);
    }
    id = pin_af->type;
    printf("Found ADC%d chan=%d\n", pin_af->unit, pin_af->type);

    if (MP_STATE_PORT(pyb_adc_obj_all)[pin_af->unit] == NULL) {
        // The functionality exists on another adc that hasn't yet been
        // initialized, so initialize it.

        pyb_adc_obj_t *adc2 = m_new0(pyb_adc_obj_t, 1);
        adc2->base.type = &pyb_adc_type;
        adc2->id = pin_af->unit;
        adc2->bits = self->bits;
        adc2->average = self->average;
        adc2->calibrating = 0;
        adc2->use_aref = self->use_aref;
        adc2->busy = false;
        adc2->adc = pyb_ADC[pin_af->unit];
        MP_STATE_PORT(pyb_adc_obj_all)[pin_af->unit] = adc2;

        adc_init(adc2);
    }

    return adc_new_channel_obj(MP_STATE_PORT(pyb_adc_obj_all)[pin_af->unit], id, args.diff.u_bool);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_adc_channel_obj, 1, pyb_adc_channel);

STATIC const mp_map_elem_t pyb_adc_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&pyb_adc_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_channel), (mp_obj_t)&pyb_adc_channel_obj },
};
STATIC MP_DEFINE_CONST_DICT(pyb_adc_locals_dict, pyb_adc_locals_dict_table);

const mp_obj_type_t pyb_adc_type = {
    { &mp_type_type },
    .name = MP_QSTR_ADC,
    .print = pyb_adc_print,
    .make_new = pyb_adc_make_new,
    .locals_dict = (mp_obj_t)&pyb_adc_locals_dict,
};
