#!/bin/sh
set -x
teensy_loader_cli -v -mmcu=mk20dx256 -w -s upy-teensy-3.1.hex
