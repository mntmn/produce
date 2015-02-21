#include "glv_draw.h"
#include "glv_widget.h"

namespace glv{

bool Widget::widgetKeyDown(View * v, GLV& g){
	Widget& w = *(Widget *)v;
	switch(g.keyboard().key()){
		case Key::Down:	++w.sy; break;
		case Key::Up:	--w.sy; break;
		case Key::Right:++w.sx; break;
		case Key::Left:	--w.sx; break;
		default: return true;
	}
	w.clipIndices();
	return false;
}


Widget::Widget(
	const Rect& r, space_t pad, bool moment, bool mutExc, bool drawGrid
)
:	View(r), mPadding(pad), sx(0), sy(0), mMin(0), mMax(1), mUseInterval(true)
{
	property(DrawGrid, drawGrid);
	property(MutualExc, mutExc);
	property(Momentary, moment);
//	addCallback(Event::KeyDown, widgetKeyDown);
}

void Widget::drawGrid(GraphicsData& g){
	
	if(enabled(DrawGrid) && size()>1){
		using namespace glv::draw;
		color(colors().border); lineWidth(1);
		draw::grid(g, 0,0,w,h, sizeX(), sizeY(), false);
	}
}

void Widget::drawSelectionBox(){
	if(enabled(DrawSelectionBox)){
		draw::lineWidth(1);
		draw::color(colors().selection);
		draw::frame(sx*dx(), sy*dy(), (sx+1)*dx(), (sy+1)*dy());
	}
}

void Widget::onDraw(GLV& g){
	drawSelectionBox();
	drawGrid(g.graphicsData());
	g.graphicsData().reset();
}

bool Widget::onEvent(Event::t e, GLV& g){
	switch(e){
		case Event::KeyDown:
			switch(g.keyboard().key()){
				case Key::Down:	++sy; break;
				case Key::Up:	--sy; break;
				case Key::Right:++sx; break;
				case Key::Left:	--sx; break;
				default: return true;
			}
			clipIndices();
			return false;

		case Event::MouseUp:
			if(enabled(Momentary)) setValue(mPrevVal);
			return false;

//		case Event::MouseDown:
//			sx = (int)((g.mouse.xRel() / w) * sizeX());
//			sy = (int)((g.mouse.yRel() / h) * sizeY());
//			clipIndices();
//			return false;

		default:;
	}
	return true;
}

void Widget::onDataModelSync(){
	IndexDataMap::iterator it = variables().begin();

	for(; it!=variables().end(); ++it){
		int idx = it->first;

		if(validIndex(idx)){
			const Data& dat = it->second;
			if(data().slice(idx, data().size()-idx) != dat){
				assignData(dat, idx);
			}
		}
	}
}

// note: indices passed in are always valid
bool Widget::onAssignData(Data& d, int ind1, int ind2){

	if(data().isNumerical()){
		if(enabled(MutualExc)){
			double v = 0;
			if(useInterval()) v = glv::clip(v, mMax, mMin);
			data().assignAll(v);
		}

		if(useInterval()){
			for(int i=0; i<d.size(); ++i){
				double v = d.at<double>(i);
				v = glv::clip(v, max(), min());
				d.assign(v, i);
			}
		}
	}

	int idx = data().indexFlat(ind1,ind2);	// starting index of model

	// Update any attached variables containing this index
	IndexDataMap::iterator it = variables().begin();
	for(; it != variables().end(); ++it){
		Data& v = it->second;
		
		// get destination/source intervals in terms of model indices
		int id0 = it->first;
		int id1 = id0 + v.size();
		int is0 = idx;
		int is1 = is0 + d.size();
		
		// the intersection
		int i0 = glv::max(id0, is0);
		int i1 = glv::min(id1, is1);
		
		if(i0 < i1){
//			printf("[%d, %d), [%d, %d), [%d, %d)\n", id0, id1, is0, is1, i0, i1);
			v.slice(i0-id0, i1-i0).assign(
				d.slice(i0-is0, i1-i0)
			);
		}
	}
	
//	if(variables().count(idx)){
//		Data& v = variables()[idx];
//		printf("1: %s %s\n", v.toToken().c_str(), d.toToken().c_str());
//		v.assign(d);
//		printf("2: %s %s\n", v.toToken().c_str(), d.toToken().c_str());
//	}

	Data modelOffset = data().slice(idx, data().size()-idx);

	if(d != modelOffset){
		data().assign(d, ind1, ind2);
		ModelChange modelChange(data(), idx);
		notify(this, Update::Value, &modelChange);
	}

	return true;
}

void Widget::selectFromMousePos(GLV& g){
	select(
		(g.mouse().xRel() / w) * sizeX(),
		(g.mouse().yRel() / h) * sizeY()
	);
}

Widget& Widget::select(int ix, int iy){
	clipIndices(ix,iy);
	int iold = selected();
	int inew = data().indexFlat(ix,iy);
	if(iold != inew){
		onCellChange(iold, inew);
		sx=ix; sy=iy;
		mPrevVal = data().at<double>(selected());
	}
	return *this;
}

Widget& Widget::setValueMax(){
	for(int i=0; i<size(); ++i){ setValue(max(), i); } return *this;
}

Widget& Widget::setValueMid(){
	for(int i=0; i<size(); ++i){ setValue(mid(), i); } return *this;
}

} // glv::
