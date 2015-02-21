#ifndef INC_GLV_WIDGET_H
#define INC_GLV_WIDGET_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"
#include "glv_model.h"

namespace glv {

typedef ChangedValue<Data> ModelChange;

/// Base class for views having a regular grid of elements
class Widget : public View {
public:

	/// @param[in] r		geometry
	/// @param[in] pad		padding from border
	/// @param[in] toggles	whether the value elements toggle
	/// @param[in] mutExc	whether value elements operate mutually exclusively
	/// @param[in] drawGrid	whether to draw grid separater for multiple elements
	Widget(
		const Rect& r, space_t pad=3,
		bool momentary=false, bool mutExc=false, bool drawGrid=true
	);

	/// Get selected element value type-casted to template parameter
	template <class T>
	T getValue() const { return getValue<T>(selectedX(), selectedY()); }
	
	/// Get element value at 1D index type-casted to template parameter
	template <class T>
	T getValue(int i) const { return data().Data::at<T>(i); }
	
	/// Get element value at 2D index type-casted to template parameter
	template <class T>
	T getValue(int i1, int i2) const { return data().Data::at<T>(i1, i2); }

	/// Returns whether this element coordinate is selected
	bool isSelected(int x, int y) const { return x == selectedX() && y == selectedY(); }

	const double& max() const { return mMax; }			///< Get maximum of value interval
	const double& min() const { return mMin; }			///< Get minimum of value interval
	double mid() const { return (max()+min())/2; }		///< Get middle point of value interval

	space_t padding() const { return mPadding; }	///< Get element padding amount

	int selected() const { return data().indexFlat(sx, sy); }	///< Get selected element index
	int selectedX() const { return sx; }			///< Get selected element x coordinate
	int selectedY() const { return sy; }			///< Get selected element y coordinate
	int size () const { return data().size(); }	///< Get total number of elements
	int sizeX() const { return data().size(0); }	///< Get number of elements along x
	int sizeY() const { return data().size(1); }	///< Get number of elements along y
	bool useInterval() const { return mUseInterval; }


	/// Attach a single variable at a specified index
	template <class V>
	void attachVariable(V& val, int i=0){
		variables()[i] = Data(val);
	}

	/// Set interval for numerical values
	Widget& interval(const double& max, const double& min=0){
		glv::sort(mMin=min, mMax=max); return *this;
	}

	/// Set element padding amount
	Widget& padding(space_t v){ mPadding=v; return *this; }

	/// Select element at 1D index
	Widget& select(int i){ int i1,i2; data().indexDim(i1,i2,i); return select(i1,i2); }

	/// Select element at 2D index
	Widget& select(int ix, int iy);

	virtual void onCellChange(int indexOld, int indexNew){}

	template <class T>
	Widget& setValue(const T& v);

	/// Set value at specified 1D index
	template <class T>
	Widget& setValue(const T& v, int i){ select(i); return setValue(v); }

	/// Set value at specified 2D index
	template <class T>
	Widget& setValue(const T& v, int ix, int iy){ select(ix,iy); return setValue(v); }

	Widget& setValue(const char * v){ return setValue(std::string(v)); }

	/// Set all values to maximum
	Widget& setValueMax();
	
	/// Set all values to middle value
	Widget& setValueMid();

	Widget& useInterval(bool v){ mUseInterval=v; return *this; }
	
	virtual void onDataModelSync();
	virtual const char * className() const { return "Widget"; }

protected:
	typedef std::map<int, glv::Data> IndexDataMap;

	IndexDataMap mVariables;		// external variables to sync to, index-Data
	space_t mPadding;				// num pixels to inset icon
	int sx, sy;						// last clicked position
	double mMin, mMax;				// value interval for all elements
	double mPrevVal;					//
	bool mUseInterval;

	IndexDataMap& variables(){ return mVariables; }
	const IndexDataMap& variables() const { return mVariables; }

	float dx(int dim=0) const { return w/data().size(dim); } // width, in pixels, per element
	float dy(int dim=1) const { return h/data().size(dim); } // height, in pixels, per element
	int index(int ix, int iy) const { return ix + iy*sizeX(); }

	static void clip(int& i, int max){ i<0 ? i=0 : i>=max ? i=max-1 : 0; }
	void clipIndices(int& x, int& y){ clip(x, sizeX()); clip(y, sizeY()); }
	void clipIndices(){ clipIndices(sx,sy); }
	bool validIndex(int i) const { return (i < size()) && (i >= 0); }

	double diam() const { return max()-min(); }
	double to01(const double& v) const { return (v-min())/diam(); }
	double toInterval(const double& v) const { return v*diam() + min(); }

	template <class T>
	Widget& setValue(const T& v, int i, double mn, double mx){
		double omn=min(), omx=max();
		interval(mx, mn);
		setValue(v,i);
		return interval(omx, omn);
	}

	virtual bool onAssignData(Data& d, int ind1, int ind2);
	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);

	void drawGrid(GraphicsData& g);
	void drawSelectionBox();	
	void selectFromMousePos(GLV& g);
	
	static bool widgetKeyDown(View * v, GLV& g);
};


template <class T>
Widget& Widget::setValue(const T& v){
	T t = v;
	Data d(t);
	assignData(d, selectedX(), selectedY());
	return *this;
}

} // glv::
#endif
