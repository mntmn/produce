/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_sliders.h"
#include "glv_draw.h"

namespace glv{

Sliders::Sliders(const Rect& r, int nx, int ny, bool dragSelect)
:	Widget(r, 1, false, false, false),
	mAcc(0)
{
	data().resize(Data::DOUBLE, nx,ny);
	property(SelectOnDrag, dragSelect);
}

void Sliders::onDraw(GLV& g){
	Widget::onDraw(g);

	using namespace glv::draw;
	float x=padding()*0.5, xd=dx(), yd=dy();

//	TODO: dial drawing code...
//		for(int i=0; i<sizeX(); ++i){
//			float y=padding()*0.5;
//		
//			for(int j=0; j<sizeY(); ++j){
//				int ind = index(i,j);
//				if(isSelected(i,j)) color(colors().fore);
//				else color(colors().fore, colors().fore.a*0.5);
//				
//				color(colors().fore, colors().fore.a*0.5);
//				disc<32>(x,y,xd,yd);
//				lineWidth(1);
//				float v = Base::values()[ind];
//				color(colors().fore);
//				static const float pi = 3.141592653589793;
//				shape(Lines, xd/2, yd/2, xd/2 + xd/2*cos(v*2*pi+pi/2), yd/2 + yd/2*sin(v*2*pi-pi/2));
//
//				y += yd;
//			}
//			x += xd;	
//		}


	if(isVertical()){
		for(int i=0; i<sizeX(); ++i){
		
			float y=padding()*0.5;
		
			for(int j=0; j<sizeY(); ++j){
				int ind = index(i,j);
				if(isSelected(i,j)) color(colors().fore);
				else color(colors().fore, colors().fore.a*0.5);

				float v01 = to01(getValue(ind));
				float y0 = to01(0)*yd;
				//rect(x + x0, y, f*xd+x, y+yd-padding());
				
				rectangle(x, y+yd-v01*yd, x+xd-padding(), y+yd-y0);

				// if zero line showing
				if(mMax>0 && mMin<0){
					color(colors().border);
					float linePos = draw::pix(y+yd-y0);
					shape(draw::Lines, x, linePos, x+xd, linePos);
				}
				y += yd;
			}
			x += xd;	
		}
	}
	else{
		for(int i=0; i<sizeX(); ++i){
		
			float y=padding()*0.5;
		
			for(int j=0; j<sizeY(); ++j){
				int ind = index(i,j);
				if(isSelected(i,j)) color(colors().fore);
				else color(colors().fore, colors().fore.a*0.5);

				float v01 = to01(getValue(ind));
				float x0 = to01(0)*xd;
				rectangle(x + x0, y, v01*xd+x, y+yd-padding());

				// if zero line showing
				if(mMax>0 && mMin<0){
					color(colors().border);
					float linePos = draw::pix(x+x0);
					shape(draw::Lines, linePos, y, linePos, y+yd);
				}
				y += yd;
			}
			x += xd;
		}
	}
}

bool Sliders::onEvent(Event::t e, GLV& g){
	Widget::onEvent(e,g);
	
	switch(e){
		case Event::MouseDrag:
			// if drag settable
			if(enabled(SelectOnDrag)){
				selectSlider(g, false);
			}
			if(g.mouse().right() || g.mouse().left()) {
				// accumulate differences
				mAcc += diam()*(isVertical() ? -g.mouse().dy()/h*sizeY() : g.mouse().dx()/w*sizeX()) * g.mouse().sens();
				setValue(mAcc);
			}
			return false;
			
		case Event::MouseDown:
			selectSlider(g, true);
			return false;
			
		case Event::KeyDown:{
			switch(g.keyboard().key()){
			case 'x':
			case 'a': setValue(getValue() + diam()/32.); return false;
			case 'z': setValue(getValue() - diam()/32.); return false;
			default:;
			}
			break;
		
		case Event::KeyUp:
		case Event::MouseUp: return false;
		}

		default:;
			
	}
	return true;
}

void Sliders::selectSlider(GLV& g, bool click){

	const Mouse& m = g.mouse();
	
	int oldIdx = selected();
	selectFromMousePos(g);
	int idx = selected();
	
	float val = isVertical() ? (1-(m.yRel()/h*sizeY() - selectedY())) : (m.xRel()/w*sizeX() - selectedX());
	val = toInterval(val);
	
	// if left-button, set value
	if(m.left() && !m.right()){
		setValue(val);
	}
	
	// if click or new slider, reset accumulator
	if(click || (oldIdx != idx)){
		if(m.left() && !m.right()) mAcc = val;
		else mAcc = getValue(idx);
	}
}


// Slider2D

Slider2D::Slider2D(const Rect& r, double valX, double valY, space_t knobSize)
:	SliderVector<2>(r), knobSize(knobSize)
{
	setValue(valX, 0);
	setValue(valY, 1);
}

bool Slider2D::onEvent(Event::t e, GLV& g){

	switch(e){
		case Event::MouseDrag:
			valueAdd( g.mouse().dx()/w * diam() * g.mouse().sens(), 0);
			valueAdd(-g.mouse().dy()/h * diam() * g.mouse().sens(), 1);
			return false;
			
		case Event::MouseDown:
			if(g.mouse().left() && !g.mouse().right()){
				setValue(toInterval(    g.mouse().xRel()/w), 0);
				setValue(toInterval(1.f-g.mouse().yRel()/h), 1);
			}
			return false;
			
		case Event::MouseUp: clipAccs();
		case Event::KeyUp: return false;
		
		case Event::KeyDown:
			switch(g.keyboard().key()){
				case 'x': valueAdd(-diam()/w, 0); return false;
				case 'c': valueAdd( diam()/w, 0); return false;
				case 'a': valueAdd( diam()/h, 1); return false;
				case 'z': valueAdd(-diam()/h, 1); return false;
				default:;
			}
			break;
		default:;
	}
	return true;
}


void Slider2D::drawKnob(const Slider2D& s){
	using namespace glv::draw;
	float sz = s.knobSize;	// size of indicator block
	float sz2 = sz * 0.5f;
	float posX = sz2 + (s.w - sz) * s.to01(s.getValue(0));
	float posY = sz2 + (s.h - sz) * (1.f - s.to01(s.getValue(1)));
	
	color(s.colors().fore);
	rectangle(pix(posX - sz2), pix(posY - sz2), pix(posX + sz2), pix(posY + sz2));
}

/*
static void drawQuad(const Slider2D& s){
	using namespace glv::draw;
	float posX = s.w * s.getValue(0);
	float posY = s.h * (1.f - s.getValue(1));
	
	color(s.colors().fore);
	
	begin(Quads);
		vertex(0.f, posY);
		vertex(posX, s.h);
		vertex(s.w, posY);
		vertex(posX, 0.f);
	end();
	
	// clock hand
	//shape(Lines, posX, posY, s.w*0.5, s.h*0.5);
}
*/

void Slider2D::onDraw(GLV& g){

	drawKnob(*this);
	//drawQuad(*this);
	
//	float hh = h * 0.5f;
//	float hw = w * 0.5f;

	// Web
//	glBegin(GL_LINES);
//		glVertex2f(posX, posY); glVertex2f(hw, h);
//		glVertex2f(posX, posY); glVertex2f(w, hh);
//		glVertex2f(posX, posY); glVertex2f(hw, 0.f);
//		glVertex2f(posX, posY); glVertex2f(0.f, hh);
//
//		glVertex2f(posX, posY); glVertex2f(0.f, h);
//		glVertex2f(posX, posY); glVertex2f(w, h);
//		glVertex2f(posX, posY); glVertex2f(w, 0.f);
//		glVertex2f(posX, posY); glVertex2f(0.f, 0.f);
//
////		glVertex2f(0.f, posY); glVertex2f(w, posY);
////		glVertex2f(posX, 0.f); glVertex2f(posX, h);
//	glEnd();
	
	//glColor4f(glvColor4(bgColor));
	//glRectf(posX, h, posX+1, 0.f);
	//glRectf(0.f, posY, w, posY+1);

//	if((posX > 6.f) && (posY < (h - 6.f))) return;

	// fade color in
	//s->sliderColor->glColor3(1.f - ((posX < h - posY) ? posX : h - posY) * 0.16667f);

	// double diagonal indicators
//	glBegin(GL_LINES);
//		glVertex2f(posX, posY); glVertex2f(w, hh);
//		glVertex2f(posX, posY); glVertex2f(hw, 0.f);
//	glEnd();
	
	// x
//	glBegin(GL_LINES);
//		float d = 6.f;
//		glVertex2f(posX - d, posY - d);
//		glVertex2f(posX + d, posY + d);
//		glVertex2f(posX, posY);
//		glVertex2f(posX + d, posY - d);
//	glEnd();
	
	// overline
//	glBegin(GL_LINES);
//		float d = 6.f;
//		glVertex2f(0.f, h - d);
//		glVertex2f(posX, h - d);
//		glVertex2f(d, h);
//		glVertex2f(d, posY);
//	glEnd();
}



SliderRange::SliderRange(const Rect& r, double val1, double val2)
:	SliderVector<2>(r), mDragMode(0), mJump(0.1)
{
	extrema(val1, val2);
}

double SliderRange::center() const { return (getValue(0) + getValue(1))*0.5; }
double SliderRange::jump() const { return mJump; }
double SliderRange::range() const { return getValue(1)-getValue(0); }

SliderRange& SliderRange::center(double v){ return centerRange(v, range()); }

SliderRange& SliderRange::centerRange(double c, double r){
//	double mn = c-(r/2.);
//	double mx = mn+r;
//	if(mn<0){ mx -= mn  ; mn=0; }
//	if(mx>1){ mn -= mx-1; mx=1; }
//	return extrema(mn,mx);
	double mn = c-(r/2.);
	double mx = mn+r;
	// adjust min/max values to preserve range
	if(mn<mMin){ mx += mMin-mn; mn=mMin; }
	if(mx>mMax){ mn -= mx-mMax; mx=mMax; }
	return extrema(mn,mx);
}

SliderRange& SliderRange::extrema(double min, double max){
	glv::sort(min,max);
	setValue(min,0);
	setValue(max,1);
	return *this;
}

SliderRange& SliderRange::jump(double v){ mJump=v; return *this; }
SliderRange& SliderRange::range(double v){ return centerRange(center(), v); }

void SliderRange::onDraw(GLV& g){
	using namespace glv::draw;

	float v1 = to01(getValue(0));
	float v2 = to01(getValue(1));
	if(v2<v1){ float t=v1; v1=v2; v2=t; }
	
	// prevent degeneracy
	float p1 = 1./((w>h)?w:h); // 1 pixel
	if(v2-v1 <= p1){ v1-=p1*0.5; v2+=p1*0.5; }

	color(colors().fore);
	if(w>h){	// horizontal
		rectangle(v1*w,0, v2*w,h);
	}
	else{
		rectangle(0,v1*h, w,v2*h);
	}
}

bool SliderRange::onEvent(Event::t e, GLV& g){

	float value0 = to01(getValue(0));
	float value1 = to01(getValue(1));
	
	//printf("%f %f\n", value0, value1);

	float dv = (w>h) ? g.mouse().dx()/w : g.mouse().dy()/h;
	float mp = (w>h) ? g.mouse().xRel()/w : g.mouse().yRel()/h;
	float d1 = mp-value0; if(d1<0) d1=-d1;
	float d2 = mp-value1; if(d2<0) d2=-d2;
	float rg = range();
	float endRegion = 4./(w>h ? w:h);

	switch(e){
	case Event::MouseDown:
		
		// NOTE: There is more than one way we might want to move the slider 
		// on a click. We can set its center or increment its center in the
		// direction of the click. Also, we may not want to move the slider
		// if the click lands within the range...
		if(g.mouse().left()){
			float v1 = value0;
			float v2 = value1;
			float center_ = to01(center());
			
			// click outside of range
			if(mp<(v1-endRegion) || mp>(v2+endRegion)){
				float dc = mp - center_;
				float dcAbs = dc<0 ? -dc : dc;
				if(jump() > dcAbs){
					center(toInterval(mp));
				}
				else{
					center(toInterval(center_ + (dc<0 ? -jump() : jump())));
				}
				mDragMode=0;
			}
			
			// click on lower edge
			else if(mp > (v1-endRegion) && mp < (v1+endRegion)){
				mDragMode=1;
			}
			
			// click on upper edge
			else if(mp > (v2-endRegion) && mp < (v2+endRegion)){
				mDragMode=2;
			}
			
			// click on range
			else{
				mDragMode=3;
			}
		}
		
		return false;
	
	case Event::MouseDrag:
		dv *= diam() * g.mouse().sens();
		if(3==mDragMode){
			valueAdd(dv, 0, mMin, mMax-rg);
			valueAdd(dv, 1, mMin+rg, mMax);
		}
		else if(1==mDragMode) valueAdd(dv, 0);
		else if(2==mDragMode) valueAdd(dv, 1);

		return false;

	case Event::MouseUp:
		if(3==mDragMode){
			mAcc[0] = glv::clip(mAcc[0], mMax-rg, mMin);
			mAcc[1] = glv::clip(mAcc[1], mMax, mMin+rg);
		}
		else if(1==mDragMode) mAcc[0] = glv::clip(mAcc[0], mMax, mMin);
		else if(2==mDragMode) mAcc[1] = glv::clip(mAcc[1], mMax, mMin);
		return false;

	default:;
	}

	return true;
}





FunctionGraph::FunctionGraph(const Rect& r, int nKnots, int res)
: View(r), mTension(0.), mKnobSize(3), mCurrentKnot(-1), mNKnots(nKnots), mKnots(0)
{
	//minimum 3 knots
	if(mNKnots < 3) mNKnots = 3;
	
	mKnots = new Knot[mNKnots];
	for(int i=0; i < mNKnots; i++) {
		mKnots[i].x = ((float)i)/(mNKnots-1);
		mKnots[i].y = mKnots[i].x*mKnots[i].x;
	}
	
	for(int i=0; i < mNKnots-1; i++) {
		mCurves.push_back(new Curve(res));
	}
	calcCurves();
}

FunctionGraph::~FunctionGraph()
{
	if(mKnots) delete[] mKnots;
	std::vector<Curve *>::iterator it = mCurves.begin();
	std::vector<Curve *>::iterator it_e = mCurves.end();
	for(; it != it_e; ++it){
		delete *it;
	}
}

void FunctionGraph::calcCurves()
{
	std::vector<Curve *>::iterator it = mCurves.begin();
	std::vector<Curve *>::iterator it_e = mCurves.end();
	int i=0;
	for(; it != it_e; ++it){
		//for now we duplicate boundry points
		int idx0, idx1, idx2, idx3;
		
		if(i==0) {
			idx0 = idx1 = 0;
			idx2 = 1;
			idx3 = 2;
		}
		else if(i == (mNKnots-2)) {
			idx0 = i-1;
			idx1 = i;
			idx2 = idx3 = mNKnots-1;
		}
		else {
			idx0 = i-1;
			idx1 = i;
			idx2 = i+1;
			idx3 = i+2;
		}
	
		Curve &c = *(*it);
	
		float mu = 0.;
		float dmu = 1./(float)(c.size()-1);
		for(int t = 0; t < c.size(); t++) {
			c[t] = HermiteInterpolate(mKnots[idx0].y, mKnots[idx1].y, mKnots[idx2].y, mKnots[idx3].y, mu, mTension, 0);
			mu += dmu;
		}
		
		i++;
	}
}

void FunctionGraph::eval(int n, float *vals)
{
	float dx = 1./(n-1);
	float x = 0;
	int idx = 0;
	int k = 0;
	vals[idx] = mKnots[k].y;
	
	int idx0 = 0;
	int idx1 = 0;
	int idx2 = 1;
	int idx3 = 2;

	for(; idx < n-1; idx++) {
		if(x > mKnots[k+1].x) {
			while(x > mKnots[k+1].x) {
				k++;
			}
			
			if(k == (mNKnots-2)) {
				idx0 = k-1;
				idx1 = k;
				idx2 = idx3 = mNKnots-1;
			}
			else {
				idx0 = k-1;
				idx1 = k;
				idx2 = k+1;
				idx3 = k+2;
			}
		}
		
		float mu = (x - mKnots[k].x) / (mKnots[k+1].x - mKnots[k].x);
		vals[idx] = HermiteInterpolate(mKnots[idx0].y, mKnots[idx1].y, mKnots[idx2].y, mKnots[idx3].y, mu, mTension, 0);
		
		x += dx;
	}
	
	vals[idx] = mKnots[k].y;
}

void FunctionGraph::onDraw(GLV& g){
	using namespace glv::draw;
	
	std::vector<Curve *>::iterator it = mCurves.begin();
	std::vector<Curve *>::iterator it_e = mCurves.end();
	
	GraphicsData& gd = g.graphicsData();
	
	color(mStyle->color.fore);

//	begin(LineStrip);
//	int i=0;
//	for(; it != it_e; ++it){
//		Curve &c = *(*it);
//		
//		float dx = (mKnots[i+1].x - mKnots[i].x)/(c.size()-1);
//		float x = mKnots[i].x;
//		for(int t = 0; t < c.size(); t++) {
//			vertex(x*w, (1.-c[t])*h);
//			x += dx;
//		}
//		i++;
//	}
//	end();
	int i=0;
	for(; it != it_e; ++it){
		Curve &c = *(*it);
		
		float dx = (mKnots[i+1].x - mKnots[i].x)/(c.size()-1);
		float x = mKnots[i].x;
		for(int t = 0; t < c.size(); t++) {
			gd.addVertex(x*w, (1.-c[t])*h);
			x += dx;
		}
		i++;
	}
	draw::paint(LineStrip, gd);
	
	color(mStyle->color.fore, mStyle->color.fore.a*0.5);

	gd.reset();
	i=0;
	it = mCurves.begin();
	for(; it != it_e; ++it){
		Curve &c = *(*it);
		
		float dx = (mKnots[i+1].x - mKnots[i].x)/(c.size()-1);
		float x = mKnots[i].x;
		for(int t = 0; t < c.size(); t++) {
			gd.addVertex(x*w, (1.-c[t])*h);
			gd.addVertex(x*w, h);
			x += dx;
		}
		i++;
	}
	draw::paint(TriangleStrip, gd);
	
	for(int k = 0; k < mNKnots; k++) {
		int cx = mKnots[k].x*w;
		int cy = (1.-mKnots[k].y)*h;
		frame(cx-mKnobSize, cy-mKnobSize, cx+mKnobSize, cy+mKnobSize);
	}
}

bool FunctionGraph::onEvent(Event::t e, GLV& glv)
{
	switch(e){
	case Event::MouseDrag: {
		if(glv.mouse().left() && mCurrentKnot >= 0) {
			if(mCurrentKnot == 0 || mCurrentKnot == (mNKnots-1)) {
				mKnots[mCurrentKnot].y = 1.-(glv.mouse().yRel()/h);
				mKnots[mCurrentKnot].y = (mKnots[mCurrentKnot].y < 0.) ? 0. : 
												((mKnots[mCurrentKnot].y > 1.) ? 1. : mKnots[mCurrentKnot].y);
			}
			else {
				mKnots[mCurrentKnot].x = (glv.mouse().xRel()/w);
				mKnots[mCurrentKnot].y = 1.-(glv.mouse().yRel()/h);
				
				mKnots[mCurrentKnot].x = (mKnots[mCurrentKnot].x < 0) ? 0 : 
											((mKnots[mCurrentKnot].x > 1.) ? 1 : mKnots[mCurrentKnot].x);
				
				mKnots[mCurrentKnot].y = (mKnots[mCurrentKnot].y < 0.) ? 0. : 
												((mKnots[mCurrentKnot].y > 1.) ? 1. : mKnots[mCurrentKnot].y);
				
				//check if we went beyond neighboring knots
				if(mKnots[mCurrentKnot].x < mKnots[mCurrentKnot-1].x && mCurrentKnot != mCurrentKnot+1) {
					Knot kt;
					kt.x = mKnots[mCurrentKnot].x;
					kt.y = mKnots[mCurrentKnot].y;
					
					mKnots[mCurrentKnot].x = mKnots[mCurrentKnot-1].x;
					mKnots[mCurrentKnot].y = mKnots[mCurrentKnot-1].y;
					
					mKnots[mCurrentKnot-1].x = kt.x;
					mKnots[mCurrentKnot-1].y = kt.y;
					
					mCurrentKnot--;
				}
				else if(mKnots[mCurrentKnot].x > mKnots[mCurrentKnot+1].x && mCurrentKnot != mNKnots-2) {
					Knot kt;
					kt.x = mKnots[mCurrentKnot].x;
					kt.y = mKnots[mCurrentKnot].y;
					
					mKnots[mCurrentKnot].x = mKnots[mCurrentKnot+1].x;
					mKnots[mCurrentKnot].y = mKnots[mCurrentKnot+1].y;
					
					mKnots[mCurrentKnot+1].x = kt.x;
					mKnots[mCurrentKnot+1].y = kt.y;
					
					mCurrentKnot++;
				}
			}
			
			calcCurves();
		}
	}
	break;
	
	case Event::MouseDown: {
		if(glv.mouse().left()) {
			mCurrentKnot = knotHitTest(glv.mouse().xRel(), glv.mouse().yRel());
		}
	}
	break;
	
	default:
	break;
	}
	
	return false;
}

int FunctionGraph::knotHitTest(space_t x, space_t y)
{
	int idx = -1;
	float min_dsq = mKnobSize*mKnobSize*2.5;
	
	for(int k = 0; k < mNKnots; k++) {
		float dx = mKnots[k].x*w - x;
		float dy =(1.-mKnots[k].y)*h - y;
		float dsq = dx*dx+dy*dy;
		if(dsq < min_dsq) {
			min_dsq = dsq;
			idx = k;
		}
	}
	
	return idx;
}

} // glv::
