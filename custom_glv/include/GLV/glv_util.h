#ifndef INC_GLV_UTIL_H
#define INC_GLV_UTIL_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <list>
#include <vector>

namespace glv {


/// Return value clipped to interval [min, max)
template <class T>
inline T clip(const T& v, const T& max=T(1), const T& min=T(0)){ return v<min? min:(v>max? max:v); }

template <class T> inline T floor(const T& v, const T& step){ return ::floor(v/step)*step; }

template <class T> inline T max(const T& a, const T& b){ return a<b ? b:a; }
template <class T> inline T min(const T& a, const T& b){ return a<b ? a:b; }
template <class T> inline T mean(const T& a, const T& b){ return (a+b)*T(0.5); }

/// Sort arguments so that a<b
template<class T> inline void sort(T& a, T& b){ if(a>b){ T t=a; a=b; b=t; }  }



/// Smart pointer functionality to avoid deleting references.

///	When a class contains pointers to other objects, sometimes it is not clear
///	if it is responsible for freeing the memory associated with those objects.
class SmartPointer{
public:

	/// @param[in] del	whether the object is deleted with smartDelete()
	SmartPointer(bool del=true): mDeletable(del){}

	virtual ~SmartPointer(){}

	/// Deletes object only if it is deletable, i.e. not a reference.
	
	/// Returns true if the object destructor was called, false otherwise.
	///
	bool smartDelete(){
		//printf("%p %c\n", this, mDeletable ? 'y' : 'n');
		if(deletable()){ delete this; return true; }
		return false;
	}
	
	/// Set whether the object can be deleted.
	void deletable(bool v){ mDeletable = v; }
	
	/// Returns whether the object can be deleted.
	bool deletable(){ return mDeletable; }
	
//	void operator delete (void * p){
//		SmartPointer * pc = static_cast<SmartPointer *>(p);
//		if(pc && pc->mIsDeletable){
//			free(pc);
//			printf("freed\n");
//		}
//		else printf("denied\n");
//	}
	
private:
	bool mDeletable;
};



/// Mixin class for doing advanced memory management

/// The template parameter should be the base class in the derivation chain.
///
template <class BaseClass>
struct SmartObject {

	typedef std::list<void *> NewObjectList;

	SmartObject();

	void * operator new(size_t sz);
	void * operator new(size_t sz, int flag);
	void * operator new(size_t sz, void * m){ return m; }
	void operator delete(void * m){ free(m); }
	
	/// Returns true if the object was created dynamically with the new operator, false otherwise.
	bool dynamicAlloc() const { return mDynamicAlloc; }

private:
	bool mDynamicAlloc;
	
	// Returns true if this is within memory footprint of the base class
	bool withinFootPrint(void * m);
	
	// Temporarily list of 'new' objects
	static NewObjectList& newObjects(){
		static NewObjectList * l = new NewObjectList; return *l;
	}
};



/// Array
template <class T>
class Array{
public:

	Array(): mSizeX(0), mSizeY(0), mVal(0){}
	Array(int nx, int ny=1): mSizeX(0), mSizeY(0), mVal(0) { resize(nx, ny); }
	
	~Array(){ clear(); }

	// interface methods
	T& operator[](int i){ return mVal[i]; }
	const T& operator[](int i) const { return mVal[i]; }

	Array& operator= (const T& v){ for(int i=0; i<size(); ++i) (*this)[i]=v; return *this; }
	
	Array& data(){ return *this; }
	const Array& data() const { return *this; }
	
	void resize(int nx, int ny){
		if(nx*ny != size()){
			clear();
			mVal = new T[nx*ny];
			mSizeX = nx; mSizeY = ny;
		}
	}
	
	int size()  const { return sizeX() * sizeY(); }
	int sizeX() const { return mSizeX; }
	int sizeY() const { return mSizeY; }

