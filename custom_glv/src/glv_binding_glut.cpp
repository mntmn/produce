/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <stdio.h>
#include "glv_binding.h"
#include "glv_core.h"

#if defined (__APPLE__) || defined (OSX)
	#include <GLUT/glut.h>
#endif
#if defined(__linux__)
	#include <GL/glut.h>
#endif
#ifdef WIN32
	#include <GL/glut.h>
#endif

#include <map>

namespace glv {

// GLUT window implementation
class Window::Impl{
public:
	Impl(Window *window, int winID)
	: mWindow(window)
	, mID(winID)
	, mInGameMode(false){
		windows()[mID] = this;
	}
	
	~Impl(){
		windows().erase(mID);
		if(mInGameMode) windows().erase(mIDGameMode);
		glutDestroyWindow(mID);
	}

	void draw(){
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		if(mWindow->shouldDraw()){
			mWindow->mGLV->drawGLV(mWindow->width(), mWindow->height(), mWindow->fps());
			glutSwapBuffers();
		}
	}
	
	void scheduleDraw(){
		scheduleDrawStatic(mInGameMode ? mIDGameMode : mID);
	}
	
	void showing(bool v){ mShowing=v; }
	
	// Returns the implementation of the currently selected window
	static Impl * getWindowImpl(){ return getWindowImpl(glutGetWindow()); }
	
	
	static Impl * getWindowImpl(int id){
		if(windows().count(id) > 0){
			return windows()[id];
		}
		return 0;
	}
	
	// Returns the currently selected window or 0 if invalid
	static Window * getWindow(){
		Impl * w = getWindowImpl();
		if(w) return w->mWindow;
		return 0;
	}
	
	// Returns the currently selected window's GLV or 0 if invalid
	static GLV * getGLV(){
		Window * w = getWindow();
		if(w) return w->mGLV;
		return 0;
	}

private:

	typedef std::map<int, Impl *> WindowsMap;

	// schedule draws of a specific window
	static void scheduleDrawStatic(int winID){
		Impl *impl = getWindowImpl(winID);
		
		// If there is a valid implementation, then draw and schedule next draw...
		if(impl){
			int current = glutGetWindow();
			if(winID != current) glutSetWindow(winID);
			impl->draw();
			glutTimerFunc((unsigned int)(1000.0/getWindow()->fps()), scheduleDrawStatic, winID);
			if(current) glutSetWindow(current);
		}
	}

	// Map of windows constructed on first use to avoid static intialization
	// order problems.
	static WindowsMap& windows(){
		static WindowsMap* ans = new WindowsMap;
		return *ans;
	}

	Window *mWindow;
	int mID;
	int mIDGameMode;
	bool mInGameMode;
	bool mShowing;
    
