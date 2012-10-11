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

#include <stdio.h>
#include <memory.h>
#include "mpbcd.h"

void mp_neg(const mp_num *op, mp_num *result)
{
  mp_num one;
  int i;
  // 9's complement
  for(i=0; i<mp_digits; i++)
  {
    result->digits[i] = 9 - op->digits[i];
  }

  // add least-significant digit '1'
  memset(one.digits, 0, mp_digits);
  one.digits[mp_digits-1] = 1;
  mp_add(result, &one, result);
}

int mp_isneg(const mp_num *val)
{
  if (val->digits[0] == 9) return 1;
  return 0;
}

void mp_sub(const mp_num *op1, const mp_num *op2, mp_num *result)
{
  mp_num op2_neg;
  mp_neg(op2,&op2_neg);
  mp_add(op1, &op2_neg, result);
}

void mp_add(const mp_num *op1, const mp_num *op2, mp_num *result)
{
  mp_num temp;
  int i;
  int8_t over = 0;
  
  for(i=mp_digits-1; i>=0; i--)
  {
    int8_t result = op1->digits[i]+op2->digits[i]+over;
    if (result > 9)
    {
      over = 1;
      result -= 10;
    }
    else over = 0;
      
    temp.digits[i] = result;
  }

  // save output
  *result = temp;
}

// unoptimized as hell, but WTF.. 
void mp_shr(const mp_num *op, int bits, mp_num *result)
{
  mp_num temp = *op;
  int i,b, spilover;
  
  for(b=0; b<bits; b++)
  {
    spilover = (temp.digits[0]==9) ? 5 : 0; // sign bit extending!
    for(i=1; i<mp_digits; i++)
    {
      int8_t d = (temp.digits[i] >> 1) + spilover;
      
      if ((temp.digits[i] & 1) != 0)
        spilover = 5;
      else
        spilover = 0;
        
      temp.digits[i] = d;
    }
  }
  // do rounding based on 'spilover!'
  if (spilover != 0)
  {
    mp_num one;
    memset(one.digits, 0, mp_digits);
    one.digits[mp_digits-1] = 1;
    
    if (temp.digits[0] == 0)
      mp_add(&temp, &one, &temp);
    else
      mp_sub(&temp, &one, &temp);
  }
  *result = temp;
}

void mp_rawprint(const mp_num *val)
{
  int i;
  if (val->digits[0] == 9)
  {
    printf("-");
  }
  
  for(i=1; i<mp_digits; i++)
  {
    printf("%c", '0' + val->digits[i]);
  }
  
}

void mp_print(const mp_num *val)
{
  mp_num temp = *val;
  
  int i;
  if (temp.digits[0] == 9)
  {
    printf("-");
    mp_neg(&temp,&temp);
  }
  
  for(i=1; i<mp_digits; i++)
  {
    if (i==2) printf(".");
    printf("%c", '0' + temp.digits[i]);
  }
}

int mp_load(const char *val, mp_num *result)
{
  int neg = 0;
  int i = 1;
  int idx = 0;
  
  if (val[0] == '-')
    neg = 1;
    
  memset(result->digits, 0, mp_digits);

  while(i < mp_digits)
  {
    if (val[idx] == 0)
      break; // report error
    
    if ((val[idx] < '0') || (val[idx] > '9'))
    {
      // handle stuff?
    }
    else
    {
      result->digits[i] = val[idx] - '0';
      i++;
    }
    
    idx++;
  }
  
  if (neg == 1)
    mp_neg(result,result);
    
  return 0;
}