#include <stdint.h>
const char mp_frozen_str_names[] = {
"boot.py\0"
"main.py\0"
"\0"};
const uint32_t mp_frozen_str_sizes[] = {
707,
436,
};
const char mp_frozen_str_content[] = {
"import pyb\nimport machine\ntry:\n    # This will run boot.py from /sd if it exists, otherwise it will run the\n    # code from within the except portion.\n    import os\n    os.stat('/sd/boot.py')\n    execfile('/sd/boot.py')\nexcept:\n    print(\"Executing frozen boot.py\")\n\n    def pins():\n        for pin_name in dir(pyb.Pin.board):\n            pin = pyb.Pin(pin_name)\n            print('{:10s} {:s}'.format(pin_name, str(pin)))\n\n    def af():\n        for pin_name in dir(pyb.Pin.board):\n            pin = pyb.Pin(pin_name)\n            print('{:10s} {:s}'.format(pin_name, str(pin.af_list())))\n\n    def cat(filename):\n        with open(filename, 'rb') as f:\n            for line in f:\n                print(line)\n\0"
"try:\n    # This will run main.py from /sd if it exists, otherwise it will run the\n    # code from within the except portion.\n    import os\n    os.stat('/sd/main.py')\n    execfile('/sd/main.py')\nexcept:\n    import pyb\n\n    print(\"Executing frozen main.py\")\n\n    led = pyb.LED(1)\n    \n    while(1):\n        led.on()\n        pyb.delay(100)\n        led.off()\n        pyb.delay(100)\n        led.on()\n        pyb.delay(100)\n        led.off()\n\0"
};
