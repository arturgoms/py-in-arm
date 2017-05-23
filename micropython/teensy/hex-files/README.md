The 3 hex files in this diretory built from this checkout:
https://github.com/dhylands/micropython/tree/teensy-usb

This version should have support for:
- GPIO
- Timers
- ADC
- sdcard
- UART (currently no LPUART support)

There is currently no internal filesystem (it is possible to compile
scripts into the firmware).

The builtin boot.py and main.py will both check to see if /sd/boot.py or
/sd/main.py exist (respectively) and execute the versions from the sd card.

The builtin main.py will flash the LED twice each times it runs.

If you don't see the LED flash twice after flashing, try flashing again. I've
noticed issues flashing especially on the 3.6 version.

[rshell](https://github.com/dhylands/rshell) version 0.0.6 should automatically
recognize the teensy and allow files to be copied on and off the sd card, as
well as being able to run the REPL.

The flashing scripts included in this directory assume that teensy_loader_cli
is available in your PATH.
