/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include "glv.h"
#include <assert.h>

using namespace glv;

void ntValue(const Notification& n){
	bool& b = *(bool *)n.receiver();
	b = true;
	
//	View& v = *n.sender<View>();
//	int idx = ((ModelChange*)n.data())->index();
//	printf("%s: %s (i = %d)\n", v.className(), (v.model().toToken()).c_str(), idx);
}

void ntSetBool1(const Notification& n){
	bool& b = *(bool *)n.receiver();
	b = true;
}
void ntSetBool2(const Notification& n){
	bool& b = *(bool *)n.receiver();
	b = true;
}



int main(){

	#define SET4(x, a,b,c,d) x[0]=a; x[1]=b; x[2]=c; x[3]=d
	#define EQ4(x, a,b,c,d) (x[0]==a && x[1]==b && x[2]==c && x[3]==d)

	{	//for(int i=0;i<4;++i) printf("%g\n", f4[i]);
		//printf("%s\n", s1.c_str());
		bool b1;
		float f1;
		double d1;
		std::string s1;
		
		bool b4[4];
		float f4[4];
		double d4[4];
		
		fromToken(f1, "1e-2");		assert(f1 == float(1e-2));
		fromToken(f1, "1000");		assert(f1 == float(1000));
		fromToken(d1, "1e-2");		assert(d1 == double(1e-2));
		fromToken(d1, "1000");		assert(d1 == double(1000));
		fromToken(b1, "0");			assert(b1 == 0);
		fromToken(b1, "1");			assert(b1 == 1);
		fromToken(s1, "\"test\"");	assert(s1 == "test");
		
		fromToken(b4,4,1, "{1,0,1,1}");				assert(EQ4(b4, 1,0,1,1));
		fromToken(b4,4,1, "{0,  1, 0,0}");			assert(EQ4(b4, 0,1,0,0));
		fromToken(f4,4,1, "{1, -1.2, 1e10, +.1}");	assert(EQ4(f4, 1.f,-1.2f,1e10f,+.1f));
		fromToken(d4,4,1, "{1, -1.2, 1e10, +.1}");	assert(EQ4(d4, 1,-1.2,1e10,+.1));

		toToken(s1, 1000.f);		assert(s1 == "1000");
		toToken(s1, 2000.0);		assert(s1 == "2000");
		//toToken(rs, -2.1e100);		printf("%s\n", rs.c_str()); //assert(rs == "2e100");
		//toToken(rs, (float)-2.1e38);		printf("%s\n", rs.c_str()); //assert(rs == "2e100");
		toToken(s1, true);			assert(s1 == "1");
		toToken(s1, false);			assert(s1 == "0");
		toToken(s1, "test");		assert(s1 == "\"test\"");
		
		SET4(b4, 1,0,1,1);		toToken(s1, b4,4,1);	assert(s1 == "{1, 0, 1, 1}");
		SET4(f4,-1,0.1,3,1e10);	toToken(s1, f4,4,1);	assert(s1 == "{-1, 0.1, 3, 1e+10}");
		SET4(d4,-1,0.1,3,1e10);	toToken(s1, d4,4,1);	assert(s1 == "{-1, 0.1, 3, 1e+10}");
	}


	// test View linked list implementation
	{
		View n00, n10, n11, n20;

		n00.add(n10);
		n00.add(n11);
		n10.add(n20);
		
		// check all of our links
		assert( n00.child == &n10);
		assert(!n00.parent);
		assert(!n00.sibling);
		assert( n10.parent == &n00);
		assert( n10.sibling == &n11);
		assert( n10.child == &n20);
		assert( n11.parent == &n00);
		assert(!n11.child);
		assert(!n11.sibling);
		assert( n20.parent == &n10);
		assert(!n20.child);
		assert(!n20.sibling);
		
		n10.makeLastSibling();
		assert(!n10.sibling);
		assert(n11.sibling == &n10);
		n11.makeLastSibling();

		n10.remove();
		assert(!n10.parent);
		assert(!n10.sibling);
		assert(!n00.child->sibling);
		
		n11.remove();
		assert(!n00.child);

		assert(n10.child == &n20);
	}

	
	// test View memory management
	{
		View * v0d = new View;
		View v1s;
		View * v1d = new View;
		
		assert(v0d->dynamicAlloc());
		assert(!v1s.dynamicAlloc());

		*v0d << v1s << v1d;
		
		delete v0d;
	}



	// Notifications	
	{
		bool bv1=false, bv2=false, bf=false;
		Notifier n;
	
		assert(n.numObservers(Update::Value) == 0);

		n.attach(ntSetBool1, Update::Value, &bv1);
		n.attach(ntSetBool2, Update::Value, &bv2);
		n.attach(ntSetBool1, Update::Focus, &bf);

		assert(n.numObservers(Update::Value) == 2);
		assert(n.numObservers(Update::Focus) == 1);

		n.notify(Update::Value);
		assert(bv1);
		assert(bv2);
		assert(!bf);

		n.notify(Update::Focus);
		assert(bf);
		
		bv1=bv2=false;
		n.detach(ntSetBool1, Update::Value, &bv1);
		n.notify(Update::Value);
		assert(!bv1);
		assert(bv2);

		bv1=bv2=false;
		n.detach(ntSetBool2, Update::Value, &bv2);
		n.notify(Update::Value);
		assert(!bv1);
		assert(!bv2);
	}
	
	{
		bool b=false;
		Button w;
		w.attach(ntValue, Update::Value, &b);
		w.setValue(true);
		assert(b);
		assert(w.getValue() == true);

// Setting to current value should NOT send notification to avoid infinite loops
		b=false;
		w.setValue(w.getValue());
		assert(!b);

// Boolean model variable
		bool v = false;
		w.attachVariable(v);
		w.setValue(true);		assert(v == true);
		w.setValue(false);		assert(v == false);
		
		v = true;
		w.onDataModelSync();		assert(w.getValue() == true);

		w.setValue(true);

		assert(w.data().toToken() == "1");
		w.setValue(false);
		assert(w.data().toToken() == "0");
		w.modelFromString("0");	assert(w.getValue() == false);
		w.modelFromString("1");	assert(w.getValue() == true);

		assert(!w.modelFromString("invalid"));
	}

	{
		Buttons w;
		bool b=false;
		w.attach(ntValue, Update::Value, &b);
		w.setValue(true, 0);
		assert(b);
		assert(w.getValue(0) == true);
		
		w.data().resize(2,2);
		w.data().assignAll(0);

		bool v1 = false;
		bool v2 = false;
		w.attachVariable(v1, 0);
		w.attachVariable(v2, 2);

		w.setValue(true, 0);		assert(v1 == true);
		w.setValue(true, 2);		assert(v2 == true);
		w.setValue(false, 0);		assert(v1 == false);
		w.setValue(false, 2);		assert(v2 == false);

		w.setValue(false, 0);
		w.setValue(false, 1);
		w.setValue(false, 2);
		w.setValue(false, 3);
		assert(w.data().toToken() == "{0, 0, 0, 0}");
		
		v1 = v2 = false;
		w.modelFromString("{1,1,1,1}");
		assert(w.getValue(0) && w.getValue(1) && w.getValue(2) && w.getValue(3));
		assert(v1 && v2);
	}

	{
		bool b=false;
		Slider w;
		w.attach(ntValue, Update::Value, &b);
		w.setValue(0.99f);
		assert(b);
		assert(w.getValue() == 0.99f);

// Setting to current value should NOT send notification to avoid infinite loops
		b=false;
		w.setValue(w.getValue());
		assert(!b);

		float v;
		w.attachVariable(v);
		w.setValue(0.99);
		assert(v == 0.99f);
		
		v = 0;
		w.onDataModelSync();
		assert(w.getValue() == 0);

		std::string s;
		w.setValue(0.25);
		assert(w.data().toToken() == "0.25");
		w.modelFromString("0.5");	assert(w.getValue() == 0.5);
									assert(v == 0.5);

		assert(!w.modelFromString("invalid"));
		assert(w.getValue() == 0.5);
	}

	{
		bool b=false;
		Sliders w(Rect(1), 2,2);
		w.attach(ntValue, Update::Value, &b);
		w.setValue(0.99f, 3);
		assert(b);
		assert(w.getValue(3) == 0.99f);

		float v1, v2;
		w.attachVariable(v1, 0);
		w.attachVariable(v2, 1);

		w.setValue(0.1f, 0); assert(v1 == 0.1f);
		w.setValue(0.2f, 0); assert(v1 == 0.2f);
		
		v1 = 0.8f; v2 = 0.9f;
		w.onDataModelSync();
		assert(w.getValue(0) == 0.8f);
		assert(w.getValue(1) == 0.9f);

		w.setValue(0.1f, 0);
		w.setValue(0.2f, 1);
		w.setValue(0.3f, 2);
		w.setValue(0.4f, 3);
		assert(w.data().toToken() == "{0.1, 0.2, 0.3, 0.4}");
		
		v1=v2=0;
		w.modelFromString("{0.4,0.3,0.2,0.1}");
		assert(w.getValue(0) == 0.4);
		assert(w.getValue(1) == 0.3);
		assert(w.getValue(2) == 0.2);
		assert(w.getValue(3) == 0.1);
		assert(v1 == (float)w.getValue(0) && v2 == (float)w.getValue(1));
		
//		double vs[4] = {0.11, 0.22, 0.33, 0.44};
//		w.attachVariable(vs,4);
//		w.onDataModelSync();
//		for(int i=0; i<4; ++i) printf("%f\n", w.values()[i]);
//		
//		w.getValue(0.44, 0);
//		w.getValue(0.33, 0);
//		w.getValue(0.22, 0);
//		w.getValue(0.11, 0);
//		for(int i=0; i<4; ++i) printf("%f\n", vs[i]);
	}

	{
		bool b=false;
		Slider2D w;
		w.attach(ntSetBool1, Update::Value, &b);
		w.setValue(0.01f, 0);
		assert(b);
		assert(w.getValue(0) == 0.01f);

// Setting to current value should NOT send notification to avoid infinite loops
		b=false;
		w.setValue(0.01f, 0);
		assert(!b);
		
		b=false;
		w.setValue(0.00f, 0);
		w.setValue(0.01f, 1);
		assert(b);
		assert(w.getValue(1) == 0.01f);
		
		b=false;
		w.setValueMax();
		assert(b);
		assert(w.getValue(0) == 1.f);
		assert(w.getValue(1) == 1.f);
		
		float v1, v2;
		w.attachVariable(v1, 0);
		w.attachVariable(v2, 1);
		
		w.setValue(0.5f, 0);	assert(v1 == 0.5f);
		w.setValue(0.6f, 1);	assert(v2 == 0.6f);
		
		v1 = 0.1;
		v2 = 0.2;
		
		w.onDataModelSync();
		assert(w.getValue(0) == v1);
		assert(w.getValue(1) == v2);
	
		w.setValue(0.2, 0);
		w.setValue(0.3, 1);
		assert(w.data().toToken() == "{0.2, 0.3}");
		
		v1 = v2 = 0;
		w.modelFromString("{0.7, 0.8}");
		assert(w.getValue(0)==0.7 && w.getValue(1)==0.8);
		assert(v1==(float)w.getValue(0) && v2==(float)w.getValue(1));
	}

	{
		bool b=false;
		SliderGrid<4> w;
		w.attach(ntValue, Update::Value, &b);
		w.setValue(0.01f, 0);
		assert(b);
		assert(w.getValue(0) == 0.01f);

		float v1, v2;
		w.attachVariable(v1, 0);
		w.attachVariable(v2, 1);

		w.setValue(0.1f, 0); assert(v1 == 0.1f);
		w.setValue(0.2f, 0); assert(v1 == 0.2f);
		
		v1 = 0.8f; v2 = 0.9f;
		w.onDataModelSync();
		assert(w.getValue(0) == 0.8f);
		assert(w.getValue(1) == 0.9f);

		w.setValue(0.1f, 0);
		w.setValue(0.2f, 1);
		w.setValue(0.3f, 2);
		w.setValue(0.4f, 3);
		assert(w.data().toToken() == "{0.1, 0.2, 0.3, 0.4}");
		
		v1=v2=0;
		w.modelFromString("{0.4,0.3,0.2,0.1}");
		assert(w.getValue(0) == 0.4);
		assert(w.getValue(1) == 0.3);
		assert(w.getValue(2) == 0.2);
		assert(w.getValue(3) == 0.1);
		assert(v1 == (float)w.getValue(0) && v2 == (float)w.getValue(1));
	}
	
	{
		bool b=false;
		SliderRange w;
		w.attach(ntValue, Update::Value, &b);
		w.setValue(0.01f, 0);
		w.setValue(0.02f, 1);
		assert(b);
		assert(w.getValue(0) == 0.01f);
		assert(w.getValue(1) == 0.02f);
		assert(w.center() == 0.015f);
		assert(w.range() == 0.01f);
		
		w.extrema(0,1);
		assert(w.getValue(0) == 0);
		assert(w.getValue(1) == 1);
		
		w.centerRange(0.5, 0.25);
		assert(w.getValue(0) == (0.5-0.25/2));
		assert(w.getValue(1) == (0.5+0.25/2));

		float v1,v2;
		w.attachVariable(v1, 0);
		w.attachVariable(v2, 1);

		w.setValue(0.2, 0);
		w.setValue(0.3, 1);
		assert(w.data().toToken() == "{0.2, 0.3}");
		
		v1 = v2 = 0;
		w.modelFromString("{0.7, 0.8}");
		assert(w.getValue(0)==0.7 && w.getValue(1)==0.8);
		assert(v1==(float)w.getValue(0) && v2==(float)w.getValue(1));
	}

	{
		bool b=false;
		Label w;

		w.attach(ntValue, Update::Value, &b);
		w.setValue("test");
		assert(b);
		assert(w.getValue() == "test");
		
		assert(w.data().toToken() == "\"test\"");
		
		w.setValue("");
		w.modelFromString("\"test\"");	assert(w.getValue() == "test");
	}

	{
		bool b=false;
		NumberDialer w(1,2,1,0);
		w.attach(ntValue, Update::Value, &b);
		w.setValue(0.75);
		assert(b);
		assert(w.getValue() == 0.75);

// Setting to current value should NOT send notification to avoid infinite loops
		b=false;
		w.setValue(w.getValue());
		assert(!b);

		double v=0;
		w.attachVariable(v);
		w.setValue(0.5);	assert(v == 0.5);
		
		v = 0.25;
		w.onDataModelSync();
		assert(w.getValue() == v);

		w.setValue(0.2);
		assert(w.data().toToken() == "0.2");
		
		v = 0;
		w.modelFromString("0.8");
		assert(w.getValue()==0.8 && w.getValue()==v);
	}

	{
		bool b=false;
		TextView w;
		w.attach(ntValue, Update::Value, &b);
		w.setValue("hello");
		assert(b);
		assert(w.getValue() == "hello");
		
		assert(w.data().toToken() == "\"hello\"");

		std::string v = "test";
		w.attachVariable(v);
		
		w.onDataModelSync();
		assert(w.getValue() == "test");

		w.modelFromString("\"world\"");
		assert(w.getValue() == "world");
		assert(v == "world");
	}


	// Multidimensional array
	{
	
		// basics
		{
			const int s1=5, s2=4, s3=3, s4=2;
			Data d(Data::INT, s1,s2,s3,s4);

			assert(d.hasData());
			assert(d.type() == Data::INT);
			assert(d.sizeType() == sizeof(int));
			assert(d.isNumerical());
			assert(d.offset() == 0);
			assert(d.order() == 4);
			assert(d.stride() == 1);

			assert(d.size(0) == s1);
			assert(d.size(1) == s2);
			assert(d.size(2) == s3);
			assert(d.size(3) == s4);
			assert(d.size(0,1) == s1*s2);
			assert(d.size(0,1,2) == s1*s2*s3);
			assert(d.size(0,1,2,3) == d.size());
		
			assert(d.indexFlat(0,0,0,0) ==          0);
			assert(d.indexFlat(1,0,0,0) ==          1);
			assert(d.indexFlat(0,1,0,0) ==       s1*1);
			assert(d.indexFlat(0,2,0,0) ==       s1*2);
			assert(d.indexFlat(0,0,1,0) ==    s2*s1*1);
			assert(d.indexFlat(0,0,2,0) ==    s2*s1*2);
			assert(d.indexFlat(0,0,0,1) == s3*s2*s1*1);
			assert(d.indexFlat(0,0,0,2) == s3*s2*s1*2);
			
			for(int i=0; i<d.size(); ++i){
				int i1,i2,i3,i4;
				d.indexDim(i1,i2,i3,i4, i);
				assert(d.inBounds(i1,i2,i3,i4));
				assert(d.indexFlat(i1,i2,i3,i4) == i);
			}
			
			for(int i=0; i<d.size(); ++i) d.assign(i, i);
			for(int i=0; i<d.size(); ++i) assert(d.elem<int>(i) == i);
			
			{
				Data e = d;
				assert(e == d);
				
				e.clone();
				assert(d.elems<int>() != e.elems<int>());
				assert(e == d);
			}
			
			{				
				Data e = d.reversed();
				for(int i=0; i<e.size(); ++i)
					assert(d.elem<int>(d.size()-1-i) == e.elem<int>(i));

				e.clone();
				for(int i=0; i<e.size(); ++i)
					assert(d.elem<int>(d.size()-1-i) == e.elem<int>(i));
				
				e += d;
				for(int i=0; i<e.size(); ++i)
					assert(e.elem<int>(i) == e.size()-1);

			}
		}


		// reference counting
		{
			Data d1(Data::INT, 4,4);
			assert(Data::references(d1.elems<int>()) == 1);

			{
				Data d2 = d1;
				assert(d1.elems<int>() == d2.elems<int>());
				assert(Data::references(d1.elems<int>()) == 2);
			}
			assert(Data::references(d1.elems<int>()) == 1);

			{
				Data d2 = d1.slice(1);
				Data d3 = d2.slice(1);
				assert(Data::references(d1.elems<int>()) == 3);
			}
			assert(Data::references(d1.elems<int>()) == 1);
		}
		
	
		const float cf = 10;
		float f = 10;
		std::string s = "hello";
		
		Data d(cf);

		assert(d.hasData());
		assert(d.size() == 1);
		assert(d.type() == Data::FLOAT);
		assert(d.at<float>(0) == cf);
		assert(d.toString() == "10");
		assert(d.at<std::string>(0) == "10");
		
//		d.put(std::string("8"));
//		assert(d.at<float>(0) == 8);
		
		d.assign(11.f);
		assert(d.at<float>(0) == 11.f);
		
		d.clone();
		assert(d.hasData());
		assert(d.size() == 1);
		assert(d.type() == Data::FLOAT);

		d.assign(11.f);
		assert(d.at<float>(0) == 11.f);

		d.assign(100);
		assert(d.at<float>(0) == 100);
		
		d.set(f);
		assert(d.elems<float>() == &f);
		assert(d.at<float>(0) == f);
		
		d.set(s);
		assert(d.type() == Data::STRING);
		assert(d.size() == 1);
		assert(d.elems<std::string>() == &s);
		assert(d.at<std::string>(0) == s);
		
		d.clone();
		assert(d.elems<std::string>() != &s);
		assert(d.at<std::string>(0) == s);

		d.set("hello");
		assert(d.type() == Data::STRING);
		assert(d.at<std::string>(0) == "hello");

		{
			std::string s = "d2";
			Data d2(s);
			
			d.set("d");
			d2.assign(d);
			assert(d2.at<std::string>(0) == d.at<std::string>(0));
		}
	}

	// model to string conversion
	{
		Label l;
		TextView tv;
		Button b;
		Buttons bs(Rect(), 2, 2);
		Slider s;
		Sliders ss(Rect(), 1, 4);
		Slider2D s2D;
		NumberDialer nd(1,4,1,-1);
		std::string strings[] = {"test1", "test2", "test3"};
	
		l.setValue("Hello Label!").name("l");
		tv.setValue("Hello TextView!").name("tv");
		b.setValue(true).name("b");
		bs.setValue(true, 0,1).name("bs");
		s.setValue(0.5).name("s");
		ss.setValue(0.4, 1).name("ss");
		s2D.setValue(0.5, 0).setValue(0.1, 1).name("s2D");
		nd.setValue(-0.54941).name("nd");

		const View * views[] = {&l, &tv, &b, &bs, &s, &ss, &s2D, &nd};
		for(unsigned i=0; i<sizeof(views)/sizeof(View *); ++i){
			//const Data& d = views[i]->data();
			//printf("%p: %s\n", &d, d.toString().c_str());
			//views[i]->model().print(); printf("\n");
		}

		View top;
		top << l << tv << b << bs << s << ss << s2D << nd;
	
		std::string str1, str2;
//		top.modelToString(str1, "test model");		
//		printf("%s\n", str1.c_str());
//
//		l.setValue("changed...");
//		top.modelFromString(str1);
//		top.modelToString(str2);
//		printf("%s\n", str2.c_str());
//		assert(str1 == str2);

		ModelManager mm;
		
		mm.add("l", l);
		mm.add("tv", tv);
		mm.add("b", b);
		mm.add("bs", bs);
		mm.add("s" , s);
		mm.add("ss", ss);
		mm.add("s2D",s2D);
		mm.add("nd", nd);
		
		Data dat(strings, 3);
		//Data dat(10);
		//dat.print();
		//mm.add("strings", dat);
		//mm.add("strings", *new Data(strings, 3));
		
//		mm.toToken(str1, "test");
//		printf("%s\n", str1.c_str());
//		
//		mm.fromToken(str1);
//		mm.toToken(str2, "test");
//		printf("%s\n", str2.c_str());

		std::string snapshotString1 =
"[\"test 1\"] = {\n\
	l = \"Label 1\",\n\
	tv = \"TextView 1\",\n\
	b = 0,\n\
	bs = {0,0,0,0},\n\
}";

		std::string snapshotString2 =
"[\"test 2\"] = {\n\
	l = \"Label 2\",\n\
	tv = \"TextView 2\",\n\
	b = 1,\n\
	bs = {1,1,1,1},\n\
	s = 1,\n\
	ss = {1,1,1,1},\n\
	s2D = {1,1},\n\
	nd = 1,\n\
}";
		
		mm.snapshotsFromString("{" + snapshotString1 + ",\r\n" + snapshotString2 + "}");

//		mm.snapshots()["test 1"]["l"].print();
//		mm.snapshots()["test 1"]["b"].print();
//		mm.snapshots()["test 1"]["bs"].print();
//		mm.snapshots()["test 1"]["s"].print();
//		mm.snapshots()["test 1"]["ss"].print();
//		mm.snapshots()["test 1"]["s2D"].print();
//		mm.snapshots()["test 1"]["nd"].print();
//		mm.snapshots()["test 2"]["l"].print();
//		mm.snapshots()["test 2"]["b"].print();
//		mm.snapshots()["test 2"]["bs"].print();
//		mm.snapshots()["test 2"]["s"].print();
//		mm.snapshots()["test 2"]["ss"].print();
//		mm.snapshots()["test 2"]["s2D"].print();
//		mm.snapshots()["test 2"]["nd"].print();
//
//		mm.snapshotsToFile("snapshots.txt");
//		
//		mm.clearSnapshots();
//		mm.snapshotsFromFile("snapshots.txt");
//
//		str1.clear();
//		mm.snapshotsToString(str1);
//		printf("%s\n", str1.c_str());
	}

//	{
//		std::string str( "a bc ab ca ab" );
//		std::string searchString("ab"); 
//		std::string replaceString("hello");
//
//		std::string::size_type pos = 0;
//		while((pos = str.find(searchString, pos)) != std::string::npos){
//			str.replace(pos++, searchString.size(), replaceString);
//		}
//		printf("%s\n", str.c_str());
//	}

//	printf("sizeof(Notifier): %d\n", sizeof(Notifier));
//	printf("sizeof(View): %d\n", sizeof(View));
//	printf("sizeof(Data): %d\n", sizeof(Data));
//	printf("sizeof(Model): %d\n", sizeof(Model));
//	printf("sizeof(ModelManager): %d\n", sizeof(ModelManager));

	return 0;
}
