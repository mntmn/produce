#ifndef INC_GLV_SLIDERS_H
#define INC_GLV_SLIDERS_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"
#include "glv_widget.h"
#include <vector>

namespace glv {


/// Base class for sliders that control a fixed-size vector of values

/// The foreground color determines the color of the slider.
///
template <int Dim>
class SliderVector : public Widget {
public:

	/// Constructor
	SliderVector(const Rect& r);

	/// Get value at selected index
	double getValue() const { return Widget::getValue<double>(); }
	
	/// Get value at 1D index
	double getValue(int i) const { return Widget::getValue<double>(i); }

protected:
	double mAcc[Dim];

	// Add amount to value at 1D index and notify observers
	SliderVector& valueAdd(double val, int idx);
	SliderVector& valueAdd(double val, int idx, double min, double max);

	void clipAccs(){ for(int i=0; i<Dim; ++i) mAcc[i]=glv::clip(mAcc[i],mMax,mMin); }
	virtual bool onAssignData(Data& d, int ind1, int ind2);
};



/// A 2-D slider
class Slider2D : public SliderVector<2>{
public:

	/// @param[in] r			geometry
	/// @param[in] valX			initial value along x
	/// @param[in] valY			initial value along y
	/// @param[in] knobSize		size of slider knob in pixels
	Slider2D(const Rect& r=glv::Rect(100), double valX=0, double valY=0, space_t knobSize=12);

	space_t knobSize;	///< Size of slider knob
	
	virtual const char * className() const { return "Slider2D"; }
	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);
	
	static void drawKnob(const Slider2D& s);	
};



/// A slider with an adjustable interval
class SliderRange : public SliderVector<2>{
public:

	/// @param[in] r			geometry
	/// @param[in] val1			initial value on left or top
	/// @param[in] val2			initial value on right or bottom
	SliderRange(const Rect& r=glv::Rect(100,20), double val1=0.25, double val2=0.5);
	
	SliderRange& center(double v);							///< Set center of interval
	SliderRange& centerRange(double center, double range);	///< Set center and range of interval
	SliderRange& extrema(double min, double max);			///< Set extrema of interval
	
	/// Sets how much the slider should move when an empty region is clicked.
	
	/// The slider is constrained not jump past the click point.
	///
	SliderRange& jump(double v);
	SliderRange& range(double v);	///< Set range
	
	double center() const;			///< Get center of interval
	double jump() const;			///< Get click jump amount
	double range() const;			///< Get distance of interval
	
	virtual const char * className() const { return "SliderRange"; }
	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);
	
private:
	int mDragMode;	// 0,1,2,3: off, lower, upper, center
	double mJump;
};



/// A multi-parameter slider grid

/// The cells of the grid represent all the pair permutations of parameters.
/// The parameter values run from 0 to N, going left-to-right along the x-axis
/// and bottom-to-top along th y-axis. Cells along the diagonal control the
/// parameters individually.
template <int Dim>
class SliderGrid : public SliderVector<Dim>{
public:

	/// @param[in] r			geometry
	/// @param[in] knobSize		size of slider knob in pixels
	SliderGrid(const Rect& r=glv::Rect(100), space_t knobSize=4);

	space_t knobSize; ///< Knob size

	virtual const char * className() const { return "SliderGrid"; }
	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);

	using SliderVector<Dim>::colors;
	using SliderVector<Dim>::w;
	using SliderVector<Dim>::h;
	using SliderVector<Dim>::getValue;
	using SliderVector<Dim>::clipAccs;
	using SliderVector<Dim>::diam;
	using SliderVector<Dim>::toInterval;
protected:
	int cx, cy;
};



/// A grid of 1-D sliders
class Sliders: public Widget{
public:

	/// @param[in] r			geometry
	/// @param[in] nx			number along x (ignored by fixed size value types)
	/// @param[in] ny			number along y (ignored by fixed size value types)
	/// @param[in] dragSelect	whether new sliders are selected while dragging
	Sliders(const Rect& r, int nx, int ny, bool dragSelect=false);

	virtual ~Sliders(){}

	/// Get value at selected index
	double getValue() const { return Widget::getValue<double>(); }
	
	/// Get value at 1D index
	double getValue(int i) const { return Widget::getValue<double>(i); }
	
	/// Get value at 2D index
	double getValue(int i1, int i2) const { return Widget::getValue<double>(i1,i2); }

	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);
	virtual const char * className() const { return "Sliders"; }
	
protected:
	double mAcc;

	void selectSlider(GLV& g, bool click);
	bool isVertical() const { return dy() > dx(); }
};


class Slider: public Sliders{
public:
	/// @param[in] r			geometry
	/// @param[in] v			initial value between 0 and 1
	Slider(const Rect& r=Rect(100, 20), double v=0)
	:	Sliders(r, 1,1)
	{	setValue(v); }

	virtual const char * className() const { return "Slider"; }
};




//there must be at least 3 points
class FunctionGraph : public View{
public:
	class Curve : public Array<float>{
	public:
		Curve(int res)
		: Array<float>(res)
		{}
		
		~Curve() {}
	};
	
	struct Knot {
	public:
		Knot() : x(0), y(0)
		{}
		
		~Knot() {}
		
		Knot(const Knot& k) : x(k.x), y(k.y)
		{}
		
		float x;
		float y;
	};

public:
	FunctionGraph(const Rect& r, int nKnots, int res);
	virtual ~FunctionGraph();
	
	virtual const char * className() const { return "FunctionGraph"; }
	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);

	void eval(int n, float *vals);
	void tension(float v) {mTension = v; calcCurves();}
	float tension() {return mTension;}

