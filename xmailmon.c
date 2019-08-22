#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "config.h"
#include "envelope.xbm"
#include "utils.h"

typedef struct Mailbox Mailbox;

struct Mailbox
{
	char *name;
	char *path;
	int   count;
};

static void  mkconfig(void);
static void* checkmail(void*);
static void	 winit(void);
static void  wend(void);
static void  wrun(void);
static void  wupdate(void);
static void  wdraw(void);

static Mailbox**	mailboxes;
static int			mailboxcount;
static pthread_t	thread;
static Display		*dpy;
static int			screen;
static Visual		*visual;
static Colormap 	cmap;
static Window		win;
static GC			gc;
static Pixmap		pixmap;
static XftFont		*font;
static XftFont		*font2;
static XftDraw		*draw;
static XftColor 	color;
static int 			lineheight;

int
main(int argc, char *argv[])
{
	mkconfig();
	winit();
	pthread_create(&thread, NULL, checkmail, NULL);
	wrun();
	pthread_cancel(thread);
	wend();
	return 0;
}

void
mkconfig(void)
{
	Mailbox *b;
	char buf[PATH_MAX+1], *m;
	int i, l;

	m = expandpath(Cmaildir);
	mailboxcount = 0;
	for(i = 0; Cmailboxes[i] != NULL; i++)
		++mailboxcount;
	mailboxes = calloc(mailboxcount, sizeof(Mailbox));
	if(mailboxes == NULL)
		die("cannot allocate memory");
	for(i = 0; i < mailboxcount; i++){
		l = snprintf(buf, PATH_MAX+1, "%s/%s/new", m, Cmailboxes[i]);
		if(l < 0)
			die("cannot create mailbox path (%s)\n", strerror(errno));
		b = malloc(sizeof(Mailbox));
		if(b == NULL)
			die("cannot allocate memory");
		b->name = strdup(Cmailboxes[i]);
		b->path = strdup(buf);
		b->count = 0;
		mailboxes[i] = b;
	}
}

void*
checkmail(void *data)
{
	USED(data);
	Mailbox *m;
	int i, c, n;

	for(;;){
		n = 0;
		for(i = 0; i < mailboxcount; i++){
			m = mailboxes[i];
			c = dirfilecount(m->path);
			if(c != m->count){
				m->count = c;
				n = 1;
			}
		}
		if(n)
			wupdate();
		sleep(10);
	}
	return NULL;
}

void
winit(void)
{
	XGCValues values;
	XGlyphInfo extents;

	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL)
		die("can not open display");
	screen = DefaultScreen(dpy);
	visual = DefaultVisual(dpy, screen);
	cmap   = DefaultColormap(dpy, screen);
	win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
		10, 10, 290, 80, 1,
		BlackPixel(dpy, screen),
		WhitePixel(dpy, screen));
	XSelectInput(dpy, win, ExposureMask | KeyPressMask);
	values.foreground = BlackPixel(dpy, screen);
	values.background = WhitePixel(dpy, screen);
	gc = XCreateGC(dpy, win, GCForeground|GCBackground, &values);
	pixmap = XCreatePixmapFromBitmapData(dpy, win,
		envelope_bits, envelope_width, envelope_height,
		BlackPixel(dpy, screen), WhitePixel(dpy, screen),
		DefaultDepth(dpy, screen));
	font = XftFontOpenName(dpy, screen, Cregularfont);
	if(font == NULL)
		die("can not load regular font");
	font2 = XftFontOpenName(dpy, screen, Cboldfont);
	if(font2 == NULL)
		die("can not load bold font");
	XftColorAllocName(dpy, visual, cmap, "#000000", &color);
	draw = XftDrawCreate(dpy, win, visual, cmap);
	XftTextExtents8(dpy, font2, (XftChar8*)"X", 1, &extents);
	lineheight = extents.height;
}

void
wend(void)
{
	XFreePixmap(dpy, pixmap);
	XFreeGC(dpy, gc);
	XftFontClose(dpy, font);
	XftFontClose(dpy, font2);
	XftColorFree(dpy, visual, cmap, &color);
	XftDrawDestroy(draw);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void
wupdate(void)
{
	XEvent e;

	memset(&e, 0, sizeof(e));
	e.type = Expose;
	e.xexpose.window = win;
	XSendEvent(dpy, win, False, ExposureMask, &e);
	XFlush(dpy);
}

void
wrun(void)
{
	XEvent e;
	char buf[1];
	KeySym key;

	XMapWindow(dpy, win);
	for(;;){
		XNextEvent(dpy, &e);
		switch(e.type){
		case Expose:
			wdraw();
			XFlush(dpy);
			break;
		case KeyPress:
			XLookupString(&e.xkey, buf, 1, &key, 0);
			if(key == XK_Escape)
				return;
			break;
		}
	}
}

void
wdraw(void)
{
	Mailbox *m;
	int i, l, y;
	char buf[32];

	XClearWindow(dpy, win);
	y = lineheight + Cpadding;
	for(i = 0; i < mailboxcount; i++){
		m = mailboxes[i];
		XCopyArea(dpy, pixmap, win, gc, 0, 0, envelope_width, envelope_height, Cpadding, y - lineheight);
		l = sprintf(buf, "%-20s %4d", m->name, m->count);
		XftDrawString8(draw, &color,
			m->count > 0 ? font2 : font,
			3 * Cpadding, y,
			(const FcChar8*)buf, l);
		y += lineheight + Cpadding;
	}
}

