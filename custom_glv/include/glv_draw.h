#ifndef INC_GLV_DRAW_H
#define INC_GLV_DRAW_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <math.h>
#include <stdio.h>
#include "glv_conf.h"
#include "glv_color.h"

#define GLV_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

namespace glv {

/// Icon function type
typedef void (* iconFunc)(float l, float t, float r, float b);

/// Two-dimensional point
struct Point2{
	Point2(){}
	Point2(float x_, float y_): x(x_), y(y_){}
	void operator()(float x_, float y_){x=x_; y=y_;}
	union{
		struct{ float x,y; };
		float elems[2];
	};
};

/// Three-dimensional point
struct Point3{
	Point3(){}
	Point3(float x_, float y_, float z_): x(x_), y(y_), z(z_){}
	void operator()(float x_, float y_, float z_){x=x_; y=y_; z=z_;}
	union{
		struct{ float x,y,z; };
		float elems[3];
	};
};


/// Buffers of vertices, colors, and indices
class GraphicsData{
public:

	GraphicsData(): mColors(1){}

	/// Get color buffer
	const Buffer<Color>& colors() const { return mColors; }
	
	/// Get index buffer
	const Buffer<unsigned>& indices() const { return mIndices; }
	
	/// Get 2D vertex buffer
	const Buffer<Point2>& vertices2() const { return mVertices2; }
	
	/// Get 3D vertex buffer
	const Buffer<Point3>& vertices3() const { return mVertices3; }

	/// Reset all buffers
	void reset(){
		mVertices2.reset(); mVertices3.reset();
		mColors.reset(); mIndices.reset();
	}

	/// Append color
	void addColor(const Color& c){
		colors().append(c); }

	/// Append colors
	void addColor(const Color& c1, const Color& c2){
		addColor(c1); addColor(c2); }

	/// Append colors
	void addColor(const Color& c1, const Color& c2, const Color& c3){
		addColor(c1,c2); addColor(c3); }

	/// Append colors
	void addColor(const Color& c1, const Color& c2, const Color& c3, const Color& c4){
		addColor(c1,c2,c3); addColor(c4); }

	/// Append index
	void addIndex(unsigned i){
		indices().append(i); }

	/// Append indices
	void addIndex(unsigned i1, unsigned i2){
		addIndex(i1); addIndex(i2); }

	/// Append indices
	void addIndex(unsigned i1, unsigned i2, unsigned i3){
		addIndex(i1,i2); addIndex(i3); }

	/// Append indices
	void addIndex(unsigned i1, unsigned i2, unsigned i3, unsigned i4){
		addIndex(i1,i2,i3); addIndex(i4); }

	/// Append 2D vertex
	void addVertex(double x, double y){ addVertex2(x,y); }

	/// Append 3D vertex
	void addVertex(double x, double y, double z){ addVertex3(x,y,z); }

	/// Append 2D vertex
	void addVertex2(double x, double y){
		vertices2().append(Point2(x,y)); }

	/// Append 2D vertices
	void addVertex2(double x1, double y1, double x2, double y2){
		addVertex2(x1,y1); addVertex2(x2,y2); }

	/// Append 2D vertices
	void addVertex2(double x1, double y1, double x2, double y2, double x3, double y3){
		addVertex2(x1,y1,x2,y2); addVertex2(x3,y3); }

	/// Append 2D vertices
	void addVertex2(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4){
		addVertex2(x1,y1,x2,y2,x3,y3); addVertex2(x4,y4); }

	/// Append 2D vertex
	template <class VEC2>
	void addVertex2(const VEC2& v){ addVertex2(v[0], v[1]); }

	/// Append 3D vertex
	void addVertex3(double x, double y, double z){ vertices3().append(Point3(x,y,z)); }

	/// Append 3D vertex
	template <class VEC3>
	void addVertex3(const VEC3& v){ addVertex3(v[0], v[1], v[2]); }

	/// Get mutable color buffer
	Buffer<Color>& colors(){ return mColors; }

	/// Get mutable index buffer
	Buffer<unsigned>& indices(){ return mIndices; }

	/// Get mutable 2D vertex buffer
	Buffer<Point2>& vertices2(){ return mVertices2; }

