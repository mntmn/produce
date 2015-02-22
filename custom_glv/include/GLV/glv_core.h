#ifndef INC_GLV_CORE_H
#define INC_GLV_CORE_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <map>
#include <string>
#include <list>
#include "glv_rect.h"
#include "glv_observer_pattern.h"
#include "glv_color.h"
#include "glv_draw.h"
#include "glv_font.h"
#include "glv_model.h"
#include "glv_util.h"

namespace glv {

class GLV;
class View;

/// Type for graphical space values, i.e. dimensions, extents, positions, etc..
typedef float space_t;

/// Type for View rectangle geometry
typedef TRect<space_t> Rect;

/// Type for a drawing callback
typedef void (*drawCallback)(View * v, GLV& g);

/// Type for an event callback

/// The first parameter is the View receiving the event and the second is the
/// GLV context sending the event.  The function returns true if the event is
/// to be bubbled up to the receiver's parent View.
typedef bool (*eventCallback)(View * v, GLV& g);

/// Type for a list of event callbacks
typedef std::list<eventCallback> eventCallbackList;



/// View property flags
namespace Property{
	enum t{
		Visible			=1<< 0,	/**< Whether to draw myself */
		DrawBack		=1<< 1,	/**< Whether to draw back rect */
		DrawBorder		=1<< 2,	/**< Whether to draw border */
		CropChildren	=1<< 3,	/**< Whether to crop children when drawing */
		CropSelf		=1<< 4,	/**< Whether to crop own drawing routine(s) */
		Focused			=1<< 5,	/**< Whether View is focused */
		FocusHighlight	=1<< 6,	/**< Whether to highlight border when focused */
		FocusToTop		=1<< 7, /**< Whether to bring to top when focused */
		HitTest			=1<< 8,	/**< Whether View can be clicked */
		Controllable	=1<< 9,	/**< Whether View can be controlled through events. Bubbles all events. */
		AlwaysBubble	=1<<10, /**< Whether to bubble all events to parent */
		Maximized		=1<<11, /**< Whether geometry is matched to parent's */
		KeepWithinParent=1<<12, /**< Ensure that View is fully contained within parent */
		Animate			=1<<13, /**< Whether to animate */
		
		DrawGrid		=1<<27,	/**< Whether to draw grid lines between widget elements */
		DrawSelectionBox=1<<28,	/**< Whether to draw a box around selected widget elements */
		Momentary		=1<<29,	/**< Whether widget element goes back to initial value on mouse up */
		MutualExc		=1<<30,	/**< Whether only one element of a widget can be non-zero */
		SelectOnDrag	=1<<31,	/**< Whether a new element of a widget is selected while dragging */
	};
	inline t operator| (const t& a, const t& b){ return t(int(a) | int(b)); }
	inline t operator& (const t& a, const t& b){ return t(int(a) & int(b)); }
	inline t operator^ (const t& a, const t& b){ return t(int(a) ^ int(b)); }
};

using namespace Property;


/// Direction types
struct Direction{

	/// Direction enumeration
	enum{
		N = 1<<0,	/**< North */
		E = 1<<1,	/**< East */
		S = 1<<2,	/**< South */
		W = 1<<3	/**< West */
	};

	Direction(): val(0){}
	Direction(int v): val(v){}
	operator int(){ return val; }
	int val;
};



/// Recognized event types.
namespace Event{

	/// Event type enum
	enum t{
		Null = 0,		/**< No event */
		
		// core events
		Quit,			/**< Application quit */
		WindowCreate,	/**< Sent by window upon creation to its attached GLV */ 
		WindowDestroy,	/**< Sent by window upon destruction to its attached GLV */ 
		WindowResize,	/**< Sent by window upon resize to its attached GLV */
		
		// view events
		FocusGained,	/**< View got focus */
		FocusLost,		/**< View lost focus */
		
		// mouse events
		MouseDown,		/**< Mouse button pressed */
		MouseUp,		/**< Mouse button released */
		MouseMove,		/**< Mouse has moved */
		MouseDrag,		/**< Mouse has moved with a button pressed */
		MouseWheel,		/**< Mouse wheel moved */
		
		// keyboard events
		KeyDown,		/**< Keyboard key pressed*/
		KeyUp,			/**< Keyboard key released */
		KeyRepeat,		/**< Keyboard key auto-repeated */
		
