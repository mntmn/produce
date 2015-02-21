#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

typedef void (*custom_x11_event_callback)(XEvent* e);

extern "C" {
  void fg_set_custom_x11_event_callback(custom_x11_event_callback cb);
}

Display* x11_dsp = NULL;

void get_windowsize_x11(int* w, int* h) {
  XID wid = 0;
  XWindowAttributes xw_attr;

  wid = DefaultRootWindow(x11_dsp);
  Status ret = XGetWindowAttributes(x11_dsp, wid, &xw_attr);
  *w = xw_attr.width;
  *h = xw_attr.height;
}

int xdnd_debug = 0;

#include "freeglut_internal.h"
extern SFG_Structure fgStructure;

static Atom XdndAware;
static Atom XdndTypeList;
static Atom XdndSelection;

static Atom XdndEnter;
static Atom XdndPosition;
static Atom XdndStatus;
static Atom XdndLeave;
static Atom XdndDrop;
static Atom XdndFinished;

static Atom XdndActionCopy;
static Atom XdndActionMove;
static Atom XdndActionLink;
static Atom XdndActionAsk;
static Atom XdndActionPrivate;


/* XdndEnter */
#define XDND_THREE 3
#define XDND_ENTER_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_ENTER_THREE_TYPES(e)		(((e)->xclient.data.l[1] & 0x1UL) == 0)
#define XDND_ENTER_THREE_TYPES_SET(e,b)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_ENTER_VERSION(e)			((e)->xclient.data.l[1] >> 24)
#define XDND_ENTER_VERSION_SET(e,v)		(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~(0xFF << 24)) | ((v) << 24)
#define XDND_ENTER_TYPE(e,i)			((e)->xclient.data.l[2 + i])	/* i => (0, 1, 2) */

/* XdndPosition */
#define XDND_POSITION_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_POSITION_ROOT_X(e)			((e)->xclient.data.l[2] >> 16)
#define XDND_POSITION_ROOT_Y(e)			((e)->xclient.data.l[2] & 0xFFFFUL)
#define XDND_POSITION_ROOT_SET(e,x,y)	(e)->xclient.data.l[2]  = ((x) << 16) | ((y) & 0xFFFFUL)
#define XDND_POSITION_TIME(e)			((e)->xclient.data.l[3])
#define XDND_POSITION_ACTION(e)			((e)->xclient.data.l[4])

/* XdndStatus */
#define XDND_STATUS_TARGET_WIN(e)			((e)->xclient.data.l[0])
#define XDND_STATUS_WILL_ACCEPT(e)			((e)->xclient.data.l[1] & 0x1L)
#define XDND_STATUS_WILL_ACCEPT_SET(e,b)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_STATUS_WANT_POSITION(e)		((e)->xclient.data.l[1] & 0x2UL)
#define XDND_STATUS_WANT_POSITION_SET(e,b)  (e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x2UL) | (((b) == 0) ? 0 : 0x2UL)
#define XDND_STATUS_RECT_X(e)				((e)->xclient.data.l[2] >> 16)
#define XDND_STATUS_RECT_Y(e)				((e)->xclient.data.l[2] & 0xFFFFL)
#define XDND_STATUS_RECT_WIDTH(e)			((e)->xclient.data.l[3] >> 16)
#define XDND_STATUS_RECT_HEIGHT(e)			((e)->xclient.data.l[3] & 0xFFFFL)
#define XDND_STATUS_RECT_SET(e,x,y,w,h)		{(e)->xclient.data.l[2] = ((x) << 16) | ((y) & 0xFFFFUL); (e)->xclient.data.l[3] = ((w) << 16) | ((h) & 0xFFFFUL); }
#define XDND_STATUS_ACTION(e)				((e)->xclient.data.l[4])

/* XdndLeave */
#define XDND_LEAVE_SOURCE_WIN(e)	((e)->xclient.data.l[0])

/* XdndDrop */
#define XDND_DROP_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_DROP_TIME(e)			((e)->xclient.data.l[2])

/* XdndFinished */
#define XDND_FINISHED_TARGET_WIN(e)	((e)->xclient.data.l[0])

#define NUM_MIMES 1

static Atom supported[NUM_MIMES];

static Display* dpy;
static Window win;


void send_finished(Window from, Window to)
{
  XEvent xevent;
  memset(&xevent, 0, sizeof (xevent));
  xevent.xany.type = ClientMessage;
  xevent.xany.display = dpy;
  xevent.xclient.window = to;
  xevent.xclient.message_type = XdndFinished;
  xevent.xclient.format = 32;
  XDND_FINISHED_TARGET_WIN (&xevent) = from;
  XSendEvent(dpy, to, 0, 0, &xevent);
};

static Time drop_time;

char* get_dropped_filenames(XEvent* xev, Window src)
{
  
  Window owner;
  owner = XGetSelectionOwner(dpy, XdndSelection);

  printf("dnd selection owner: %d\n",owner);
    
  char* filename = "";  
  if (xev->xselection.property == None)
  {
    return filename;
  }

  printf("xev->xselection.property: 0x%x\n",xev->xselection.property);
    
  unsigned long   bytesRemaining;
  unsigned long   numItems = 0;
  unsigned char*  s = NULL;
  Atom            actualType;
  int             actualFormat;
  int             offset = 0;
  
  //send_finished(xev);
  
  do
  {
    // xev->xany.window
    // xev->xselection.property
    if (XGetWindowProperty(dpy, xev->xany.window, XdndSelection, offset / sizeof(unsigned char *), 1024, False, AnyPropertyType, 
                          &actualType, &actualFormat, &numItems, &bytesRemaining, &s) != Success)
    {
//XFree(s);
      printf("XGetWindowProperty unsuccessful\n");
      return filename;
    }
        
    filename = (char*)s;
    printf("numItems: %d filename in loop: %s\n",numItems,s);
//XFree(s);
    offset += numItems;
  }
  while (bytesRemaining > 0);
    
  //send_finished(xev);
    
  return filename;
}

