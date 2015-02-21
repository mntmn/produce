/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "test_glv.h"

using namespace glv;

void updateSlider12(const Notification& n){
	Slider2D& s2 = *(Slider2D *)n.receiver();
	Slider& s = *(Slider *)n.sender();
	s2.setValue(s.getValue(), 0);
}

void updateSlider21(const Notification& n){
//	Slider2D& s2 = *(Slider2D *)n.sender();
//	Slider& s = *(Slider *)n.receiver();
//	s.setValue(s2.value(0));
}

void updateFocus(const Notification& n){
	Slider& s = *(Slider *)n.sender();
	Label& l = *(Label *)n.receiver();
	l.setValue(s.enabled(Focused) ? "get focus" : "lose focus");
}

void updateLabel(const Notification& n){
	Label& l = *(Label *)n.receiver();
	ModelChange v = *(ModelChange *)n.data();
	char buf[16]; snprintf(buf, sizeof(buf), "[%2d] = ", v.index());
	l.setValue(buf + v.value().slice(v.index(), 1).toToken());
}


int main(int argc, char ** argv){

	GLV glv;

	Window win(320, 400);
	win.setGLV(glv);

	// Make some widgets
	Button button(Rect(32));
	Buttons buttons(Rect(64), 4,4); buttons.enable(SelectOnDrag);
	Slider slider(Rect(10, 10, 100, 20), 0); slider.interval(1,-1);
	Slider2D slider2(Rect(10, 40, 100, 100));
	SliderGrid<4> sliderGrid(Rect(100));
	SliderRange sliderRange(Rect(100,20));
	//NumberDialer numberDialer(Rect(12*4, 12), 1,2);
	NumberDialer numberDialer(12,0,0, 1,2, 10,-10);
	numberDialer.padding(2);

	// Create labels for update notifications
	Label::Spec spec(Place::CR, 10, 0);
	Label lbButton("", spec), lbButtons("", spec), lbSlider("", spec), 
		lbSlider2("", spec), lbSliderGrid("", spec), lbSliderRange("", spec),
		lbNumberDialer("", spec);

	// Attach some notifications
	button.attach(updateLabel, Update::Value, &lbButton);
	buttons.attach(updateLabel, Update::Value, &lbButtons);
	slider.attach(updateLabel, Update::Value, &lbSlider);
	slider2.attach(updateLabel, Update::Value, &lbSlider2);
	sliderGrid.attach(updateLabel, Update::Value, &lbSliderGrid);
	sliderRange.attach(updateLabel, Update::Value, &lbSliderRange);
	numberDialer.attach(updateLabel, Update::Value, &lbNumberDialer);
	
	slider.attach(updateFocus, Update::Focus, &lbSlider);
	slider.attach(updateSlider12, Update::Value, &slider2);
	slider2.attach(updateSlider21, Update::Value, &slider);

	Placer placer(glv, Direction::S, Place::TL, 10, 10);

	placer	<< (button << lbButton)
			<< (buttons << lbButtons)
			<< (slider << lbSlider)
			<< (slider2 << lbSlider2)
			<< (sliderGrid << lbSliderGrid)
			<< (sliderRange << lbSliderRange)
			<< (numberDialer << lbNumberDialer);

	Application::run();
}