		// window events
	//	WindowActivated,	
	//	WindowDeactivated,
		Unused,			/**< Add to this for runtime event types */
		NumTypes		/**< Number of event types */
	};

	/// Returns a string of event type.
	const char * toString(const Event::t e);
}



/// Constants of keyboard keys.
namespace Key{
	enum t{
		
		// Standard ASCII non-printable characters
		Enter		=3,		/**< */
		BackSpace	=8,		/**< */
		Tab			=9,		/**< */
		Return		=13,	/**< */
		Escape		=27,	/**< */
		Delete		=127,	/**< */
			
		// Non-standard, but common keys.
		F1=256, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, 
		Insert, Left, Up, Right, Down, PageDown, PageUp, End, Home,
	};
}



/// Placement types
namespace Place{
	enum t{
		TC=1<<0,	/**< Top-center */
		TR=1<<1,	/**< Top-right */
		CR=1<<2,	/**< Center-right */
		BR=1<<3,	/**< Bottom-right */
		BC=1<<4,	/**< Bottom-center */
		BL=1<<5,	/**< Bottom-left */
		CL=1<<6,	/**< Center-left */
		TL=1<<7,	/**< Top-left */
		CC=1<<8		/**< Center-center */
	};
}



/// Holds keyboard state.
class Keyboard{
	friend class GLV;
public:

	/// Constructor.
	Keyboard();
	~Keyboard(){}
	
	int key() const;		///< Returns ASCII key code of last key event
	int keyAsNumber() const;///< Returns decimal number correlating to key code
	bool alt() const;		///< Whether an alt key is down
	bool caps() const;		///< Whether capslock is down
	bool ctrl() const;		///< Whether a ctrl key is down
	bool meta() const;		///< Whether a meta (e.g. windows, apple) key is down
	bool shift() const;		///< Whether a shift key is down
	bool isDown() const;	///< Whether last event was button down
	bool isNumber() const;	///< Whether key is a number key
	bool key(int k) const;	///< Whether the last key was 'k'

	void alt  (bool v);		///< Set alt key state
	void caps (bool v);		///< Set alt key state
	void ctrl (bool v);		///< Set ctrl key state
	void meta (bool v);		///< Set meta key state
	void shift(bool v);		///< Set shift key state

	void print();			///< Print keyboard state to stdout

protected:
	int	mKeycode;			// last key event key number
	bool mIsDown;			// last key event state (pressed or released)
	bool mModifiers[5];		// Modifier key state array (shift, alt, ctrl, caps, meta)
};



/// Holds mouse state.
class Mouse{
friend class GLV;
public:

	/// Mouse button enumeration
	enum{
		Left = 0,							/**< Left mouse button */
		Middle,								/**< Middle mouse button */
		Right,								/**< Right mouse button */
		Extra = (GLV_MAX_MOUSE_BUTTONS - 1)	/**< Extra mouse button */
	};

	/// Constructor.
	Mouse();
	~Mouse(){};

	space_t x() const;				///< Current x position
	space_t y() const;				///< Current y position
	space_t x(int button) const;	///< Get button's last down absolute x position
	space_t y(int button) const;	///< Get button's last down absolute y position
	space_t w() const;				///< Current wheel position
	
	space_t dx() const;				///< Current x velocity
	space_t dy() const;				///< Current y velocity
	space_t dw() const;				///< Current wheel velocity
	space_t ddx() const;			///< Current x acceleration
	space_t ddy() const;			///< Current y acceleration
	space_t ddw() const;			///< Current wheel acceleration

	space_t xRel() const;			///< Current x position relative to current listener
	space_t yRel() const;			///< Current y position relative to current listener
	space_t xRel(int button) const;	///< Get button's last down relative x position
	space_t yRel(int button) const;	///< Get button's last down relative y position
	
	int button() const;				///< Last event button number
	int clicks() const;				///< Number of sequential clicks of buttons
	bool isDown() const;			///< Whether last event was button down
	bool isDown(int button) const;	///< Whether button is currently down
	bool left() const;				///< Whether left button is currently down
	bool middle() const;			///< Whether middle button is currently down
	bool right() const;				///< Whether right button is currently down
	
	float sens() const { return (left() && right()) ? 0.25 : 1; }

