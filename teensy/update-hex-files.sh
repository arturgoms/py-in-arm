#!/bin/sh

for ver in 3.1 3.5 3.6; do
    BOARD=TEENSY_${ver}
    make BOARD=${BOARD} clean
    make BOARD=${BOARD}
done

# Do copy after build, so that the built images don't show up as "dirty"

for ver in 3.1 3.5 3.6; do
    HEX=upy-teensy-${ver}.hex
    BOARD=TEENSY_${ver}
    cp build-${BOARD}/micropython.hex hex-files/${HEX}
done
