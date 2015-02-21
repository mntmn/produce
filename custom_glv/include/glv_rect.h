#ifndef INC_GLV_RECT_H
#define INC_GLV_RECT_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <stdio.h>

namespace glv {

/// Rectangle shape.

/// Knows how to calculate useful things such as centre, intersection areas
/// with other rects, etc.
template <class T>
class TRect {
public:
	union{
		struct{
			T l;					///< Left edge position
			T t;					///< Top edge position
			T w;					///< Width
			T h;					///< Height
		};
		struct{
			T positionVector[2];	///< Top-left position vector
			T extentVector[2];		///< Width-height extent vector
		};
	};

	TRect();

	/// Construct from square extent with position set to (0,0).

	/// @param[in] extent	initial width and height
	///	
	TRect(T extent);

	/// Construct from width and height with position set to (0,0).

	/// @param[in] width	initial width
	/// @param[in] height	initial height	
	TRect(T width, T height);

	/// @param[in] left		initial left edge position
	/// @param[in] top		initial top edge position
	/// @param[in] width	initial width
	/// @param[in] height	initial height
	TRect(T left, T top, T width, T height);
	
	/// @param[in]	other	other rectangle to set geometry from
	TRect(const TRect& other);
	
	virtual ~TRect(){}

	const TRect& operator= (const TRect& r);

	void copyUnder(const TRect<T>& from, T by=0);
	void extent(T v);				///< Set extent to be square.
	void extent(T w, T h);			///< Set extent.
	void fitSquare(T v);			///< Set extent to fit in square.
	void fixNegativeExtent();		///< Fixes negative width or height to be positive.
	void insetFrom(const TRect<T>& from, T inset);	///< Make TRect relative inset from 'from'
	void pos(T left, T top);		///< Set left-top position.
	void posAdd(T x, T y);			///< Translate by [x, y] units.
	void posUnder(const TRect<T>& r, T by=0);
	
	/// Position myself relative to another rect.
	
	/// rxf and ryf determine the fractional position along the source r. xf and xy
	/// determine the fractional position along this rect. x and y are absolute
	/// offsets.
	void posRelTo(const TRect<T>& r, float rxf, float ryf, float xf, float yf, float x=0, float y=0);
	
	void posRightOf(const TRect<T>& r, T by=0);
	void resizeLeftTo(T v);			///< Resize by moving left edge to value.
	void resizeTopTo(T v);			///< Resize by moving top edge to value.
	void resizeRightTo(T v);		///< Resize by moving right edge to value.
	void resizeBottomTo(T v);		///< Resize by moving bottom edge to value.
	void resizeEdgesBy(T v);		///< Resize by moving edges by value.
	void set(T left, T top, T width, T height);	///< Set components.
	void set(const TRect<T>& r);		///< Set components from other TRect.
	void transpose();				///< Swaps width and height.
	
	void left(T v);					///< Set left position.
	void top(T v);					///< Set top position.
	void width(T v);				///< Set width.
	void height(T v);				///< Set height.
	void bottom(T v);				///< Set bottom edge preserving height.
	void right(T v);				///< Set right edge preserving width.
	
	// Accessors
	const TRect& rect() const;		///< Returns self
	
	T left() const { return l; }	///< Get left edge position
	T top() const { return t; }		///< Get top edge position
	T width() const { return w; }	///< Get width
	T height() const { return h; }	///< Get height
	T right() const;				///< Get right edge position
	T bottom() const;				///< Get bottom edge position
	
	T area() const;					///< Returns area (w x h).
	void center(T & x, T & y) const;///< Gets center point
	T centerX() const;				///< Returns center along x axis.
	T centerY() const;				///< Returns center along y axis.
	
	bool containsPoint(T x, T y) const;	///< Returns whether the point is inside the TRect.
	
	TRect<T> extent() const;
	
	/// Calculates the intersection of the two Rects.
	void intersection(const TRect<T>& check, TRect<T>& intersection) const;
	
	bool intersects(const TRect<T>& rect) const;///< Returns whether the two Rects intersect.
	bool isVertical() const;					///< Returns true if h > w, false otherwise.
	void unionOf(const TRect<T>& check, TRect<T>& result) const;
	bool withinXBounds(T xmin, T xmax) const;	///< Returns whether the TRect is within the x range.
	bool withinYBounds(T ymin, T ymax) const;	///< Returns whether the TRect is within the y range.

	/// Called when the width or height change.  Changes in extent are passed in.
	virtual void onResize(T dx, T dy){}
	