	void print() const;				///< Print out information about Mouse to stdout

	void setContext(View * v);		///< Translate relative pos into sub-View
	void unsetContext(View * v);	///< Translate relative pos out of sub-View

protected:

	// Current (0) and previous (1,2) absolute mouse positions
	space_t mX[3], mY[3], mW[3];
	
	// Mouse coords in the current listener's relative coordinate space.
	space_t mXRel, mYRel;
	
	bool b[GLV_MAX_MOUSE_BUTTONS];			// button-down states
	space_t bx[GLV_MAX_MOUSE_BUTTONS];		// button-down absolute coordinates
	space_t by[GLV_MAX_MOUSE_BUTTONS];	
	space_t bxRel[GLV_MAX_MOUSE_BUTTONS];	// button-down relative coordinates
	space_t byRel[GLV_MAX_MOUSE_BUTTONS];	

	int	mButton;
	int mClicks;
	bool mIsDown;
	
	void bufferPos(space_t newPos, space_t * prev3Pos);
	void pos(int x1, int y1);
	void posRel(space_t relx, space_t rely);
	void updateButton(int btn, bool pressed, int clicks);
};



/// Color style for View appearance.
class StyleColor{
public:
	StyleColor();
	
	/// Color style presets
	enum Preset{
		BlackOnWhite,		/**< */
		Gray,				/**< */
		SmokyGray,			/**< */
		WhiteOnBlack		/**< */
	};

	Color back;			///< Background
	Color border;		///< Border outline
	Color fore;			///< Foreground
	Color selection;	///< Selection
	Color text;			///< Text

	/// Set style based on preset
	void set(Preset preset);
	
	/// Set style based on a single color
	void set(const Color& c, float contrast=0.4);
};

//class StyleShape{
//public:
//
//	void (* back  )(float l, float t, float r, float b);
//	void (* border)(float l, float t, float r, float b);
//};


/// Overall appearance scheme.
class Style : public SmartPointer{
public:
	Style(bool deletable=false);
	
	StyleColor color;	///< Color style

	/// Get reference to standard style
	static Style& standard(){
	
		// Note: This uses a Construct On First Use Idiom to avoid unpredictable
		// static initialization order.  The memory allocated will get freed from 
		// the heap when the program exits.
		static Style * v = new Style(false);
		return *v;
	}
};



/// The base class of all GUI elements.
class View : public Rect, public DataModel, public Notifier, public SmartObject<View> {

friend class GLV;

public:

	/// @param[in] left		Initial left edge position
	/// @param[in] top		Initial top edge position
	/// @param[in] width	Initial width
	/// @param[in] height	Initial height
	/// @param[in] cb		Drawing callback
	View(space_t left, space_t top, space_t width, space_t height, drawCallback cb=0);

	
	/// @param[in] rect		Rect geometry of View
	/// @param[in] anchor	Anchor place
	/// @param[in] cb		Drawing callback
	View(const Rect& rect=Rect(200, 200), Place::t anchor=Place::TL, drawCallback cb=0);

	virtual ~View();

	virtual const char * className() const { return "View"; } ///< Get class name
	virtual void onAnimate(double dsec, GLV& g){}	///< Animation callback
	virtual void onDraw(GLV& g);					///< Drawing callback
	virtual bool onEvent(Event::t e, GLV& g);		///< Event callback to be called after those in callback list
	virtual void onResize(space_t dx, space_t dy);	///< Resize callback
	virtual void onDataModelSync(){}				///< Update internal values if different from attached model variables

	// Doubly linked tree list of views
	// TODO: move this stuff to a Node sub-class
	View * parent;				///< My parent view
	View * child;				///< My first child (next to be drawn)
	View * sibling;				///< My next sibling view (drawn after all my children)
	void add(View& child);		///< Add a child view to myself, and update linked lists
	void add(View* child);		///< Add a child view to myself, and update linked lists
	void makeLastSibling();		///< Put self at end of sibling chain
	void remove();				///< Remove myself from the parent view, and update linked lists

	/// An action to be called when traversing the node tree
	struct TraversalAction{
		virtual ~TraversalAction(){}
		virtual bool operator()(View * v, int depth) = 0;
	};
	struct ConstTraversalAction{
		virtual ~ConstTraversalAction(){}
		virtual bool operator()(const View * v, int depth) = 0;
	};

