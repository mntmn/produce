/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: draw::text

This demonstrates how to quickly render text using the draw::text function.

*/

#include "example.h"

struct MyGLV : public GLV{
	void onDraw(GLV& g){
		using namespace glv::draw;

		color(0);
		lineWidth(1);

		// example sentence using all letters
		text("Amazingly few discotheques provide jukeboxes.", 10, 10);

		// the printable ASCII set
		text(
			"!\"#$%&'()*+,-./0\n"
			"123456789:;<=>?@\n"
			"ABCDEFGHIJKLMNOP\n"
			"QRSTUVWXYZ[\\]^_`\n"
			"abcdefghijklmnop\n"
			"qrstuvwxyz{|}~",
			10, 30
		);

		// tabbing is supported
		text("\
		zero	one		two		three\n\
zero	0		0		0		0\n\
one		0		1		2		3\n\
two		0		2		4		6\n\
three	0		3		6		9",
			200, 30
		);

		text("20 pixels", 10, 120, 20);
		text("12 pixels", 10, 150, 12);

		lineWidth(2);
		text("bold face", 10, 170);

		lineWidth(1);
		color(HSV(0./3));
		text("red", 200, 170);

		color(HSV(1./3));
		text("green", 200, 190);

		color(HSV(2./3));
		text("blue", 200, 210);
	}
};

int main(){
	MyGLV glv;			
	Window win(600,300, "Example: draw::text", &glv);
	Application::run();
}

