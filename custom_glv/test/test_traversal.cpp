/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "test_glv.h"

using namespace glv;

int main(int argc, char ** argv){
	
	GLV top;
	
	Slider v0, v1;
	Button v00, v01, v10, v11;
	Slider2D v110, v111;
	
	top << (v0 << v00 << v01) << (v1 << v10 << (v11 << v110 << v111));
	
	printf("\n");
	top.printDescendents();
	
	//Window win(800, 600, "GLV Examples", &top);
	//Application::run();
}

