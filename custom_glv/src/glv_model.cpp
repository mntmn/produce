#include "glv_model.h"
//#include <cctype>	// isalnum, isblank
#include <ctype.h>	// isalnum, isblank
#include <stdio.h>	// sscanf, FILE
#include <string.h>	// strchr, strpbrk

#ifndef WIN32
#define	sprintf_s(buffer, buffer_size, stringbuffer, ...) (snprintf(buffer, buffer_size, stringbuffer, __VA_ARGS__))
#endif

namespace glv{

bool isBlank(char c){
	return (c==' ') || (c=='\t');
}


bool isIdentifier(const std::string& s){
	if(isalpha(s[0]) || s[0]=='_'){
		unsigned i=1;
		for(; i<s.size(); ++i){
			if(!isalnum(s[i])) break;
		}
		if(s.size()==i) return true;
	}
	return false;
}

template<> int toString<bool>(std::string& dst, const bool& src){
	dst = src ? "1" : "0";
	return 1;
}
template<> int toString<int>(std::string& dst, const int& src){
	char buf[20]; sprintf_s(buf, sizeof(buf), "%i", src);
	dst = buf;
	return 1;
}
template<> int toString<float>(std::string& dst, const float& src){
	char buf[32]; sprintf_s(buf, sizeof(buf), "%g", src); // "%.24g"
	dst = buf;
	return 1;
}
template<> int toString<double>(std::string& dst, const double& src){
	char buf[40]; sprintf_s(buf, sizeof(buf), "%lg", src); // "%.32lg"
	dst = buf;
	return 1;
}
template<> int toString<std::string>(std::string& dst, const std::string& src){
	dst = src; return 1;
}
int toString(std::string& dst, const char * src){ return toString(dst, std::string(src)); }

template<> int fromToken<bool>(bool& dst, const std::string& src){
	char v = dst;
	int r = sscanf(src.c_str(), "%hhi", &v) > 0;
	dst = v;
	return r;
}
template<> int fromToken<int>(int& dst, const std::string& src){
	return sscanf(src.c_str(), "%i", &dst) > 0;
}
template<> int fromToken<float>(float& dst, const std::string& src){
	return sscanf(src.c_str(), "%g", &dst) > 0;
}
template<> int fromToken<double>(double& dst, const std::string& src){
	return sscanf(src.c_str(), "%lg", &dst) > 0;
}
template<> int fromToken<std::string>(std::string& dst, const std::string& src){
	int b = src.find_first_not_of("\"");
	int e = src.find_last_not_of("\"");
	dst = src.substr(b, e-b+1);
	return 1;
}
template<class T>
static int fromToken(
	T * dst, int size, int stride, const std::string& src, const char * match, const char * format
){
	const char * s = src.c_str();
	int i=-1;
	while(++i<size && s){
		if(!(s = strpbrk(s, match))) break;
		T v; sscanf(s, format, &v);
		dst[i*stride] = v;
		s = strpbrk(s, " ,");
	}
	return i;
}
template<> int fromToken<bool>(bool * dst, int size, int stride, const std::string& src){
	return fromToken(dst,size,stride, src, "01","%hhi");
}
template<> int fromToken<int>(int * dst, int size, int stride, const std::string& src){
	return fromToken(dst,size,stride, src, "0123456789-+","%i");
}
template<> int fromToken<float>(float * dst, int size, int stride, const std::string& src){
	return fromToken(dst,size,stride, src, "0123456789-+.","%g");
}
template<> int fromToken<double>(double * dst, int size, int stride, const std::string& src){
	return fromToken(dst,size,stride, src, "0123456789-+.","%lg");
}
template<> int fromToken<std::string>(std::string * dst, int size, int stride, const std::string& src){
	const char * s = src.c_str();
	int i=-1;
	while(++i<size && s){
		if(!(s = strchr(s, '\"'))) break;
		++s;
		const char * e = strchr(s, '\"');
		if(!e) break;
		dst[i*stride].assign(s, e-s);
		s = strpbrk(e, " ,");
	}
	return i;
}

template<> int toToken<std::string>(std::string& dst, const std::string& src){
	dst = "\"" + src + "\"";
	return 1;
}
int toToken(std::string& dst, const char * src){ return toToken(dst, std::string(src)); }


int fromToken(Data& d, const std::string& s){
	switch(d.type()){
	case Data::BOOL:	return glv::fromToken(       d.elems<bool>(), d.size(), d.stride(), s);
	case Data::INT:		return glv::fromToken(        d.elems<int>(), d.size(), d.stride(), s);
	case Data::FLOAT:	return glv::fromToken(      d.elems<float>(), d.size(), d.stride(), s);
	case Data::DOUBLE:	return glv::fromToken(     d.elems<double>(), d.size(), d.stride(), s);
	case Data::STRING:	return glv::fromToken(d.elems<std::string>(), d.size(), d.stride(), s);
	default: return 0;
	}
}
int toString(std::string& s, const Data& d){
	if(!d.hasData()) return 0;
	switch(d.type()){
	case Data::BOOL:	return glv::toString(s,        d.elems<bool>(), d.size(), d.stride());
	case Data::INT:		return glv::toString(s,         d.elems<int>(), d.size(), d.stride());
	case Data::FLOAT:	return glv::toString(s,       d.elems<float>(), d.size(), d.stride());
	case Data::DOUBLE:	return glv::toString(s,      d.elems<double>(), d.size(), d.stride());
	case Data::STRING:	return glv::toString(s, d.elems<std::string>(), d.size(), d.stride());
	default: return 0;
	}
}
int toToken(std::string& s, const Data& d){
	if(!d.hasData()) return 0;
	switch(d.type()){
	case Data::BOOL:	return glv::toToken(s,        d.elems<bool>(), d.size(), d.stride());
	case Data::INT:		return glv::toToken(s,         d.elems<int>(), d.size(), d.stride());
	case Data::FLOAT:	return glv::toToken(s,       d.elems<float>(), d.size(), d.stride());
	case Data::DOUBLE:	return glv::toToken(s,      d.elems<double>(), d.size(), d.stride());
	case Data::STRING:	return glv::toToken(s, d.elems<std::string>(), d.size(), d.stride());
	default: return 0;
	}
}


// Get iteration count for element-wise operations
static inline int count(const Data& a, const Data& b){
	return a.size()<b.size() ? a.size() : b.size();
}

//#define PDEBUG printf("%p %s\t", this, __func__); print(); printf("\n")
#define PDEBUG
/*
	pointer mData;				// pointer to first element of source data
	pointer mElems;				// pointer to first element in this slice
	int mStride;				// stride factor
	int mSizes[DATA_MAXDIM];	// sizes of each dimension
	Type mType;	
*/

Data::Data()
:	mData(0), mElems(0), mStride(1), mType(Data::NONE)
{
	shapeAll(0);
	PDEBUG;
}

Data::Data(Data& v)
:	mData(0), mElems(0), mStride(1), mType(Data::NONE)
{ shapeAll(0); *this = v; PDEBUG; }

Data::Data(const Data& v)
:	mData(0), mElems(0), mStride(1), mType(Data::NONE)
{ shapeAll(0); *this = v; PDEBUG; }

Data::Data(Data::Type type, int n1, int n2, int n3, int n4)
:	mData(0), mElems(0), mStride(1), mType(type)
{
	int s[] = {n1,n2,n3,n4};
	shape(s,4);
	clone();
	if(isNumerical()) assignAll(0);
	PDEBUG;
}

Data::~Data(){
	free();
}

Data& Data::operator= (const Data& v){
	if(&v != this){
		setRaw(v.mData, v.offset(), v.stride(), v.type());
		for(int i=0;i<maxDim();++i) mSizes[i]=v.mSizes[i];
	}
	return *this;
}

bool Data::operator==(const Data& v) const {
	if(hasData() && v.hasData()){
		// TODO: how do we compare numbers and strings?

		#define OP(t1, t2)\
			for(int i=0; i<count(*this,v); ++i){\
				if(elem<t1>(i) != v.elem<t2>(i)) return false;\
			} return true

		#define OPALL(t)\
			switch(v.type()){\
			case Data::BOOL:	OP(t, bool);\
			case Data::INT:		OP(t, int);\
			case Data::FLOAT:	OP(t, float);\
			case Data::DOUBLE:	OP(t, double);\
			default: return false;\
			}

		switch(type()){
		case Data::BOOL:	OPALL(bool);
		case Data::INT:		OPALL(int);
		case Data::FLOAT:	OPALL(float);
		case Data::DOUBLE:	OPALL(double);
		case Data::STRING:	OP(std::string, std::string);
		default:;
		}
	}
	return false;
	#undef OP
	#undef OPALL
}

Data& Data::operator+=(const Data& v){

	#define OP(t1, t2)\
	for(int i=0; i<count(*this,v); ++i){ elem<t1>(i) += v.elem<t2>(i); } break

	#define OPALL(t)\
		switch(v.type()){\
		case BOOL:		OP(t, bool);\
		case INT:		OP(t, int);\
		case FLOAT:		OP(t, float);\
		case DOUBLE:	OP(t, double);\
		case STRING:\
		default:;\
		} break

	switch(type()){
	case Data::BOOL:	OPALL(bool);
	case Data::INT:		OPALL(int);
	case Data::FLOAT:	OPALL(float);
	case Data::DOUBLE:	OPALL(double);
	case Data::STRING: //printf("copy strings: %p -> %p\n", &v, this);
		switch(v.type()){
//		case Data::BOOL:	v.toString(data<std::string>());
		case Data::STRING:	OP(std::string, std::string);
		default:;
		} break;
	default:;
	}
	return *this;
	#undef OP
	#undef OPALL
}

Data& Data::assign(const Data& v, int ind1, int ind2){

	if(hasData() && v.hasData()){
		int id= indexFlat(ind1,ind2);	// starting index of destination
		int nd= size()-id;				// number of destination elements to assign
		int n = nd < v.size() ? nd : v.size();

		#define OP(t1, t2)\
		for(int i=0; i<n; ++i){ elem<t1>(i+id) = v.elem<t2>(i); } break

		#define OPALL(t)\
			switch(v.type()){\
			case BOOL:		OP(t, bool);\
			case INT:		OP(t, int);\
			case FLOAT:		OP(t, float);\
			case DOUBLE:	OP(t, double);\
			case STRING:\
			default:;\
			} break

		switch(type()){
		case Data::BOOL:	OPALL(bool);
		case Data::INT:		OPALL(int);
		case Data::FLOAT:	OPALL(float);
		case Data::DOUBLE:	OPALL(double);
		case Data::STRING: //printf("copy strings: %p -> %p\n", &v, this);
			switch(v.type()){
//			case Data::BOOL:	v.toString(data<std::string>());
//			case Data::FLOAT:	OP(t, float);
//			case Data::DOUBLE:	OP(t, double);
			case Data::STRING:	OP(std::string, std::string);
			default:;
			} break;
		default:;
		}
	}
	return *this;
	#undef OP
	#undef OPALL
}

void Data::clone(){
	if(hasData()){
		int cnt = references(mData);
		
		// cnt == 0		data points to external, unmanaged data
		// cnt  > 1		data points to another Data's cloned data
		// cnt == 1		data points to my own cloned data
		// allocate new memory if not sole owner of data
		if(cnt!=1){
			Data old(*this);
			realloc(type());
			assign(old);
		}
	}
	else{
		realloc(type());
	}
}

void Data::free(){
PDEBUG;
	if(release(mData)){
		switch(type()){
		case Data::BOOL:	delete[] data<bool>(); break;
		case Data::INT:		delete[] data<int>(); break;
		case Data::FLOAT:	delete[] data<float>(); break;
		case Data::DOUBLE:	delete[] data<double>(); break;
		case Data::STRING:	delete[] data<std::string>(); break;
		default:;
		}
//		::free(mData);
	}
	mData=0;
	mElems=0;
	mStride=1;
	shapeAll(0);
	mType=Data::NONE;
PDEBUG;
}

int Data::order() const {
	int r=0;
	for(int i=0; i<maxDim(); ++i){
		if(size(i)>0) ++r; 
	}
	return r;
}

// return highest dimension not equal to 1
static int highestDim(const Data& d){
	int i=d.maxDim()-1;
	for(; i>=0; --i){
		if(d.size(i) > (i?1:0)) break;
	}
	return i+1;
}

void Data::print() const{
	printf("%p + %d, (%d", mData, offset(), size(0));
	for(int i=1;i<highestDim(*this);++i){
		printf(",%d", size(i));
	}
	printf("):%+d, %s %s\n", stride(), typeToString(type()).c_str(), toToken().c_str());
}

// TODO: copy over as much of the previous values as possible
// if sizes==0, then keep current shape
void Data::realloc(Data::Type t, const int * sizes, int n){
	if(sizes){
		free();
		shape(sizes, n);
	}
	else{
		Data old(*this);
		free();
		shape(old.mSizes, old.maxDim());
	}	

	if(size()){
		mType  = t;
		mStride= 1;
		switch(type()){
		case Data::BOOL:	mData = pointer(new bool[size()]); break;
		case Data::INT:		mData = pointer(new int[size()]); break;
		case Data::FLOAT:	mData = pointer(new float[size()]); break;
		case Data::DOUBLE:	mData = pointer(new double[size()]); break;
		case Data::STRING:	mData = pointer(new std::string[size()]); break;
		default:			return;
		}
		acquire(mData);
		offset(0);
		if(hasData() && isNumerical()) assignAll(0);
	}
}

Data& Data::resize(const int * sizes, int n){
	return resize(type(), sizes, n);
}

Data& Data::resize(Data::Type t, const int * sizes, int n){
PDEBUG;
	if(t!=NONE && (type()!=t || size()!=product(sizes,n))){
		realloc(t, sizes,n);
	}
PDEBUG;
	return *this;
}

void Data::setRaw(void * dt, int off, int stride, Type ty){
	// increment reference count first to avoid problems when dt == mData
	incrementCount(pointer(dt));
	free();
	mData  = pointer(dt);
	mStride= stride;
	mType  = ty;
	offset(off);
}

// TODO: conform sizes to current size
Data& Data::shape(const int * sizes, int n){
	if(n){
		if(n <= maxDim()){
			for(int i=0;i<n;++i) mSizes[i]=sizes[i];
			for(int i=n;i<maxDim();++i) mSizes[i]=1;
		}
		else{
			for(int i=0;i<maxDim();++i) mSizes[i]=sizes[i];
		}
	}
	return *this; 
}

Data& Data::shapeAll(int n){
	for(int i=0;i<maxDim();++i) mSizes[i]=n;
	return *this;
}

int Data::sizeType() const {
	switch(type()){
	case Data::BOOL:	return sizeof(bool);
	case Data::INT:		return sizeof(int);
	case Data::FLOAT:	return sizeof(float);
	case Data::DOUBLE:	return sizeof(double);
	case Data::STRING:	return sizeof(std::string);
	default:			return 1; // non-zero to avoid divide by 0 exceptions
	}
}

Data Data::slice(int beg, int sz, int st) const {
	Data r(*this);
	int b = r.offset() + beg;
	
	// constrain offset and size to limits of parent data
	if(b < 0) b = 0;

	// TODO: constrain size
	int s = sz;
	
	r.offset(b);
	r.stride(st);
	r.shape(&s, 1);
	return r;
}

Data& Data::type(Data::Type ty){
	if(type()!=ty && size()){
		resize(ty, mSizes, maxDim());
	}
	else{
		mType=ty;
	}
	return *this;
}

std::string Data::typeToString(Type t){
	switch(t){
	case BOOL:	return "bool";
	case INT:	return "int";
	case FLOAT:	return "float";
	case DOUBLE:return "double";
	case STRING:return "std::string";
	default:	return "void";
	}
}

// TODO:	need to delete allocated Models
//			need to be able to remove Data
//void ModelManager::add(const std::string& name, Data& v){
//	if(isIdentifier(name)) mNameVal[name] = new Model(v);
//}

void ModelManager::add(const std::string& name, Model& v){
	if(isIdentifier(name)) mState[name] = &v;
}
void ModelManager::add(const std::string& name, const Model& v){
	if(isIdentifier(name)) mConstState[name] = &v;
}

void ModelManager::remove(const std::string& name){
	if(mState.count(name)){
//		Model * m = mNameVal[name];
		mState.erase(name);
//		if(allocated(m)){
//			delete m;
//		}
	}
}



//struct TableWriter{
//
//	virtual void onKeyValue(const std::string& key, const std::string& val) = 0;
//
//	void operator()(std::string& dst, const std::string& modelName){
//		#define NEWLINE "\r\n"
//		if(modelName.size())	dst = "[\"" + modelName + "\"] = {"NEWLINE;
//		else					dst = "{"NEWLINE;
//
//		std::string t;
//		{
//			NamedModels::const_iterator i = mState.begin();
//			for(; i!=mState.end(); ++i){
//				dst += "\t" + namedDataToString(i->first, i->second->model());
//			}
//		}
//		{
//			NamedConstModels::const_iterator i = mConstState.begin();
//			for(; i!=mConstState.end(); ++i){
//				dst += "\t" + namedDataToString(i->first, i->second->model());
//			}
//		}
//		dst += "}"NEWLINE;
//		#undef NEWLINE
//	}
//
//};


int ModelManager::snapshotsToFile(const std::string& path) const {
	std::string s;
	if(!snapshotsToString(s)) return 0;

	int r=0;
	FILE * fp = fopen(path.c_str(), "w");
	if(fp){
		r = fwrite(s.c_str(), sizeof(std::string::value_type), s.size(), fp);
		fclose(fp);
	}
	return r;
}

int ModelManager::snapshotsFromFile(const std::string& path, bool add){
	int r=0;
	FILE * fp = fopen(path.c_str(), "r");
	if(fp){
		
		char buf[512];
		std::string s;
		while(!feof(fp)){
			int n = fread(buf, sizeof(buf[0]), sizeof(buf), fp);
			s.append(buf, n);
			r += n;
		}
		
		if(!add) clearSnapshots();
		snapshotsFromString(s);

		fclose(fp);
	}
	return r;
}

bool ModelManager::stateToToken(std::string& dst, const std::string& modelName) const {
	#define NEWLINE "\r\n"
	if(modelName.size())	dst = "[\"" + modelName + "\"] = {"NEWLINE;
	else					dst = "{"NEWLINE;

	std::string t;
	{
		NamedModels::const_iterator i = mState.begin();
		for(; i!=mState.end(); ++i){
			dst += "\t" + namedDataToString(i->first, i->second->getData());
		}
	}
	{
		NamedConstModels::const_iterator i = mConstState.begin();
		for(; i!=mConstState.end(); ++i){
			dst += "\t" + namedDataToString(i->first, i->second->getData());
		}
	}
	dst += "}"NEWLINE;
	#undef NEWLINE
	return true;
}

int ModelManager::snapshotsToString(std::string& dst) const {
	Snapshots::const_iterator it = mSnapshots.begin();
	if(it == mSnapshots.end()) return 0;

	if(name().size())	dst = name() + " = ";
	else				dst = "";

	dst += "{\r\n";
	while(it != mSnapshots.end()){
		dst += "[\"" + it->first + "\"] = {\r\n";
		
		const Snapshot& snapshot = it->second;
		Snapshot::const_iterator jt = snapshot.begin();
		while(jt != snapshot.end()){
			dst += "\t" + namedDataToString(jt->first, jt->second);
			++jt;
		}
		dst += "},\r\n\r\n";
		++it;
	}
	dst += "}\r\n";
	return dst.size();
}


struct KeyValueParser{

