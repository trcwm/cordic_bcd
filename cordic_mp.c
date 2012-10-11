/*
  Calculating high-precision cos(x) and sin(x) based on CORDIC rotations
  and BCD arithmetic. This program is meant as an experiment and has not
  been optimized for speed.
   
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
#include <stdint.h>

// Set the number of cordic iterations/stages
#define stages 75

#include "mpbcd.h"

// Define a vector type consisting of a real and imaginary part.
typedef struct 
{
	mp_num		real;	// Real part, or cos(x)
	mp_num		imag; // Imaginary part, or sin(x)
} coord_t;

/** Initialize the CORDIC table with atan(1/x),
    where x = 1,2,3 .. etc.
    
    When x is small, atan(1/x) is approximately 1/x,
    so from entry 24 onwards, the table is completed
    by halving the preceding table entry.
 */

mp_num angle_tbl[stages];

void init_table()
{
  int i;
  mp_load("0.78539816339744830961566084581987572104929234984377", angle_tbl);
  mp_load("0.46364760900080611621425623146121440202853705428612", angle_tbl+1);
  mp_load("0.24497866312686415417208248121127581091414409838118", angle_tbl+2);
  mp_load("0.12435499454676143503135484916387102557317019176980", angle_tbl+3);
  mp_load("0.06241880999595734847397911298550511360627388779749", angle_tbl+4);
  mp_load("0.03123983343026827625371174489249097703249566372540", angle_tbl+5);
  mp_load("0.01562372862047683080280152125657031891111413980090", angle_tbl+6);
  mp_load("0.00781234106010111129646339184219928162122281172501", angle_tbl+7);
  mp_load("0.00390623013196697182762866531142438714035749011520", angle_tbl+8);
  mp_load("0.00195312251647881868512148262507671393161074677723", angle_tbl+9);
  mp_load("0.00097656218955931943040343019971729085163419701581", angle_tbl+10);
  mp_load("0.00048828121119489827546923962564484866619236113313", angle_tbl+11);
  mp_load("0.00024414062014936176401672294325965998621241779097", angle_tbl+12);
  mp_load("0.00012207031189367020423905864611795630093082940901", angle_tbl+13);
  mp_load("0.00006103515617420877502166256917382915378514353683", angle_tbl+14);
  mp_load("0.00003051757811552609686182595343853601975094967511", angle_tbl+15);
  mp_load("0.00001525878906131576210723193581269788513742923814", angle_tbl+16);
  mp_load("0.00000762939453110197026338848234010509058635074391", angle_tbl+17);
  mp_load("0.00000381469726560649628292307561637299372280525730", angle_tbl+18);
  mp_load("0.00000190734863281018703536536930591724416871434216", angle_tbl+19);
  mp_load("0.00000095367431640596087942067068992311239001963412", angle_tbl+20);
  mp_load("0.00000047683715820308885992758382144924707587049404", angle_tbl+21);
  mp_load("0.00000023841857910155798249094797721893269783096898", angle_tbl+22);
  mp_load("0.00000011920928955078068531136849713792211264596758", angle_tbl+23);
  
  for(i=24; i<stages; i++)
  {
    mp_shr(angle_tbl+i-1, 1, angle_tbl+i);  // divide the preceding table entry by 2
  }
}

/** Perfom a cordic vector rotation, based on the residual angle given by angle_in.
    If angle_in is positive, the vector is rotated anti-clockwise, else it is rotated
    clockwise.
    
    The basis of the CORDIC rotation is the regular 2D rotation:
    x' = x * cos(angle) - y * sin(angle)
    y' = x * sin(angle) + y * cos(angle)
    
    To avoid using cos() and sin(), the equations are written as:
    x' = (1/cos(angle)) * (x - y * tan(angle))
    y' = (1/cos(angle)) * (x * tan(angle) + y)
    
    Then, the 1/cos(angle) factors are removed:
    x'' = x - y * tan(angle)
    y'' = x * tan(angle) + y
    ,which means we're not only rotating, but also increasing the vector length
    by cos(angle). This increase in length is termed the 'CORDIC gain' and must
    be compensated either at the start or at the end of the algorithm.
    
    The tan(angle) operation is removed by choosing only angles that result
    in tan(angle) = 2^-n, which allows the multiplication by tan(angle) to
    be replaced by a arithmetic right shift operation. The resulting rotation
    equation is now:
    
    x'' = x - y shr n
    y'' = x shr n + y
    
    For a clockwise rotation, the signs are inverted:
    
    x'' = x + y shr n
    y'' = x shr n - y
    
    The stage variable determines 'n'. As 'n' increases, the rotation angle decreases.
    The output angle 'angle_out' is the input angle 'angle_in' minus the rotation
    caused by the CORDIC stage.
    
 */
void cordic_rot(const coord_t *coord_in, const mp_num *angle_in, coord_t *coord_out, uint32_t stage, mp_num *angle_out)
{
	coord_t newcoord;
	mp_num angle, delta_angle;
	mp_num s_imag, s_real;
	
  angle = *angle_in;
  
  // Lookup the angle of rotation that this stage will cause
	delta_angle = angle_tbl[stage];
	
  // Calculate the shift-right operation in advance.
  mp_shr(&(coord_in->imag), stage, &s_imag);
  mp_shr(&(coord_in->real), stage, &s_real);
  
  // if the input angle is positive, rotate the input vector anti-clockwise...
  // else clockwise.
	if (!mp_isneg(&angle))
	{
    // do rotation
    mp_sub(&(coord_in->real), &(s_imag), &(newcoord.real));
    mp_add(&(coord_in->imag), &(s_real), &(newcoord.imag));
    // calculate the new vector angle
    mp_sub(&angle, &delta_angle, &angle);
	}
	else
	{
    // do rotation
    mp_add(&(coord_in->real), &(s_imag), &(newcoord.real));
    mp_sub(&(coord_in->imag), &(s_real), &(newcoord.imag));
		// calculate the new vector angle
    mp_add(&angle, &delta_angle, &angle);		
	}
	*coord_out = newcoord;
	*angle_out = angle;
}


/** The main routine */
int main(int argc, char *argv[])
{
  mp_num angle;
	coord_t c;
  
  // Initialize the CORDIC angle table
  init_table();
    
  // Set the angle in radians!
  // For now, only angles between 0 and 1/2*pi are supported.
  
  //mp_load("1.570796326794897", &angle); // 90 degrees
  //mp_load("0.78539816339744830961566084581987572104929234984345", &angle);    // 45 degrees
  mp_load("0.52359877559829887307710723054658381403286156656251", &angle);  // 30 degrees
  
  // Initialize the start vector to (1,0) and pre-divide the vector by the
  // total CORDIC gain.
  // Note: for other quadrants, set a different start vector:
  // (0,1), (-1,0) or (0,-1) 
  
  mp_load("0.60725293500888125616944675250492826311239085215007", &(c.real));
  mp_load("0.0", &(c.imag));
  
  // Perform the CORDIC rotations
	int i;
	for(i=0; i<stages; i++)
	{
		cordic_rot(&c, &angle, &c, i, &angle);
    printf("stage %d:   angle ", i);
    mp_print(&angle);
    printf(" -> ");
    mp_print(&(c.real));
    printf(" ");
    mp_print(&(c.imag));
    printf("\n");
	}
	printf("\n\nFinal results:\nsin(x) = ");
  mp_print(&(c.real));
  printf("\ncos(x) = ");
  mp_print(&(c.imag));
  printf("\n");
  
  return 0;
}
