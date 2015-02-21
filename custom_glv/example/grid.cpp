/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */
/*
Example: 
*/

#include "example.h"

int main(){

	// Create the Views
	GLV top;
	
	top.colors().set(StyleColor::WhiteOnBlack);
	//top.colors().set(StyleColor::SmokeyGray);

	//Grid v1(Rect(700,500));
	//v1.pos(Place::CC).anchor(0.5,0.5);
	
	Grid v1(Rect(0,0));
	v1.stretch(1,1);
	//v1.disable(CropSelf);
	top << v1;
	
	Window win(800,800, "Grid", &top);

	Application::run();
}

