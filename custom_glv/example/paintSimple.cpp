/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */
/*
Example:
Demonstration of a simple painting application using the mouse as a brush.
The left mouse button controls the brush up and down and the right button
controls the eraser up and down.
*/

#include "example.h"
using namespace glv;

struct Canvas : public Plot{
	Canvas(const Rect& r): Plot(r){
		data().resize(Data::FLOAT, 1,64,64);
	}

	bool onEvent(Event::t e, GLV& g){

		float gx = pixToGrid(0, g.mouse().xRel());	// convert mouse to grid position
		float gy = pixToGrid(1, g.mouse().yRel());	// convert mouse to grid position
		int cx = (gx*0.5+0.5) * data().size(1);		// convert grid to cell position
		int cy = (gy*0.5+0.5) * data().size(2);		// convert grid to cell position

		switch(e){
		case Event::MouseDown:
		case Event::MouseDrag:
			if(data().inBounds(0, cx, cy)){
				data().assign(g.mouse().button() ? 0:1, 0,cx,cy);
			}
			break;
		case Event::KeyDown:
			switch(g.keyboard().key()){
			case 'c': data().assignAll(0); break;
			}
			break;
		default:;
		}
		
		return true;
	}
};


int main(){
	GLV top;
	Canvas v1(Rect(000,0, 00,00));
	v1.add(*new PlotDensity(Color(1)));
	
	v1.stretch(1,1);
	top << v1;

	Window win(600,600, "Simple Paint", &top);
	Application::run();
	return 0;
}

