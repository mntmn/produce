/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

/*
Example: Widgets

This is a simple GUI allowing various GLV widgets to be explored.
*/

#include "example.h"

using namespace glv;

const int pages = 15;
Buttons tabs(Rect(120, pages*20), 1, pages, false, true);
Group groups[pages];

Button bt1(Rect(20), false, draw::check), bt2(bt1, false, draw::triangleU, draw::triangleD);
Button btl(Rect(100, 20), true);
Buttons bts14(Rect(60, 60), 1, 4, false, true);
Buttons bts41(Rect(120, 15), 4, 1, false, true);
Buttons bts44(Rect(60), 4, 4);
Buttons btsLED(Rect(120), 8, 8, false, false, draw::disc<12>, draw::circle<12>);
Buttons btv(Rect(80), 4, 4);
FunctionGraph fg(Rect(200, 100), 4, 10);
Plot plotXY(Rect(150), *new PlotFunction2D(Color(0,0,0.5)));
Plot plotX(Rect(150,60), *new PlotFunction1D(Color(0,0.5,0)));
Plot plotY(Rect(60,150), *new PlotFunction1D(Color(0.5,0,0)));
Plot dplot(Rect(200), *new PlotDensity(HSV(0.2, 0.5), 1./16));

NumberDialer nd1(12,0,0, 4,0, 9999,-9999), nd2(16,0,0, 1,8, 8,0);
Slider sl1H(Rect(100, 20)), sl1V(Rect(20, 100)), sl1HS(sl1H,0), sl1VS(sl1V,0);
Slider2D sl2(Rect(100));
SliderGrid<3> sg3(Rect(100));
SliderGrid<4> sg4(Rect(100));
SliderRange slRH(Rect(100,20)), slRV(Rect(20,100));
Sliders sliders1(Rect(100), 10, 1, true), sliders2(sliders1, 1, 10, true);
Table table(	". v - -,"
				"> p ^ q,"
				"| < x >,"
				"| b v d", 12, 6, Rect(0));
TextView tv1(Rect(200,16), 8);

struct FontView : View {
	FontView(const Rect& r=Rect(0)): View(r){}
	
	virtual void onDraw(GLV& g){
		draw::color(colors().text);
		float x = tabs.width()+10;
		const char * str = "Amazingly few discotheques provide jukeboxes.";
		font.size( 6).render(str, x, 10);
		font.size( 8).render(str, x, 30);
		font.size(12).render(str, x, 50);
	}
	
	Font font;
} fontView;

struct SubView3D : View3D{
	SubView3D(const Rect& r): View3D(r), angle(0){
		addCallback(Event::MouseDrag, Behavior::mouseResizeCorner);
		addCallback(Event::MouseDrag, Behavior::mouseMove);
		enable(KeepWithinParent);
	}
	
	virtual void onDraw3D(GLV& g){
		using namespace glv::draw;
		translateZ(-3);
		rotateY(angle+=1);
		const int n=12;

		GraphicsData& buf = g.graphicsData();

		for(int i=0; i<n; ++i){
			float p = float(i)/n;
			float x = cos(p*6.283)*1.2;
			float z = sin(p*6.283)*1.2;
			
			buf.addVertex(x, -0.7, z);
			buf.addVertex(x, +0.7, z);
			buf.addColor(HSV(p,1,1));
			buf.addColor(0);
		}

		paint(Triangles, buf);
	}

	unsigned angle;
} v3D(Rect(200)), v3D2(Rect(100));


void drawCB(View * v, GLV& g){
	using namespace glv::draw;
	for(int i=0; i<tabs.size(); ++i) groups[i].property(Visible, tabs.getValue(i));
}



