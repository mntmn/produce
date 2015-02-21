/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <stdio.h>
#include "glv_color.h"

namespace glv{

void Color::setHSV6(float h, float s, float v){
	if( s == 0.f ) {
		r = g = b = v;	// achromatic (gray)
		return;
	}
										// sector 0 to 5
	unsigned int i = (unsigned int)(h);	// integer part of h
	float f = h - (float)i;				// fractional part of h
	float p = v * ( 1.f - s );

	// depends on hue section being even or odd
	float q = v * (1.f - s * ( i & 1U ? f : 1.f - f));

	switch( i ) {
		case 0: r = v; g = q; b = p; break;
		case 1:	r = q; g = v; b = p; break;
		case 2:	r = p; g = v; b = q; break;
		case 3:	r = p; g = q; b = v; break;
		case 4: r = q; g = p; b = v; break;
		default:r = v; g = p; b = q; break;
	}
}

void Color::getHSV6(float &h, float &s, float &v) const{

	float min = r < g ? (r < b ? r : b) : (g < b ? g : b);
	float max = r > g ? (r > b ? r : b) : (g > b ? g : b);

	v = max;					// v
	float delta = max - min;	// delta RGB value

	if ( delta != 0.f && max != 0.f ){		// chromatic data...
		s = delta / max;		// s
		
		float hl;
		if     ( r == max )	hl =       ( g - b ) / delta;	// between yellow & magenta
		else if( g == max )	hl = 2.f + ( b - r ) / delta;	// between cyan & yellow
		else				hl = 4.f + ( r - g ) / delta;	// between magenta & cyan

		if( hl < 0.f ) hl += 6.f;

		h = hl;
	}
	else{				// this is a gray, no chroma...
	   h = 0.f;
	   s = 0.f;
	}
}

void Color::print(const char * append, FILE * fp) const{
 	fprintf(fp, "[%.2f, %.2f, %.2f, %.2f]%s", r, g, b, a, append);
}

} // end namespace glv
