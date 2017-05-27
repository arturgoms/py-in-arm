try:
    # This will run main.py from /sd if it exists, otherwise it will run the
    # code from within the except portion.
    import os
    os.stat('/sd/main.py')
    execfile('/sd/main.py')
except:
    import pyb

    print("Executing frozen main.py")

    led = pyb.LED(1)

    led.on()
    pyb.delay(100)
    led.off()
    pyb.delay(100)
    led.on()
    pyb.delay(100)
    led.off()