	virtual ~KeyValueParser(){}

	virtual void onKeyValue(const std::string& key, const std::string& val) = 0;
	
	int operator()(const std::string& src){

		unsigned p = src.find_first_of("{");
		if(std::string::npos == p) return 0;		// no table found, so return
		
		std::string key, val;
		const char * b = &src[p+1];	// start 1 character after opening '{'

		while(*b && *b!='}'){
			if(isalpha(*b) || *b=='_'){	// is character valid start of identifier?
				const char * e = b+1;
				while(isalnum(*e) || *e=='_') ++e;	// go to end of key name
				key.assign(b, e-b);

				while(isBlank(*e) || *e=='=') ++e;	// go to '='
				
				// find next valid token
				b=e=strpbrk(e, "\"{0123456789.-+");

				if(!b) b=e=&src[src.size()];	// no more valid tokens, so go to end of string

				if(*b){
					// munch characters until end of token
					if(*e == '\"'){
						++e; while(*e!='\"') ++e; ++e;
					}
					else if(*e == '{'){
						while(*e!='}') ++e; ++e;
					}
					else{
						while((isdigit(*e) || *e=='.' || *e=='-' || *e=='+')) ++e;
					}

					val.assign(b,e-b);

					onKeyValue(key, val);
				}
				b=e;
			}
			++b;
		}
		//printf("%d\n", b-&v[0]);
		
		if(*b == '}')	return b+1-&src[0];
		else			return b-&src[0];	
	}
};


int ModelManager::stateFromToken(const std::string& src){

	struct It : KeyValueParser {
		It(NamedModels& v): m(v){}
		void onKeyValue(const std::string& key, const std::string& val){
			if(m.count(key)){
				//printf("%s = %s\n", key.c_str(), val.c_str());
				m[key]->modelFromString(val);
			}
		}
		NamedModels& m;
	} it(mState);
	
	return it(src);
}



static bool goToNext(unsigned& p, char c, const std::string& str){
	unsigned n = str.substr(p).find_first_of(c);
//	printf("%c: %d %d (%.5s ...)\n", c, p, n, str.substr(p).c_str());
	if(std::string::npos != n){
		p += n;
		return true;
	}
	return false;
}

static bool goToNextPrintablePast(unsigned& p, char c, const std::string& str){
	unsigned n = str.substr(p).find_first_not_of(" \t\r\n");
//	printf("%c: %d %d (%.5s ...)\n", c, p, n, str.substr(p).c_str());
	if(std::string::npos != n){
		p += n;
//		printf("\t%c\n", str[p]);
		if(str[p] == c){ ++p; return true; }
	}
	return false;
}

//static int Data::interpretToken(Data::Type& type, int *& sizes, std::string& s){
//	unsigned p=0;
//	int level=-1;
//	int elems=0;
//	
//	for(int i=0;i<maxDim();++i) sizes[i]=0;
//
//	do{
//		switch(s[p]){
//		case '{': ++level; break;
//		case '}': --level; break;
//		case ',':
//		default:;
//		}
//	} while(++p < s.size());
//
////	if(s[p] == '{'){
////	
////	}
////	else{
////	
////	}
//}

int ModelManager::snapshotFromString(const std::string& src){
//	printf("%s\n", src.c_str());
	unsigned r = src.size();
	unsigned p=0, p2=0;

	// look for table name
	if(!goToNextPrintablePast(p, '[' , src)) return r;
	if(!goToNextPrintablePast(p, '\"', src)) return r;
	if(!goToNext(p2=p, '\"', src)) return r;

	std::string name = src.substr(p, p2-p);
//	printf("%s\n", name.c_str());
	p = p2+1;

	if(!goToNextPrintablePast(p, ']' , src)) return r;
	if(!goToNextPrintablePast(p, '=' , src)) return r;
//	printf("%s\n", src.substr(p).c_str());

	// retrieve key-value pairs
	struct It : KeyValueParser {
		It(Snapshot& vs, NamedModels& vm): s(vs), m(vm){}
		void onKeyValue(const std::string& key, const std::string& val){
//			printf("%s = %s\n", key.c_str(), val.c_str());

			// Only convert value string if main state contains key 
			// with same name.
			NamedModels::const_iterator it = m.find(key);
			if(it != m.end()){
				Data& ds = s[key];
				const Data& dm = it->second->getData();
				
				// Use main state as prototype
				ds.resize(dm.type(), dm.shape(), dm.maxDim());
				ds.fromToken(val);
			}
		}
		Snapshot& s;
		NamedModels& m;
	} it(mSnapshots[name], mState);
	
	p += it(src.substr(p));
	return p;
}


int ModelManager::snapshotsFromString(const std::string& src){
	
	unsigned r = src.size();
	unsigned p=0;
	if(!goToNext(p, '{' , src)) return r;
	++p;
	
	do{
		p += snapshotFromString(src.substr(p));
		if(!goToNext(p, ',', src)) return r;
		++p;
	} while(p < src.size());

	return p;
}


void ModelManager::saveSnapshot(const std::string& name){
	if(mSnapshots.count(name)){} // TODO: always overwrite existing?

	Snapshot& snapshot = mSnapshots[name];

	NamedModels::iterator it = mState.begin();
	while(it != mState.end()){
		(snapshot[it->first] = it->second->getData()).clone();
		++it;
	}
}


void ModelManager::loadSnapshot(const std::string& name){
	if(mSnapshots.count(name)){
		Snapshot& snapshot = mSnapshots[name];

		NamedModels::iterator it = mState.begin();
		while(it != mState.end()){
			if(snapshot.count(it->first)){
				it->second->setData(snapshot[it->first]);
			}
			++it;
		}
	}
}

} // glv::
