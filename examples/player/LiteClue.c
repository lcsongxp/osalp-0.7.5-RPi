/* 
LiteClue.c - LiteClue widget
	See LiteClue documentation
	Version 1.5

Copyright 1996 COMPUTER GENERATION, INC.,

The software is provided "as is", without warranty of any kind, express
or implied, including but not limited to the warranties of
merchantability, fitness for a particular purpose and noninfringement.
In no event shall Computer Generation, inc. nor the author be liable for
any claim, damages or other liability, whether in an action of contract,
tort or otherwise, arising from, out of or in connection with the
software or the use or other dealings in the software.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.

Author:
Gary Aviv 
Computer Generation, Inc.,
gary@compgen.com
www.compgen.com/widgets

Thanks to Contributers:
J Satchell
Eric Marttila
Addy Klos

*/
/* Revision History:
$Log: LiteClue.c,v $
Revision 1.1  2001/02/20 20:29:59  daservis
Makefiles and movement for configure scripts

Revision 1.1  2000/08/20 04:47:58  bforsberg
Add mixing class and made several fixes

Revision 1.16  1998/09/07 14:06:19  gary
Added const to prototype of XcgLiteClueAddWidget at request from user

Revision 1.15  1998/07/30 16:05:16  gary
Add NO_FONT_SET to usr FontStruct rather than FontSet

Revision 1.14  1998/01/06 15:30:33  gary
If font specified by resource can not be converted, use fixed
font as fallback. If no font at all can be converted, prevent
crash, just disable widget entirely.

Revision 1.13  1997/07/07 14:55:04  gary
Cancel timeouts when XcgLiteClueDeleteWidget is called to prevent
errant timeout event on deleted widget.

Revision 1.12  1997/06/20 20:09:09  gary
Add XcgLiteClueDispatchEvent to enable clues for insensitive widgets.

Revision 1.11  1997/06/15 14:10:24  gary
Add XcgLiteClueDispatchEvent to enable clues for insensitive widgets.

Revision 1.10  1997/04/14 13:02:33  gary
Attempt to fix problem when we get multiple enter events bu no leave event.

Revision 1.9  1997/03/10 14:42:41  gary
Attempt to fix problem when we get multiple enter events bu no leave event.
Add C++ wrapper to allow linking with C++ programs. (In HView.h)

Revision 1.8  1997/01/17 13:44:14  gary
Support of cancelWaitPeriod resource: this is a period from the point
a help popdown occurs in which the normal waitPeriod is suspended
for the next popup

Revision 1.7  1996/12/16 22:35:38  gary
Fix double entry problem

Revision 1.6  1996/11/18 14:52:21  gary
remove some compile warnings pointed out by a user

Revision 1.5  1996/11/12 20:56:43  gary
remove some compile warnings

Revision 1.4  1996/10/20  13:38:16  gary
Version 1.2 freeze

Revision 1.3  1996/10/19 16:16:30  gary
Compile warning removed with cast

Revision 1.2  1996/10/19 16:07:38  gary
a) R4 back compatibility
b) Delay before pop up of help, waitPeriod resource (def 500 ms).
	Thanks to J Satchell for this.
c) Button press in watched widget pops down help

Revision 1.1  1996/10/18 23:14:58  gary
Initial


$log
Remapping of clue added when clue is displayed off screen.
$log
*/
#include <unistd.h>
#include <signal.h>
/* #include <Xm/XmP.h> */
#include <X11/IntrinsicP.h> 
#include <X11/StringDefs.h>

#include "LiteClueP.h"

#include <stdio.h>

#define CheckWidgetClass(routine) \
	if (XtClass(w) != xcgLiteClueWidgetClass) \
		wrong_widget(routine)


static Boolean setValues( Widget _current, Widget _request, Widget _new, ArgList args, Cardinal * num_args);
static void Initialize(Widget treq, Widget tnew, ArgList args, Cardinal *num_args);
struct liteClue_context_str * alloc_liteClue_context(void);

/* keep information about each widget we are keeping track of */
struct liteClue_context_str
{
	ListThread next;	/* next in list */
	Widget watched_w;	/* the widget we are watching */
	XcgLiteClueWidget cw;	/* pointer back to the liteClue widget */
	Position  abs_x, abs_y;
	Boolean sensitive;	/* if False, liteClue is suppressed */
	char * text;		/* text to display */
	short text_size;	/* its size */
};

