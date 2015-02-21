/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_core.h"

namespace glv{

Keyboard::Keyboard()
:	mKeycode(-1), mIsDown(false)
{
	for(int i=0; i<5; ++i) mModifiers[i] = false;
}

void Keyboard::print(){
	printf("code:%3d (%c) down:%d a:%d C:%d c:%d m:%d s:%d\n", 
		key(), key(), isDown(), alt(), caps(), ctrl(), meta(), shift());
}



Mouse::Mouse()
:	mXRel(0), mYRel(0), mButton(0), mClicks(0), mIsDown(false)
{
	mX[0] = mX[1] = mX[2] = mY[0] = mY[1] = mY[2] = mW[0] = mW[1] = mW[2] = 0;
	
	for (int i=0; i<GLV_MAX_MOUSE_BUTTONS; i++){
		b[i] = false;
		bx[i] = by[i] = bxRel[i] = byRel[i] = 0;
	}
}

void Mouse :: print() const {
	printf("abspos: [%i, %i]\n", (int)x(), (int)y());
	printf("relpos: [%f, %f]\n", (float)xRel(), (float)yRel());
	printf("dpos:   [%i, %i]\n", (int)dx(), (int)dy());
	printf("ddpos:  [%i, %i]\n", (int)ddx(), (int)ddy());
	printf("button: ");
	for(int i=0; i<GLV_MAX_MOUSE_BUTTONS; i++){
		printf(b[i] ? "|" : ".");
	}
	for(int i=0; i<GLV_MAX_MOUSE_BUTTONS; i++){
		printf("\nbpos%i: [%i, %i]", i, (int)bx[i], (int)by[i]);
	}
	printf("\n\n");
}

void Mouse::updateButton(int btn, bool pressed, int clicks){
	if(btn >= 0 && btn < GLV_MAX_MOUSE_BUTTONS) {
		mButton = btn;
		mClicks = clicks;
		mIsDown = b[mButton] = pressed;
		bx[mButton] = x();
		by[mButton] = y();
		bxRel[mButton] = xRel();
		byRel[mButton] = yRel();
	}
}

} // end namespace glv
