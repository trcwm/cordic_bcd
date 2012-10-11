/*

  Tiny fixed-point BCD MATH library. This program is meant as an experiment and
  has not been optimized for speed.
  
  Copyright (c) 2012 Niels A. Moseley

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this softwareand associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is f
  urnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#ifndef __mpbcd_h
#define __mpbcd_h

#include <stdint.h>

#define mp_digits 24
#define mp_intdigits 1

/** Define a structure that contains a fixed-point BCD number
    The decimal point is not specified in this structure
    and is assumed to be between the second and third byte.
    
    The BCD number is stored as:
      digits[0]: sign byte, '0' is positive, '9' is negative.
      digits[1]: integer portion of the BCD number.
      digits[2 .. ]: fractional portion of the BCD number.
      
    The BCD number is stored most-significant digit first.
    Note: The digits are not ASCII values!
*/
typedef struct
{
  int8_t digits[mp_digits];   
} mp_num;

/** Print a BCD number */
void mp_print(const mp_num *val);

/** Negate (9's complement) a BCD number given by 'op'
    and store it in 'result'. Input and output pointers
    may point to the same memory address.
*/
void mp_neg(const mp_num *op, mp_num *result);

/** Add two BCD numbers 'op1' and 'op2' and store the result
    in 'result'. No overflow checking is done. Input and
    output pointers may point to the same memory address.
*/
void mp_add(const mp_num *op1, const mp_num *op2, mp_num *result);

/** Subtract two BCD numbers: 'op1' - 'op2' and store the result
    in 'result'. No overflow checking is done. Input and
    output pointers may point to the same memory address.
*/
void mp_sub(const mp_num *op1, const mp_num *op2, mp_num *result);

/** Shift-right a BCD number 'op' by 'bits' bits and store
    the result in 'result'. 'bits' must be positive; no
    range checking is done!
    
    Input and output pointers may point to the same memory address.
    
    Note: this routine is *very* inefficient.
*/
void mp_shr(const mp_num *op, int bits, mp_num *result);

/** Load a BCD number using an ASCII string 'numstr'
    All ASCII characters not belonging to the set 0-9
    are ignored, including the decimal point.
    
    One exception to the above is the minus sign.
    When a negative number is desired, the first
    ASCII char must be '-'.
    
    The routine keeps reading 'numstr' until the
    maximum number of BCD digits have been read,
    or until a NULL-terminator is found.
    
    Examples:
      mp_load("-0.1") loads the BCD result with -0.1
      mp_load("1") loads the BCD result with 1
      mp_load("0001") loads the BCD result with 0.001
  
    Note: you can probably crash this routine pretty
    easily.
*/
int mp_load(const char *numstr, mp_num *result);

/** Check if a BCD number is negative.
    If it is, 1 is returned, else 0.
*/
int mp_isneg(const mp_num *val);

#endif
