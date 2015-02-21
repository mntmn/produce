/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "test_glv.h"

using namespace glv;

void drawCB(View * v, GLV& g){
	using namespace glv::draw;
	float numIcons = 10;
	float x = 20, y = 20, inc = pix((v->w - 2*x) / numIcons);
	float d = pix((v->w - 2*x - 10 * (numIcons-1)) / numIcons);
	
	color(0.2);
	lineWidth(2);
	rectangle	(x, y, x+d, y+d); x += inc;
	frame		(x, y, x+d, y+d); x += inc;
	triangleR	(x, y, x+d, y+d); x += inc;
	triangleL	(x, y, x+d, y+d); x += inc;
	triangleU	(x, y, x+d, y+d); x += inc;
	triangleD	(x, y, x+d, y+d); x += inc;
	draw::x		(x, y, x+d, y+d); x += inc;
	plus		(x, y, x+d, y+d); x += inc;
	minus		(x, y, x+d, y+d); x += inc;
	check		(x, y, x+d, y+d); x += inc;
	
	x = 20; y += d + 10;

	polygon<3,1,0>(x,y, x+d,y+d); x += inc;
	polygon<4,1,0>(x,y, x+d,y+d); x += inc;
	polygon<5,1,0>(x,y, x+d,y+d); x += inc;
	polygon<6,1,0>(x,y, x+d,y+d); x += inc;
	polygon<7,1,0>(x,y, x+d,y+d); x += inc;
	
	spokes<3,90>(x,y, x+d,y+d); x += inc;
	spokes<4,45>(x,y, x+d,y+d); x += inc;
	spokes<5,0>(x,y, x+d,y+d); x += inc;
	spokes<6,0>(x,y, x+d,y+d); x += inc;
	spokes<7,0>(x,y, x+d,y+d); x += inc;

	x = 20; y += d + 10;
	disc<3>(x, y, x+d, y+d); x += inc;
	disc<4>(x, y, x+d, y+d); x += inc;
	disc<5>(x, y, x+d, y+d); x += inc;
	disc<6>(x, y, x+d, y+d); x += inc;
	disc<7>(x, y, x+d, y+d); x += inc;
	
	x = 20; y += d + 10;
	//linesH(x,y,d,d, 5);				x += inc;
	//linePattern(x,y,d,d, 9, "1 1");	x += inc;
	
	
	x = 20; y += d + 10;
	GraphicsData gd;
	for(int i=2; i<5; ++i){ for(int j=2; j<5; ++j){ grid(gd, x,y,d,d,i,j); x+=inc; }}
	
	x = 20; y += d + 10; y = (int)y;
	color(0); lineWidth(1);
	text("Amazingly few discotheques provide jukeboxes.", x, y);
	
	x = 20; y += d + 10;
	y = (int)y;

	text(
		"!\"#$%&'()*+,-./0\n"
		"123456789:;<=>?@\n"
		"ABCDEFGHIJKLMNOP\n"
		"QRSTUVWXYZ[\\]^_`\n"
		"abcdefghijklmnop\n"
		"qrstuvwxyz{|}~",
		x,y
	);

//	for(int i=0; i<(127 - '!'); ++i){
//		float dx = (i%16) * 8;
//		float dy = (i/16) * 16;
//		character(i + '!', x + dx, y + dy);
//	}

//	{	Icon icon = Rectangle();
//		icon.draw(0,0,100,100);
//	}
//	{	Icon& icon = *new Rectangle();
//		icon.draw(100,0,200,100);
//	}
//	{	Icon * icon = new Rectangle;
//		icon->draw(200,0,300,100);
//	}
}


int main (int argc, char ** argv){
	GLV top(drawCB);
	top.colors().back.set(1);
	
	Window win(400, 400, "GLV", &top);
	Application::run();
}