void free_widget_context(XcgLiteClueWidget cw, struct liteClue_context_str * obj);
/*
Widget resources: eg to set LiteClue box background:
 *XcgLiteClue.background: yellow
       
*/
#define offset(field) XtOffsetOf(LiteClueRec, field)
static XtResource LC_resources[] =
{
	{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
		offset(liteClue.foreground), XtRString, "black"},
#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)
	{XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
		offset(liteClue.fontset), XtRString, 
          "-adobe-new century schoolbook-bold-r-normal-*-14-*-*-*-*-*-*-*"},
#else
	{XtNfontSet, XtCFontSet, XtRFontSet, sizeof(XFontSet),
		offset(liteClue.fontset), XtRString, "-adobe-new century schoolbook-bold-r-normal-*-12-*"},
#endif
	{XgcNwaitPeriod, XgcCWaitPeriod, XtRInt , sizeof(int),
		offset(liteClue.waitPeriod),XtRString, "500" },

	{XgcNcancelWaitPeriod, XgcCCancelWaitPeriod, XtRInt , sizeof(int),
		offset(liteClue.cancelWaitPeriod),XtRString, "2000" },
};

#undef offset

#if 0
static XtActionsRec actions[] = 
}; /* actions */
#endif


LiteClueClassRec xcgLiteClueClassRec =
{
    {
	(WidgetClass)&overrideShellClassRec,	/* superclass */
	"XcgLiteClue",				/* class_name */
	(Cardinal)sizeof(LiteClueRec),		/* widget size */
	NULL,					/* class_init */
	(XtWidgetClassProc)NULL,		/* class_part_init */
	(XtEnum)FALSE,				/* class_inited */
	(XtInitProc)Initialize,			/* initialize */
	(XtArgsProc)NULL,			/* init_hook */
	XtInheritRealize,			/* realize */
	(XtActionList)0,			/* actions */
	(Cardinal)0,			/* num_actions */
	(XtResourceList)LC_resources,		/* resources */
	(Cardinal)XtNumber(LC_resources),		/* num_resources */
	NULLQUARK,				/* xrm_class */
	TRUE,					/* compress_motion */
	(XtEnum)FALSE,				/* compress_exposur */
	TRUE,					/* compress enterleave */
	FALSE,					/* visibility_interest */
	(XtWidgetProc)NULL,			/* destroy */
	XtInheritResize,
	XtInheritExpose,			/* expose, */
	(XtSetValuesFunc)setValues,		/* set_values */
	(XtArgsFunc)NULL,			/* set_values_hook */
	XtInheritSetValuesAlmost,		/* set_values_almost */
	(XtArgsProc)NULL,			/* get_values_hook */
	XtInheritAcceptFocus,		/* accept_focus */
	XtVersion,				/* version */
	(XtPointer)NULL,			/* callback_private */
	XtInheritTranslations,
	XtInheritQueryGeometry,			/* query_geometry */
	XtInheritDisplayAccelerator,		/* display_accelerator */
	(XtPointer)0,			/* extension */
    },
    { /*** composite-Class ***/
	XtInheritGeometryManager,	/* geometry_manager   	*/    	
	XtInheritChangeManaged,	/* change_managed		*/	
	XtInheritInsertChild,	/* insert_child		*/	
	XtInheritDeleteChild,	/* delete_child		*/	
	NULL	/* extension		*/	
    }, 
	{ /* Shell */
	(XtPointer) NULL,       /* pointer to extension record      */
	},
	/* Override Shell */
	{
	0,
	},
	/* LiteClue */
	{
	0,
	},
};

WidgetClass xcgLiteClueWidgetClass = (WidgetClass) & xcgLiteClueClassRec;

/* doubly linked list processing */

/*
	 initialize header - both pointers point to it
*/
static void xcgListInit(ListThread *newbuf)
{
	newbuf->back = newbuf;
	newbuf->forw = newbuf;
}


/*
	 insert newbuf before posbuf
*/
static void xcgListInsertBefore(ListThread *newlist, ListThread *poslist)
{
	ListThread *prevbuf;

	prevbuf = poslist->back;

	poslist->back = newlist;
	newlist->forw = poslist;
	newlist->back = prevbuf;
	prevbuf->forw = newlist;
}


