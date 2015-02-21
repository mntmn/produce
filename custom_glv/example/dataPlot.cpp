/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */
/*
Example: 
*/

#include "example.h"

namespace glv{

struct PlotVector : public Plottable{

	PlotVector(): Plottable(draw::Triangles){}

	void onMap(GraphicsData& b, const Data& d, const Indexer& i){
		double dx = 2./d.size(1);
		double dy = 2./d.size(2);
		while(i()){
			double x = i.frac(0)*2 - 1 + dx/2;
			double y = i.frac(1)*2 - 1 + dy/2;
			double vx=d.at<double>(0,i[0],i[1],i[2])*dx*2;
			double vy=d.at<double>(1,i[0],i[1],i[2])*dy*2;

			double r = 0.2/hypot(vx, vy);
			double Nx = vy*r*dx;
			double Ny =-vx*r*dy;

//			b.addVertex2(
//				x, y,
//				x + d.at<double>(0,i[0],i[1],i[2])*dx,
//				y + d.at<double>(1,i[0],i[1],i[2])*dy
//			);
			b.addVertex2(
				x+Nx, y+Ny,
				x-Nx, y-Ny,
				x + vx,
				y + vy
			);
			b.addColor(
				Color(0,0),
				Color(0,0),
				Color(0)
			);
		}
	}
};


struct MyGLV : GLV {

	MyGLV(): phase(0){
		data().resize(Data::DOUBLE, 2,16,16);
	}

	void onAnimate(double dt, GLV& g){
		phase += 0.013;
		Data& d = data();
		
		Indexer i(d.size(1), d.size(2));

		while(i()){
			float x = i.frac(0);
			float y = i.frac(1);
			x = (cos(x*7 + phase) + cos(x*-2 + phase*1.01))/2;
			y = (sin(y*5 + phase) + sin(y*-3 + phase*1.01))/2;
			d.assign(x, 0,i[0],i[1]);
			d.assign(y, 1,i[0],i[1]);
		}
	}

	double phase;
};
}

int main(){

	// Create the Views
	MyGLV top;
	Plot v1(Rect(000,0, 400,400), *new PlotDensity);
	Plot v2(Rect(400,0, 400,400), *new PlotVector);
	Plot v3(Rect(800,0, 400,400), (new PlotFunction2D)->prim(draw::Points).stroke(2));

	v1.data() = top.data();
	v2.data() = top.data();
	v3.data() = top.data();
	
	top << v1 << v2 << v3;

	Window win(80,80, "Data Plots", &top);
	win.fit();
	Application::run();
	return 0;
}