protected:
	void calcCurves();
	
	/*
	Tension: 1 is high, 0 normal, -1 is low
	Bias: 0 is even,
	positive is towards first segment,
	negative towards the other
	*/
	inline float HermiteInterpolate( float y0, float y1,
						float y2, float y3,
						float mu,
						float tension,
						float bias)
	{
		float m0,m1,mu2,mu3;
		float a0,a1,a2,a3;

		mu2 = mu * mu;
		mu3 = mu2 * mu;
		m0  = (y1-y0)*(1+bias)*(1-tension)/2;
		m0 += (y2-y1)*(1-bias)*(1-tension)/2;
		m1  = (y2-y1)*(1+bias)*(1-tension)/2;
		m1 += (y3-y2)*(1-bias)*(1-tension)/2;
		a0 =  2*mu3 - 3*mu2 + 1;
		a1 =    mu3 - 2*mu2 + mu;
		a2 =    mu3 -   mu2;
		a3 = -2*mu3 + 3*mu2;
		
		return (a0*y1+a1*m0+a2*m1+a3*y2);
	}
	
	//returns -1 on failure (> mKnobSize pixels away)
	int knotHitTest(space_t x, space_t y);

	float mTension;
	int mKnobSize;
	int mCurrentKnot;
	int mNKnots;
	Knot *mKnots;
	std::vector<Curve *> mCurves;
};




// Implementation ______________________________________________________________

// SliderVector

#define TEM template <int Dim>

TEM SliderVector<Dim>::SliderVector(const Rect& r)
:	Widget(r, 0, false, false, false)
{
	data().resize(Data::DOUBLE, Dim,1);
	memset(mAcc, 0, sizeof(double) * Dim);
}

TEM inline bool SliderVector<Dim>::onAssignData(Data& d, int ind1, int ind2){
	Data t(mAcc, Dim);
	int idx = data().indexFlat(ind1,ind2);
	t.slice(idx, t.size()-idx).assign(d);
	return Widget::onAssignData(d,ind1,ind2);
}

TEM inline SliderVector<Dim>& SliderVector<Dim>::valueAdd(double add, int dim){
	return valueAdd(add,dim,mMin,mMax);
}

TEM inline SliderVector<Dim>& SliderVector<Dim>::valueAdd(double add, int dim, double min, double max){	
	if(validIndex(dim)){
		mAcc[dim] += add;
		setValue(mAcc[dim], dim, min, max);
		//setValue(glv::clip(mAcc[dim], max, min), dim);
	}
	return *this;
}



TEM SliderGrid<Dim>::SliderGrid(const Rect& r, space_t knobSize)
:	SliderVector<Dim>(r), knobSize(knobSize), cx(0), cy(0)
{
	//this->cropSelf = false;
	this->disable(CropSelf);
}

TEM void SliderGrid<Dim>::onDraw(GLV& g){
	using namespace glv::draw;

	float rDim = 1./Dim;
	color(colors().fore);
	lineWidth(1);
	frame(cx * w * rDim, (Dim-cy-1) * h * rDim, (cx+1) * w * rDim, (Dim-cy) * h * rDim);
	draw::enable(LineStipple);
	lineStipple(1, 0xAAAA);
	grid(g.graphicsData(), 0,0,w,h,Dim,Dim, false);
	shape(Lines,0,h,w,0);
	draw::disable(LineStipple);

	/*
	float dx = rDim*w;
	float dy = rDim*h;

	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);
	for(int i=0; i<Dim; ++i){
		float x = i*dx;
		HSV xc(i/(float)Dim, 0.8, 0.2);
		
		for(int j=0; j<Dim; ++j){
			float y = (Dim-j-1)*dy;
			
			HSV yc(j/(float)Dim, 0.8, 0.2);
			HSV mid( (xc.h+yc.h)*0.5, 0.8, 0.2);
			
			color(Color(yc));
			glVertex3f(x, y, 0);
			
			glColor4f(1, 1, 1, 1.);
			glVertex3f(x, y+dy, 0);
			
			color(Color(xc));
			glVertex3f(x+dx, y+dy, 0);
			
			color(Color(mid));
			glVertex3f(x+dx, y, 0);
		}
	}
	glEnd();

	glColor4f(1, 1, 1, 1.);*/
	pointSize(knobSize);

	Point2 pts[Dim*Dim];
	for(int i=0; i<Dim; ++i){
		float f = (i+to01(getValue(i))) * rDim;
		float x = f*w;
		
		for(int j=0; j<Dim; ++j){
			pts[i*Dim+j](x, (1.-(j+to01(getValue(j))) * rDim) * h);
		}
	}
	paint(Points, pts, GLV_ARRAY_SIZE(pts));

}

TEM bool SliderGrid<Dim>::onEvent(Event::t e, GLV& g){

	switch(e){
	case Event::MouseDrag:
					valueAdd( g.mouse().dx()/w * diam() * Dim * g.mouse().sens(), cx);
		if(cx!=cy)	valueAdd(-g.mouse().dy()/h * diam() * Dim * g.mouse().sens(), cy);
		break;
		
	case Event::MouseDown:
		cx = int(g.mouse().xRel()/w * Dim);
		cy = int(g.mouse().yRel()/h * Dim);
		cy = (Dim-1) - cy;
		cx = glv::clip(cx, Dim-1);
		cy = glv::clip(cy, Dim-1);

		if(g.mouse().left() && !g.mouse().right()){
			float cw = w/Dim;
			float ch = h/Dim;
						setValue(toInterval(    (g.mouse().xRel()/cw - cx)), cx);
			if(cx!=cy)	setValue(toInterval(1.f-(g.mouse().yRel()/ch - ((Dim-1)-cy))), cy);
		}
		break;
		
	case Event::MouseUp: clipAccs(); break;
	default: break;
	}
	return false;
}

#undef TEM

} // glv::

#endif
