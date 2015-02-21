/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <stdio.h>
#include "glv_core.h"
#include "glv_binding.h"

namespace glv{

void Application::run(){
	// This dummy is necessary to detect when the application is closed by an
	// event that is not caught by the windowing implementation such as clicking
	// the close button on the window frame or pressing ctrl- or cmd-q.
	// The dummy's destructor will be called before the program exits and will
	// automatically call Application::quit().
	static Application dummy;	

	implRun();
}

void Application::quit(){
	
	static bool doActions = true;	// ensure that these actions are only done once

	if(doActions){
		doActions = false;
	
		// Send out Quit event to all GLV views
		for(unsigned i=0; i<windows().size(); ++i){
			Window& w = *windows()[i];
			if(w.glv()) w.mGLV->broadcastEvent(Event::Quit);
			//w.onWindowDestroy(); // this also gets called by ~Window
		}
		
		implQuit();
	}
}

std::vector<Window *>& Application::windows(){
	static std::vector<Window *> * v = new std::vector<Window *>;
	return *v;
}



Window::Window(unsigned w, unsigned h, const std::string& title, GLV * glv_, double framerate, int mode)
:	mGLV(0), mFPS(framerate),
	mTitle(title),
	mDispMode(mode),
	mFullScreen(false), mGameMode(false), mHideCursor(false), mIsActive(false)
{

	// Initialize windowing impl when going from 0 to 1 windows
	if(0 == Application::windows().size()){ implInitialize(); }

	Application::windows().push_back(this);

	implCtor(0,0, w,h);

	mIsActive = true;	// assume that window has been successfully created
	if(glv_) setGLV(*glv_);
	setGLVDims(w, h);
}

Window::~Window(){
	
	std::vector<Window *>& wins = Application::windows();
	
	// remove self from application's list
	for(unsigned i=0; i<wins.size(); ++i){
		if(wins[i] == this){
			wins.erase(wins.begin() + i);
			break;
		}
	}
	
	if(0 == wins.size()) implFinalize();
	
	onWindowDestroy();
	implDtor();
}

Window::Dimensions Window::dimensions() const{
	return implWinDims();
}

void Window::dimensions(const Dimensions& d){
	resize(d.w, d.h);
	position(d.l, d.t);
}

void Window::fit(){
	if(glv()){
		mGLV->fit();
		if(mGLV->w > 0 && mGLV->h > 0){
			resize(mGLV->w, mGLV->h);
		}
	}
}

void Window::fps(double v){ mFPS=v; }

void Window::gameModeToggle(){ gameMode(!gameMode()); }

void Window::gameMode(bool on){
	if(on && !gameMode()){		// go fullscreen	
		mWinDims = dimensions();		// store current window dimensions
		mGameMode = true;
		onWindowDestroy();
		mIsActive = false;
		implGameMode();
		mIsActive = true;
		hideCursor(mHideCursor);	// set windowed cursor visibility
		onWindowCreate();
	}
	else if(!on && gameMode()){	// exit fullscreen
		mGameMode = false;
		onWindowDestroy();
		mIsActive = false;
		implGameMode();
		mIsActive = true;
		dimensions(mWinDims);
		setGLVDims(mWinDims.w, mWinDims.h); // ensure GLV gets previous window dimensions
		onWindowCreate();
	}
}

void Window::hide(){ implHide(); }

void Window::hideCursor(bool hide){
	mHideCursor = hide;
	implHideCursor(hide);
}

void Window::iconify(){ implIconify(); }

void Window::fullScreen(bool on){
#ifdef GLV_PLATFORM_UNIX
	mFullScreen=on;
	gameMode(on); // fullscreen leaves borders, so use game mode...
#else
	if(on && !fullScreen()){
		mFullScreen = true;
		mWinDims = dimensions();		// store current window dimensions
		implFullScreen();
	}
	else if(!on && fullScreen()){
		mFullScreen = false;

		dimensions(mWinDims);
	}
#endif
}

void Window::fullScreenToggle(){ fullScreen(!fullScreen()); }

void Window::onWindowCreate(){
	if(active()){
		GLV_PLATFORM_INIT_CONTEXT
		if(glv()) mGLV->broadcastEvent(Event::WindowCreate);
	}
}

void Window::onWindowDestroy(){
	// This checks to make sure our pointed to GLV hasn't been deleted.
	// Sometimes when the program exits, the GLV gets deleted before the Window 
	// leaving a pointer to invalid memory.
	if(!GLV::valid(glv())) return;
	if(glv()) mGLV->broadcastEvent(Event::WindowDestroy);
}

void Window::position(unsigned l, unsigned t){
	implPosition(l,t);
}

void Window::resize(unsigned w, unsigned h){

	//Dimensions d = dimensions();
	//printf("GLUT: %d %d %d %d\n", d.l, d.t, d.w, d.h);
	//printf("Wind: %d %d %d %d\n", mWinDims.l, mWinDims.t, mWinDims.w, mWinDims.h);

	if(width()!=w || height()!=h){
		//printf("resize(%d, %d) call implResize()\n",w,h);
		implResize(w, h);
	}
}

void Window::setGLVDims(unsigned w, unsigned h){//printf("setGLVDims\n");
	if(glv() && (glv()->w != w || glv()->h != h)){//printf("did it\n");
		mGLV->extent(w, h);
		if(active()) mGLV->broadcastEvent(Event::WindowResize);
	}
}

void Window::setGLV(GLV& g){ 
	mGLV = &g;
	onWindowCreate();
	setGLVDims(width(), height());
}

bool Window::shouldDraw(){ return glv() && active() /*&& visible()*/; }

void Window::show(){ implShow(); }

void Window::title(const std::string& v){ mTitle=v; implTitle(); }

} // glv::
