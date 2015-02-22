#ifndef INC_GLV_ICON_H
#define INC_GLV_ICON_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_draw.h"

namespace glv{

/*
These are intended to replace the function pointer icons in draw::.
They are a bit more difficult to handle than function pointers, such as when 
passing into a constructor. C++ polymorphism forces us to use either pointers
or references.
*/

struct Icon{
	virtual ~Icon(){}
	virtual void draw(float l, float t, float r, float b){}
};

struct Check : public Icon {
	void draw(float l, float t, float r, float b){
		draw::shape(draw::LineStrip, l,0.5*(t+b), l+(r-l)*0.3,b, r,t);
	}
};

struct Cross : public Icon{
	void draw(float l, float t, float r, float b){
		draw::shape(draw::Lines, l,t, r,b, l,b, r,t);
	}
};

struct Rectangle : public Icon {
	void draw(float l, float t, float r, float b){
		draw::shape(draw::TriangleStrip, l,t, l,b, r,t, r,b);
	}
};

} // glv::
#endif