/*
	remove rembuf from queue
*/
static ListThread * xcgListRemove(ListThread *rembuf)
{
	ListThread *prevbuf, *nextbuf;

	prevbuf = rembuf->back;
	nextbuf = rembuf->forw;

	prevbuf->forw = nextbuf;
	nextbuf->back = prevbuf;		

	rembuf->back = (ListThread *) NULL;	/* for safety to cause trap if ..*/
	rembuf->forw = (ListThread *) NULL;	/* .. mistakenly refed */
	return rembuf;
}

/*
The font_information is derived 
*/

#if XtSpecificationRelease < 5 || defined(NO_FONT_SET)

/* R4 and below code */
/*
Return XFontSet for passed font_string. 
return status
*/
static int string_to_FontSet (XcgLiteClueWidget cw, char * font_string, XFontStruct ** out) 
{
	Boolean sts;
	XrmValue from;
	XrmValue to;
	
	to.size = sizeof(out);
	to.addr = (void *) out;
	from.size = strlen(from.addr = font_string );
	sts = XtConvertAndStore((Widget) cw, XtRString, &from, XtRFontStruct, &to);
	return sts;
}

static void compute_font_info(XcgLiteClueWidget cw)
{
	int direction_return;
	int font_ascent_return, font_descent_return; 
	XCharStruct oret;
	if (!cw->liteClue.fontset)
		string_to_FontSet(cw, "fixed", &cw->liteClue.fontset) ;
	if (!cw->liteClue.fontset)
	{
		fprintf(stderr,"LiteClue: can not find resource font nor fallback fixed font\n");
		return;
	}
	XTextExtents( cw->liteClue.fontset, "1", 1,
		&direction_return,
		&font_ascent_return, &font_descent_return, &oret);

	cw->liteClue.font_baseline = oret.ascent;	/* y offset from top to baseline, 
			don't know why this is returned as negative */
	cw->liteClue.font_width = oret.width;	/* the width and height of the object */
	cw->liteClue.font_height = oret.ascent+oret.descent;
}

#else
/*
Return XFontSet for passed font_string. 
return status
*/
static int string_to_FontSet (XcgLiteClueWidget cw, char * font_string, XFontSet * out) 
{
	Boolean sts;
	XrmValue from;
	XrmValue to;
	
	to.size = sizeof(out);
	to.addr = (void *) out;
	from.size = strlen(from.addr = font_string );
	sts = XtConvertAndStore((Widget) cw, XtRString, &from, XtRFontSet, &to);
	return sts;
}

/* R5 and above code */
static void compute_font_info(XcgLiteClueWidget cw)
{
	XRectangle ink;
	XRectangle logical;

	if (!cw->liteClue.fontset)
		string_to_FontSet(cw, "fixed", &cw->liteClue.fontset) ;
	if (!cw->liteClue.fontset)
	{
		fprintf(stderr,"LiteClue: can not find resource font nor fallback fixed font\n");
		return;
	}
	XmbTextExtents(cw->liteClue.fontset, "1", 1,&ink, &logical);

	cw->liteClue.font_baseline = -logical.y;	/* y offset from top to baseline, 
			don't know why this is returned as negative */
	cw->liteClue.font_width = logical.width;	/* the width and height of the object */
	cw->liteClue.font_height = logical.height;
}
#endif

/*
 Creates the various graphic contexts we will need 
*/
static void create_GC(XcgLiteClueWidget cw )
{
	XtGCMask valuemask;
	XGCValues myXGCV;


	valuemask = GCForeground | GCBackground | GCFillStyle ;
	myXGCV.foreground = cw->liteClue.foreground;
	myXGCV.background = cw->core.background_pixel;
	myXGCV.fill_style = FillSolid; 

#if XtSpecificationRelease < 5	|| defined(NO_FONT_SET)
	valuemask |= GCFont ;
	myXGCV.font = cw->liteClue.fontset->fid; 
#endif	/* end R4 hack */

	if (cw->liteClue.text_GC )
		XtReleaseGC((Widget) cw, cw->liteClue.text_GC );
	cw->liteClue.text_GC = XtGetGC((Widget)cw, valuemask, &myXGCV);
}


/* a routine to halt execution and force  
a core dump for debugging analysis	
when a public routine is called with the wrong class of widget
*/
static void wrong_widget(char * routine)
{
	int mypid = getpid(); 
	fprintf(stderr, "Wrong class of widget passed to %s\n", routine);
	fflush(stderr); 
	kill(mypid, SIGABRT); 
}

