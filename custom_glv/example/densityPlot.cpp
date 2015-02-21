/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: Density Plot

This demonstrates how to use multiple density plots to visualize a 3D array
of color values in various ways.
*/

#include "example.h"

int main(){

	const int N = 8;
	const int N3 = N*N*N;
	float w = 100;

	// Initialize raw color data
	Color bulk[N3];
	for(int i=0; i<N3; ++i){
		float f = float(i)/N3;
		bulk[i].set(f, 1-f, cos(f*6.2832*6)*0.5+0.5, 1);
	}

	// Create a wrapper around raw data to obtain slices
	Data cols(&(bulk[0][0]), 4,N,N,N);

	GLV glv;

	// These plots will show all the xy slices	
	Plot plots[N];
	for(int i=0; i<N; ++i){
		plots[i].add(*new PlotDensity);
		plots[i].set(Rect(w*i,0, w,w));
		plots[i].showGrid(false);
		plots[i].data() = cols.slice(cols.size(0,1,2)*i).shape(4,N,N);
		//plots[i].data() = cols.slice(cols.size(0,1,2)*i + 2).shape(1,N,N).stride(4);
		glv << plots[i];
	}

	// This plot will show a flattened 1D color array
	Plot plot1(Rect(0,w+40*0, w*N,40), *new PlotDensity);
	
	// These plots will show flattened color component arrays
	Plot plotR(Rect(0,w+40*1, w*N,40), *new PlotDensity(Color(1,0,0)));
	Plot plotG(Rect(0,w+40*2, w*N,40), *new PlotDensity(Color(0,1,0)));
	Plot plotB(Rect(0,w+40*3, w*N,40), *new PlotDensity(Color(0,0,1)));

	plot1.data() = cols.slice(0).shape(4, cols.size(1,2,3));
	plotR.data() = cols.slice(0).shape(1, cols.size(1,2,3)).stride(4);
	plotG.data() = cols.slice(1).shape(1, cols.size(1,2,3)).stride(4);
	plotB.data() = cols.slice(2).shape(1, cols.size(1,2,3)).stride(4);

	glv << plot1 << plotR << plotG << plotB;
			
	Window win(0,0, "Density Plot", &glv);
	win.fit();
// 3% = base
// 7% = rectangles

//	plot1.name("plot1");
//	plotR.name("plotR");
//	std::string s;
//	glv.modelToString(s);
//	printf("%s\n", s.c_str());

	Application::run();
}

