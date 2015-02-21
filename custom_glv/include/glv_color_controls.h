#ifndef INC_GLV_COLOR_CONTROLS_H
#define INC_GLV_COLOR_CONTROLS_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"
#include "glv_sliders.h"

namespace glv {


//class ColorSliders : public View{
//public:
//
//	ColorSliders(const Rect& r, bool isHSV=true, bool controlsAlpha=false)
//	: View(r), mSliderStyle(false), mIsHSV(isHSV), mControlsAlpha(controlsAlpha)
//	{
//		//mDeletesChildren = false;
//		
//		mComp1 = new Slider;
//		mComp2 = new Slider;
//		mComp3 = new Slider;
//		mCompA = new Slider;
//		
//		int num = mControlsAlpha ? 4 : 3;
//		mComp1->anchor(0,0./num).stretch(1,1./num).set(0,0./num,0,0);
//		mComp2->anchor(0,1./num).stretch(1,1./num).set(0,1./num,0,0);
//		mComp3->anchor(0,2./num).stretch(1,1./num).set(0,2./num,0,0);
//		mCompA->anchor(0,3./num).stretch(1,1./num).set(0,3./num,0,0);
//		mComp1->value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
//		mComp2->value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
//		mComp3->value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
//		mCompA->value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
//		setColor();
//		(*this) << mComp1 << mComp2 << mComp3 << mCompA;
//		mCompA->property(Visible, mControlsAlpha);
//		
////		printf("%x\n", this);
////		printf("\t%x\n", &mComp1);
////		printf("\t%x\n", &mComp2);
////		printf("\t%x\n", &mComp3);
////		printf("\t%x\n", &mCompA);
//	}
//	
//	bool isHSV() const { return mIsHSV; }
//	const Color& value() const { return mColor; }
//	ColorSliders& value(const Color& c){ setColor(c); return *this; }
//	
//	virtual const char * className() const { return "ColorSliders"; }
//	
//protected:
//	Color mColor;
//	Style mSliderStyle;
//	Slider *mComp1, *mComp2, *mComp3, *mCompA;
//	bool mIsHSV, mControlsAlpha;
//
//	void setColor(const Color& c){
//		float c1,c2,c3;
//		if(isHSV()){ HSV h=c; c1=h.h; c2=h.s; c3=h.v; }
//		else{ c1=c.r; c2=c.g; c3=c.b; }
//		// assuming value() does not send notification...
//		mComp1->value(c1); mComp2->value(c2); mComp3->value(c3);
//		mCompA->value(c.a);
//		setColor();
//	}
//
//	void setColor(){
//		float c1=mComp1->value(), c2=mComp2->value(), c3=mComp3->value();
//		if(isHSV()){	mColor = HSV(c1,c2,c3);}
//		else{			mColor.set(c1,c2,c3);}
//		if(mControlsAlpha) mColor.a = mCompA->value();
//		mSliderStyle.color.fore = mColor;
//		//value().print();
//	}
//
//	static void updateComp(const Notification& n){
//		ColorSliders& c = *(ColorSliders *)n.receiver();
//		c.setColor();
//	}
//};

class ColorSliders : public View{
public:

	ColorSliders(const Rect& r, bool isHSV=true, bool controlsAlpha=false)
	: View(r), mSliderStyle(false), mIsHSV(isHSV), mControlsAlpha(controlsAlpha)
	{
		int num = mControlsAlpha ? 4 : 3;
		mComp1.anchor(0,0./num).stretch(1,1./num).set(0,0./num,0,0);
		mComp2.anchor(0,1./num).stretch(1,1./num).set(0,1./num,0,0);
		mComp3.anchor(0,2./num).stretch(1,1./num).set(0,2./num,0,0);
		mCompA.anchor(0,3./num).stretch(1,1./num).set(0,3./num,0,0);
		mComp1.value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
		mComp2.value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
		mComp3.value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
		mCompA.value(1).style(&mSliderStyle).attach(updateComp, Update::Value, this);
		setColor();
		(*this) << mComp1 << mComp2 << mComp3 << mCompA;
		mCompA.property(Visible, mControlsAlpha);
	}
	
	bool isHSV() const { return mIsHSV; }
	const Color& value() const { return mColor; }
	ColorSliders& value(const Color& v){ setColor(v); return *this; }
	ColorSliders& value(const HSV& v){ setColor(v); return *this; }

	virtual const char * className() const { return "ColorSliders"; }
	
protected:
	Color mColor;
	Style mSliderStyle;
	Slider mComp1, mComp2, mComp3, mCompA;
	bool mIsHSV, mControlsAlpha;

	void setColor(const Color& v){
		float c1,c2,c3;
		if(isHSV()){ HSV h=v; c1=h.h; c2=h.s; c3=h.v; }
		else{ c1=v.r; c2=v.g; c3=v.b; }
		// assuming value() does not send notification...
		mComp1.value(c1); mComp2.value(c2); mComp3.value(c3);
		mCompA.value(v.a);
		setColor();
	}

	void setColor(const HSV& v){
		float c1,c2,c3;
		if(isHSV()){ c1=v.h; c2=v.s; c3=v.v; }
		else{ Color c=v; c1=c.r; c2=c.g; c3=c.b; }
		// assuming value() does not send notification...
		mComp1.value(c1); mComp2.value(c2); mComp3.value(c3);
		//mCompA.value(v.a);
		setColor();
	}

	void setColor(){
		float c1=mComp1.value(), c2=mComp2.value(), c3=mComp3.value();
		if(isHSV()){	mColor = HSV(c1,c2,c3);}
		else{			mColor.set(c1,c2,c3);}
		if(mControlsAlpha) mColor.a = mCompA.value();
		mSliderStyle.color.fore = mColor;
		//value().print();
	}

	static void updateComp(const Notification& n){
		ColorSliders& c = *(ColorSliders *)n.receiver();
		c.setColor();
	}
};

} // glv::

#endif
