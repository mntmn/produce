/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: Attaching variables to numerical widgets

A single variable is attached to two different sliders. This demonstrates how
a slider can automatically update a program variable and how a slider is
automatically synchronized to changes in its attached variable. When one slider
is moved it sets the value of the variable which then causes the other slider
to synchronize to the changed variable. If working correctly, moving one slider
will move the other slider.
*/

#include "example.h"

int main(int argc, char ** argv){

	GLV glv;
	Slider sl1(Rect(10,10,200,20)), sl2(Rect(10,40,200,20));
	
	float var = 0;
	
	sl1.attachVariable(var);
	sl2.attachVariable(var);
	
	glv << sl1 << sl2;

	Window win(220, 70, "", &glv);
	Application::run();
}
