#ifndef INC_GLV_TEXTURE_H
#define INC_GLV_TEXTURE_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv_conf.h"

namespace glv{

/// 2-D texture
class Texture2{
public:

	/// This constructor will allocate an internal pixel buffer
	Texture2(
		GLsizei width, GLsizei height, 
		GLenum format=GL_RGB, GLenum type=GL_UNSIGNED_BYTE);

	/// Constructor
	Texture2(
		GLsizei width, GLsizei height, GLvoid * pixels=0,
		GLenum format=GL_RGB, GLenum type=GL_UNSIGNED_BYTE,
		bool doesLoad=false);

	virtual ~Texture2();

	/// Get pointer to local texture memory
	template <class T>
	T * buffer() const { return static_cast<T*>(mBuffer); }

	GLuint  id() const { return mID; }
	GLsizei width() const { return w; }		///< Get width
	GLsizei height() const { return h; }	///< Get height

	void begin() const;										///< Bind self to current context
	void end() const;										///< Binds default texture

	/// Allocate local texture memory. 
	
	/// Returns total number of bytes allocated.
	///
	int alloc(int w, int h);
	
	/// Free local texture memory
	void dealloc();

	Texture2& bind();										///< Bind self to current context
	Texture2& draw(											///< Draw texture to rectangular quad
		float ql, float qt, float qr, float qb,
		float tl=0, float tt=1, float tr=1, float tb=0
	);
	
	void destroy();
	
	Texture2& create(GLsizei w, GLsizei h, GLvoid * pixels = 0);	///< Create new texture on graphics card
	Texture2& create();										///< Reload texture onto GPU
	Texture2& send();										///< Send pointed to pixels to GPU

	Texture2& format(GLenum v);								///< Set the color format
	Texture2& type(GLenum v);								///< Set the color data type
	Texture2& magFilter(GLenum v);

private:
	GLuint mID;
	GLsizei w, h;
	GLvoid * mPixels;	// pointer to the client-side pixel data (0 if none)
	GLvoid * mBuffer;	// internally allocated pixel buffer
	GLenum mFormat;		// format of the pixel data:
						//   GL_COLOR_INDEX, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, 
						//   GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_LUMINANCE, and GL_LUMINANCE_ALPHA.
					
	GLenum mType;		// type of the pixel data:
						//   GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT

	GLenum mMagFilter;	// GL_LINEAR, GL_NEAREST

	void sendParams();
};


} //glv::

#endif
