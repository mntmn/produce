#ifndef INC_GLV_LAYOUT_H
#define INC_GLV_LAYOUT_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"


namespace glv{


class Placer;


/// View used for forming a group of child Views.

/// By default, a Group will not be drawn to screen nor interact with any 
/// user input. It is meant to simply act as an invisible frame for child Views.
struct Group : public View{
	Group(const Rect& r=Rect(0))
	:	View(r)
	{	disable(DrawBack | DrawBorder | CropChildren | FocusHighlight | Controllable); }
	
	virtual const char * className() const { return "Group"; }
};





/// View placer

/// A Placer is used for aligning a series of views. It can position views
/// according to an absolute increment and/or increment relative to the views'
/// geometric extent.
class Placer{
public:

	Placer(space_t absX=0, space_t absY=0);

	Placer(View& parent, Direction dir, Place::t align=Place::TL, space_t x=0, space_t y=0, space_t pad=4);

	/// Set a view's position
	Placer& operator<< (View& v);
	Placer& operator<< (View* v);

	/// Set the absolute increment amounts
	Placer& abs(space_t vx, space_t vy);
	
	/// Set view alignment location
	Placer& align(space_t vx, space_t vy);
	
	/// Set view alignment place
	Placer& align(Place::t p);
	
	/// Set anchor place of Views
	Placer& anchor(Place::t p);
	
	/// Reassign the placement flow direction
	Placer& flow(Direction d);
	
	/// Set the placement flow along with absolute and relative incrementing values
	Placer& flow(Direction d, space_t abs, space_t rel=1);
	
	/// Set the current position
	Placer& pos(space_t vx, space_t vy);
	
	/// Set the current x-position
	Placer& posX(space_t v);
	
	/// Set the current y-position
	Placer& posY(space_t v);
	
	/// Set the relative increment factors
	Placer& rel(space_t vx, space_t vy);

protected:
	space_t x, y;	// trajectory point
	space_t rx, ry;	// relative increment factors
	space_t ax, ay;	// absolute increment amounts
	space_t fw, fh; // view alignment fraction [0, 1]
	Place::t mAnchor;
	View * parent;
};



/// A Group that automatically fits itself to child Views
struct Box : public Group{
	Box(Direction dir=Direction::E, Place::t align=Place::TL, space_t pad=4)
	:	p(*this, dir, align, 0,0, pad){}

	Box(View& v1, View& v2, Direction dir=Direction::E, Place::t align=Place::TL, space_t pad=4)
	:	p(*this, dir, align, 0,0, pad){
		p << v1 << v2;
		fit();
	}

	Box(View& v1, View& v2, View& v3, Direction dir=Direction::E, Place::t align=Place::TL, space_t pad=4)
	:	p(*this, dir, align, 0,0, pad){
		p << v1 << v2 << v3;
		fit();
	}

	Box(View& v1, View& v2, View& v3, View& v4, Direction dir=Direction::E, Place::t align=Place::TL, space_t pad=4)
	:	p(*this, dir, align, 0,0, pad){
		p << v1 << v2 << v3 << v4;
		fit();
	}
	
	Box& operator<< (View* v){ p<<v; fit(); return *this; }
	Box& operator<< (View& v){ p<<v; fit(); return *this; }

	virtual const char * className() const { return "Box"; }
private:
	Placer p;
};



// Base interface class for View auto-layout functors
struct LayoutFunc{

	/// @param[in] w		Width of rect element
	/// @param[in] h		Height of rect element
	/// @param[in] bounds	Layout bounding rect
	LayoutFunc(space_t w, space_t h, const Rect& bounds);
	virtual ~LayoutFunc();
	
	/// Returns next rect element of layout function
	virtual Rect& operator()() = 0;

	void layoutChildren(View& v);
	
	Rect elem;		///< The current rect element
	Rect bounds;		///< The layout bounding rect
	View * parent;
};



// Grid auto-layout functor

// Generates evenly distributed rects within a bounding region.
//
struct LayoutGrid : public LayoutFunc{