	void zero(){ memset(mVal, 0, size()*sizeof(T)); }
	
protected:
	int mSizeX, mSizeY;
	T * mVal;
	void clear(){ if(mVal) delete[] mVal; mVal=0; }
};


/// Array optimized for dynamically changing its size
template <class T, class Alloc=std::allocator<T> >
class Buffer : protected Alloc{
public:

	/// @param[in] capacity		capacity of buffer
	explicit Buffer(int capacity=0): mSize(0), mElems(capacity){}

	~Buffer(){}

	/// Get mutable element at index
	T& operator[](int i){ return mElems[i]; }
	
	/// Get const element at index
	const T& operator[](int i) const { return mElems[i]; }

	/// Returns number of elements before needing to allocate more memory
	int capacity() const { return mElems.capacity(); }

	/// Returns number of elements
	int size() const { return mSize; }

	/// Assign value to elements

	/// This function fills a Buffer with n copies of the given value. Note that
	/// the assignment completely changes the buffer and that the resulting size
	/// is the same as the number of elements assigned.  Old data may be lost.
	void assign(int n, const T& v){ mElems.assign(n,v); }

	/// Empty buffer
	void clear(){ mElems.clear(); reset(); }

	/// Set size of buffer back to 0
	void reset(){ setSize(0); }

	/// Resize buffer
	
	/// This will set both the size and capacity of the buffer to the requested 
	/// size. If the number is smaller than the current size the buffer is 
	/// truncated, otherwise the buffer is extended and new elements are
	/// default-constructed.
	void resize(int n){
		mElems.resize(n); setSize(n);
	}
	
	/// Set size of buffer avoiding memory reallocation when possible
	
	/// If the requested size is larger than the current capacity, then the 
	/// buffer will be resized.
	void size(int n){
		(capacity() >= n) ? setSize(n) : resize(n);
	}

	/// Appends element to end of buffer growing its size if necessary
	void append(const T &v, double growFactor=2){
		if(size() >= capacity()){
			T vcopy = v; // 'v' may become invalidated after resize
			mElems.resize(size() ? size()*growFactor : 4);
			Alloc::construct(&mElems[0] + size(), vcopy);
		}
		else{
			Alloc::construct(&mElems[0] + size(), v);
		}
		++mSize;
	}

private:
	int mSize;
	std::vector<T, Alloc> mElems;
	void setSize(int n){ mSize=n; }
};


/// A closed interval [min, max]

/// An interval is a connected region of the real line. Geometrically, it
/// describes a 0-sphere. Order is strongly enforced so that the endpoints will
/// always satisfy min <= max.
template <class T=double>
class Interval{
public:

	Interval()
	:	mMin(0), mMax(1){}

	Interval(const T& max)
	{ endpoints(0,max); }

	Interval(const T& min, const T& max)
	{ endpoints(min,max); }

	T center() const { return (max()+min())/T(2); }	///< Returns center point

	/// Test is point is contained exclusively within interval
	bool contains(const T& v) const { return v>=min() && v<=max(); }

	bool degenerate() const { return min()==max(); }///< Returns true if diameter is zero
	T diameter() const { return max()-min(); }		///< Returns absolute difference of endpoints
	const T& max() const { return mMax; }			///< Get maximum endpoint
	const T& min() const { return mMin; }			///< Get minimum endpoint
	bool proper() const { return min()!=max(); }	///< Returns true if diameter is non-zero
	T radius() const { return diameter()/T(2); }	///< Returns one-half the diameter

	/// Linearly map point in unit interval to point in interval
	T fromUnit(const T& v) const { return v*diameter() + min(); }

	/// Linearly map point in interval to point in the unit interval
	T toUnit(const T& v) const { return (v-min())/diameter(); }
	
	/// Linearly map point from interval to argument interval
	template <class U>
	T to(const T& v, const Interval<U>& i){ return i.fromUnit(toUnit(v)); }
	
