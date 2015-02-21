/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: Views

This demonstrates how the hierarchical structure of Views works and some of the
basic View properties.

*/

#include "test_glv.h"

using namespace glv;

int main(){

	// Create the Views
	GLV glv;

	Label v1("\
		zero	one		two		three\n\
zero	0		0		0		0\n\
one		0		1		2		3\n\
two		0		2		4		6\n\
three	0		3		6		9"
	);

	Label v2("letter spacing");

	v1.size(16);
	v2.font().letterSpacing(2);
	v2.size(16);

	View* views[] = {&v1, &v2};
	for(unsigned i=0; i<sizeof(views)/sizeof(views[0]); ++i){
		views[i]->addCallback(Event::MouseDrag, Behavior::mouseMove);
		views[i]->enable(KeepWithinParent);
		views[i]->enable(DrawBack);
		glv << views[i];
		if(i>0) views[i]->posUnder(*views[i-1], 2);
	}
	
	// Set color styles
	glv.cloneStyle().colors().set(StyleColor::WhiteOnBlack);
	views[0]->colors().set(Color(0.2,0.4,1,0.8), 0.7);

	Window win(800,600, "Views", &glv);

	//win.fit();

	Application::run();

}

