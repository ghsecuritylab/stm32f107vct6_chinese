#!/bin/bash

arm-none-eabi-nm  --print-size --size-sort --line-number build/stm32f107vct6_base.elf  | grep -e " B " -e " D " -e " b " -e " d "