	/// Get mutable 3D vertex buffer
	Buffer<Point3>& vertices3(){ return mVertices3; }

protected:
	Buffer<Point2> mVertices2;
	Buffer<Point3> mVertices3;
	Buffer<Color> mColors;
	Buffer<unsigned> mIndices;
};



/// Drawing routines
namespace draw{

const double C_PI = 4. * atan(1.);
const double C_2PI = 2. * C_PI;

#ifdef check
	#undef check
#endif

/// Primitives
enum{
	Points        = GL_POINTS,
	Lines         = GL_LINES,
	LineStrip     = GL_LINE_STRIP,
	LineLoop      = GL_LINE_LOOP,
	Triangles     = GL_TRIANGLES,
	TriangleStrip = GL_TRIANGLE_STRIP,
	TriangleFan   = GL_TRIANGLE_FAN,
#ifdef GLV_USE_OPENGL_ES
	Quads         = GL_POINTS,
	QuadStrip     = GL_TRIANGLE_STRIP,
#else
	Quads         = GL_QUADS,
	QuadStrip     = GL_QUAD_STRIP,
#endif
};

/// Capabilities (for disable() and enable())
enum{
	Blend			= GL_BLEND,
	DepthTest		= GL_DEPTH_TEST,
	Fog				= GL_FOG,
	LineSmooth		= GL_LINE_SMOOTH,
	LineStipple		= GL_LINE_STIPPLE,
	PolygonSmooth	= GL_POLYGON_SMOOTH,
	PointSmooth		= GL_POINT_SMOOTH,
	ScissorTest		= GL_SCISSOR_TEST,
	Texture2D		= GL_TEXTURE_2D
};

/// Attribute masks
enum{
	ColorBufferBit	= GL_COLOR_BUFFER_BIT,
	DepthBufferBit	= GL_DEPTH_BUFFER_BIT,
	EnableBit		= GL_ENABLE_BIT,
	ViewPortBit		= GL_VIEWPORT_BIT
};

/// Transform matrices
enum{
	ModelView		= GL_MODELVIEW,
	Projection		= GL_PROJECTION,
	Texture			= GL_TEXTURE
};


// Basic rendering commands
void blendFunc(int sfactor, int dfactor);			///< Set blending function
void blendTrans();									///< Set blending function to transparent
void blendAdd();									///< Set blending function to additive
void clear(int mask);								///< Clear drawing buffers
void clearColor(float r, float g, float b, float a=1);	///< Set clear color
void clearColor(const Color& c);					///< Set clear color
void color(float gray, float a=1);					///< Set current draw color
void color(float r, float g, float b, float a=1);		///< Set current draw color
void color(const Color& c);							///< Set current draw color
void color(const Color& c, float a);				///< Set current draw color, but override alpha component
void fog(float end, float start, const Color& c=Color(0));	///< Set linear fog parameters
void genEllipse(Point2 * pts, int num, int stride, double ang01, double loops, float l, float t, float r, float b);
void identity();									///< Load identity transform matrix
void lineStipple(char factor, short pattern);		///< Specify line stipple pattern
void lineWidth(float val);							///< Set width of lines
void matrixMode(int mode);							///< Set current transform matrix
void ortho(float l, float r, float b, float t);		///< Set orthographic projection mode
void paint(int prim, Point2 * verts, int numVerts);	///< Draw array of 2D vertices
void paint(int prim, const GraphicsData& gb);		///< Render graphics data
void paint(int prim, Point2 * verts, Color * cols, int numVerts);
void paint(int prim, Point2 * verts, unsigned * indices, int numIndices); ///< Draw indexed array of 2D vertices
void paint(int prim, Point2 * verts, Color * cols, unsigned * indices, int numIndices); 
void paint(int prim, Point3 * verts, int numVerts);	///< Draw array of 3D vertices
void paint(int prim, Point3 * verts, Color * cols, int numVerts);
void paint(int prim, Point3 * verts, unsigned * indices, int numIndices); ///< Draw indexed array of 3D vertices
void paint(int prim, Point3 * verts, Color * cols, unsigned * indices, int numIndices); 
void pointSize(float val);							///< Set size of points
void pointAtten(float c2=0, float c1=0, float c0=1); ///< Set distance attenuation of points. The scaling formula is clamp(size * sqrt(1/(c0 + c1*d + c2*d^2)))
void pop();											///< Pop current transform matrix stack
void pop(int matrixMode);							///< Pop a transform matrix stack also setting as current matrix
void pop2D();										///< Pop 2-D pixel space
void pop3D();										///< Pop 3-D signed normalized cartesian space
void popAttrib();									///< Pop last pushed attributes from stack
int printError(const char * pre="", bool verbose=true, FILE * out=stdout); ///< Print rendering errors to file
void push();										///< Push current transform matrix stack 
void push(int matrixMode);							///< Push a transform matrix stack also setting as current matrix
void push2D(float w, float h);						///< Push 2-D pixel space
void push3D(float w, float h, float near=0.1, float far=100, float fovy=45);	///< Push 3-D signed normalized cartesian space
void pushAttrib(int attribs);						///< Push current attributes onto stack
void rotate(float degx, float degy, float degz);
void rotateX(float deg);
void rotateY(float deg);
void rotateZ(float deg);
void scale(float v);								///< Scale all dimensions by amount
void scale(float x, float y, float z=1.f);
void scissor(float x, float y, float w, float h);
void stroke(float w);								///< Sets width of lines and points
void texCoord(float x, float y);
void translate(float x, float y, float z=0.f);
void translateX(float x);
void translateY(float y);
void translateZ(float z);
void viewport(float x, float y, float w, float h);



// icons
void check		(float l, float t, float r, float b);	///< Check mark
template<int N>
void circle		(float l, float t, float r, float b);	///< Circle with N edges
template<int N>
void disc		(float l, float t, float r, float b);	///< Disc with N edges
void frame		(float l, float t, float r, float b);	///< Rectangular frame
void minus		(float l, float t, float r, float b);	///< Minus
template <int N, int M, int A>
void polygon	(float l, float t, float r, float b);	///< Regular polygon with N edges, M loops, and angle A
void plus		(float l, float t, float r, float b);	///< Plus
void rectangle	(float l, float t, float r, float b);	///< Solid rectangle
template<int N, int M, int L>
void rose		(float l, float t, float r, float b);	///< Rose curve with N edges and M+L loops
template<int N, int A>
void spokes		(float l, float t, float r, float b);	///< N spokes rotated by angle A
void triangleR	(float l, float t, float r, float b);	///< Right pointing triangle
void triangleL	(float l, float t, float r, float b);	///< Left pointing triangle
void triangleU	(float l, float t, float r, float b);	///< Upward pointing triangle
void triangleD	(float l, float t, float r, float b);	///< Downward pointing triangle
void x			(float l, float t, float r, float b);	///< X mark


/// Parallel horizontal and vertical lines
void grid(GraphicsData& g, float l, float t, float w, float h, float divx, float divy, bool incEnds=true);

/// Converts a float to its pixel value.
inline int pix(float v);

/// Draw vertices using primitive
void shape(int prim, float x0, float y0, float x1, float y1);

/// Draw vertices using primitive
void shape(int prim, float x0, float y0, float x1, float y1, float x2, float y2);

/// Draw vertices using primitive
void shape(int prim, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);

/// Draws a text string, including new lines and tabs.
void text(const char * s, float l=0, float t=0, unsigned fontSize=8, float lineSpacing=1, unsigned tabSpaces=4);



/// Disables rendering capabilities
struct Disable{