/*
Find the target in the widget list. Return context pointer if found,
NULL if not
*/
static struct liteClue_context_str * find_watched_widget(XcgLiteClueWidget cw,
	Widget target)
{
	struct liteClue_context_str * obj;

	for (obj = (struct liteClue_context_str *) cw->liteClue.widget_list.forw; 
		obj != (struct liteClue_context_str *) & cw->liteClue.widget_list; 
		obj = (struct liteClue_context_str *)obj->next.forw )
	{
		if (target == obj->watched_w)
			return obj;
	}
	return NULL;
}

/*
	allocate and initialize a widget context
*/
struct liteClue_context_str * alloc_liteClue_context(void)
{
	struct liteClue_context_str * out;
	out = (struct liteClue_context_str *) XtMalloc(sizeof(struct liteClue_context_str));
	memset(out, 0, sizeof(struct liteClue_context_str));
	xcgListInit(&out->next);	
	return out ;
}

/*
	allocate, initialize and link a liteClue context to the list
*/
static struct liteClue_context_str * alloc_link_liteClue_context(XcgLiteClueWidget cw )
{
	struct liteClue_context_str * out = alloc_liteClue_context();

	/* link as new last */
	xcgListInsertBefore(&out->next, &cw->liteClue.widget_list);
	out->cw = cw;	/* initialize this emeber - its always the same */
	return out;
}

/*
	free a widget context
*/
void free_widget_context(XcgLiteClueWidget cw, struct liteClue_context_str * obj)
{
	xcgListRemove((ListThread *)obj);
	/* free up all things object points to */
	obj->sensitive = False;
	if (obj->text )
		XtFree(obj->text);
	XtFree((char *) obj);
}


/* -------------------- Widget Methods ---------------------- */
/* Initialize method */
static void Initialize(Widget treq, Widget tnew, ArgList args, 
Cardinal *num_args)
{
	XcgLiteClueWidget cw = (XcgLiteClueWidget) tnew;


	cw->liteClue.text_GC = NULL;
	cw->liteClue.HelpIsUp = False;
	cw->liteClue.HelpPopDownTime = 0;
	cw->liteClue.interval_id = (XtIntervalId)0;
	xcgListInit(&cw->liteClue.widget_list);	/* initialize empty list */
	compute_font_info(cw);
	create_GC(cw );
}

static Boolean setValues( Widget _current, Widget _request, Widget _new, ArgList args, Cardinal * num_args)
{
	XcgLiteClueWidget cw_new = (XcgLiteClueWidget) _new;
	XcgLiteClueWidget cw_cur = (XcgLiteClueWidget) _current;

	/* values of cw_new->liteClue.cancelWaitPeriod and
	   cw_new->liteClue.waitPeriod are accepted without checking */

	if (cw_new->liteClue.foreground != cw_cur->liteClue.foreground 
	||  cw_new->core.background_pixel != cw_cur->core.background_pixel )
	{
		create_GC(cw_new);
	}

	return FALSE;
}

/* ----------------- Event handlers ------------------------*/


