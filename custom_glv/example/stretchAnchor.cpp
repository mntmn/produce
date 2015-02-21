/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: Using stretch and anchor

This demonstrates how the stretch and acnhor factors can be used to control
the resizing behavior of child Views.

*/

#include "example.h"

int main(int argc, char ** argv){

	GLV glv;
	View v1(Rect(100));
	View v2(Rect( 40));
	
	glv.colors().set(StyleColor::WhiteOnBlack);
	glv.cloneStyle();
	v1.colors().set(StyleColor::SmokyGray);
	
	glv << v1;
	v1 << v2;
		
	Window win(200, 200, "", &glv);	

	v1.stretch(1,1);
	v1.anchor(0,0);
	
	v2.anchor(1,1);

	Application::run();
}

