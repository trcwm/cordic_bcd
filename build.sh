#!/bin/sh

gcc cordic.c -o cordic
gcc mpbcd.c mpbcd_test.c -o mpbcd_test
#gcc mpbcd.c atan_mp.c -o atan_mp
gcc mpbcd.c cordic_mp.c -o cordic_mp