	/// Traverse tree depth-first applying an action at each node
	void traverseDepth(TraversalAction& action);
	void traverseDepth(ConstTraversalAction& action) const;

	/// Add a child view to myself
	View& operator << (View& newChild){ add(newChild); return *this; }
	View& operator << (View* newChild){ add(newChild); return *this; }

	
	/// Map of event callback sequences.
	std::map<Event::t, eventCallbackList> callbackLists;
	
	drawCallback draw;		///< Drawing callback
	
	
	bool absToRel(View * target, space_t& x, space_t& y) const;
	StyleColor& colors() const;					///< Get style colors
	const std::string& descriptor() const;		///< Get descriptor
	int enabled(Property::t v) const;			///< Returns whether a property is set
	Font& font();								///< Get font
	bool hasCallback(Event::t e, eventCallback cb) const; ///< Returns whether a particular callback has been registered
	bool hasCallbacks(Event::t e) const;		///< Returns whether there are callback(s) registered for a particular event
	const std::string& name() const;			///< Get name
	int numCallbacks(Event::t e) const;			///< Returns number of registered callbacks
	const View * posAbs(space_t& al, space_t& at) const; ///< Computes absolute left-top position. Returns topmost parent view.
	void printDescendents() const;				///< Print tree of descendent Views to stdout
	void printFlags() const;
	bool showing() const;						///< Returns whether View is being shown
	Style& style() const { return *mStyle; }	///< Get style object
	const View * toAbs(space_t& x, space_t& y) const; ///< Converts relative View x-y coordinates to absolute. Returns topmost parent view.
	std::string valueString() const;			/// Returns model value(s) as string
	int visible() const;						///< Returns whether View visibility flag is set

	View& anchor(space_t mx, space_t my);		///< Set translation factors relative to parent resize amount
	View& anchor(Place::t parentPlace);			///< Set translation factors relative to parent resize amount

	/// Push an event callback onto the back of the event callbacks list.

	/// If the event callback already exists in the list, then it will be not be
	/// added.
	/// If multiple callbacks are registered for the same event type, then two
	/// types of prioritization take place. First, callbacks on the front of the
	/// list get called first. Second, if a callback returns false (for 
	/// bubbling), then it cancels execution of any subsequent callbacks in the
	/// list.
	void addCallback(Event::t type, eventCallback cb);
	View& operator()(Event::t e, eventCallback cb){ addCallback(e, cb); return *this; }

	View& disable(Property::t p);				///< Disable property flag(s)
	View& enable(Property::t p);				///< Enable property flag(s)
	View& property(Property::t p, bool v);		///< Set property flag(s) to a specfic value	
	View& toggle(Property::t p);				///< Toggle property flag(s)

	View& bringToFront();						///< Brings to front of parent View
	View& cloneStyle();							///< Creates own copy of current style
	void constrainWithinParent();				///< Force to remain in parent	


	/// Returns View under these absolute coordinates or 0 if none.
	
	/// The coordinates are modified to be relative to the returned View's.
	///
	View * findTarget(space_t& x, space_t& y);

	void fit();									///< Fit geometry so all children are visible
	void focused(bool b);						///< Set whether I'm focused
	void move(space_t x, space_t y);			///< Translate constraining within parent.
	
	void on(Event::t e, eventCallback cb=0);	///< Set first callback for a specific event type.
	void removeCallback(Event::t e, eventCallback cb);	///< Remove a callback for a given event (if found)
	void removeAllCallbacks(Event::t e);		///< Detach all callbacks for a given event	

	/// Set descriptor, e.g. tooltip text
	View& descriptor(const std::string& v);

	/// Set identifier string
	
	/// The name should follow the same conventions as identifiers in C. I.e.,
	/// it can be any string of letters, digits, and underscores, not beginning 
	/// with a digit. If the argument is not a valid identifier, then the
	/// name will not be set.
	View& name(const std::string& v);

	using Rect::pos;							// template inheritance fix
	View& pos(Place::t p, space_t x, space_t y);///< Position a specific place at point (x,y)
	View& pos(Place::t p);						///< Position a specific place at point (0,0)
	View& stretch(space_t mx, space_t my);		///< Set parent resize stretch factors
	View& style(Style * style);					///< Set pointer to style