	/// @param[in] w		Width of rect element
	/// @param[in] h		Height of rect element
	/// @param[in] bounds	Layout bounding rect
	/// @param[in] numV		Number of vertical divisions
	/// @param[in] numH		Number of horizontal divisions
	LayoutGrid(space_t w, space_t h, const Rect& bounds, int numV, int numH);
	virtual ~LayoutGrid();
	
	
	/// Constructor for auto-sized elements with uniform padding 
	
	/// @param[in] pad		Padding between elements
	/// @param[in] bounds	Layout bounding rect
	/// @param[in] numV		Number of vertical divisions
	/// @param[in] numH		Number of horizontal divisions	
	LayoutGrid(space_t pad, const Rect& bounds, int numV, int numH);
	
	
	LayoutGrid(View& parent, int numH, int numV, space_t pad);
	
	/// Generate next layout rect going left-to-right, then top-to-bottom.
	virtual Rect& operator()();


	LayoutGrid& operator<< (View& v){
		if(parent) (*parent) << v;
		v.set((*this)());
		return *this;
	}
	
protected:
	int numV, numH, cntV, cntH;
};




/// Table class for doing rectangular layouts.
class Table : public Group{
public:

	/// @param[in] arrangement	Arrangement string (see arrangement())
	/// @param[in] padX			Padding in x-direction between cells
	/// @param[in] padY			Padding in y-direction between cells
	/// @param[in] r			Geometry
	Table(const char * arrangement, space_t padX=4, space_t padY=4, const Rect& r=Rect(0));

	/// Arrange child Views according to cell arrangement specification.
	
	/// If there are more children than the arrangement accounts for, then the
	/// arrangement string will copied the appropriate number of times.
	Table& arrange();
	
	/// Set table cell arrangement.
	
	/// The arrangement string specifies where Views will lie in the table, the
	/// alignment within each cell, and how cells should span multiple columns
	/// and/or rows. The string cell alignment characters are: \n
	///	\verbatim
	///	p ^ q     top-left     top-center     top-right
	///	< x >     center-left  center-center  center-right
	///	b v d     bottom-left  bottom-center  bottom-right \endverbatim
	///	
	/// The string layout characters are: \n
	/// \verbatim
	/// .         empty region
	/// -         span first leftward neighboring element rightward
	/// |         span first upward neighboring element downward
	/// ,         end of row \endverbatim
	///
	/// Views added as children will fill in the cells with an alignment 
	/// specification starting at the top-left cell and then proceeding 
	/// column-by-column, row-by-row.
	///
	/// The alignment code will set the View's anchor factors.
	Table& arrangement(const char * v);

	/// Get arrangement string
	const std::string& arrangement() const { return mAlign; }

	space_t padX() const { return mPad1; }
	space_t padY() const { return mPad2; }

	virtual const char * className() const { return "Table"; }
	virtual void onDraw(GLV& g);

protected:

	struct Cell{
		Cell(int posX, int posY, int spanX, int spanY, char code_, View * view_=0)
		:	view(view_), x(posX), y(posY), w(spanX), h(spanY), code(code_){}
	
		View * view; int x,y,w,h; char code;
	};

	int mSize1, mSize2;
	std::vector<Cell> mCells;
	std::string mAlign;
	space_t mPad1, mPad2;
	std::vector<space_t> mColWs, mRowHs;
	
	bool isAlignCode(char c){
		return	c=='<' || c=='>' || c=='^' || c=='v' || c=='x' || 
				c=='p' || c=='q' || c=='b' || c=='d';
	}
	
	int size() const { return mSize1*mSize2; }
	
	space_t sumSpan(const space_t * src, int end, int begin=0){
		space_t r=0;
		for(int i=begin; i<end; ++i) r += src[i];
		return r;
	}
};



} // glv::

#endif
