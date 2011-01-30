#!/bin/bash
# Thanks to ooPo for making this open source toolchain and to the PSL1GHT Team
# Pre-Compiled Toolchain Made By Goblom - psgroove.com
# Downloaded from https://gihub.com/Goblom/allps3tools.git
# Soon to me moved to https://github.com/Goblom/ooPo-Pre.Comp.ps3toolchain
# Run this every time you start Cygwin

## Set up the PS3DEV environment.
export PS3DEV=/usr/local/ps3dev
export PATH=$PATH:$PS3DEV/bin
export PATH=$PATH:$PS3DEV/host/ppu/bin
export PATH=$PATH:$PS3DEV/host/spu/bin

## Set up the PSL1GHT environment.
export PSL1GHT=$PS3DEV/psl1ght
export PATH=$PATH:$PSL1GHT/host/bin
