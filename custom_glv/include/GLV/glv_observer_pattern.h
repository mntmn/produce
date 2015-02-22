#ifndef INC_GLV_NOTIFICATION_H
#define INC_GLV_NOTIFICATION_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <vector>

namespace glv {

class Notifier;


/// Notification update types
namespace Update{
	enum t{
		Focus=0,	/**< Focus update */
		Value,		/**< Value update */
		NumTypes
	};
}


/// Generic index/value struct for widget notifications

/// Since some widgets have multiple values, an index is used to specify which
/// element corresponds to the update. If a widget only has one value, an index
/// of zero is used.
template <class T>
class ChangedValue{
public:
	
	/// @param[in] v	value
	/// @param[in] i	index of value
	ChangedValue(const T& v, int i=0): mIndex(i), mValue(v){}
	
	const T& value() const { return mValue; }	///< Get value
	int index() const { return mIndex; }		///< Get index of value

private:
	int mIndex;
	T mValue;
};



/// Notification object

/// This is passed into notification callbacks
///
class Notification{
public:

	/// @param[in] sndr		Pointer to sending object (subject)
	/// @param[in] rcvr		Pointer to receiving object (observer)
	/// @param[in] data		Pointer to data object
	Notification(void * sndr, void * rcvr=0, const void * data=0)
	:	mSender(sndr), mReceiver(rcvr), mData(data)
	{}

	/// Get pointer to sending object
	template <class T>
	T * sender() const { return static_cast<T *>(sender()); }

	/// Get raw pointer to sending object
	void * sender() const { return mSender; }

	/// Get pointer to receiving object
	template <class T>
	T * receiver() const { return static_cast<T *>(receiver()); }

	/// Get raw pointer to receiving object
	void * receiver() const { return mReceiver; }

	/// Get raw pointer to data object
	const void * data() const { return mData; }

	/// Get pointer to data object
	template <class T>
	const T * data() const { return static_cast<T *>(data()); }

protected:
	void * mSender;
	void * mReceiver;
	const void * mData;
};




/// Notifies one or more observers of state change
class Notifier{
private:
	typedef void (* callback)(const Notification& n);

public:
	
	Notifier(): mHandlers(0){}

	// Non-virtual unless someone will delete a derived-class object via
	// a Notifier pointer...
	~Notifier(){ delete[] mHandlers; }

	/// Attach a new notification callback, type, and receiver
	void attach(callback cb, Update::t n, void * rcvr=0){
		handlers()[n].push_back(Handler(cb, rcvr));
	}

	
	/// Detach an existing notification callback, type, and receiver
	void detach(callback cb, Update::t n, void * rcvr=0){
	
		if(hasHandlers()){
			for(unsigned int i=0; i<handlers()[n].size(); i++){
				Handler& h = handlers()[n][i];
				if(h.handler == cb && h.receiver == rcvr)
					handlers()[n].erase(handlers()[n].begin() + i);
			}		
		}
	}


	/// Notify observers of a specific update type
	void notify(void * sender, Update::t n, void * data=0){
	
		if(!hasHandlers() || handlers()[n].empty()) return;

		// call handlers in FIFO order
		int i=handlers()[n].size();
		while(i--){
			Handler& h = handlers()[n][i];
			if(h.handler) h.handler(Notification(sender, h.receiver, data));
		}
	}

	/// Notify observers of a specific update type
	void notify(Update::t n, void * data=0){ notify(this, n, data); }
	
	/// Notify observers of a specific update type
	template <class T>
	void notify(Update::t n, const ChangedValue<T>& v){
		notify(n, (void *)&v);		
	}

	/// Returns number of observers for this update type
	int numObservers(Update::t n) const {
		if(hasHandlers()) return handlers()[n].size();
		return 0;
	}

protected:

	struct Handler{
		Handler(callback c, void * r): handler(c), receiver(r){}
		callback handler;
		void * receiver;
	};

	// Array of vectors for each notification type
	std::vector<Handler> * mHandlers;
	
	// Setter dynamically allocates new memory
	std::vector<Handler> * handlers(){
		if(0 == mHandlers){
			mHandlers = new std::vector<Handler>[Update::NumTypes];
		}
		return mHandlers;
	}

	// Getter just returns pointer (even if 0)
	const std::vector<Handler> * handlers() const {
		return mHandlers;
	}
	
	bool hasHandlers() const { return handlers() != 0;  }
};

} // glv::
#endif