	View& maximize();							///< Matches geometry to parent
	View& restore();							///< Undoes maximization

	void rectifyGeometry();						///< Correct geometry for proper display 

protected:
	Property::t mFlags;				// Property flags
	Style * mStyle;					// Visual appearance
	space_t mAnchorX, mAnchorY;		// Position anchoring factors when parent is resized
	space_t mStretchX, mStretchY;	// Stretch factors when parent is resized
	Rect mRestore;					// Restoration geometry
	std::string mName;				// Settable name identifier
	std::string mDescriptor;		// String describing view
	Font * mFont;					// constructed on first use

	void doDraw(GLV& g);
	bool hasName() const { return ""!=mName; }
	void reanchor(space_t dx, space_t dy);	// Reanchor when parent resizes
};



/// The topmost view

/// The GLV is the topmost view that serves as an interface between OS events
/// (windowing, device input) and GLV events. The GLV is responsible for sending
/// events to views and rendering the GUI.
class GLV : public View{
public:

	/// @param[in] drawFunc		drawing callback
	/// @param[in] width		width
	/// @param[in] height		height
	GLV(drawCallback drawFunc=0, space_t width=800, space_t height=600);

	virtual ~GLV();

	virtual const char * className() const { return "GLV"; }

	const Keyboard& keyboard() const { return mKeyboard; };	///< Get current keyboard state
	const Mouse& mouse() const { return mMouse; }			///< Get current mouse state


	/// Get last event type propagated
	Event::t eventType() const { return mEventType; }

	/// Get currently focused View
	View * focusedView() const { return mFocusedView; }

	/// Get reference to temporary graphics data for rendering
	GraphicsData& graphicsData(){ return mGraphicsData; }


	/// Sends an event to everyone in tree (including self)
	void broadcastEvent(Event::t e);
	
	/// GLV MAIN RENDER LOOP: draw all Views in the GLV

	/// The assumption is that we are inside an OpenGL context of size [w, h]
	/// dsec is the time, in seconds, from the last frame
	virtual void drawGLV(unsigned int w, unsigned int h, double dsec);
	
	/// Draws all acive widgest in the GLV
	void drawWidgets(unsigned int w, unsigned int h, double dsec);
	
	/// Set event type to propagate
	void eventType(Event::t e){ mEventType = e; }

	/// Propagates event through view tree adhering to bubbling rules

	/// Be sure to set the eventType first!
	/// Returns whether the event was consumed.
	bool propagateEvent();

	/// Set current event target
	void setFocus(View * v); 
	
	/// Sets keyboard and GLV event state
	void setKeyDown(int keycode);
	
	/// Sets keyboard and GLV event state
	void setKeyUp(int keycode);
	
	/// Set state from mouse down event
	
	/// Input coordinates will be modified to be relative to the target View's origin
	///
	void setMouseDown(space_t& x, space_t& y, int button, int clicks);

	/// Set state from mouse drag event
	
	/// Input coordinates will be modified to be relative to the target View's origin
	///
	void setMouseDrag(space_t& x, space_t& y);

	/// Set mouse position state
	void setMousePos(int x, int y, space_t relx, space_t rely);

	/// Set state from mouse up event
	
	/// Input coordinates will be modified to be relative to the target View's origin
	///
	void setMouseUp(space_t& x, space_t& y, int button, int clicks);

	/// Set state from mouse motion event
	
	/// Input coordinates will be modified to be relative to the target View's origin
	///
	void setMouseMotion(space_t& x, space_t& y, Event::t e);

	/// Set state from mouse motion event
	
	/// Input coordinates will be modified to be relative to the target View's origin
	///	
	void setMouseMove(space_t& x, space_t& y);

	/// Set state from mouse wheel event
	void setMouseWheel(int wheelDelta);	// Sets mouse and GLV event state


	/// Returns true if there is a valid GLV instance at this address
	static bool valid(const GLV * g);

	/// Get model manager
	ModelManager& modelManager(){ return mMM; }

	/// Add models of named children to model manager
	void refreshModels();

protected:
	Keyboard mKeyboard;
	Mouse mMouse;

	View * mFocusedView;	// current focused widget
	Event::t mEventType;	// current event type
	ModelManager mMM;
	GraphicsData mGraphicsData;

	// Returns whether the event should be bubbled to parent
	bool doEventCallbacks(View& target, Event::t e);
	