	Disable(){}
	Disable(int cap){ *this << cap; }
	Disable(int cap1, int cap2){ *this << cap1 << cap2; }

	/// Disable a capability
	const Disable& operator()(int cap) const { return *this << cap; }
	const Disable& operator()(int cap1, int cap2) const { return *this << cap1<<cap2; }
	const Disable& operator()(int cap1, int cap2, int cap3) const { return *this << cap1<<cap2<<cap3; }
	
	/// Disable a capability
	const Disable& operator<< (int cap) const { glDisable(cap); return *this; }
};

/// Global Disable functor
static Disable disable;



/// Enables rendering capabilities
struct Enable{

	Enable(){}
	Enable(int cap){ *this << cap; }
	Enable(int cap1, int cap2){ *this << cap1 << cap2; }

	/// Enable a capability
	const Enable& operator()(int cap) const { return *this << cap; }
	const Enable& operator()(int cap1, int cap2) const { return *this << cap1<<cap2; }
	const Enable& operator()(int cap1, int cap2, int cap3) const { return *this << cap1<<cap2<<cap3; }
	
	/// Enable a capability
	const Enable& operator<< (int cap) const { glEnable(cap); return *this; }
};

/// Global Enable functor
static Enable enable;





// Implementation ______________________________________________________________

inline void check(float l, float t, float r, float b){ shape(LineStrip, l,0.5*(t+b), l+(r-l)*0.3,b, r,t); }

template<int N>
inline void circle(float l, float t, float r, float b){ return polygon<N,1,1>(l,t,r,b); }

template <int N>
void disc(float l, float t, float r, float b){
	Point2 pts[N+2];
	genEllipse(pts+1,N,1, 0, 1, l,t,r,b);
	pts[0](0.5*(l+r), 0.5*(t+b));
	pts[N+1] = pts[1];
	paint(TriangleFan, pts, GLV_ARRAY_SIZE(pts));
}

inline void frame(float l, float t, float r, float b){ shape(LineLoop, l, t, l, b, r, b, r, t); }
inline void minus(float l, float t, float r, float b){ float my=0.5*(t+b); shape(Lines, l,my, r,my); }

template <int N, int M, int A>
void polygon(float l, float t, float r, float b){
	Point2 pts[N];
	genEllipse(pts,N,1, A/360.,M, l,t,r,b);
	paint(LineLoop, pts, GLV_ARRAY_SIZE(pts));
}

template<int N, int A>
void spokes(float l, float t, float r, float b){
	int N2 = N<<1;
	Point2 pts[N2];
	float cx = (l+r)/2;
	float cy = (t+b)/2;
	genEllipse(pts+1,N,2, A/360.,1, l,t,r,b);
	for(int i=0; i<N2; i+=2) pts[i](cx,cy);
	paint(Lines, pts, GLV_ARRAY_SIZE(pts));
}


inline void paint(int prim, Point2 * verts, int numVerts){
	//glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(prim, 0, numVerts);
	//glDisableClientState(GL_VERTEX_ARRAY);
}

inline void paint(int prim, Point2 * verts, Color * cols, int numVerts){
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(4, GL_FLOAT, 0, cols);
	glDrawArrays(prim, 0, numVerts);
	glDisableClientState(GL_COLOR_ARRAY);
}

inline void paint(int prim, Point2 * verts, unsigned * indices, int numIndices){
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawElements(prim, numIndices, GL_UNSIGNED_INT, indices);
}

inline void paint(int prim, Point2 * verts, Color * cols, unsigned * indices, int numIndices){
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(4, GL_FLOAT, 0, cols);
	glDrawElements(prim, numIndices, GL_UNSIGNED_INT, indices);
	glDisableClientState(GL_COLOR_ARRAY);
}

inline void paint(int prim, Point3 * verts, int numVerts){
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(prim, 0, numVerts);
}

inline void paint(int prim, Point3 * verts, Color * cols, int numVerts){
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glColorPointer(4, GL_FLOAT, 0, cols);
	glDrawArrays(prim, 0, numVerts);
	glDisableClientState(GL_COLOR_ARRAY);
}

inline void paint(int prim, Point3 * verts, unsigned * indices, int numIndices){
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawElements(prim, numIndices, GL_UNSIGNED_INT, indices);
}

inline void paint(int prim, Point3 * verts, Color * cols, unsigned * indices, int numIndices){
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glColorPointer(4, GL_FLOAT, 0, cols);
	glDrawElements(prim, numIndices, GL_UNSIGNED_INT, indices);
	glDisableClientState(GL_COLOR_ARRAY);
}

// [-2,-1) -> -1.5
// [-1, 0) -> -0.5
// [ 0, 1) ->  0.5
// [ 1, 2) ->  1.5
inline int pix(float v){ return v>=0 ? (int)(v+0.5) : (int)(v-0.5); }

inline void plus(float l, float t, float r, float b){
	float mx = 0.5*(l+r);
	float my = 0.5*(t+b);
	shape(Lines, mx,t, mx,b, l,my, r,my);
}

template<int N, int M, int L>
void rose(float l, float t, float r, float b){
	Point2 pts1[N], pts2[N];
	genEllipse(pts1,N,1, -0.25, M, l,t,r,b);
	genEllipse(pts2,N,1, -0.25, L, l,t,r,b);
	for(int i=0; i<N; ++i){
		pts1[i].x = (pts1[i].x+pts2[i].x)*0.5;
		pts1[i].y = (pts1[i].y+pts2[i].y)*0.5;
	}
	paint(LineLoop, pts1,N);
}

//template<int N, int M>
//inline void star(float l, float t, float r, float b){ pgon(l,t,r-l,b-t,N,-0.25,M); }

inline void shape(int prim, float x0, float y0, float x1, float y1){
	float v[] = {x0,y0,x1,y1};
	paint(prim, (Point2 *)v, 2);
}

inline void shape(int prim, float x0, float y0, float x1, float y1, float x2, float y2){
	float v[] = {x0,y0,x1,y1,x2,y2};
	paint(prim, (Point2 *)v, 3);
}

inline void shape(int prim, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3){
	float v[] = {x0,y0,x1,y1,x2,y2,x3,y3};
	paint(prim, (Point2 *)v, 4);
}

inline void rectangle(float l, float t, float r, float b){ shape(TriangleStrip, l,t, l,b, r,t, r,b); }
inline void stroke(float w){ lineWidth(w); pointSize(w); }
inline void triangleD(float l, float t, float r, float b){ shape(Triangles, 0.5*(l+r),b, r,t, l,t); }
inline void triangleL(float l, float t, float r, float b){ shape(Triangles, l,0.5*(t+b), r,b, r,t); }
inline void triangleR(float l, float t, float r, float b){ shape(Triangles, r,0.5*(t+b), l,t, l,b); }
inline void triangleU(float l, float t, float r, float b){ shape(Triangles, 0.5*(l+r),t, l,b, r,b); }
inline void x(float l, float t, float r, float b){ shape(Lines, l,t, r,b, l,b, r,t); }


// core drawing routines
inline void clearColor(const Color& c){ clearColor(c.r, c.g, c.b, c.a); }
inline void color(float v, float a){ color(v,v,v,a); }
inline void color(const Color& c){ color(c.r, c.g, c.b, c.a); }
inline void color(const Color& c, float a){ color(c.r, c.g, c.b, a); }
inline void pop(int mode){ matrixMode(mode); pop(); }
inline void push(int mode){ matrixMode(mode); push(); }
inline void rotate(float degx, float degy, float degz){	rotateX(degx); rotateY(degy); rotateZ(degz); }
inline void scale(float v){ scale(v,v,v); }
inline void translateX(float x){ translate(x, 0, 0); }
inline void translateY(float y){ translate(0, y, 0); }
inline void translateZ(float z){ translate(0, 0, z); }


// platform dependent
inline void blendFunc(int sfactor, int dfactor){ glBlendFunc(sfactor, dfactor); }
inline void blendAdd(){ glBlendEquation(GL_FUNC_ADD); blendFunc(GL_SRC_COLOR, GL_ONE); }
inline void blendTrans(){ glBlendEquation(GL_FUNC_ADD); blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
inline void clear(int mask){ glClear(mask); }
inline void clearColor(float r, float g, float b, float a){ glClearColor(r,g,b,a); }
inline void color(float r, float g, float b, float a){ glColor4f(r,g,b,a); }
inline void identity(){ glLoadIdentity(); }
inline void lineStipple(char factor, short pattern){ glLineStipple(factor, pattern); }
inline void lineWidth(float v){ glLineWidth(v); }
inline void matrixMode(int mode){ glMatrixMode(mode); }
inline void ortho(float l, float r, float b, float t){ gluOrtho2D(l,r,b,t); }
inline void pointSize(float v){ glPointSize(v); }
inline void pointAtten(float c2, float c1, float c0){
	GLfloat att[3] = {c0, c1, c2};
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
}
inline void push(){ glPushMatrix(); }
inline void pushAttrib(int attribs){ glPushAttrib(attribs); }
inline void pop() { glPopMatrix(); }
inline void popAttrib(){ glPopAttrib(); }
inline void rotateX(float deg){ glRotatef(deg, 1.f, 0.f, 0.f); }
inline void rotateY(float deg){ glRotatef(deg, 0.f, 1.f, 0.f); }
inline void rotateZ(float deg){ glRotatef(deg, 0.f, 0.f, 1.f); }
inline void scale(float x, float y, float z){ glScalef(x,y,z); }
inline void scissor(float x, float y, float w, float h){ glScissor((GLint)x,(GLint)y,(GLsizei)w,(GLsizei)h); }
inline void texCoord(float x, float y){ glTexCoord2f(x,y); }
inline void translate(float x, float y, float z){ glTranslatef(x,y,z); }
inline void viewport(float x, float y, float w, float h){ glViewport((GLint)x,(GLint)y,(GLsizei)w,(GLsizei)h); }

} // draw::

} // glv::

#endif

