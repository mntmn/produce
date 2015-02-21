#include <ctype.h>		// isgraph
#include <string.h>		// strlen
#include "glv_draw.h"
#include "glv_font.h"

namespace glv{

struct Glyph{
	enum{ Once=(1<<7), Dot1=(1<<5), Dot2=(2<<5), MaskDots=96, MaskSize=31 };
	
	const unsigned char field;
	float x[8], y[8];
	
	static float cap()		{ return 0; }
	static float median()	{ return 3; }
	static float baseline()	{ return 8; }
	static float descent()	{ return 12; }
	static float width()	{ return 8; }
	
	unsigned char dots() const { return (field & MaskDots) >> 5; }
	unsigned char once() const { return (field           ) >> 7; }
	unsigned char size() const { return (field & MaskSize)     ; }
};

// graphical ASCII characters lie in decimal range [33, 126]
static Glyph glyphs[94] = {
	{Glyph::Once | Glyph::Dot1 | 2, {3,4,4}, {7,0,5}},		// !
	{Glyph::Once | 4, {3,3,5,5}, {0,3,0,3}},				// "
	{Glyph::Once | 8, {1,7,1,7,3,3,5,5}, {3,3,5,5,1,7,1,7}},// #
	{0},/* composite of S and | */							// $
	{Glyph::Once | Glyph::Dot2 | 2, {1,6,7,1}, {0,7,0,8}},	// %
	{7, {7,3,4,5,1,4,7}, {8,1,0,1,6,8,5}},					// &
	{Glyph::Once | 2, {5,3}, {0,3}},						// '
	{4, {5,3,3,5}, {0,2,6,8}},								// (
	{4, {3,5,5,3}, {0,2,6,8}},								// )
	{Glyph::Once | 6, {1,7,4,4,7,1}, {2,6,0,8,2,6}},		// *
	{Glyph::Once | 4, {1,7,4,4},{4,4,1,7}},					// +
	{Glyph::Once | 2, {4,2}, {7,10}},						// ,
	{2, {1,7}, {5,5}},										// -	
	{Glyph::Dot1, {3}, {7}},								// .
	{Glyph::Once | 2, {6,2}, {0,8.5}},						// /
	{6, {6,1,1,6,6,1}, {0,0,8,8,0,6}},						// 0
	{3, {2,4,4}, {1,0,8.5}},								// 1
	{6, {1,2,5,6,1,6}, {2,0,0,2,8,8}},						// 2
	{7, {1,6,6,2,6,6,1}, {0,0,2,3,4,7,8}},					// 3
	{4, {6,1,5,5}, {6,6,0,8.5}},							// 4
	{6, {6,1,1,6,6,1}, {0,0,2,3,7,8}},						// 5
	{6, {6,1,1,6,6,1}, {0,0,8,8,4,4}},						// 6
	{3, {1,6,3}, {0,0,8.5}},								// 7
	{7, {6,1,1,6,6,1,1}, {4,4,0,0,8,8,4}},					// 8
	{6, {6,1,1,6,6,1}, {4,4,0,0,8,8}},						// 9
	{Glyph::Dot2, {3,3}, {1,7}},							// :
	{Glyph::Once | Glyph::Dot1 | 2, {3,4,2}, {1,7,10}},		// ;
	{3, {7,1,7}, {0,4,8.5}},								// <
	{Glyph::Once | 4, {1,7,1,7}, {3,3,6,6}},				// =
	{3, {1,7,1}, {0,4,8.5}},								// >
	{Glyph::Dot1 | 5, {3,1,6,6,3,3}, {7,0,0,3,4,5}},		// ?
	{8, {7,3,3,7,7,1,1,7}, {2,3,6,6,0,0,8,8}},				// @
	{Glyph::Once | 6, {1,4,4,7,2,6}, {8.5,0,0,8.5,5,5}},	// A
	{8, {1,1,5,5,1,6,6,1}, {8,0,1,4,4,4,8,8}},				// B
	{4, {6,1,1,6}, {0,0.5,7.5,8}},							// C
	{5, {1,6,6,1,1}, {0,2,7,8,0}},							// D
	{Glyph::Once | 8, {1,1,1,6,1,5,1,6}, {0,8,0,0,4,4,8,8}},// E
	{Glyph::Once | 6, {1,1,1,6,1,5}, {0,8,0,0,4,4}},		// F
	{6, {6,1,1,6,6,4}, {0,1,7,8,5,5}},						// G
	{Glyph::Once | 6, {1,1,1,6,6,6}, {0,8.5,4,4,0,8.5}},	// H
	{Glyph::Once | 6, {2,6,4,4,2,6}, {0,0,0,8,8,8}},		// I
	{4, {1,1,6,6}, {6,8,8,-0.5}},							// J
	{Glyph::Once | 6, {1,1,6,1,1,6}, {-0.5,8.5,-0.5,4,4,8.5}},	// K
	{3, {1,1,6}, {-0.5,8,8}},								// L
	{5, {1,1,4,7,7}, {8.5,0,5,0,8.5}},						// M
	{4, {1,1,6,6}, {8.5,0,8,0}},							// N
	{5, {1,6,6,1,1}, {0,0,8,8,0}},							// O
	{5, {1,1,6,6,1}, {8.5,0,1,4,4}},						// P
	{6, {5,1,1,6,5,7}, {7,7,0,0,7,9}},						// Q
	{6, {1,1,6,6,1,6}, {8.5,0,1,4,4,8}},					// R
	{6, {6,1,1,6,6,1}, {0,0,3,5,8,8}},						// S
	{Glyph::Once | 4, {1,7,4,4}, {0,0,0,8.5}},				// T
	{4, {1,1,6,6}, {0,8,8,0}},								// U
	{3, {1,4,7}, {0,8,0}},									// V
	{5, {1,2,4,6,7}, {0,8,4,8,0}},							// W
	{Glyph::Once | 4, {1,6,6,1}, {0,8,0,8}},				// X
	{Glyph::Once | 6, {1,4,4,7,4,4}, {0,4,4,0,4,8.5}},		// Y
	{4, {1,6,1,6}, {0,0,8,8}},								// Z
	{4, {5,3,3,5}, {0,0,8,8}},								// [
	{Glyph::Once | 2, {2,6}, {0,8.5}},						/* \ */
	{4, {3,5,5,3}, {0,0,8,8}},								// ]
	{3, {1,4,7}, {4,0,4}},									// ^
	{Glyph::Once | 2, {1,7}, {9,9}},						// _
	{Glyph::Once | 2, {3,5}, {0,3}},						// `
	{6, {2,6,6,1,1,6}, {3,3,8,8,5,4}},						// a
	{5, {1,1,6,6,1}, {-0.5,8,8,4,3}},						// b
	{4, {6,1,1,6}, {3,4,8,8}},								// c
	{5, {6,6,1,1,6}, {-0.5,8,8,4,3}},						// d
	{6, {1,6,6,1,1,6}, {6,5,3,3,8,8}},						// e
	{Glyph::Once | 6, {7,3,3,3,5.5,1.5}, {0,0,0,8.5,3,3}},	// f
	{6, {6,1,1,6,6,2}, {8,8,3,3,11,11}},					// g
	{Glyph::Once | 6, {1,1,1,6,6,6}, {-0.5,8.5,3,4,4,8.5}},	// h
	{Glyph::Dot1 | 3, {3,3,4,4}, {0,3,3,8.5}},				// i
	{Glyph::Dot1 | 4, {4,4,5,5,1}, {0,3,3,10,10}},			// j
	{Glyph::Once | 6, {1,1,5,1,1,6}, {-0.5,8.5,3,5,5,8,5}},	// k
	{3, {3,3,5}, {0,8,8}},									// l
	{7, {1,1,3,4,4,6,7}, {8.5,3,3,7,3,3,8.5}},				// m
	{4, {1,1,5,6}, {8.5,3,3,8.5}},							// n
	{5, {1,6,6,1,1}, {3,3,8,8,3}},							// o
	{5, {1,1,6,6,1}, {11,3,3,7,8}},							// p
	{5, {6,1,1,6,6}, {8,7,3,3,11}},							// q
	{4, {1,1,3,6}, {8.5,4,3,3}},							// r
	{6, {6.6,1,1,6,6,0.5}, {3,3,5,6,8,8}},					// s
	{Glyph::Once | 4, {1,6,3,3}, {3,3,-0.5,8.5}},			// t
	{4, {1,1.5,5.5,6}, {2.5,8,8,2.5}},						// u
	{3, {1,4,7}, {2.5,8,2.5}},								// v
	{5, {1,2,4,6,7}, {2.5,8,5,8,2.5}},						// w
	{Glyph::Once | 4, {1,6,1,6}, {8.5,2.5,2.5,8.5}},		// x
	{Glyph::Once | 4, {1,4,6,3}, {2.5,8,2.5,11}},			// y
	{4, {1,6,1,6.5}, {3,3,8,8}},							// z
	{7, {5,3,3,1,3,3,5}, {0,0,3,4,5,8,8}},					// {
	{Glyph::Once | 2, {4,4}, {-1,9}},						// |
	{7, {3,5,5,7,5,5,3}, {0,0,3,4,5,8,8}},					// }
	{4, {1,3,5,7}, {2,0,2,0}}								// ~
};

//static float round(float v, float r, float rInv, float offset){
//	return int(v*rInv) * r + offset;
//}

static bool character(int c, float dx, float dy, float ps){

	// composite character
	if(c == '$'){ character('S',dx,dy,ps); return character('|',dx,dy,ps); }

	if(isgraph(c)){	// is graphical character?

		// align points so that lines start in center of pixels
//		float po = ps/2;
//		float psInv = 1./ps;
//		#define ALIGN(x) round(x,ps,psInv,po)
		#define ALIGN(x) (x)
		#define MUL(x) (x)/ps
	
		Point2 xy[32];
		int ind=-1;
	
		c -= 33;
		
		float * x = glyphs[c].x;
		float * y = glyphs[c].y;
		int n     = glyphs[c].size();
		int dots  = glyphs[c].dots();

		if(dots){
			// 8*2 = 16
			for(int j=0; j<dots; ++j){
				float l = x[j] + dx;	float t = y[j] + dy;
				float r = l + 1;		float b = t + 1;
				
				l = ALIGN(l);
				t = ALIGN(t);
				r = ALIGN(r);
				b = ALIGN(b);
				
				xy[++ind](l,t);
				xy[++ind](r,t); xy[++ind](r,t);
				xy[++ind](r,b); xy[++ind](r,b);
				xy[++ind](l,b); xy[++ind](l,b);
				xy[++ind](l,t);
				
//				vertex(l, t); vertex(r, t); vertex(r, t); vertex(r, b);
//				vertex(r, b); vertex(l, b);	vertex(l, b); vertex(l, t);
			}
			if(n == 0) goto render;
			x += dots; y += dots;
		}

		--n;
		// 16 + 1 = 17
		//vertex(x[0] + dx, y[0] + dy);
		xy[++ind](ALIGN(x[0]+dx), ALIGN(y[0]+dy));
		
		// 17 + 7*2 = 31
		if(glyphs[c].once() == 0){	// line strip
			for(int i=1; i<n; ++i){
				
				float px = ALIGN(x[i]+dx);
				float py = ALIGN(y[i]+dy);
				xy[++ind](px, py);
				xy[++ind](px, py);
				
				//vertex(x[i] + dx, y[i] + dy);
				//vertex(x[i] + dx, y[i] + dy);
			}
		}
		else{		// normal lines
			for(int i=1; i<n; ++i) xy[++ind](ALIGN(x[i]+dx), ALIGN(y[i]+dy)); //vertex(x[i] + dx, y[i] + dy);
		}
		
		// 31 + 1 = 32
		xy[++ind](ALIGN(x[n]+dx), ALIGN(y[n]+dy));
		//vertex(x[n] + dx, y[n] + dy);
		
		render:
    glLineWidth(2);
		draw::paint(draw::Lines, xy, ind+1);
		return true;
	}
	
	return c == ' ';
}


struct TextIterator{

