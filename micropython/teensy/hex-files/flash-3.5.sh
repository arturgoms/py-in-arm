#!/bin/sh
set -x
teensy_loader_cli -v -mmcu=mk64fx512 -w -s upy-teensy-3.5.hex
