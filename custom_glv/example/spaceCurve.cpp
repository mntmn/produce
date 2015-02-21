/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "example.h"

const int N = 3600;

struct Scene : View3D{

	Scene()
	:	spin(0),
		gui("<<<, >--, >--"),
		freqX(20,0,0, 4,0, N/2,-N/2),
		freqY(20,0,0, 4,0, N/2,-N/2),
		freqZ(20,0,0, 4,0, N/2,-N/2)
	{
		freqX.setValue(1); freqY.setValue(2); freqZ.setValue(3);
		stretch(1,1);
		disable(DrawBorder);
		gui << freqX << freqY << freqZ;
		gui.arrange();
		
		tz.extent(gui.width() - 2*gui.padX(), 20);

		prim.add("Points").add("LineLoop").add("Triangles").add("TriangleStrip");

		gui << tz << prim;
		gui.arrange();
		
		(*this) << gui;
	}

	virtual void onDraw3D(GLV& g){
		using namespace glv::draw;
		
		float fx = freqX.getValue();
		float fy = freqY.getValue();
		float fz = freqZ.getValue();
		
		for(int i=0; i<N; ++i){
			float f = float(i)/N;
			float p = f * 2*M_PI;
			
			float x = cos(fx*p);
			float y = sin(fy*p);
			float z = sin(fz*p);
			
			vertices[i](x, y, z);
			colors[i] = HSV(0.6, f, z*0.45+0.55);
		}

		if((spin+=0.5) > 360) spin -= 360;

		translateZ(tz.getValue()*4-4);
		rotateY(spin);
		stroke(2);
		
		int prims[] = {Points, LineLoop, Triangles, TriangleStrip};
		paint(prims[prim.selected()], vertices, colors, N);
		//paint(draw::Triangles, vertices, colors, N);
	}
	
	float spin;
	Point3 vertices[N];
	Color colors[N];
		
	Table gui;
	NumberDialer freqX, freqY, freqZ;
	Slider tz;
	ListSelect prim;
};

Scene scene;

int main (int argc, char ** argv){
	GLV top;
	top.colors().set(Color(HSV(0.6,0.5,0.5), 0.9), 0.4);
	top << scene;

	Window win(800, 600, "Space Curve", &top);
	Application::run();
}

