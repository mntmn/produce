/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "test_glv.h"
#include <iostream>

using namespace glv;

class TestWindow : public Group {
public:
    TestWindow()
        : Group(Rect(640, 640))
        , other(NULL)
    {
		stretch(1,1);
        LayoutGrid layout(*this, 2, 2, 10);
        layout << *new CharView;
        slider = new Slider(Rect(300,300));
        *slider << new Label("This one causes change in other window");
        layout << *slider;
        passive_slider = new Slider(Rect(300,300));
        layout << *passive_slider;
        *passive_slider << new Label("This one is manipulated by other window");
        slider->attach(onNotify, Update::Value, this);
    }
	
    static void onNotify(const glv::Notification& n){
        TestWindow *this_ = static_cast<TestWindow *>(n.receiver());
        std::cout << this_->slider->getValue() << std::endl;
        if(this_->other)
            this_->other->passive_slider->setValue(this_->slider->getValue());
    }

    Slider *slider;
    Slider *passive_slider;
    TestWindow *other;
};


int main(int argc, char ** argv){

    using namespace glv;
    
	GLV glv1;
	Window win1(640, 640, "Window One", &glv1);
    TestWindow test1;
    glv1 << test1;
    
	GLV glv2;
	Window win2(640, 640, "Window Two", &glv2);
	win2.position(win1.right(), 0);
    TestWindow test2;
    glv2 << test2;
    
    test1.other = &test2;
    test2.other = &test1;
    
	Application::run();
	return 0;
}