	void doFocusCallback(bool get); // Call get or lose focus callback of focused view

	// Keep track of all instances to avoid deletion order problems on program exit.
	// This is necessary for Window since it has a pointer to a GLV.
	static std::vector<GLV *>& instances();
};





// Implementation ______________________________________________________________

// View
inline int View::enabled(Property::t v) const { return mFlags & v; }
inline const std::string& View::descriptor() const { return mDescriptor; }
inline const std::string& View::name() const { return mName; }
inline int View::visible() const { return enabled(Visible); }

inline View& View::disable	(Property::t p){ mFlags = mFlags & Property::t(~int(p)); return *this; }
inline View& View::enable	(Property::t p){ mFlags = mFlags | p; return *this; }
inline View& View::property	(Property::t p, bool v){ v ? enable(p) : disable(p); return *this; }
inline View& View::toggle	(Property::t p){ mFlags = mFlags ^ p; return *this; }
inline View& View::descriptor(const std::string& v){ mDescriptor=v; return *this; }

// Keyboard
inline int Keyboard::key() const { return mKeycode; }
inline int Keyboard::keyAsNumber() const { return key() - 48; }
inline bool Keyboard::isDown() const { return mIsDown; }
inline bool Keyboard::isNumber() const { return (key() >= '0') && (key() <= '9'); }
inline bool Keyboard::alt()   const { return mModifiers[1]; }
inline bool Keyboard::caps()  const { return mModifiers[3]; }
inline bool Keyboard::ctrl()  const { return mModifiers[2]; }
inline bool Keyboard::meta()  const { return mModifiers[4]; }
inline bool Keyboard::shift() const { return mModifiers[0]; }
inline bool Keyboard::key(int k) const { return mKeycode == k; }
inline void Keyboard::alt  (bool state){mModifiers[1] = state;}
inline void Keyboard::caps (bool state){mModifiers[3] = state;}
inline void Keyboard::ctrl (bool state){mModifiers[2] = state;}
inline void Keyboard::meta (bool state){mModifiers[4] = state;}
inline void Keyboard::shift(bool state){mModifiers[0] = state;}

// Mouse
inline space_t Mouse::x() const { return mX[0]; }
inline space_t Mouse::y() const { return mY[0]; }
inline space_t Mouse::x(int button) const { return bx[button]; }
inline space_t Mouse::y(int button) const { return by[button]; }
inline space_t Mouse::w() const { return mW[0]; }
inline space_t Mouse::dx() const { return mX[0] - mX[1]; }
inline space_t Mouse::dy() const { return mY[0] - mY[1]; }
inline space_t Mouse::dw() const { return mW[0] - mW[1]; }
inline space_t Mouse::ddx() const { return mX[0] - 2 * mX[1] + mX[2]; }
inline space_t Mouse::ddy() const { return mY[0] - 2 * mY[1] + mY[2];; }
inline space_t Mouse::ddw() const { return mW[0] - 2 * mW[1] + mW[2];; }
inline space_t Mouse::xRel() const { return mXRel; }
inline space_t Mouse::yRel() const { return mYRel; }
inline space_t Mouse::xRel(int button) const { return bxRel[button]; }
inline space_t Mouse::yRel(int button) const { return byRel[button]; }
inline int Mouse::button() const { return mButton; }
inline int Mouse::clicks() const { return mClicks; }
inline bool Mouse::isDown() const { return mIsDown; }
inline bool Mouse::isDown(int button) const { return b[button]; }
inline bool Mouse::left() const { return b[Left]; }
inline bool Mouse::middle() const { return b[Middle]; }
inline bool Mouse::right() const { return b[Right]; }

inline void Mouse::  setContext(View * v){mXRel -= v->l; mYRel -= v->t;}
inline void Mouse::unsetContext(View * v){mXRel += v->l; mYRel += v->t;}
inline void	Mouse::pos(int x, int y) {
	bufferPos((space_t)x, mX);
	bufferPos((space_t)y, mY);
}
inline void Mouse::bufferPos(space_t newPos, space_t * pos){
	pos[2] = pos[1];
	pos[1] = pos[0];
	pos[0] = newPos;
}
inline void	Mouse::posRel(space_t rx, space_t ry){ mXRel=rx; mYRel=ry;}


} // glv::

#endif