	/// Returns true if both endpoints are equal
	template <class U>
	bool operator == (const Interval<U>& v){ return min()==v.min() && max()==v.max(); }

	/// Returns true if endpoints are not equal
	template <class U>
	bool operator != (const Interval<U>& v){ return !(*this == v); }
	
	template <class U>
	Interval& operator +=(const Interval<U>& v){ endpoints(min()+v.min(), max()+v.max()); return *this; }

	template <class U>
	Interval& operator -=(const Interval<U>& v){ endpoints(min()-v.max(), max()-v.min()); return *this; }
	
	template <class U>
	Interval& operator *=(const Interval<U>& v){
		T a=min()*v.min(), b=min()*v.max(), c=max()*v.min(), d=max()*v.max();
		mMin = min(min(a,b),min(c,d));
		mMax = max(max(a,b),max(c,d));
		return *this;
	}

	template <class U>
	Interval& operator /=(const Interval<U>& v){
		T a=min()/v.min(), b=min()/v.max(), c=max()/v.min(), d=max()/v.max();
		mMin = min(min(a,b),min(c,d));
		mMax = max(max(a,b),max(c,d));
		return *this;
	}

	/// Set center point preserving diameter
	Interval& center(const T& v){ return centerDiameter(v, diameter()); }

	/// Set diameter (width) preserving center
	Interval& diameter(const T& v){ return centerDiameter(center(), v); }

	/// Set center and diameter
	Interval& centerDiameter(const T& c, const T& d){
		mMin = c - d*T(0.5);
		mMax = mMin + d;
		return *this;
	}

	/// Set the endpoints
	Interval& endpoints(const T& min, const T& max){
		mMax=max; mMin=min;
		if(mMin > mMax){ T t=mMin; mMin=mMax; mMax=t; }
		return *this;
	}

	/// Translate interval by fixed amount
	Interval& translate(const T& v){ mMin+=v; mMax+=v; return *this; }

	/// Set maximum endpoint
	Interval& max(const T& v){ return endpoints(min(), v); }
	
	/// Set minimum endpoint
	Interval& min(const T& v){ return endpoints(v, max()); }

private:
	T mMin, mMax;