	TextIterator(const Font& f_, const char *& s_): x(0),y(0),l(0),t(0),w(0),h(0), s(s_), f(f_){}

	virtual ~TextIterator(){}

	void run(){
		while((*this)()){}
	}

	bool operator()(){
		if(*s){
			char c = *s++;
			l = x; t = y;
//			w = f.advance(c) * (1+f.letterSpacing()); // varies per character if proportional
//			h = 1 * f.cap() * f.lineSpacing();
//			float tabUnits = f.advance('M') * f.tabSpaces() * (1+f.letterSpacing());
			w = Glyph::width(); // varies per character if proportional
			h = (Glyph::descent() - Glyph::cap());
			float tabUnits = Glyph::width() * f.tabSpaces() * (1+f.letterSpacing());
			float dx = w * (1+f.letterSpacing());
			float dy = h * f.lineSpacing();

			switch(c){
				case '\t': x = (int(x/tabUnits) + 1) * tabUnits; w = x-l; break;
				case '\r':
				case '\n': x = 0; y += dy; break;
				case '\b': x -= w; break;
				default:   if(onPrintable(c)) x += dx;
			}
			return true;
		}
		return false;
	}	

	virtual bool onPrintable(char c){ return true; }

	float x,y;			// raster position
	float l,t,w,h;		// bounding box of current character
	const char *& s;
	const Font& f;
};



Font::Font(unsigned size_)
:	mLetterSpacing(0), mLineSpacing(1), mTabSpaces(4)
{
	size(size_);
}

Font::~Font(){}

void Font::render(const char * v, float x, float y, float z){
	using namespace glv::draw;
	draw::push(ModelView);
//	identity();
//	translate(int(x), int(y), int(z));

	float sx = mScaleX;
	float sy = mScaleY;
	float tx = x;
	float ty = y;
	float tz = z;
	//float sh = -0.5*sy; // TODO: shear needs to be done an a per-line basis
	float sh = 0;
	
	//tx=ty=tz=0;

	float m[16] = {
		sx, 0, 0, 0,
		sh,sy, 0, 0,
		 0, 0,sy, 0,
		tx,ty,tz, 1
	};
	glMultMatrixf(m);

	struct RenderText : public TextIterator{
		RenderText(const Font& f_, const char *& s_, float muly_): TextIterator(f_,s_), muly(muly_){}
		bool onPrintable(char c){
			return character(c, x, y, muly);
		}
		float muly;
	} renderText(*this, v, 1./mScaleY);

	renderText.run();

//	glv::text(v, 0,0, lineSpacing(), tabSpaces(), mScaleY);
	draw::pop(ModelView);
}

Font& Font::letterSpacing(float v){ mLetterSpacing=v; return *this; }

Font& Font::lineSpacing(float v){ mLineSpacing=v; return *this; }

Font& Font::size(unsigned v){
	mSize = v;
	mScaleX = v/Glyph::width();
	mScaleY = v/Glyph::width();
	return *this;
}

Font& Font::tabSpaces(unsigned v){ mTabSpaces=v; return *this; }

float Font::advance(const char *text) const {
	return advance(' ') * strlen(text);
	//return Glyph::width() * mScaleX * strlen(text);
}

float Font::advance(char c) const { return Glyph::width() * mScaleX; }

void Font::getBounds(float& w, float& h, const char * text) const {
	w=h=0;
	if(!text) return;

	TextIterator ti(*this, text);
	while(ti()){
		if(ti.x > w) w = ti.x;
	}

	h = (ti.y + ti.h) * mScaleY;
	//h = ti.y*mScaleY + cap();

	w *= mScaleX;
}

float Font::cap() const { return (Glyph::baseline() - Glyph::cap()) * mScaleY; }
float Font::xheight() const { return (Glyph::baseline() - Glyph::median()) * mScaleY; }
float Font::baseline() const { return Glyph::baseline() * mScaleY; }
float Font::descent() const { return (Glyph::descent() - Glyph::baseline()) * mScaleY; }


} // glv::
