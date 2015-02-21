#include "glv_draw.h"
#include "glv_view3D.h"


namespace glv{

View3D::View3D(const Rect& r)
: View(r), mNear(0.1), mFar(100), mFOVY(45)
{
	for(int j=0; j<4; ++j){
	for(int i=0; i<4; ++i){
		int k = i + j*4; 
		mMatrix[k] = i==j;
	}}
	
//	double rxy = 0.1; // rotates opposite direction because its modelview?
//	mMatrix[0] = cos(rxy);
//	mMatrix[1] =-sin(rxy);
//	mMatrix[4] =-mMatrix[1];
//	mMatrix[5] = mMatrix[0];	
//	mMatrix[12] = 1;
//	mMatrix[13] = 1;
//	mMatrix[14] = 1;
}

void View3D::onDraw(GLV& g){

	using namespace glv::draw;

	// Viewport needs to be in absolute screen coordinates, 
	// so we need to convert from relative...
	space_t ax,ay;
	const View * top = posAbs(ax,ay);

	// y starts at top in GLV, but at bottom in screen coords, so need to flip
	ay = top->h - (ay + h);
	
	// View is outside window, so just return...
	if(ax > top->w || ay > top->h) return;

	// create viewport just at widget location
	glViewport((int)ax, (int)ay, (int)w, (int)h);
	
	// save OpenGL state
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	draw::enable(DepthTest);
	draw::disable(Blend);
	draw::enable(ScissorTest);
	{
		space_t sl=ax, sb=ay, sr=w+sl, st=h+sb;		
		if(sl<0) sl=0;
		if(sr>top->w) sr=top->w;
		if(sb<0) sb=0;
		if(st>top->h) st=top->h;
		scissor(sl, sb, sr-sl, st-sb);
	}

	clearColor(colors().back);
	clear(ColorBufferBit | DepthBufferBit);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(mFOVY, (GLfloat)w/(GLfloat)h, mNear, mFar);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd(mMatrix);

	// Do all 3D drawing
	onDraw3D(g);

	// return OpenGL state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glPopAttrib();
	
	// return viewport to being entire top-level rect
	glViewport(0, 0, (int)top->w, (int)top->h);
	
	// do 2D drawing
	onDraw2D(g);
}

} // glv::

