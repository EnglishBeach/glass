//#include <keys.h>
#define F1  315
#define F2  316
#define F3  317
#define F9  323
#define F10 324
#define Enter 13
#define Esc 27
#define Ins 338
#define Del 339
#define Alt_S 287
#define Alt_X 301
#define Up 328
#define Down 336
#define Right 333
#define Left 331
#define PgUp 329
#define PgDn 337
#include <string.h>
#include <stdlib.h>
#include	<alloc.h>	/***/
#include "chem.h"
#include "wserv.h"
#include "stdarg.h"

#define widy 90
#define UP true
#define DOWN false

extern int wcolors[];

static char *screen;
static int cur_line=0;
static struct {
	coor x,y;
	int len,id,act;
} *action;
static int cur_act;
static int tot_act;
static int tot_lines;
static int typed_char;
static char *type_cont;
static coor old_x,old_y,old_len=0;
static Bool view_enabled = true;

static void bell (void)

{
	if (options.env.sound) wbell();
}

/*----------------------------------------------------------------*/
/*					Video modules for displayng a Task window				*/
/*----------------------------------------------------------------*/

/*
	Store text into buffer (use after store_initial)
*/

void store_continued (int len,...)

{
	int i;
	void *ap=...;
	char *p;

	while ((p=va_arg(ap,char*))!=NULL) {
		i=strlen(p);
		if (i+typed_char>len) i=len-typed_char;
		memcpy (type_cont,p,i);
		type_cont+=i;
		typed_char+=i; }
}

/*
	Store text into buffer from desired position
*/

void store_initial(coor x, coor y, int len, ...)

{
	void *ap=...;
	char *p;
	int i;

	typed_char=0;
	type_cont=screen+widy*y+x;
	while ((p=va_arg(ap,char*))!=NULL) {
		i=strlen(p);
		if (i+typed_char>len) i=len-typed_char;
		memcpy (type_cont,p,i);
		type_cont+=i;
		typed_char+=i; }
}

/*
	Fill field started with store_initial and width of <width>
	with spaces
*/

void store_clear (int width)

{
	int i;
	for (i=typed_char;i<width;i++) *type_cont++=' ';
}

/*
	Get curren offset from field start
*/

int store_getx (void)
{
	return (typed_char);
}

void display_core (void)

{
	char buffer[30],*p;

	p=wgeta(51,2);
	wputs(&p,EDVIDEO_TEXT_1);
	wputs(&p,itoa((int)(coreleft()/1024),buffer,10));
	wputs(&p,"K  ");
}

/*
	Change background of bar started from <x,y> and length
	of <len> to color number <attr>
*/

static void place_bar (coor x,coor y,int len,int attr)

{
	char *p=wgeta(x+3,y+3)+1;
	int i,j=wcolors[attr];
	for (i=0;i<len;i++) {
		*p=j;
		p+=2; }
}

/*
	Remove old marker and, if <move> is true, set new one in
	current position
*/

static void set_marker(Bool move)

{
	if (old_len!=0) place_bar (old_x,old_y,old_len,32);
	if (move)
	  place_bar (action[cur_act].x,action[cur_act].y,action[cur_act].len,33);
	old_x=action[cur_act].x;
	old_y=action[cur_act].y;
	old_len=action[cur_act].len;
}

/*
	Checks an ability to move marker one item up only on
	current screen; return true and move marker if possible,
	otherwise return false
*/

static Bool match_up (void)

{
	int next_act,sline,delta,i,j;

	next_act=cur_act;
	if (tot_act>0) {
		sline=-1;
		delta=999;
		for (i=cur_act-1;i>=0;i--) {
			if (action[i].y==action[cur_act].y) continue;
			if (sline<0) sline=action[i].y;
			if (action[i].y<sline) break;
			if ((j=abs(action[i].x-action[cur_act].x))<delta) {
				delta=j;
				next_act=i; } } }
	if (next_act==cur_act)
		return (false);
	else {
		cur_act=next_act;
		set_marker (true);
		return (true); }
}

static Bool match_down (void)

/*
	Checks an ability to move marker one item down only on
	current screen; return true and move marker if possible,
	otherwise return false
*/

