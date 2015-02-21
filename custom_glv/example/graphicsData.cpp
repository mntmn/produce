/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: GraphicsData

This demonstrates how to use GraphicsData to draw a heptagram shape.
*/

#include "example.h"

struct Scene : View3D{

	virtual void onDraw3D(GLV& g){

		GraphicsData& gd = g.graphicsData();
		
		draw::lineWidth(2);

		// generate rainbow heptagon
		for(int i=0; i<7; ++i){
			float p = float(i)/7;
			gd.addColor(HSV(p));
			gd.addVertex(cos(p*2*M_PI), sin(p*2*M_PI), -3);
		}

		// draw heptagon
		draw::paint(draw::LineLoop, gd);

		// generate indices 0, 3, 6, 2, 5, 1, 4
		for(int i=0; i<7; ++i){
			gd.addIndex(i*3 % 7);
		}

		// draw star heptagon
		draw::paint(draw::LineLoop, gd);
	}
};


int main (int argc, char ** argv){

	GLV top;
	Scene scene;
	
	scene.stretch(1,1);
	
	top << scene;

	Window win(600, 600, "Example: GraphicsData");
	win.setGLV(top);
	Application::run();
}

