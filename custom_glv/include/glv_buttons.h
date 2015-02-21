#ifndef INC_GLV_BUTTONS_H
#define INC_GLV_BUTTONS_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"
#include "glv_icon.h"
#include "glv_widget.h"

namespace glv {

/// One or more buttons on a grid
class Buttons : public Widget {
public:

	/// @param[in] r			geometry
	/// @param[in] nx			number along x
	/// @param[in] ny			number along y
	/// @param[in] momentary	whether the button state matches button press state
	/// @param[in] mutExc		whether multiple buttons can be on
	/// @param[in] on			the on state icon
	/// @param[in] off			the off state icon
	Buttons(
		const Rect& r=Rect(), int nx=1, int ny=1,
		bool momentary=false, bool mutExc=false,
		iconFunc on=draw::rectangle, iconFunc off=0
	);

	/// Get off state icon
	const iconFunc& iconOff() const { return mIconOff; }
	
	/// Get on state icon
	const iconFunc& iconOn () const { return mIconOn; }

	/// Set off state icon
	Buttons& iconOff(const iconFunc& f){ mIconOff=f; return *this; }

	/// Set on state icon
	Buttons& iconOn (const iconFunc& f){ mIconOn =f; return *this; }

	virtual const char * className() const { return "Buttons"; }
	virtual void onDraw(GLV& g);
	virtual bool onEvent(Event::t e, GLV& g);

	bool getValue() const { return Widget::getValue<bool>(); }
	bool getValue(int i) const { return Widget::getValue<bool>(i); }
	bool getValue(int i1, int i2) const { return Widget::getValue<bool>(i1, i2); }

protected:
	iconFunc mIconOff, mIconOn;	// state icons
//	Icon * mIconOff, mIconOn;
};



/// Single button
class Button : public Buttons {
public:
	/// @param[in] r			geometry
	/// @param[in] momentary	whether the button state matches button press state
	/// @param[in] on			the on state icon
	/// @param[in] off			the off state icon
	Button(const Rect& r=Rect(20), bool momentary=false, iconFunc on=draw::rectangle, iconFunc off=0)
	:	Buttons(r, 1,1, momentary, false, on, off)
	{}
	
	virtual const char * className() const { return "Button"; }
};


} // glv::

#endif