/* At this point the help may be popup 
*/
static void timeout_event( XtPointer client_data, XtIntervalId *id)
{
#define OFFSET_X 4	
#define OFFSET_Y 4
#define BorderPix 2

	struct liteClue_context_str * obj = (struct liteClue_context_str *) client_data;
	XcgLiteClueWidget cw = obj->cw;
	Position  abs_x, abs_y;
	Dimension clue_width, clue_height;

	/* variables to retrieve info about the screen size */
	Display            *display;
	int                 screen_num;
	int                 display_width;
	int                 display_height;
	Position            clue_x, clue_y;

	XRectangle ink;
	XRectangle logical;
	Position   w_height;	
	Widget w;

	if (cw->liteClue.interval_id == (XtIntervalId)0)
		return;	/* timeout was removed but callback happened anyway */
	cw->liteClue.interval_id = (XtIntervalId)0;
	if (obj->sensitive == False)
		return;

	w = obj->watched_w;
	XtVaGetValues(w, XtNheight, &w_height, NULL );
	/* position just below the widget */
	XtTranslateCoords(w, 0, 0, &abs_x, &abs_y);

#if XtSpecificationRelease < 5	|| defined(NO_FONT_SET)
	{
	int direction_return;
	int font_ascent_return, font_descent_return; 
	XCharStruct oret;
	XTextExtents( cw->liteClue.fontset ,obj->text , obj->text_size,
		&direction_return,
		&font_ascent_return, &font_descent_return, &oret); 
	logical.width = oret.width;
	}
#else
	XmbTextExtents(cw->liteClue.fontset, obj->text , obj->text_size ,&ink, &logical);
#endif
	clue_width =   2*BorderPix +logical.width;
	clue_height =  2*BorderPix + cw->liteClue.font_height;
	XtResizeWidget((Widget) cw, clue_width, 
			clue_height, 
			cw->core.border_width );

	display = XtDisplay(w);
	screen_num  = DefaultScreen(display);
  
	display_width  = DisplayWidth(display, screen_num);
	display_height = DisplayHeight(display, screen_num);

	/* deal with the Y coordinate */
	clue_y = abs_y +w_height+OFFSET_Y; /* default position below watched widget */
	if (clue_y + clue_height > display_height)	/* off bottom of screen ? */
		/* then place on top of watched widget */
		clue_y = abs_y - clue_height - OFFSET_Y;

	/* now deal with the X coordinate */
	clue_x = abs_x + OFFSET_X; /* default position a little right of watched widget */ 
	if (clue_x < 0 )	/* off left of screen ? */
		clue_x = 0;
	else if (clue_x + clue_width > display_width)	/* off right of screen ? */
		clue_x = display_width - clue_width - 1;

	XtMoveWidget((Widget) cw, clue_x, clue_y);
	XtPopup((Widget) cw, XtGrabNone);
	cw->liteClue.HelpIsUp = True;

#if XtSpecificationRelease < 5	|| defined(NO_FONT_SET)
	XDrawImageString(XtDisplay((Widget) cw), XtWindow((Widget) cw), 
		cw->liteClue.text_GC , BorderPix, 
		BorderPix + cw->liteClue.font_baseline, obj->text, obj->text_size);
#else
	XmbDrawImageString(XtDisplay((Widget) cw), XtWindow((Widget) cw), 
		cw->liteClue.fontset,
		cw->liteClue.text_GC , BorderPix, 
		BorderPix + cw->liteClue.font_baseline, obj->text, obj->text_size);
#undef OFFSET_X
#undef OFFSET_Y
#endif
}

/*
Pointer enters watched widget, set a timer at which time it will
popup the help
*/
static void Enter_event(Widget w, XtPointer client_data, XEvent * xevent, Boolean * continue_to_dispatch )
{
	struct liteClue_context_str * obj = (struct liteClue_context_str *) client_data;
	XcgLiteClueWidget cw = obj->cw;
	XEnterWindowEvent * event = & xevent->xcrossing;
	int current_waitPeriod ;

	if (obj->sensitive == False || !cw->liteClue.fontset)
		return;
	/* check for two enters in a row - happens when widget is
	   exposed under a pop-up */
	if (cw->liteClue.interval_id != (XtIntervalId)0) 
		return;
	if(event->mode != NotifyNormal)
		return;

	/* if a help was recently popped down, don't delay in poping up
	   help for next watched widget
	*/
	if ((event->time -  cw->liteClue.HelpPopDownTime) > 
			cw->liteClue.cancelWaitPeriod ) 
		current_waitPeriod = cw->liteClue.waitPeriod,timeout_event;
	else
		current_waitPeriod = 0;

	cw->liteClue.interval_id = XtAppAddTimeOut(
			XtWidgetToApplicationContext(w),
			current_waitPeriod, timeout_event, client_data);
}

/*
Remove timer, if its pending. Then popdown help.
*/
static void Leave_event(Widget w, XtPointer client_data, XEvent * xevent, Boolean * continue_to_dispatch )
{
	struct liteClue_context_str * obj = (struct liteClue_context_str *) client_data;
	XcgLiteClueWidget cw = obj->cw;
	XEnterWindowEvent * event = & xevent->xcrossing;

	if (cw->liteClue.interval_id != (XtIntervalId)0) 
	{
		XtRemoveTimeOut(cw->liteClue.interval_id);
		cw->liteClue.interval_id= (XtIntervalId)0;
	}

	if (obj->sensitive == False)
		return;
	if (cw->liteClue.HelpIsUp)
	{
		XtPopdown((Widget) cw);
		cw->liteClue.HelpIsUp = False;
		cw->liteClue.HelpPopDownTime = event->time;
	}
}

