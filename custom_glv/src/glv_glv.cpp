/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"

namespace glv{

GLV::GLV(drawCallback cb, space_t width, space_t height)
:	View(0, 0, width, height, cb), mFocusedView(this)
{
	disable(DrawBorder | FocusHighlight);
//	cloneStyle();
	instances().push_back(this);
}

GLV::~GLV(){ //printf("~GLV\n");
	for(unsigned i=0; i<instances().size(); ++i){
		if(instances()[i] == this){
			instances().erase(instances().begin() + i);
		}
	}
}


void GLV::broadcastEvent(Event::t e){ 

	struct A : TraversalAction{
		GLV& glv; Event::t event;
		A(GLV& g, Event::t e): glv(g), event(e){}
		bool operator()(View * v, int depth){
			glv.doEventCallbacks(*v, event);
			return true;
		}
		bool operator()(const View * v, int depth){ return false; }
	} a(*this, e);
	
	traverseDepth(a);
}



// The bubbling return values from the virtual and function pointer callbacks
// are ANDed together.
bool GLV::doEventCallbacks(View& v, Event::t e){
//	printf("doEventCallbacks: %s %d\n", v.className(), e);

	// TODO: which is better?
//	if(!v.enabled(Controllable)) return false;	// cancels all events w/o handling
	if(!v.enabled(Controllable)) return true;	// bubbles all events w/o handling

//	bool bubble = v.onEvent(e, *this);					// Execute virtual callback
//	
//	if(bubble){
//		if(v.hasCallbacks(e)){
//			const eventCallbackList& cbl = v.callbackLists[e];
//			
//			// Execute callbacks in list
//			for(eventCallbackList::const_iterator it = cbl.begin(); it != cbl.end(); it++){
//				//if(*it) bubble |= (*it)(&v, *this);
//				if(*it){
//					bool r = (*it)(&v, *this);
//					bubble &= r;
//					if(!r) break;
//				}
//			}
//		}
//	}
//	
//	return bubble | v.enabled(AlwaysBubble);

	bool bubble = true;

	if(v.hasCallbacks(e)){
		const eventCallbackList& cbl = v.callbackLists[e];
		
		// Execute callbacks in list
		for(eventCallbackList::const_iterator it = cbl.begin(); it != cbl.end(); it++){
			//if(*it) bubble |= (*it)(&v, *this);
			if(*it){
				bool r = (*it)(&v, *this);
				bubble &= r;
				if(!bubble) goto end;
			}
		}
	}
	
	bubble &= v.onEvent(e, *this);
	end:
	return bubble || v.enabled(AlwaysBubble);
}

void GLV::doFocusCallback(bool get){

	Event::t e = get ? Event::FocusGained : Event::FocusLost;

	if(mFocusedView){
		mFocusedView->focused(get);
		
		if(mFocusedView->numCallbacks(e) ){
			eventType(e);
			doEventCallbacks(*mFocusedView, e);
		}
	}
}

void GLV::drawGLV(unsigned int w, unsigned int h, double dsec){
	glDrawBuffer(GL_BACK);
	drawWidgets(w, h, dsec);
}


static void drawContext(float tx, float ty, View * v, float& cx, float& cy, View *& c){
	using namespace glv::draw;

	cx += tx; cy += ty;	// update absolute coordinates of drawing context

	identity();								// clear model matrix (assumed set already)

	// these weird calls are necessary so that we draw on the center of pixels
	// rather than on the boundaries
	translate(pix(cx) + 0.5, pix(cy) + 0.5);	// offset to center of top-left pixel
	//scale(v->w/(v->w+1), v->h/(v->h+1));	// scale to lose 1 pixel on each edge

	c = v;
}

static void computeCrop(std::vector<Rect>& cr, int lvl, space_t ax, space_t ay, View * v){
	if(v->enabled(CropChildren)){
		cr[lvl].set(ax, ay, v->w, v->h);	// set absolute rect
		
		// get intersection with myself and previous level
		if(lvl>0) cr[lvl].intersection(cr[lvl-1], cr[lvl]);
	}
	
	// if no child cropping, then inherit previous level's crop rect
	else{ cr[lvl] = cr[lvl-1]; }
}

// Views are drawn depth-first from leftmost to rightmost sibling
void GLV::drawWidgets(unsigned int w, unsigned int h, double dsec){
	using namespace draw;

	float cx = 0, cy = 0; // drawing context absolute position
	View * const root = this;
	View * cv = root;

	// The crop region is the intersection of all parent rects up to the top 
	// view. The intersections also need to be done in absolute coordinates.	
	std::vector<Rect> cropRects(16, Rect(w, h));	// index is hierarchy level
	int lvl = 0;	// start at root = 0
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_INDEX_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY); // note: enabling this messes up glColor, so leave it off
	//glColorPointer(4, GL_FLOAT, 0, 0);

	push2D(w, h);	// initialise the OpenGL renderer for our 2D GUI world

	graphicsData().reset();
	if(enabled(Animate)) onAnimate(dsec, *this);
	doDraw(*this);