{
	int next_act,sline,delta,i,j;

	next_act=cur_act;
	if (tot_act>0) {
		sline=-1;
		delta=999;
		for (i=cur_act+1;i<tot_act;i++) {
			if (action[i].y==action[cur_act].y) continue;
			if (sline<0) sline=action[i].y;
			if (action[i].y>sline) break;
			if ((j=abs(action[i].x-action[cur_act].x))<delta) {
				delta=j;
				next_act=i; } } }
	if (next_act==cur_act)
		return (false);
	else {
		cur_act=next_act;
		set_marker (true);
		return (true); }
}

/*
	Scroll window contents one line up, blank bottom line
*/

static void scroll_window_up (void)

{
	char buffer[3000];
	int i;

	wgettext (3,3,63,21,buffer);
	for (i=2646;i<2772;) {
		buffer[i++]=' ';
		buffer[i++]=wcolors[32]; }
	wputtext (3,3,63,21,buffer+126);
}

/*
	Scroll window contents one line down, blank top line
*/

static void scroll_window_down (void)

{
	char buffer[3000];
	int i;

	wgettext (3,3,63,21,buffer+126);
	for (i=0;i<126;) {
		buffer[i++]=' ';
		buffer[i++]=wcolors[32]; }
	wputtext (3,3,63,21,buffer);
}

/*
	Copy a line from to screen, setting up fields breaks into
	<action> array only if <store> is true. First offset in
	<action> assumed to be <ap>
*/

static void store_line (int line, int ap, Bool store)

{

	char *p=wgeta(3,line+3);
	char *r;
char sp[]="                                                              ";
	int i,j=0;
	Bool rest=false;

	rest=false;
	if (cur_line+line<tot_lines)
		r=screen+(cur_line+line)*widy;
	else
		r=sp;
	for (i=0,j=0;i<widy&&j<63;i++)
		switch (*r) {
		case 1:
			r++;
			if (store) {
				ins_cell (&(void*)action,tot_act++,ap,sizeof(*action));
				action[ap].y=line;
				action[ap].x=j;
				action[ap].id=*r++;
				action[ap].len=*r++;
				action[ap++].act=*r++; }
			else
				r+=3;
			break;
		case 2:
			rest=true;
		case 3:
			r++;
			break;
		default:
			if (rest)
				*p=' ';
			else
				*p=*r++;
			j++;
			p+=2; }
	for (i=0;i<63-j;i++) {
		*p=' ';
		p+=2; }
}

/*
	Recopy <count> lines starting at offset <from> from current line
*/

void refresh_line (int from, int count)

{
	int i,j;

	if (view_enabled)
		for (i=action[cur_act].y+from,j=0;j<count;i++,j++)
			if (i>=0&&i<=21) store_line (i,0,false);
}

/*
	Fill whole screen from buffer
*/

static void fill_screen_buffer (int line)

{
	int i;

	set_marker (false);
	tot_act=0;
	if ((action=realloc(action,sizeof(*action)))==NULL) exit_nomem ();
	cur_line=line;
	action[0].act=0;
	for (i=0;i<21;i++)
		store_line (i,tot_act,true);
}

/*
	Refresh screen, move marker <shift> fields
*/

void refresh_screen (int shift)

{
	if (view_enabled) fill_screen_buffer (cur_line);
	cur_act+=shift;
	if (cur_act<0) cur_act=0;
	if (view_enabled) set_marker (true);
}

/*
	Refresh screen, don't place marker
*/

void view_screen (void)

{
	old_len=0;
	fill_screen_buffer (0);
	cur_act=0;
}

/*
	Scroll window one row up, correct <action> array
*/

static void scroll_row_up (void)

{
	int i;

	if (cur_line>0) {
		set_marker (false);
		while (tot_act>=0&&action[tot_act-1].y==20) {
			del_cell (&(void*)action,tot_act,tot_act-1,sizeof(*action));
			tot_act--; }
		if (tot_act==0) cur_act=0;
		for (i=0;i<tot_act;i++) action[i].y++;
		scroll_window_down ();
		cur_line--;
		store_line (0,0,true);
		if (!match_up ()) set_marker (true); }
	else
		bell ();
}

/*
	Scroll window one row down, correct <action> array
*/

static void scroll_row_down (void)