/* ---------------- Widget API ---------------------------- */

/*
;+
XcgLiteClueAddWidget -- Add a widget to be watched. LiteClue will be given for this widget

Func:	A widget will be added to the LiteClue watched list. Clues are given for
	sensitive watched widgets when the pointer enters its window. If the
	widget is already watched, the passed text replaces its current clue
	text. If text is null, the widget is still added, if it is not already
	in the list, but no clue will appear. Text may be specified with
	XcgLiteClueAddWidget in a subsequent call. When text is null and the
	widget is already in the list, its text is not changed. When a widget
	will is added to the watched list, it automatically becomes sensitive.
	Otherwise, its sensitivity is not changed. A watched widget which is not
	sensitive retains its context but clues are suppressed.
	None of this affects the behaviour of the watched widget itself.
	LiteClue monitors enter and leave events of the watched widget's
	window passively.

Input:	w - LiteClue widget
	watch - the widget to give liteClues for
	text - pointer to liteClue text. (May be NULL)
	size - size of text. May be zero
		in which case a strlen will be done.
	option - option mask, future use, zero for now.
Output: 

Return:	

;-
*/
void XcgLiteClueAddWidget(Widget w, Widget watch, const char * text, int size, int option )
{
#	define ROUTINE "XcgLiteClueAddWidget"
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	struct liteClue_context_str * obj;
	Boolean exists = False;

	CheckWidgetClass(ROUTINE);	/* make sure we are called with a LiteClue widget */

	obj = find_watched_widget(cw, watch);
	if (obj)
	{
		exists = True;
		if (text)
		{
			if(obj->text)
				XtFree(obj->text);
			obj->text = NULL;
		}
	}
	else
	{
		obj = alloc_link_liteClue_context(cw );
		obj->watched_w = watch;
	}
	if (text && !(obj->text))
	{
		if (!size)
			size = strlen(text);
		obj->text = XtMalloc(size+1);
		memcpy(obj->text, text, size);
		obj->text[size] = 0;
		obj->text_size = size;
	}
	if (!exists)	/* was created */
	{
		XtAddEventHandler(watch, EnterWindowMask, False, 
			Enter_event, (XtPointer) obj);
		XtAddEventHandler(watch, LeaveWindowMask|ButtonPressMask, 
			False, Leave_event, (XtPointer) obj);
		obj->sensitive = True;
	}

#	undef ROUTINE
}


/*
;+
XcgLiteClueDeleteWidget -- Delete a widget that is watched. 

Func:	A widget is deleted from the watched list and its resources are
	freed. LiteClue is no longer given for the widget.
	If the widget is not watched, nothing is done.

Input:	w - LiteClue widget
	watch - the widget to delete
Output: 

Return:	

;-
*/
void XcgLiteClueDeleteWidget(Widget w, Widget watch)
{
#	define ROUTINE "XcgLiteClueDeleteWidget"
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	struct liteClue_context_str * obj;

	CheckWidgetClass(ROUTINE);	/* make sure we are called with a LiteClue widget */
	obj = find_watched_widget(cw, watch);
	if (obj)
	{
		XtRemoveEventHandler(watch, EnterWindowMask, False, 
			Enter_event, (XtPointer) obj);
		XtRemoveEventHandler(watch, LeaveWindowMask|ButtonPressMask, 
			False, Leave_event, (XtPointer) obj);
		if (cw->liteClue.interval_id != (XtIntervalId)0) 
		{
			XtRemoveTimeOut(cw->liteClue.interval_id);
			cw->liteClue.interval_id= (XtIntervalId)0;
		}
		free_widget_context(cw, obj);
	}

#	undef ROUTINE
}