int main(int argc, char ** argv){
	
	GLV top(drawCB);
	
	Window win(600, 400, "Example: Widgets", &top);

	sl1HS.interval(1,-1);
	sl1VS.interval(1,-1);
	sliders1.interval(1,-1);
	sg3.interval(20,-20);
	sl2.interval(20,-20);
	slRH.interval(2,-2);
	
	glv::Style::standard().color.set(StyleColor::BlackOnWhite);
//	glv::Style::standard().color.set(StyleColor::WhiteOnBlack);
//	glv::Style::standard().color.set(StyleColor::Gray);
//	glv::Style::standard().color.hsv(0.5,0.5,0.2);
	
	for(int i=0; i<pages; ++i){
		top << groups[i].disable(Visible | HitTest).stretch(1,1);
	}
	
	fg.tension(1.);

	table	<< new Label("col span") << new Label("row span", true)
			<< new Label("top-left") << new Label("top-center") << new Label("top-right") 
			<< new Label("center-left") << new Label("center-center") << new Label("center-right")
			<< new Label("bottom-left") << new Label("bottom-center") << new Label("bottom-right");
	table.arrange();
	//table.enable(DrawBorder);

	plotXY.data().resize(Data::FLOAT, 2, 64);
	const float ffreq = 2*M_PI/(plotXY.data().size(1)-1);
	for(int i=0; i<plotXY.data().size(1); ++i){
		float phs = i*ffreq;
		float sigX = (cos(phs) + 0.5*cos(2*phs))/(1+0.5);
		float sigY = (sin(phs) + 0.5*sin(2*phs))/(1+0.5);
		plotXY.data().assign(sigX, 0,i);
		plotXY.data().assign(sigY, 1,i);
	}
	plotXY.numbering(true);
	plotY.data() = plotXY.data().slice(0).shape(1,1,64).stride(2);
	plotX.data() = plotXY.data().slice(1).shape(1,64,1).stride(2);
	plotY.major(8,1).range(0, plotXY.data().size(1), 1);
	plotX.major(8,0).range(0, plotXY.data().size(1), 0);

	plotX.disable(glv::Controllable);

	// Produce matrix of values
	{
		dplot.data().resize(Data::FLOAT, 1,32,32);
		Data& data = dplot.data();
		for(int j=0; j<data.size(2); ++j){
			for(int i=0; i<data.size(1); ++i){
				double y = double(j)/data.size(2)*2-1;
				double x = double(i)/data.size(1)*2-1;
				double r = (sin(hypot(x,y)*8));
				data.assign(r, 0, i,j);
			}
		}
	}

	v3D << v3D2.pos(Place::BR).anchor(Place::BR);

//btsLED.enable(Momentary);
	int i=-1;
	groups[++i]<< bt1.pos(Place::BL).anchor(Place::CC)(Event::MouseDrag, Behavior::mouseResize);
	groups[  i]<< bt2.pos(Place::BR, -4,0).anchor(Place::CC)(Event::MouseDrag, Behavior::mouseResize);
	groups[  i]<< (btl.pos(Place::TR, -4, 4).anchor(Place::CC) << new Label("OK", Place::CC, 0,0));
	groups[++i]<< bts14.pos(Place::BR, -4,0).anchor(Place::CC);
	groups[  i]<< bts41.pos(Place::TL, 0, 4).anchor(Place::CC);
	groups[  i]<< bts44.pos(Place::BL).anchor(Place::CC);
	groups[  i]<< btsLED.padding(3).pos(Place::BL, bts44.w+4,0).anchor(Place::CC).enable(SelectOnDrag).disable(DrawGrid);
	groups[++i]<< fg.pos(Place::BL).anchor(Place::CC);
//	groups[++i]<< fontView.stretch(1,1);
	groups[++i]<< (new Label("Horizontal"))->pos(Place::BL).anchor(Place::CC);
	groups[  i]<< (new Label("Vertical", true))->pos(Place::BR,-16,0).anchor(Place::CC);
	groups[++i]<< nd1.pos(Place::BL).anchor(Place::CC);
	groups[  i]<< nd2.showSign(false).pos(Place::TL, 0, 4).anchor(Place::CC);
	groups[++i]<< sl1H.pos(Place::BL).anchor(Place::CC);
	groups[  i]<< sl1V.pos(Place::BR, -4,0).anchor(Place::CC);
	groups[  i]<< sl1HS.pos(Place::BL).anchor(Place::CC); sl1HS.top(sl1H.bottom()+4);
	groups[  i]<< sl1VS.pos(Place::BR, -4,0).anchor(Place::CC); sl1VS.right(sl1V.left()-4);
	groups[++i]<< sl2.pos(Place::BL).anchor(Place::CC);
	groups[++i]<< sg3.pos(Place::BL).anchor(Place::CC);
	groups[  i]<< sg4.pos(Place::TL, 0, 4).anchor(Place::CC);
	groups[++i]<< slRH.pos(Place::BL).anchor(Place::CC);
	groups[  i]<< slRV.pos(Place::BR, -4,0).anchor(Place::CC);
	groups[++i]<< sliders1.pos(Place::BR).anchor(Place::CC); 
	groups[  i]<< sliders2.pos(Place::BL, 4,0).anchor(Place::CC);
	groups[++i]<< table.pos(Place::BC).anchor(Place::CC);
	groups[++i]<< plotXY.pos(Place::BL).anchor(Place::CC);
	groups[  i]<< plotX.pos(Place::TL, 0,4).anchor(Place::CC);
	groups[  i]<< plotY.pos(Place::BR,-4,0).anchor(Place::CC);
	groups[++i]<< dplot.pos(Place::CC).anchor(Place::CC);
	groups[++i]<< tv1.pos(Place::CC, 0,0).anchor(Place::CC);
	groups[++i]<< v3D.pos(Place::CC, 0,0).anchor(Place::CC);

	float y = 1./(tabs.size()*2), dy=2*y;
	tabs << (new Label("Button"			))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Buttons"		))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("FunctionGraph"	))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Label"			))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("NumberDialer"	))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Slider"			))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Slider2D"		))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("SliderGrid"		))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("SliderRange"	))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Sliders"		))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Table"			))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Function Plots"	))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("Density Plot"	))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("TextView"		))->anchor(0.5, y).pos(Place::CC); y+=dy;
	tabs << (new Label("View3D"			))->anchor(0.5, y).pos(Place::CC); y+=dy;
	top << tabs;

	//tabs(Event::MouseDrag, Behavior::mouseResize);

	//#define PRINT_SIZE(x) printf("sizeof(%s) = %d\n", #x, sizeof(x));
	//PRINT_SIZE(View) PRINT_SIZE(Notifier) PRINT_SIZE(Rect)

	Application::run();
}