{
	int i,j=0;

	if (cur_line+20<tot_lines) {
		set_marker (false);
		while (action[j].y==0&&j<tot_act) j++;
		if (j>0) for (i=0;i<j;i++) {
			del_cell (&(void*)action,tot_act,0,sizeof(*action));
			tot_act--; }
		cur_act-=j;
		for (i=0;i<tot_act;i++) action[i].y--;
		scroll_window_up ();
		cur_line++;
		store_line (20,tot_act,true);
		if (!match_down ()) set_marker (true); }
	else
		bell ();
}

/*
	Initial settings for Task window. Called only once at run
*/

void edit_init (void)

{
	char *p;

	wblock (1,1,66,24,31,EDVIDEO_TEXT_2,0);
	wavoid ();
	wattr (34);
	place_bar (-1,-1,64,41);
	p=wgeta (6,2);
	wputs (&p,EDVIDEO_TEXT_3);
	display_core();

	tot_lines=17;
	if ((screen=calloc(widy,tot_lines))==NULL) exit_nomem ();
	if ((action=calloc(sizeof(*action),1))==NULL) exit_nomem ();
	memset(screen,' ',widy*tot_lines);
	store_initial (0,0,1000,EDVIDEO_TEXT_4,NULL);
	store_initial (0,1,1000,"� \x01\xFF\x3A\x0A                                                           �",NULL);
	store_initial (0,2,1000,"읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸",NULL);
	store_temp ();
	store_pres ();
	store_step ();
	store_ion ();
	store_initial (0,7,1000,EDVIDEO_TEXT_5,NULL);
	store_initial (0,9,1000,EDVIDEO_TEXT_6,NULL);
	store_initial (0,11,1000,EDVIDEO_TEXT_7,NULL);
	store_initial (0,13,1000,EDVIDEO_TEXT_8,NULL);
	store_initial (0,15,1000,EDVIDEO_TEXT_9,NULL);
	fill_screen_buffer (0);
	cur_act=0;
}

/*
	Edit a task.
*/

void edit_service (void)

{
	int c;

	wbkgnd (2,3,64,21,wcolors[32]);
	wbkgnd (67,1,14,13,wcolors[35]);
	set_marker (true);
	wfoot(EDVIDEO_TEXT_10);
	for (;;)
		switch (c=wgetkey(MENU)) {
		case F10:
		case Esc:
			set_marker (false);
			wbkgnd (2,3,64,21,wcolors[34]);
			return;
		case F9:
		case Alt_S:
			set_marker (false);
			wbkgnd (2,3,64,21,wcolors[34]);
			solve_service ();
			wbkgnd (2,3,64,21,wcolors[32]);
			set_marker (true);
			break;
		case Alt_X:
			leave_Chemical();
		case Up:
			if (!match_up ()) scroll_row_up ();
			break;
		case Down:
			if (!match_down ()) scroll_row_down ();
			break;
		case ' ':
		case Right:
			if (tot_act>0) {
				if (++cur_act==tot_act) cur_act=0;
				set_marker (true); }
			break;
		case Left:
			if (tot_act>0) {
				if (--cur_act<0) cur_act=tot_act-1;
				set_marker (true); }
			break;
		case PgUp:
			if (cur_line!=0) {
				set_marker (false);
				fill_screen_buffer (max(cur_line-20,0));
				cur_act=tot_act-1;
				set_marker (true); }
			else
				bell ();
			break;
		case PgDn:
			if (cur_line+21<tot_lines) {
				set_marker (false);
				fill_screen_buffer (min(cur_line+20,tot_lines-20));
				cur_act=0;
				set_marker (true); }
			else
				bell ();
			break;
		case F1:
			whelp (60+action[cur_act].act);
			break;
		case F2:
			file_save (false);
			break;
		case F3:
			file_load();
			store_task ();
			cur_act=0;
			fill_screen_buffer (0);
			set_marker (true);
			break;
		case Enter:
		case Ins:
		case Del:
			change_task (action[cur_act].id,action[cur_act].act,c);
			break; }
}

void enable_viewing (Bool enable)

{
	view_enabled=enable;
}


/*
	Insert row into screen buffer
*/

void insert_row (int n)

{
	ins_cell (&(void*)screen,tot_lines,n,widy);
	memset (screen+widy*n,' ',72);
	tot_lines++;
}

/*
	Remove a row from a screen buffer
*/

void delete_row (int n)

{
	del_cell (&(void*)screen,tot_lines,n,widy);
	tot_lines--;
}
