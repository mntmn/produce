#include <X11/Xlib.h>

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

void x11_stuff_init()
{
  x11_dsp = XOpenDisplay(NULL);
  Display *dpy = x11_dsp;

  //fgListLength(fgStructure.Windows);

  printf("SFG STRUCT: %x\n",&fgStructure);
  printf("SFG WINDOW: %x\n",&fgStructure.Window);
  
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
  XChangeProperty(dpy,(Window)&fgStructure.Window,
                  XdndAware, XA_ATOM, 32, PropModeReplace,
                  (const unsigned char*)&version, 1);

}

void x11_exit_cleanup() {
  XCloseDisplay(x11_dsp);
}