	friend class Window;
};



void Application::implQuit(){
	exit(0);
}

void Application::implRun(){
	glutMainLoop();	
}



static void glutDisplayCB(){
	// this is empty because we are using a periodic timer for drawing
}

// this must be called whenever a GLUT input event for a keyboard or mouse
// callback is generated.
static void modToGLV(){
	GLV * g = Window::Impl::getGLV();
	if(g){
		int mod = glutGetModifiers();
		const_cast<Keyboard *>(&g->keyboard())->alt  (mod & GLUT_ACTIVE_ALT);
		const_cast<Keyboard *>(&g->keyboard())->ctrl (mod & GLUT_ACTIVE_CTRL);
		const_cast<Keyboard *>(&g->keyboard())->shift(mod & GLUT_ACTIVE_SHIFT);
		//printf("a:%d c:%d s:%d\n", g->keyboard.alt(), g->keyboard.ctrl(), g->keyboard.shift());
	}
}

static void keyToGLV(unsigned int key, bool down, bool special){
	//printf("GLUT: keyboard event k:%d d:%d s:%d\n", key, down, special);

	GLV * g = Window::Impl::getGLV();
	if(g){
		if(special){

			#define CS(glut, glv) case GLUT_KEY_##glut: key = Key::glv; break;
			switch(key){
				CS(LEFT, Left) CS(UP, Up) CS(RIGHT, Right) CS(DOWN, Down)
				CS(PAGE_UP, PageUp) CS(PAGE_DOWN, PageDown)
				CS(HOME, Home) CS(END, End) CS(INSERT, Insert)
			
				CS(F1, F1) CS(F2, F2) CS(F3, F3) CS(F4, F4)
				CS(F5, F5) CS(F6, F6) CS(F7, F7) CS(F8, F8)
				CS(F9, F9) CS(F10, F10)	CS(F11, F11) CS(F12, F12)
			}
			#undef CS
		}
		else{
		
			//printf("GLUT i: %3d %c\n", key, key);

			#define MAP(i,o) case i: key=o; break

//			bool shft = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
//			if(shft && (key>32 && key<127)){
//				const char * QWERTYunshifted = 
//					" 1\'3457\'908=,-./0123456789;;,=./"
//					"2abcdefghijklmnopqrstuvwxyz[\\]6-"
//					"`abcdefghijklmnopqrstuvwxyz[\\]`"
//				;
//				key = QWERTYunshifted[key-32];
//			}

			// Reassign keycodes when CTRL is down
			#ifdef GLV_PLATFORM_OSX

			bool ctrl = glutGetModifiers() & GLUT_ACTIVE_CTRL;

			if(ctrl){
				// All alphabetical keys get dropped to lower ASCII range.
				// Some will conflict with standard non-printable characters.
				// There is no way to detect this, since the control modified
				// keycode gets sent to the GLUT callback. We will assume that
				// ctrl-key events are the printable character keys.

				//Enter		=3
				//BackSpace	=8
				//Tab		=9
				//Return	=13
				//Escape	=27
				
				if(key <= 26){ key += 96; }
				
				// only some non-alphabetic keys are wrong...
				else{
					
					switch(key){
						MAP(27, '[');
						MAP(28, '\\');
						MAP(29, ']');
						MAP(31, '-');
					};
					
				}
			}
			
			#endif
			
			#undef MAP

			//printf("GLUT o: %3d %c\n", key, key);
		}
		
		down ? g->setKeyDown(key) : g->setKeyUp(key);
		modToGLV();
		g->propagateEvent();
	}
}



static void glutKeyboardCB(unsigned char key, int x, int y){ keyToGLV(key, true, false); }
static void glutKeyboardUpCB(unsigned char key, int x, int y){ keyToGLV(key, false, false); }
static void glutSpecialCB(int key, int x, int y){ keyToGLV(key, true, true); }
static void glutSpecialUpCB(int key, int x, int y){ keyToGLV(key, false, true); }

static void glutVisibilityCB(int v){
	Window::Impl * w = Window::Impl::getWindowImpl();
	if(w){
		w->showing(v == GLUT_VISIBLE);
	}
}

static void glutMouseCB(int btn, int state, int ax, int ay){
	//printf("GLUT: mouse event x:%d y:%d bt:#%d,%d\n", ax,ay, btn, state==GLUT_DOWN);
	GLV * g = Window::Impl::getGLV();
	if(g){
		space_t x = (space_t)ax;
		space_t y = (space_t)ay;
		space_t relx = x;
		space_t rely = y;

		switch(btn){
			case GLUT_LEFT_BUTTON:		btn = Mouse::Left; break;
			case GLUT_MIDDLE_BUTTON:	btn = Mouse::Middle; break;
			case GLUT_RIGHT_BUTTON:		btn = Mouse::Right; break;
			default:					btn = Mouse::Extra;		// unrecognized button
		}

		if(GLUT_DOWN == state)		g->setMouseDown(relx, rely, btn, 0);
		else if(GLUT_UP == state)	g->setMouseUp  (relx, rely, btn, 0);

		g->setMousePos((int)x, (int)y, relx, rely);
		modToGLV();
		g->propagateEvent();
	}
}

static void motionToGLV(int ax, int ay, glv::Event::t e){
	GLV * g = Window::Impl::getGLV();
	if(g){
		space_t x = (space_t)ax;
		space_t y = (space_t)ay;
		space_t relx = x;
		space_t rely = y;

		g->setMouseMotion(relx, rely, e);
		g->setMousePos((int)x, (int)y, relx, rely);
		//modToGLV();	// GLUT complains about calling glutGetModifiers()
		g->propagateEvent();
	}
}

static void glutMotionCB(int ax, int ay){
	//printf("GLUT: motion event x:%d y:%d\n", ax, ay);
	motionToGLV(ax, ay, Event::MouseDrag);
}

static void glutPassiveMotionCB(int ax, int ay){
	//printf("GLUT: passive motion event x:%d y:%d\n", ax, ay);
	motionToGLV(ax, ay, Event::MouseMove);
}

static void glutReshapeCB(int w, int h){
	//printf("glutReshapeCB(%d, %d)\n", w, h);

	// This callback should only set the GLV's dimensions.
	Window * win = Window::Impl::getWindow();
	//if(win) win->resize(w, h);
	if(win) win->setGLVDims(w, h);
}

static void registerCBs(){
	glutDisplayFunc(glutDisplayCB);
	glutKeyboardFunc(glutKeyboardCB);
	glutKeyboardUpFunc(glutKeyboardUpCB);
	glutMouseFunc(glutMouseCB);
	glutMotionFunc(glutMotionCB);
	glutPassiveMotionFunc(glutPassiveMotionCB);
	glutReshapeFunc(glutReshapeCB);
	glutSpecialFunc(glutSpecialCB); 
	glutSpecialUpFunc(glutSpecialUpCB);
	glutVisibilityFunc(glutVisibilityCB);
}




void Window::implCtor(unsigned l, unsigned t, unsigned w, unsigned h){

    glutInitWindowSize(w, h);
    glutInitWindowPosition(0, 0);

    int bits = 
        (enabled(SingleBuf )	? GLUT_SINGLE		:0) |
        (enabled(DoubleBuf )	? GLUT_DOUBLE		:0) |
        (enabled(AccumBuf  )	? GLUT_ACCUM		:0) |
        (enabled(AlphaBuf  )	? GLUT_ALPHA		:0) |
        (enabled(DepthBuf  )	? GLUT_DEPTH		:0) |
        (enabled(StencilBuf)	? GLUT_STENCIL		:0) |
        (enabled(Stereo    )	? GLUT_STEREO		:0) |
		(enabled(Multisample)	? GLUT_MULTISAMPLE	:0);

	glutInitDisplayMode(GLUT_RGBA | bits);

//	int stat = glutGet(GLUT_DISPLAY_MODE_POSSIBLE);
//	printf("%d\n", stat);

	int winID = glutCreateWindow(mTitle.c_str());
	glutSetWindow(winID);
    glutIgnoreKeyRepeat(1);

    //mImpl.reset(new WindowImpl(this, winID));
	mImpl = new Impl(this, winID);

	registerCBs();
	mImpl->scheduleDraw();
}

void Window::implDtor(){
	if(mImpl){ delete mImpl; mImpl=0; }
}

void Window::implFinalize(){}	// no cleanup with GLUT

void Window::implFullScreen(){
	if(fullScreen()){ glutFullScreen();}
}

void Window::implGameMode(){

	// Go into game mode
	if(gameMode()){

//		"width=1024 height=768 bpp=24 hertz=60"

//	=       Equal.
//	!=      Not equal.
//	<       Less  than  and  preferring larger difference (the
//			least is best).
//	>       Greater than  and  preferring  larger  differences
//			(the most is best).
//	<=      Less  than  or equal and preferring larger differ-
//			ence (the least is best).
//	>=      Greater than or equal and preferring more  instead
//			of less.  This comparator is useful for allocating
//			resources like color precsion or depth buffer pre-
//			cision  where  the  maximum  precison is generally
//			preferred.  Contrast with the tilde (~) comprator.
//	~       Greater  than or equal but preferring less instead
//			of more.  This compartor is useful for  allocating
//			resources  such as stencil bits or auxillary color
//			buffers where you would rather not over  allocate.

//		bpp     Bits per pixel for the frame buffer.
//		height  Height of the screen in pixels.
//		hertz   Video refresh rate of the screen in hertz.
//		num     Number  of  the  window  system depenedent display
//				mode configuration.
//		width   Width of the screen in pixels.
	
//		compact mode [ width "x" height ][ ":" bitsPerPixel ][ "@" videoRate ]

		// get current screen resolution
		int sw = glutGet(GLUT_SCREEN_WIDTH);
		int sh = glutGet(GLUT_SCREEN_HEIGHT);

		// use current resolution and refresh rate
		if(sw && sh){
			char buf[32];
			snprintf(buf, sizeof(buf), "%dx%d:24", sw, sh);
			glutGameModeString(buf);
			
			//int refresh = glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE);
			//printf("%d\n", refresh);
		}
		
		// otherwise, use sensible defaults
		else{
			glutGameModeString("1024x768:24");
		}

		mImpl->mIDGameMode = glutEnterGameMode();
		Window::Impl::windows()[mImpl->mIDGameMode] = mImpl;
		mImpl->mInGameMode = true;
		registerCBs();
		mImpl->scheduleDraw();
	}
	
	// Exit game mode
	else{
		Window::Impl::windows().erase(mImpl->mIDGameMode);
		mImpl->mInGameMode = false;
		glutSetWindow(mImpl->mID); // freeglut requires this before leaving game mode
		glutLeaveGameMode();
	}
}

void Window::implHide(){ glutHideWindow(); }

void Window::implHideCursor(bool v){
	glutSetCursor(v ? GLUT_CURSOR_NONE : GLUT_CURSOR_INHERIT);
}

void Window::implIconify(){ glutIconifyWindow(); }

void Window::implInitialize(){
//	int argc = 0;
//	char * argv[] = {0};
//	glutInit(&argc,argv);
	int   argc   = 1;
	char *argv[] = {"MyApp", NULL};
	glutInit(&argc,argv);
}

void Window::implPosition(unsigned l, unsigned t){
	glutPositionWindow((int)l, (int)t);
}

void Window::implResize(unsigned w, unsigned h){
	//printf("implResize call glutReshapeWindow\n");
	glutReshapeWindow((int)w, (int)h); // this will call the reshape callback
}

void Window::implShow(){ glutShowWindow(); }

bool Window::implShowing() const {
	return mImpl->mShowing;
}

void Window::implTitle(){
	glutSetWindow(mImpl->mID);
	glutSetWindowTitle(mTitle.c_str());
}

Window::Dimensions Window::implWinDims() const{
	Dimensions d;
	d.l = glutGet(GLUT_WINDOW_X);
	d.t = glutGet(GLUT_WINDOW_Y);
	d.w = glutGet(GLUT_WINDOW_WIDTH);
	d.h = glutGet(GLUT_WINDOW_HEIGHT);
	return d;
}

} // glv::