extern void file_dropped_callback(char* uri);

void xev_callback(XEvent* e) {
  //printf("xev_callback! %x\n",e->type);
  
  Window source;

  if (e->type == ClientMessage) {
    //printf("ClientMessage!\n");
    XClientMessageEvent cme = e->xclient;
    
    //printf("ClientMessage event: %p type: %d format: %d\n",&cme,cme.message_type,cme.format);

    if (XdndPosition == cme.message_type) {
      source = cme.data.l[0];
      
      //printf("got XdndPosition! source window: %x\n",source);
    
      XEvent reply;
    
      memset(&reply,0,sizeof(reply));
      reply.xany.type = ClientMessage;
      reply.xany.display = dpy;
      reply.xclient.window = source;
      reply.xclient.message_type = XdndStatus;
      reply.xclient.format = 32;
      reply.xclient.data.l[0] = cme.window;
      reply.xclient.data.l[1] = 1;
      reply.xclient.data.l[2] = 0; // rectangle
      reply.xclient.data.l[3] = 0;
      reply.xclient.data.l[4] = XdndActionPrivate;
      
      Status status = XSendEvent(dpy,source,False,NoEventMask,&reply);

      //printf("sent reply to XdndPosition. Status: %x\n",status);
      
      XFlush(dpy);
    }
    else if (XdndDrop == cme.message_type) {
      printf("got XdndDrop!\n");
      
      Window owner;
      owner = XGetSelectionOwner(dpy, XdndSelection);
      
      XConvertSelection(dpy, XdndSelection, supported[0], XdndSelection, win, CurrentTime);
      send_finished(win, owner);
      
      XFlush(dpy);
    }
    else if (XdndEnter == cme.message_type) {
      printf("got XdndEnter!\n");
      
      int wa = XDND_STATUS_WILL_ACCEPT(e);
      int type = XDND_ENTER_TYPE(e, 0);
      printf("XdndEnter: %d our mime atom: %d\n",type,supported[0]);
    }
    else if (XdndLeave == cme.message_type) {
      //printf("got XdndLeave!\n");
    }
  } else if (e->type == SelectionNotify) {
    printf("got SelectionNotify!\n");

    Window owner;
    owner = XGetSelectionOwner(dpy, XdndSelection);
    char* fn = get_dropped_filenames(e, owner);
    printf("fn: %s\n",fn);

    if (fn) {
      file_dropped_callback(fn);
    }
  }
}

void x11_stuff_init()
{
  x11_dsp = XOpenDisplay(NULL);
  dpy = x11_dsp;

  printf("SFG STRUCT: %x\n",&fgStructure);
  printf("SFG WINDOW: %x\n",&fgStructure.Window->Window.Handle);

  win = fgStructure.Window->Window.Handle;

  XdndAware         = XInternAtom(dpy, "XdndAware",         False);
  XdndTypeList      = XInternAtom(dpy, "XdndTypeList",      False);
  XdndSelection     = XInternAtom(dpy, "XdndSelection",     False);

  /* client messages */
  XdndEnter         = XInternAtom(dpy, "XdndEnter",         False);
  XdndPosition      = XInternAtom(dpy, "XdndPosition",      False);
  XdndStatus        = XInternAtom(dpy, "XdndStatus",        False);
  XdndLeave         = XInternAtom(dpy, "XdndLeave",         False);
  XdndDrop          = XInternAtom(dpy, "XdndDrop",          False);
  XdndFinished      = XInternAtom(dpy, "XdndFinished",      False);

  /* actions */
  XdndActionCopy    = XInternAtom(dpy, "XdndActionCopy",    False);
  XdndActionMove    = XInternAtom(dpy, "XdndActionMove",    False);
  XdndActionLink    = XInternAtom(dpy, "XdndActionLink",    False);
  XdndActionAsk     = XInternAtom(dpy, "XdndActionAsk",     False);
  XdndActionPrivate = XInternAtom(dpy, "XdndActionPrivate", False);

  int version = 4;

  /* window */
  Status status;
  
  status = XChangeProperty(dpy,win,
                  XdndAware, XA_ATOM, 32, PropModeReplace,
                  (const unsigned char*)&version, 1);

  Widget widget = (Widget)dpy;

  char *mime_names[NUM_MIMES] = {
    "text/uri-list",
    //"text/plain"
  };
  
  XInternAtoms(dpy, mime_names, NUM_MIMES, False, supported);

  if ((status == BadAlloc) || (status == BadAtom) ||
     (status == BadMatch) || (status == BadValue) || (status == BadWindow)) {
    fprintf(stderr, "XChangeProperty() failed: %x\n",status);
  }

  XChangeProperty(dpy, win, XdndTypeList, XA_ATOM, 32,
                  PropModeAppend, (unsigned char *)supported, NUM_MIMES);
  
  fg_set_custom_x11_event_callback(xev_callback);
}

void x11_exit_cleanup() {
  XCloseDisplay(x11_dsp);
}

