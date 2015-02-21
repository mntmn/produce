/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: Table layout

This demonstrates how to construct a layout using a Table.

*/

#include "example.h"

int main(){

	GLV glv;
	View v1(Rect(80,40));
	View v2(Rect(40,40));
	View v3(Rect(40,80));
	View v4(Rect(40,40));
	View v5(Rect(80,80));
	View v6(Rect(40,40));

	const char * layout = 
		". x - x,"
		"x x x -,"
		"| x | . ";

	Table table(layout);

	glv << (table << v1 << v2 << v3 << v4 << v5 << v6);

	table.arrange();
			
	Window win(300,200, "Example: Table layout", &glv);

	Application::run();
}

