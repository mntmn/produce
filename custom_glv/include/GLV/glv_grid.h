#ifndef INC_GLV_GRID_H
#define INC_GLV_GRID_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"
#include "glv_font.h"

#ifdef minor
#undef minor
#endif
#ifdef major
#undef major
#endif

namespace glv{

/// Interactive grid for graphing
class Grid : public View{
public:
	
	/// @param[in] r			geometry
	/// @param[in] intervalMin	minimum displayed value for all dimensions
	/// @param[in] intervalMax	maximum displayed value for all dimensions
	/// @param[in] majorDist	distance between major grid lines
	/// @param[in] minorDiv		number of minor divisions between major lines
	Grid(const Rect& r, double intervalMin=-1, double intervalMax=1, double majorDist=1, int minorDiv=4);

	/// Get interval of a dimension
	const Interval<double>& interval(int dim) const { return mInterval[dim]; }
	
	/// Set interval of a dimension
	Interval<double>& interval(int dim){ return mInterval[dim]; }
//	double major(int dim) const { return mMajor[i]; }
//	int minor(int dim) const { return mMinor[i]; }

	/// Returns whether point is contained within grid region
	bool contains(double x, double y){
		return interval(0).contains(x) && interval(1).contains(y);
	}

	/// Returns whether axes are showing
	bool showAxes() const { return mShowAxes; }

	/// Returns whether grid lines are showing
	bool showGrid() const { return mShowGrid; }

	/// Returns whether grid line numbering is showing
	bool showNumbering() const { return mShowNumbering; }

	#define LOOP for(int i=0;i<DIM;++i)
	
	/// Set whether grid line numbering is active for all dimensions
	Grid& numbering(bool v){ LOOP{ numbering(v,i); } return *this; }

	/// Set whether grid line numbering is active for a dimension
	Grid& numbering(bool v, int dim){ mNumbering[dim]=v; return *this; }

	/// Set minor division for all dimensions
	Grid& minor(int v){ LOOP{ minor(v,i); } return *this; }

	/// Set minor division for a dimension
	Grid& minor(int v, int dim){ mMinor[dim]=v; return *this; }

	/// Set major division diameter for all dimensions
	Grid& major(double v){ LOOP{ major(v,i); } return *this; }

	/// Set major division diameter for a dimension
	Grid& major(double v, int dim){ mMajor[dim]=v; return *this; }

	/// Center grid on origin
	Grid& origin(){ LOOP{ interval(i).center(0); } return *this; }

	/// Set whether to equalize distances along axes
	Grid& equalizeAxes(bool v){ mEqualize=v; return *this; }

	/// Set all dimension intervals to [-v, v]
	Grid& range(double v){ return range(-v,v); }

	/// Set all dimension intervals to [min, max]
	Grid& range(double min, double max){ LOOP{ range(min,max, i); } return *this; }
	
	/// Set a dimension's interval to [min, max]
	Grid& range(double min, double max, int i){ interval(i).endpoints(min,max); return *this; }

	/// Set whether to show axes
	Grid& showAxes(bool v){ mShowAxes=v; return *this; }
	
	/// Set whether to show grid
	Grid& showGrid(bool v){ mShowGrid=v; return *this; }
	
	/// Set whether to show numbering
	Grid& showNumbering(bool v){ mShowNumbering=v; return *this; }

	/// Zoom grid on a point
	Grid& zoom(double amt, double x, double y);

	virtual bool onEvent(Event::t e, GLV& g);
	virtual void onAnimate(double dt, GLV& g);
	virtual void onDraw(GLV& g);
//	virtual void onResize(space_t dx, space_t dy);
	
protected:
	typedef Interval<double> interval_t;
	enum{DIM=2};

	interval_t mInterval[DIM];
	double mMajor[DIM];
	int mMinor[DIM];
	float mVel[DIM], mVelW;
	bool mNumbering[DIM];
	bool mShowAxes, mShowGrid, mShowNumbering, mEqualize;

	int addGridLines(int i, double dist, GraphicsData& gb);

	// map grid coordinate to GLV pixel coordinate
	double gridToPix(int i, double v){
		double r = interval(i).toUnit(v);
		return (i ? 1-r : r) * extentVector[i];
	}

	// map GLV pixel coordinate to grid coordinate
	double pixToGrid(int i, double v){
		if(i) v = extentVector[i]-v;
		return v/extentVector[i] * interval(i).diameter() + interval(i).min();
	}
	double pixToGridMul(int i, double v){ return (interval(i).diameter()/extentVector[i])*v; }

	// push into grid space for drawing
	void pushGrid(){
		float tx = gridToPix(0, 0);
		float ty = gridToPix(1, 0);
		float sx = gridToPix(0, interval(0).min()+1);
		float sy = gridToPix(1, interval(1).min()+1);
		draw::push();
		draw::translate(tx, ty);
		draw::scale(sx,sy-h);
	}

	// pop out of grid space
	void popGrid(){ draw::pop(); }

	void zoomOnMousePos(double amt, const Mouse& m);
	#undef LOOP
};

} // glv::
#endif
