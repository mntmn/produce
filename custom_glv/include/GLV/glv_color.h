#ifndef INC_GLV_COLOR_H
#define INC_GLV_COLOR_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_util.h"
#include <stdio.h>

namespace glv {

class HSV;

/// Color represented by red, green, blue, and alpha components
struct Color{

	union{
		struct{
			float r;	///< Red component in [0, 1]
			float g;	///< Green component in [0, 1]
			float b;	///< Blue component in [0, 1]
			float a;	///< Alpha component in [0, 1]
		};
		float components[4]; ///< RGBA component vector
	};


	/// @param[in] r			red component
	/// @param[in] g			green component
	/// @param[in] b			blue component
	/// @param[in] a			alpha component
	Color(float r, float g, float b, float a = 1.f);
	
	/// @param[in] gray			red/green/blue components
	/// @param[in] a			alpha component
	Color(float gray=1.f, float a=1.f);

	Color(const HSV& hsv, float a=1.f);
	
	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }
	
	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }

	
	Color& operator= (const HSV& hsv);				///< Set color from HSV values
	Color& operator*=(float v);						///< Multiply RGBA values by argument
	Color  operator* (float v) const;				///< Multiplies RGBA components by argument
	Color  operator+ (const Color& c) const;		///< Adds argument RGBA components
	Color  operator- (const Color& c) const;		///< Subtracts argument RGBA components
	Color blackAndWhite() const;					///< Returns nearest black or white color
	Color inverse() const;							///< Returns inverted color
	float luminance() const;						///< Get luminance value
	Color mix(const Color& c, float amt);			///< Returns linear mix with another color (0 = none)

	void clamp();									///< Clamp RGB components into [0,1]
	void invert();									///< Invert colors
	void scale(float amount);						///< Scale RGB components
	void set(float r, float g, float b, float a);	///< Set RGBA components
	void set(float r, float g, float b);			///< Set RGB components
	void set(const Color& other, float rgbScale=1);	///< Copy another Color's colors
	void set(float gray);							///< Set as gray color
	void set(float gray, float alpha);				///< Set as gray color with alpha

	
	/// Set color from HSV values in [0, 1]
	void setHSV(float h, float s, float v);

	/// Get HSV values in [0, 1] from color
	void getHSV(float &h, float &s, float &v) const;

	/// Set color from H value in [0, 6] and SV values in [0, 1].
	void setHSV6(float h, float s, float v);

	/// Get H value in [0, 6] and SV values in [0, 1] from color.
	void getHSV6(float &h, float &s, float &v) const;

	void print(const char * append="\n", FILE * fp=stdout) const;
};


/// Color represented by hue, saturation, and value
struct HSV{

	union{
		struct{
			float h;	///< Hue component in [0, 1]
			float s;	///< Saturation component in [0, 1]
			float v;	///< Value component in [0, 1]
		};
		float components[3]; ///< HSV component vector
	};

	/// @param[in] h	hue
	/// @param[in] s	saturation
	/// @param[in] v	value
	HSV(float h=0, float s=1, float v=1): h(h), s(s), v(v){}
	
	/// @param[in] c	RGB color to convert from
	HSV(const Color& c){ *this = c; }

	/// @param[in] hsv		3-vector of hsv components
	template<class T>
	HSV(T * hsv): h(hsv[0]), s(hsv[1]), v(hsv[2]){}

	/// Set from RGB color
	HSV& operator=(const Color& c){ c.getHSV(h, s, v); return *this; }
	
	/// Rotate hue in interval [0, 1)
	HSV& rotateHue(float v){ h += v; wrapHue(); return *this; }
	
	/// Wrap hue value into valid interval [0, 1)
	void wrapHue(){
		if(h>1){ h -= int(h); }
		else if(h<0){ h -= int(h)-1; }
	}
};



// Implementation ______________________________________________________________

inline Color::Color(float r, float g, float b, float a)
:	r(r), g(g), b(b), a(a)
{}

inline Color::Color(float v, float a)
:	r(v), g(v), b(v), a(a)
{}

inline Color::Color(const HSV& hsv, float a)
:	a(a)
{
	setHSV(hsv.h, hsv.s, hsv.v);
}


inline Color& Color::operator=(const HSV& hsv){
	setHSV(hsv.h, hsv.s, hsv.v); return *this;
}

inline Color Color::operator* (float v) const { return Color(r*v, g*v, b*v, a*v); }
inline Color Color::operator+ (const Color& c) const { return Color(r+c.r, g+c.g, b+c.b, a+c.a); }
inline Color Color::operator- (const Color& c) const { return Color(r-c.r, g-c.g, b-c.b, a-c.a); }
inline Color& Color::operator*=(float v){ set(r*v, g*v, b*v, a*v); return *this; }

//inline Color Color::blackAndWhite() const { return Color((r>0.5||g>0.5||b>0.5)?1:0); }
inline Color Color::blackAndWhite() const { return Color(luminance()>0.5f?1.f:0.f); }
inline Color Color::inverse() const { return Color(1.f-r, 1.f-g, 1.f-b, a); }
inline float Color::luminance() const { return r*0.299f+g*0.587f+b*0.114f; }
inline Color Color::mix(const Color& c, float f){ return (c-*this)*f + *this; }

inline void	Color::clamp(){
	r<0.f ? r=0.f : (r>1.f ? r=1.f : 0);
	g<0.f ? g=0.f : (g>1.f ? g=1.f : 0);
	b<0.f ? b=0.f : (b>1.f ? b=1.f : 0);
	a<0.f ? a=0.f : (a>1.f ? a=1.f : 0);
}

inline void	Color::invert(){ set(1.f - r, 1.f - g, 1.f - b); }
inline void	Color::scale(float v){ set(r*v, g*v, b*v); }

inline void	Color::set(float re, float gr, float bl, float al){
	r = re; g = gr; b = bl; a = al;
}

inline void	Color::set(float re, float gr, float bl){ set(re, gr, bl, a); }

inline void	Color::set(const Color& c, float rgbScale){
	set(c.r, c.g, c.b, c.a);
	scale(rgbScale);
}
inline void	Color::set(float v){ set(v,v,v); }
inline void	Color::set(float v, float al){ set(v,v,v,al); }

inline void Color::setHSV(float h, float s, float v){
	setHSV6(h * 6.f, s, v);		
}

inline void Color::getHSV(float& h, float& s, float& v) const{
	getHSV6(h,s,v);
	h *= 0.166666667f;				// 0..1
}

} // glv::

#endif

