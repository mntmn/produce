/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <ctype.h>
#include "glv_draw.h"
#include "glv_font.h"

namespace glv{
namespace draw{

int printError(const char * pre, bool verbose, FILE * out){
	GLenum err = glGetError();
	#define CS(v, desc) case GL_##v: printf("%s%s%s\n", pre, #v, verbose?": "desc :""); break;	
	switch(err){
		CS(INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
		CS(INVALID_VALUE, "A numeric argument is out of range.")
		CS(INVALID_OPERATION, "The specified operation is not allowed in the current state.")
		CS(STACK_OVERFLOW, "This command would cause a stack overflow.")
		CS(STACK_UNDERFLOW, "This command would cause a stack underflow.")
		CS(OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
//		CS(TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
		default:;
	}
	#undef CS
	return int(err);
}


void fog(float end, float start, const Color& c){
	glFogi(GL_FOG_MODE, GL_LINEAR); 
	glFogf(GL_FOG_START, start); glFogf(GL_FOG_END, end);
	float fogColor[4] = {c.r, c.g, c.b, c.a};
	glFogfv(GL_FOG_COLOR, fogColor);
}


void genEllipse(
	Point2 * pts, int num, int stride,
	double angle01, double loops,
	float l, float t, float r, float b
){
	const double theta = loops*C_2PI/num;
	double px=cos(angle01*C_2PI);
	double py=sin(angle01*C_2PI);
	double rx=cos( theta);
	double ry=sin(-theta); // negative to ensure CCW winding for front facing polygon
	double mx=0.5*(l+r), my=0.5*(t+b), sx=(r-l)*0.5, sy=(b-t)*0.5;

	for(int i=0; i<num; ++i){
		pts[i*stride](mx+px*sx, my+py*sy);
		double tx=px;
		px = px*rx - py*ry;
		py = tx*ry + py*rx;
	}
}

void grid(GraphicsData& g, float l, float t, float w, float h, float divx, float divy, bool incEnds){
	g.reset();
	double inc, r=l+w, b=t+h;

	if(divy > 0 && h>0){
		inc = (double)h/(double)divy;
		double i = incEnds ? t-0.0001 : t-0.0001+inc;
		double e = incEnds ? b : b-inc;
		for(; i<e; i+=inc) g.addVertex2(l,i, r,i);
	}

	if(divx > 0 && w>0){
		inc = (double)w/(double)divx;
		double i = incEnds ? l-0.0001 : l-0.0001+inc;
		double e = incEnds ? r : r-inc;
		for(; i<e; i+=inc) g.addVertex2(i,t, i,b);
	}
	
	paint(Lines, g);
}

//// C99 variable length array portability problem
//
//void linesH(float l, float t, float w, float h, int n){
//	double pos = 0;
//	double inc = (double)h/(double)(n-1);
//		
//	push();
//	translate(l,t);
//
//	Point2 pts[n*2];
//
//	for(int i=0; i<n*2; i+=2){
//		pts[i+0](0, pos);
//		pts[i+1](w, pos);
//		pos+=inc;
//	}
//
//	paint(Lines, pts, GLV_ARRAY_SIZE(pts));
//	pop();
//}
//
//
//void linePattern(float l, float t, float w, float h, int n, const char * pat="/");
//void linePattern(float l, float t, float w, float h, int n, const char * pat){
//	const char * p = pat;
//	double pos = 0;
//	double inc = (double)w/(double)(n-1);
//	int nDraw=0;
//	Point2 pts[n*2];
//	
//	push();
//	translate(l,t);
//
//	for(int i=0; i<n; ++i){
//		if(!*p) p = pat;
//		if(*p++ != ' '){
//			pts[nDraw+0](pos, 0);
//			pts[nDraw+1](pos, h);
//			nDraw += 2;
//		}
//		pos+=inc;
//	}
//	
//	paint(Lines, pts, nDraw);
//	pop();
//}


void paint(int prim, const GraphicsData& b){
	int Nc = b.colors().size();
	int Nv2= b.vertices2().size();
	int Nv3= b.vertices3().size();
	int Ni = b.indices().size();

	bool Ec = Nc && (Nc >= Nv2 || Nc >= Nv3);

	if(Ec){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &b.colors()[0]);
	}
	if(Nv3)	glVertexPointer(3, GL_FLOAT, 0, &b.vertices3()[0]);
	else	glVertexPointer(2, GL_FLOAT, 0, &b.vertices2()[0]);
	
	if(Ni)	glDrawElements(prim, b.indices().size(), GL_UNSIGNED_INT, &b.indices()[0]);
	else	glDrawArrays(prim, 0, Nv3 ? Nv3 : Nv2);

	if(Ec)	glDisableClientState(GL_COLOR_ARRAY);
}


// we need to push and pop matrices and viewport bit
void push2D(float w, float h){
							// to ensure polygon edges blend properly
	disable << DepthTest << PolygonSmooth;
	enable << Blend << LineSmooth;
	blendTrans();

	push(Projection); pushAttrib(ViewPortBit); identity();
		viewport(0, 0, w, h);
		ortho(0, w, h, 0);		// flat 2D world dimension L,R,B,T
	
	push(ModelView); identity();
	
	// render all primitives at integer positions, ref: OpenGL Redbook
	//translate(0.375, 0.375);
}


void pop2D(){
	popAttrib();					// for popping GL_VIEWPORT_BIT
	pop(Projection);
	pop(ModelView);
}


void push3D(float w, float h, float near, float far, float fovy){
	pushAttrib(ColorBufferBit | DepthBufferBit | EnableBit | ViewPortBit);
	enable(DepthTest);
	
	push(Projection); identity();
		gluPerspective(fovy, w/(GLfloat)h, near, far);
	
	push(ModelView); identity();
		translate(0, 0, -2.42);
}


void pop3D(){
	popAttrib();					// for popping GL_DEPTH_BUFFER_BIT
	pop(Projection);
	pop(ModelView);
}


void text(const char * s, float l, float t, unsigned fontSize, float lineSpacing, unsigned tabSpaces){
	Font f;
	f.size(fontSize);
	f.lineSpacing(lineSpacing);
	f.tabSpaces(tabSpaces);
	f.render(s, l+0.375, t+0.375, 0);
	//f.render(s, int(l)+0.5, int(t)+0.5,0);
}

} // draw::
} // glv::