	void print(FILE * fp=stdout);	///< write about TRect to a file
	
private:
	void onResizeProxy(T dx, T dy){	// calls onResize if at least 1 dimension has changed
		if(dx!=T(0) || dy!=T(0)) onResize(dx,dy);
	}
};




// Implementation ______________________________________________________________

#define TEM template <class T>

TEM TRect<T>::TRect(){ set((T)0, (T)0, (T)40, (T)40); }
TEM TRect<T>::TRect(T v){ set(0, 0, v, v); }
TEM TRect<T>::TRect(T iw, T ih){ set(0, 0, iw, ih); }
TEM TRect<T>::TRect(T il, T it, T iw, T ih){ set(il, it, iw, ih); }
TEM TRect<T>::TRect(const TRect& v){ *this = v; }

TEM const TRect<T>& TRect<T>::operator= (const TRect<T>& r){
	set(r.l, r.t, r.w, r.h);
	return *this;
}

TEM void TRect<T>::copyUnder(const TRect<T> & r, T by){
	pos(r.l, r.bottom() + by);
	extent(r.w, r.h);
}

TEM inline void TRect<T>::extent(T v){ extent(v, v); }

TEM inline void TRect<T>::extent(T wi, T he){
	T dx=wi-w, dy=he-h;
	w=wi; h=he;
	onResizeProxy(dx, dy);
}

TEM inline void TRect<T>::fitSquare(T v){ w > h ? extent(v, v * h/w) : extent(v * w/h, v); }

TEM inline void TRect<T>::fixNegativeExtent(){
	if(w < (T)0){ w = -w; l -= w; }
	if(h < (T)0){ h = -h; t -= h; }
}

TEM inline void TRect<T>::pos(T le, T to){ l = le; t = to; }
TEM inline void TRect<T>::posAdd(T x, T y){ l += x; t += y; }
TEM inline void TRect<T>::posRightOf(const TRect<T> & r, T by){ pos(r.right() + by, r.t); }

TEM inline void TRect<T>::posRelTo(const TRect<T>& r, float rxf, float ryf, float xf, float yf, float x, float y){
	l = r.l + r.w*rxf - w*xf + x;
	t = r.t + r.h*ryf - h*yf + y;
}

TEM inline void TRect<T>::posUnder(const TRect<T> & r, T by){ pos(r.l, r.bottom() + by); }
TEM inline void TRect<T>::resizeLeftTo(T v){ T dl = l-v; w += dl; l = v; onResizeProxy(dl, 0); }
TEM inline void TRect<T>::resizeTopTo(T v){	T dt = t-v; h += dt; t = v; onResizeProxy(0, dt); }
TEM inline void TRect<T>::resizeRightTo(T v){ width(v - l); }
TEM inline void TRect<T>::resizeBottomTo(T v){ height(v - t); }
TEM inline void TRect<T>::resizeEdgesBy(T v){ l -= v; t -= v; extent(w + 2 * v, h + 2 * v); }
TEM inline void TRect<T>::set(const TRect<T>& r){ set(r.l, r.t, r.w, r.h); }
TEM inline void	TRect<T>::set(T le, T to, T wi, T he){ pos(le, to); extent(wi, he); }
TEM inline void TRect<T>::transpose(){ extent(h, w); }

TEM inline void TRect<T>::left  (T v){ l = v; }
TEM inline void TRect<T>::top   (T v){ t = v; }
TEM inline void TRect<T>::width (T v){ extent(v, h); }
TEM inline void TRect<T>::height(T v){ extent(w, v); }
TEM inline void TRect<T>::bottom(T v){ t = v - h; }
TEM inline void TRect<T>::right (T v){ l = v - w; }

TEM inline const TRect<T>& TRect<T>::rect() const { return *this; }
TEM inline T TRect<T>::right() const { return l + w; }
TEM inline T TRect<T>::bottom() const { return t + h; }
TEM inline T TRect<T>::area() const { return w * h; }
TEM inline void TRect<T>::center(T & x, T & y) const { x = centerX(); y = centerY(); }
TEM inline T TRect<T>::centerX() const { return l + w * 0.5f; }
TEM inline T TRect<T>::centerY() const { return t + h * 0.5f; }

TEM inline bool	TRect<T>::containsPoint(T x, T y) const { 
	return (x >= l) && (x <= (l + w)) && (y >= t) && (y <= (t + h)); 
}

TEM inline TRect<T> TRect<T>::extent() const { return TRect<T>(0,0,w,h); }

TEM inline bool TRect<T>::intersects(const TRect<T> & r) const {
	return ((l <= r.right()) && (r.l <= right()) &&
		(t <= r.bottom()) && (r.t <= bottom()));
}

TEM inline bool TRect<T>::isVertical() const { return h > w; }

TEM inline bool	TRect<T>::withinXBounds(T xmin, T xmax) const {
	return (l < xmax) && (right() > xmin);
}

TEM void TRect<T>::unionOf(const TRect<T>& c, TRect<T>& r) const {
	T tl = l;
	T tt = t;
	T tb = bottom();
	T tr = right();

	r.l = c.l < tl ? c.l : tl;
	r.t = c.t < tt ? c.t : tt;
	r.extent(
		(c.right() > tr ? c.right() : tr) - r.l,
		(c.bottom() > tb ? c.bottom() : tb) - r.t
	);
}

TEM inline bool	TRect<T>::withinYBounds(T ymin, T ymax) const {
	return (t < ymax) && (bottom() > ymin);
}

TEM void TRect<T>::insetFrom(const TRect<T>& from, T by){ 
	l = by;
	t = by;
	by *= 2.f;
	extent(from.w - by, from.h - by);
}

TEM void TRect<T>::intersection(const TRect<T> & check, TRect<T> & inter) const{
	T l1 = l;
	T r1 = right();
	T t1 = t;
	T b1 = bottom();
		
	T l2 = check.l;
	T r2 = check.right();
	T t2 = check.t;
	T b2 = check.bottom();		
	
	inter.pos(
		(l1 > l2) ? l1 : l2,
		(t1 > t2) ? t1 : t2
	);
	
	inter.extent(
		((r1 < r2) ? r1 : r2) - inter.l,
		((b1 < b2) ? b1 : b2) - inter.t
	);
}


TEM void TRect<T>::print(FILE * fp){
	fprintf(fp, "[%.2f, %.2f, %.2f, %.2f]\n", l, t, w, h);
}

} // glv::

#undef TEM

#endif

