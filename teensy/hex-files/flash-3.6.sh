#!/bin/sh
set -x
teensy_loader_cli -v -mmcu=mk66fx1m0 -w -s upy-teensy-3.6.hex