	push(ModelView);			// push model matrix because of transformations in drawContext()
	
	draw::enable(ScissorTest);

	while(true){

		cv->onDataModelSync();		// update state based on attached model variables
		cv->rectifyGeometry();
	
		// find the next view to draw
		
		// go to child node if exists and I'm drawable
		if(cv->child && cv->visible()){
			drawContext(cv->child->l, cv->child->t, cv->child, cx, cy, cv);
			computeCrop(cropRects, ++lvl, cx, cy, cv);
		}
		
		// go to sibling node if exists
		else if(cv->sibling){
			drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
			computeCrop(cropRects, lvl, cx, cy, cv);
		}
		
		// retrace upwards until a parent's sibling is found
		else{
			while(cv != root && cv->sibling == 0){
				drawContext(-cv->l, -cv->t, cv->parent, cx, cy, cv);
				lvl--;
			}
			
			if(cv->sibling){
				drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
				computeCrop(cropRects, lvl, cx, cy, cv);
			}
			else break; // break the loop when the traversal returns to the root
		}
		
		
		// draw the current view
		if(cv->visible()){

			Rect r = cropRects[lvl-1];	// cropping region comes from parent context
			if(cv->enabled(CropSelf)) r.intersection(Rect(cx, cy, cv->w, cv->h), r); // crop my own draw?

			// LJP: using some weird numbers here, seems to work right though...
			//scissor(r.l, h - r.bottom() - 1, r.w+2, r.h+1);
			scissor(pix(r.l), pix(h - r.bottom() - 1.499), pix(r.w+1), pix(r.h+1.499));

			graphicsData().reset();
			if(cv->enabled(Animate)) cv->onAnimate(dsec, *this);
			cv->doDraw(*this);
		}
	}
	
	glDisableClientState(GL_INDEX_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_COLOR_ARRAY);
	
	pop(ModelView);
	
	// this weird call is necessary so that raster calls get scissored properly
	// not entirely sure why this works, but it does.
	scissor(0,0,(GLint)w,(GLint)h);

	pop2D();
}

std::vector<GLV *>& GLV::instances(){
	static std::vector<GLV *> * sInstances = new std::vector<GLV *>;
	return *sInstances;
}

bool GLV::propagateEvent(){ //printf("GLV::propagateEvent(): %s\n", Event::getName(eventtype));
	View * v = mFocusedView;
	Event::t e = eventType();
	while(v && doEventCallbacks(*v, e)) v = v->parent;
	return v != 0;
}

void GLV::refreshModels(){
	struct Add : TraversalAction{
		Add(ModelManager& v): m(v){}
		bool operator()(View * v, int depth){
			if(v->hasName()) m.add(v->name(), *v);
			return true;
		}
		ModelManager& m;	
	} add(mMM);

	mMM.clearModels();
	traverseDepth(add);
}

void GLV::setFocus(View * v){

	// save current event since we do not want to propagate GetFocus and LoseFocus
	Event::t currentEvent = eventType();	
											
	doFocusCallback(false);	// Call current focused View's LoseFocus callback
	mFocusedView = v;		// Set the currently focused View
	doFocusCallback(true);	// Call newly focused View's GetFocus callback

	eventType(currentEvent);
}

void GLV::setKeyDown(int keycode){
	eventType(Event::KeyDown);
	mKeyboard.mKeycode = keycode;
	mKeyboard.mIsDown = true;
}

void GLV::setKeyUp(int keycode){
	eventType(Event::KeyUp);
	mKeyboard.mKeycode = keycode;
	mKeyboard.mIsDown = false;
}

void GLV::setMouseDown(space_t& x, space_t& y, int button, int clicks){
	eventType(Event::MouseDown);	
	//if(button == Mouse::Left)
		setFocus(findTarget(x, y));
	mMouse.posRel(x,y);
	mMouse.updateButton(button, true, clicks);
}

void GLV::setMouseMotion(space_t& x, space_t& y, Event::t e){
	eventType(e);
	if(absToRel(mFocusedView, x, y)){
		x -= mFocusedView->l;
		y -= mFocusedView->t;
	}
}

void GLV::setMouseMove(space_t& x, space_t& y){
	setMouseMotion(x,y,Event::MouseMove);
}

void GLV::setMouseDrag(space_t& x, space_t& y){
	setMouseMotion(x,y,Event::MouseDrag);
}

void GLV::setMousePos(int x, int y, space_t relx, space_t rely){
	mMouse.pos(x, y);
	mMouse.posRel(relx, rely);
}

void GLV::setMouseUp(space_t& x, space_t& y, int button, int clicks){
	eventType(Event::MouseUp);
	mMouse.updateButton(button, false, clicks);
}

void GLV::setMouseWheel(int wheelDelta){
	eventType(Event::MouseWheel);
	mMouse.bufferPos(mMouse.mW[0] + (space_t)wheelDelta, mMouse.mW);
}

bool GLV::valid(const GLV * g){
	for(unsigned i=0; i<instances().size(); ++i){
		if(instances()[i] == g) return true;
	}
	return false;
}


} // glv::