/*
;+
XcgLiteClueSetSensitive -- Enable/disable sensitivity for watched widget. 

Func:	When a watched widget is sensitive, a clue is poped up when the pointer
	enters its window. When a watched widget is insensitive, the widget is
	retained in the watched list but no clue is poped. The sensitivity of a
	watched widget relative to clues is set or reset by this function. The
	Xt sensitivity of the watched widget is not altered by this function.

Input:	w - LiteClue widget
	watch - the widget to make sensitive or insensitive or NULL
		to change all watched widgets
	sensitive - True or False
Output: 

Return:	

;-
*/
void XcgLiteClueSetSensitive(Widget w, Widget watch, Boolean sensitive)
{
#	define ROUTINE "XcgLiteClueSetSensitive"
	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	struct liteClue_context_str * obj;

	CheckWidgetClass(ROUTINE);	/* make sure we are called with a LiteClue widget */
	if (watch)
	{
		obj = find_watched_widget(cw, watch);
		if (obj)
		{
			obj->sensitive = sensitive;
			return;
		}
		else
			return;
	}

	/* do them all */
	for (obj = (struct liteClue_context_str *) cw->liteClue.widget_list.forw; 
		obj != (struct liteClue_context_str *) & cw->liteClue.widget_list; 
		obj = (struct liteClue_context_str *)obj->next.forw )
	{
		obj->sensitive = sensitive;
	}

#	undef ROUTINE
}

/*
;+
XcgLiteClueGetSensitive -- Get sensitivity mode for watched widget. 

Func:	When a watched widget is sensitive, a clue is poped up when the pointer
	enters its window. When a watched widget is insensitive, the widget is
	retained in the watched list but no clue is poped. The sensitivity state
	of a watched widget relative to clues is returned by this function. The
	Xt sensitivity of a widget is a totally independent concept.

Input:	w - LiteClue widget
	watch - the widget for which to get sensitivity state. If NULL
		first watched widget is used. If there are no watched widgets,
		False is returned.
Output: 

Return:	sensitive - True or False

;-
*/
Boolean XcgLiteClueGetSensitive(Widget w, Widget watch)
{
#	define ROUTINE "XcgLiteClueGetSensitive"

	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	struct liteClue_context_str * obj;

	CheckWidgetClass(ROUTINE);	/* make sure we are called with a LiteClue widget */
	if (watch)
	{
		obj = find_watched_widget(cw, watch);
		if (obj)
			return obj->sensitive;
		else
			return False;
	}
	/* do the first one */
	obj = (struct liteClue_context_str *) cw->liteClue.widget_list.forw; 
	if (obj != (struct liteClue_context_str *) & cw->liteClue.widget_list)
		return obj->sensitive;
	else
		return False;

#	undef ROUTINE
}


/*
;+
XcgLiteClueDispatchEvent -- Dispatch event from main X event loop

Func:	This function may be used to enable clues for insensitive
	watched widgets. Normally, XtAppMainLoop (which calls
	XtDispatchEvent) will not deliver EnterNotify and LeaveNotify
	events to widgets that are not sensitive (XtSetSensitive). This
	prevents clues from poping up for these widgets. To bypass this
	limitation, you can break out XtAppMainLoop and add a call to
	XcgLiteClueDispatchEvent ass follows:

	MyXtAppMainLoop(XtAppContext app) 
	{
	    XEvent event;

	    for (;;) {
	        XtAppNextEvent(app, &event);
		XcgLiteClueDispatchEvent(w, event) ;
	        XtDispatchEvent(&event);
	    }
	} 

Input:	w - LiteClue widget
	event - received event, normally from call to XtAppNextEvent.

Output: void

Return:	True - event was dispatched to non-sensitive watched widget.
	False - not a EnterNotify or LeaveNotify event or window in
		event is not a non-sensitive watched widget.

;-
*/
Boolean XcgLiteClueDispatchEvent(Widget w, XEvent  *event)
{
#	define ROUTINE "XcgLiteClueDispatchEvent"

	XcgLiteClueWidget cw = (XcgLiteClueWidget) w;
	struct liteClue_context_str * obj;
	Boolean continue_to_dispatch;

	if (event->type != EnterNotify && event->type != LeaveNotify)
		return False;
	CheckWidgetClass(ROUTINE);	/* make sure we are called with a LiteClue widget */

	/* scan list */
	for (obj = (struct liteClue_context_str *) cw->liteClue.widget_list.forw; 
		obj != (struct liteClue_context_str *) & cw->liteClue.widget_list; 
		obj = (struct liteClue_context_str *)obj->next.forw )
	{
		if ((XtWindow(obj->watched_w) != event->xany.window)
		||  (XtIsSensitive(obj->watched_w)) )
			continue;
		/* found one */
		if (event->type == EnterNotify )
			Enter_event(obj->watched_w, (XtPointer)obj, event,  &continue_to_dispatch);
		else
			Leave_event(obj->watched_w, (XtPointer)obj, event,  &continue_to_dispatch);
		return True;
	}
	return False;

#	undef ROUTINE
}