	const T& min(const T& a, const T& b){ return a<b?a:b; }
	const T& max(const T& a, const T& b){ return a>b?a:b; }
};



// Implementation ______________________________________________________________
template<class T>
SmartObject<T>::SmartObject(): mDynamicAlloc(false){
	//printf("%x: SmartObject::SmartObject()\n", this);
	// Check new list to see if this was dynamically allocated.
	// If so, then remove from list and set internal flag.
	NewObjectList& l = newObjects();
	for(NewObjectList::iterator it=l.begin(); it!=l.end(); ++it){
		
		// check if we lie within the memory footprint of the base class
		if(withinFootPrint(*it)){
			mDynamicAlloc=true;
			l.erase(it);
			break;
		}
	}
}

template<class T>
void * SmartObject<T>::operator new(size_t sz){
	void * m = malloc(sz);	// this will point to the base class
	newObjects().push_back(m);
	//printf("%x: SmartObject::operator new(size_t)\n", m);
	return m;
}

template<class T>
void * SmartObject<T>::operator new(size_t sz, int flag){
	void * m = malloc(sz);	// this will point to the base class
	newObjects().push_back(m);
	return m;
}

template<class T>
bool SmartObject<T>::withinFootPrint(void * m){
//	T * b = (T *)m;
//	return (this >= b) && (this < (b + sizeof(T)));
	char * b = (char *)m;
	char * t = (char *)this;
	//printf("%x, %x, %x\n", t, b, b+sizeof(T));
	return (t >= b) && (t < (b + sizeof(T)));
}



///// One or more static values
//template <class T, int Nx=1, int Ny=1>
//class Values{
//public:
//
//	// interface methods
//	T& operator[](int i){ return mVal[i]; }
//	const T& operator[](int i) const { return mVal[i]; }
//	
//	Values& operator= (const T& v){ for(int i=0; i<size(); ++i) (*this)[i]=v; return *this; }
//	
//	Values& data(){ return *this; }
//	const Values& data() const { return *this; }
//	
//	void resize(int nx, int ny){}
//	
//	int size()  const { return sizeX()*sizeY(); }
//	int sizeX() const { return Nx; }
//	int sizeY() const { return Ny; }
//	
//	void zero(){ memset(mVal, 0, size()*sizeof(T)); }
//
//protected:
//	T mVal[Nx*Ny];
//};

//
//// Triply-linked node
//
//// Getters must return pointers since it is possible that some linked nodes
//// are zero, i.e. nonexistant.
//template <class T>
//class Node3{
//public:
//
//	Node3(): mParent(0), mChild(0), mSibling(0){}
//
//	virtual ~Node3(){ remove(); }
//
//	void makeLastSibling();
//
//	void pushBack(Node3 * node);		///< Add node to back of my children
//	void pushBack(Node3 & node);		///< Add node to back of my children
//	
//	/// Remove myself from parent. Does not delete instance.
//	
//	/// This method should always be called from the instance's destructor.
//	///
//	void remove();
//
//	void child(Node3 * o){ mChild=o; }
//	void parent(Node3 * o){ mParent=o; }
//	void sibling(Node3 * o){ mSibling=o; }
//	void child(Node3& o){ mChild=&o; }
//	void parent(Node3& o){ mParent=&o; }
//	void sibling(Node3& o){ mSibling=&o; }
//
//	Node3 * child() const { return mChild; }
//	Node3 * parent() const { return mParent; }
//	Node3 * sibling() const { return mSibling; }
//
//	Node3 * lastSibling();
//	
//	bool firstChild() const { return parent() && (parent()->child() == this); }
//
//private:
//	Node3 * mParent;			// My parent view
//	Node3 * mChild;				// My first child (next to be drawn)
//	Node3 * mSibling;			// My next sibling view (drawn after all my children)
//};
//
//
//
//template <class T>
//void Node3<T>::pushBack(Node3 * node){
//	if(node) pushBack(*node);
//}
//
//template <class T>
//void Node3<T>::pushBack(Node3 & node){
//	node.remove();
//	node.parent(this);
//	
//	// I didn't have any children until now
//	if(!child())	child(node);
//	else			child()->lastSibling()->sibling(node);
//}
//
//template <class T>
//Node3<T> * Node3<T>::lastSibling(){
//	Node3 * t = this;
//	while(t->sibling()) t = t->sibling();
//	return t;
//}
//
//template <class T>
//void Node3<T>::makeLastSibling(){
//	if(parent() && sibling()){
//		parent()->pushBack(this); // this automatically removes node
//	}
//}
//
//template <class T>
//void Node3<T>::remove(){
//
//	if(parent() && parent()->child()){
//
//		// re-patch parent's child?
//		if(firstChild()){
//			// remove my reference, but keep the sibling list healthy
//			parent()->child(sibling());
//		}
//		
//		// re-patch the sibling chain?
//		else{
//			// I must be one of parent->child's siblings
//			Node3 * temp = parent()->child();
//			while(temp->sibling()){
//				if(temp->sibling() == this) {
//					// I'm temp's sibling
//					// - remove my reference, keep the sibling list healthy
//					temp->sibling(this->sibling()); 
//					break; 
//				}
//				temp = temp->sibling();
//			}
//		}
//		
//		parent(0); sibling(0); // no more parent or sibling, but child is still valid
//	}
//}

//class NumberEntry{
//public:
//	#define BUFSIZE 16
//	// 123456789012345678901234567890
//	// 1234567890123456
//
//	NumberEntry(){ reset(); }
//	
//	// interpret key press
//	// returns true if successful, false otherwise
//	bool read(char c){
//	
//		if(		mPos < BUFSIZE
//			&&	(isdigit(c) || (c == '.' && !mHasPoint) || c == '-')
//		){
//			switch(c){
//			case '-': mBuf[0] = mBuf[0] == '-' ? ' ' : '-'; break;
//			case '.': mHasPoint = true;
//			default : 
//				if(mBuf[mPos] == '.') mHasPoint=false;
//				mBuf[mPos++] = c;
//				if(mPos>=mNum) mNum++;
//			}
//			//printf("%s\n", mBuf); printf("%f\n", value());
//			return true;
//		}
//		return false;
//	}
//
//	
//	double value(){		
//		char * end;
//		double v = strtod(mBuf, &end);
//		if(end != mBuf) mVal = v;
//		return mVal;
//	}
//	
//
//	void value(double v){
//		char t[BUFSIZE];
//#ifdef WIN32
//#define snprintf _snprintf
//#endif
//		int n = snprintf(t, BUFSIZE-1, "% g", v);	// %f is not precise enough!
//#ifdef WIN32
//#undef snprintf
//#endif
//		
//		if(n>1){
//			reset();
//
//			// eliminate trailing zeros
//			while(n-1){
//				if(t[n-1] == '0' || t[n-1] == '.'){ t[n-1] = '\0'; --n; }
//				else break;
//			}
//			
//			if(n==1) read('0');
//			else{
//				for(int i=0; i<n; ++i){ read(t[i]); /*printf(" %d", t[i]);*/}
//			}
//		}
//		printf("\n");
//	}
//
//
//	void reset(){
//		mBuf[0] = ' ';
//		memset(mBuf + 1, 0, (BUFSIZE-1));
//		mPos = 1, mNum = 1;
//		mHasPoint = false;
//		mVal = 0;
//	}
//
//	
//	void del(){
//		if(pos() > 1){
//			for(int i=pos(); i<=mNum; ++i){
//				if(mBuf[i-1] == '.') mHasPoint = false;
//				mBuf[i-1] = mBuf[i];
//			}
//			mBuf[mNum] = '\0';
//			bwd1(); mNum--;
//		}
//	}
//
//	
//	void bwd1(){ if(pos() > 1) mPos--; }
//	void fwd1(){ if(pos() < mNum) mPos++; }
//	
//	void addAtPos(int v){
//		char c = mBuf[pos()];
//		if(isdigit(c)){
//			c += v;
//			mBuf[pos()] = c > '9' ? '9' : c < '0' ? '0' : c;
//		}
//	}
//	
//	const char * buf(){ return mBuf; }
//	int pos(){ return mPos; }
//	
//private:
//	char mBuf[BUFSIZE];
//	int mPos, mNum;
//	double mVal;
//	bool mHasPoint;
//	#undef BUFSIZE
//};

//template <class T, int Nx=1, int Ny=1>
//class RangedValues : public Values<T, Nx, Ny>{
//public:
//
//	T& operator[](int i){ return mVal[i]; }
//	const T& operator[](int i) const { return mVal[i]; }
//
//	RangedValues& range(T max, T min=0){ mMax=max; mMin=min; return *this; }
//
//	
//
//protected:
//	T mMin, mMax;
//	
//	void valueSet(T v){
//		v>mMax ? v=mMax : v<mMin ? v=mMin : 0;
//		value()[selected()] = v;
//	}
//	
//};

//template <class V>
//class ValuesRef{
//public:
//	ValuesRef(): mVal(0){}
//
//	V& data(){ return *mVal; }
//	const V& data() const { return *mVal; }
//
//	// interface methods
//	void resize(int nx, int ny){}
//	
//	void ref(V& r){ mVal = &r; }
//	
//protected:
//	V * mVal;
//};


// We something that wraps around more atomic types to give them all a common
// interface. However, if the atomic type has the interface methods already,
// they should be used. Otherwise, the wrapper's default methods should be called.


}	// glv::

#endif

