#include <values.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <keys.h>
#include	<io.h>
#include <fcntl.h>
#include <dos.h>
#include <bios.h>
#include <ctype.h>
#include <alloc.h>
#include "wserv.h"
#include <stdlib.h>
#include "text_r.h"

#define true  1
#define false 0

typedef struct box__{
	char *buffer;
	unsigned int x,y,dx,dy;
	unsigned int ox,oy,odx,ody;
	int shape,posit;
	char footn[160];
	struct box__ *link;
	} box;

static char buffer[30];

static convert conv;

static box *head=NULL;

static coor Help_x = 0;
static coor Help_y = 0;
static char Help_file[40] = "";
static char Help_attrB;
static char Help_attrT;

static coor Win_x,Win_y,Win_dx,Win_dy;

static char *bar_ptr;
static int bar_length=-1;
static long bar_cur, bar_mul, bar_scale;

extern int far wcolors[];

int Mouse=1;
static int Mouse_dx,Mouse_dy,Sens_X=16,Sens_Y=24;

void mouse_init (void)

{
	union REGS reg;

	if (Mouse) {
		reg.x.ax=0;
		int86(0x33,&reg,&reg);
		Mouse=-reg.x.ax;
		Mouse_dx=0;
		Mouse_dy=0; }
}

static int mouse_button (int which)

{
	union REGS reg;

	reg.x.ax=5;
	reg.x.bx=which;
	int86(0x33,&reg,&reg);
	return (reg.x.bx);
}

static void mouse_qdist (int *dx,int *dy)

{
	union REGS reg;

	reg.x.ax=0x0B;
	int86(0x33,&reg,&reg);
	(*dx)+=reg.x.cx;
	(*dy)+=reg.x.dx;
}

static int mouse_key (void)

{
	int j;

	if (Mouse) {
		if (mouse_button(0)) return (Enter);
		if (mouse_button(1)) return (Esc);
		mouse_qdist(&Mouse_dx,&Mouse_dy);
		if (abs(Mouse_dx)>Sens_X) {
			j=(Mouse_dx<0)?Left:Right;
			Mouse_dx=0;
			return (j); }
		if (abs(Mouse_dy)>Sens_Y) {
			j=(Mouse_dy<0)?Up:Down;
			Mouse_dy=0;
			return (j); } }
	return (0);
}

static int mouse_menu (void)

{
	static char *choices[] = { "Ins","Del","F10" };
	int j;

	j=wmenu (1,1,3,3,3,choices,NULL,1,2,0,3,2,0,WSERV_TEXT_11,0,0,0,NULL,NULL);
	if (j!=0) wrestore();
	switch (j) {
		case 1:
			return (Ins);
		case 2:
			return (Del);
		case 3:
			return (F10); }
	return (0);
}

#pragma warn -par

int wdummy_ (char *s)
{
return (0);
}

#pragma warn +par

int __integ (char *s)

{
	int	issign = 0;
	int	k = 1;
	int	i;

	conv.i=0;
	if (strlen(s)==0) return (1);
	for (i=0;i<strlen(s);i++) {
		if (s[i]==' ') continue;
		if (isdigit(s[i])) conv.i=conv.i*10+s[i]-'0';
		else
			if (issign||i>0) return (i+1);
			else
				switch (s[i]) {
				case '-':
					k=-1;
				case '+':
					issign=1;
					break;
				default:
					return (i+1); } }
	conv.i*=k;
	return(0);
}

int __float(char *s)

{
	int signm=0;
	int signe=0;
	int km=1;
	int ke=1;
	int ise=0;
	int isp=0;
	int i;
	double ma=0;
	double ex=0;
	double cex=0;

	if (strlen(s)==0) return (1);
	for (i=0;i<strlen(s);i++) {
		if (s[i]==' ') continue;
		if (isdigit(s[i])) {
			if (ise) {
				ex=ex*10+s[i]-'0';
				signe=1; }
			else {
				ma=ma*10+s[i]-'0';
				if (isp) cex--;
				signm=1; } }
		else
			switch (s[i]) {
				case '-':
					if (ise)
						ke=-1;
					else
						km=-1;
				case '+':
					if ((ise&&signe)||(!ise&&signm)) return (i+1);
					break;
				case '.':
					if (isp||ise) return (i+1);
					signm=1;
					isp=1;
					break;
				case 'E':
				case 'e':
					if (ise) return (i+1);
					ise=1;
					break;
				default:
					return (i+1); } }
	if (ma<MINFLOAT) {
		conv.f=0;
		return (0); }
	if ((conv.f=log10(ma)+ke*ex+cex)>log10(MAXFLOAT)||conv.f<log10(MINFLOAT))
		return (strlen(s)+1);
	conv.f=km*pow(10,conv.f);
	return (0);
}

static void draw_bar (x,y,len,attr)
coor x,y;
int len;
int attr;

{
	char *p;
	int i;

	p=wgeta(x,y)+1;
	for (i=0;i<len;i++) {
		*p=wcolors[attr];
		p+=2; }
}

void wblock (x,y,dx,dy,attr,text,type)
coor x,y,dx,dy,attr,type;
char *text;

{
	char *p;
	int width,i;
	const char *bars[2] = { "ÚÄ¿³ÙÀ" ,
	                        "ÉÍ»º¼È" };

	width = wwidth ();
	wsave (x,y,dx,dy);
	wattr (attr);
	p=wgeta(x,y);
	*p=bars[type][0]; p+=2;
	for (i=x+2;i<x+dx;i++) { *p=bars[type][1]; p+=2; }
	*p=bars[type][2];
	for (i=y+2;i<y+dy;i++) {
		p+=width*2;
		*p=bars[type][3]; }
	p+=width*2;
	*p=bars[type][4];
	for (i=x+2;i<=x+dx;i++) { p-=2; *p=bars[type][1]; }
	*p=bars[type][5];
	for (i=y+2;i<y+dy;i++) {
		p-=width*2;
		*p=bars[type][3]; }
	if (text!=NULL) {
		i=(dx-strlen(text))&0xFFFE;
		p+=i-width*2;
		do {
			*p=*text++;
			p+=2;
		} while (*text); }
		wwindow (x+1,y+1,dx-2,dy-2);
}

void wsetcursor (from,to)
int from,to;

{
	union REGS reg;

	reg.h.ah=0x01;
	reg.h.ch=from;
	reg.h.cl=to;
	int86(0x10,&reg,&reg);
}

char *wgeta(x,y)
coor x,y;

{
	union REGS reg;
	reg.h.ah=0x0F;
	int86(0x10,&reg,&reg);
	return ((char *) 0xB8000000+2*((y-1)*reg.h.ah+x-1))+0x1000*reg.h.bh;
}

int wgetkey (int mkey)

{
	unsigned j;

	for (;;) {
		if (bioskey(1)>0) {
			j=bioskey(0);
			if ((j&0xFF)==0)
				return((j>>8)|0x100);
			else
				return(j&0xff); }
		else {
			j=mouse_key();
			switch (j) {
			case 0:
				break;
			case Esc:
				if (mkey==MENU) {
					j=mouse_menu();
					if (j==0) break; }
			case Enter:
				return (j);
			default:
				if (mkey==KEYONLY) break;
				return (j); } } }
}

void wsethelp (file,x,y,ab,at)
coor x,y;
int ab,at;
char *file;

{
	if (x!=0) Help_x = x;
	if (y!=0) Help_y = y;
	if (file!=NULL) strcpy(Help_file,file);
	Help_attrB = ab;
	Help_attrT = at;

}

static void _whelp(int i,int handle)

{
	int ptr,width,xref;
	char *p, *r, c;
	long l;
	char eop,move;
	long stack[15],old;
	char coord[4];
	int keyhlp=(handle!=0);

	if (keyhlp) {
		old=tell(handle);
		lseek(handle,0l,SEEK_SET); }
	else
		if ((handle=_open(Help_file,O_RDONLY|O_BINARY))<0) return;

	if (_read(handle,coord,sizeof(coord))<0) return;
	if (Help_x==0) Help_x = coord[0];
	if (Help_y==0) Help_y = coord[1];
	wblock (Help_x+((keyhlp)?3:0),Help_y-((keyhlp)?2:0),
						coord[2],coord[3],Help_attrB,WSERV_TEXT_1,0);
	if (keyhlp)
		wfoot(WSERV_TEXT_2);
	else
		wfoot(WSERV_TEXT_3);
	wattr (Help_attrT);
	wsetcursor (0x20,0);

	ptr=0;
	if (lseek(handle,i*4,SEEK_SET)<0) return;
	if (_read(handle,&l,4)<0) return;
	if (lseek(handle,l,SEEK_SET)<0) return;
	if (_read(handle,&xref,2)<0) return;

	eop=0;
	for (;;) {
		wclrscr(Help_attrT);
		stack[ptr++]=tell(handle);
		p=wgeta(Help_x+2+((keyhlp)?3:0),Help_y+1-((keyhlp)?2:0));
		r=p;
		width = wwidth ();
		for (;;) {
			if (_read(handle,&c,1)<0) return;
			switch (c) {
				case '\r':
					r+=width*2;
					p=r;
					break;
				case '\003':
					eop=1;
				case '\014':
					break;
				default:
					*p=c;
					p+=2;
				}
			if (eop||c=='\014') break; }
		p=wgeta(Help_x+coord[2]-7+((keyhlp)?3:0),
					Help_y+coord[3]-1-((keyhlp)?2:0));
		if (ptr>1) wputs (&p," \x1E ");
			else    wputs (&p,"\xC4\xC4\xC4");
		if (!eop)  wputs (&p," \x1F ");
			else    wputs (&p,"\xC4\xC4\xC4");

		move=1;
		do {
			switch (wgetkey(KEYONLY)) {
				case F1:
					if (!keyhlp) _whelp(xref,handle);
					break;
				case Esc:
					wrestore ();
					if (keyhlp)
						lseek(handle,old,SEEK_SET);
					else
						_close (handle);
					return;
				case PgUp:
					if (ptr<=1) break;
					ptr-=2;
					if (lseek(handle,stack[ptr],SEEK_SET)<0) return;
					eop=0;
					move=0;
					break;
				case PgDn:
					if (!eop) move=0;
			}
		} while (move);
	}
}

void whelp (int i)

{
	_whelp(i,0);
}

static void place_bar (x,y,i,attr1,attr,col,len)
coor x,y;
int i;
int attr1,attr,col,len;

{
char *p;
coor xn,yn;

	yn=(i-1)/col;
	xn=(i-yn*col-1)*(len+3);
	p=wgeta(x+xn+1,y+yn+1)+1;
	*p=wcolors[attr];  p+=2;
	*p=wcolors[attr1]; p+=2;
	for (i=1;i<=len;i++) {
		*p=wcolors[attr]; p+=2; }
}

unsigned wmenu (x,y,len,n,col,choice,dsabl,
			attrB,attrT,attrF,attrH,attrD,type,text,flags,init,help,footn,exitp)
coor x,y;			/* Left top corner coordinates */
int n;				/* Number of choices */
int len;			/* Choice length */
int col;			/* Number of columns of choices */
char **choice;		/* Choices themselves */
int *dsabl;		/* True if entry disabled */
int attrB;		/* Border attribute */
int attrT;		/* Normal text attribute */
int attrF;		/* First letter attribute */
int attrH;		/* Highlighted text attribute */
int attrD;		/* Disabled entry attribute */
int type;			/* Border type */
char *text;			/* Window headline */
int flags;			/* Menu flags:
			MF_let	first letter doesn't select
			MF_fn		Fn key returns
			MF_sfn	Shift/Fn key returns
			MF_afn	Alt/Fn key returns
			MF_cfn	Control/Fn key returns
			MF_xfn	All X/Fn key returns
			MF_esc   Escape don't cause an exit
			MF_pgx	PgUp/PgDn exit
			MF_cky	Ctrl/keypad exit
			MF_ckb	Ctrl/alpha exit
			MF_akb	Alt/alpha exit */

int init;			/* Initial position in menu */
int help;			/* Initial partition for help */
char *footn;
void (*exitp) (int);

{
int dx,dy,width;
int i,j,k,l,exit;
int pos,posn;
char *p,*q,*r;
int c;
char buffer[81];

	if (init==0) init=1;

	dx=(len+3)*col+1;
	dy=n/col;
	if (dy*col!=n) dy++;
	dy+=2;

	if (init>0) {
		if (!(MF_blk&flags)) {
			wblock(x,y,dx,dy,attrB,text,type);
			if (footn==NULL) {
				strcpy(buffer,WSERV_TEXT_4);
				if (!(MF_esc&flags))
					strcat(buffer,WSERV_TEXT_5);
				if (!(MF_hlp&flags)&&help!=0)
					strcat(buffer,WSERV_TEXT_6);
				wfoot(buffer); }
			else
				wfoot(footn); }
		wsetcursor (0x20,0);
		if (MF_avo&flags) wavoid (); }

	if (init>0) {
		wattr(attrT);
		p=wgeta(x+1,y+1)+2;
		width = wwidth ();

		k=0;
		for (i=1;i<=dy-2;i++) {
			r=p;
			p+=width*2;
			for (j=1;j<=col;j++) {
				if (dsabl!=NULL && dsabl[k])
					place_bar (x,y,k+1,attrD,attrD,col,len);
				else
					*(r+1)=wcolors[attrF];
				q=choice[k++];
				for (l=1;l<=len+1;l++) {
					if (*q) *r=*q++;
					r+=2; }
				if (k>=n) break;
				r+=4; } }

		if (dsabl==NULL || !dsabl[init-1])
			place_bar (x,y,init,attrH,attrH,col,len); }
	else
		init=-init;
	pos=init;
	posn=pos;
	exit=0;
	if (exitp!=NULL)
		exitp(pos-1);

	for (;;) {

		c=wgetkey(ALL);

		if (!(MF_hlp&flags)&&c==F1) {
			if (help>0)
				whelp(help+pos-1);
			else if (help<0)
				whelp(-help);
			continue; }
		if ((MF_fn&flags&&c>=F1&&c<=F10)||
			(MF_cfn&flags&&c>=Ctrl_F1&&c<=Ctrl_F10)||
			(MF_afn&flags&&c>=Alt_F1&&c<=Alt_F10)||
			(MF_sfn&flags&&c>=Shift_F1&&c<=Shift_F10)||
			(MF_akb&flags&&(c>=Alt_Q&&c<=Alt_P||c>=Alt_A&&c<=Alt_L||
									c>=Alt_Z&&c<=Alt_M))||
			(MF_ckb&flags&&c>='A'-0x40&&c<='Z'-0x40)) goto Retpack;

		switch (c) {

			case PgUp:
			case PgDn:
				if (MF_pgx&flags) goto Retpack;
				break;
			case Ins:
			case Del:
			case Ctrl_Left:
			case Ctrl_Right:
			case Ctrl_PgDn:
			case Ctrl_PgUp:
			case Ctrl_Home:
			case Ctrl_End:
				if (MF_cky&flags) goto Retpack;
				break;
			case Enter: return (pos);
			case Esc:	if (MF_esc&flags) break;
							if (!(MF_avo&flags)&&!(MF_res&flags)) wrestore ();
							return (0);
			case Left:  if (col==1) break;
							do
								if (--posn==0) posn=n;
							while (dsabl!=NULL&&dsabl[posn-1]);
							break;
			case Right: if (col==1) break;
							do
								if (++posn>n) posn=1;
							while (dsabl!=NULL&&dsabl[posn-1]);
							break;
			case Home:	posn=1;
							while (dsabl!=NULL&&dsabl[posn-1]) posn++;
							break;
			case End:	posn=n;
							while (dsabl!=NULL&&dsabl[posn-1]) posn--;
							break;
			case Up:    if (dy==3) break;
							do {
								if ((posn-=col)<1) {
									posn+=(dy-1)*col-1;
									if (posn>n) posn-=col; }
							} while (dsabl!=NULL&&dsabl[posn-1]);
							break;
			case Down:  if (dy==3) break;
							do {
								if ((posn+=col)>n) {
									posn-=(dy-1)*col-1;
									if (posn<1) posn+=col; }
							} while (dsabl!=NULL&&dsabl[posn-1]);
							break;
							}

		if (!(MF_let&flags)&&isalnum(c)&&!(0x100&c))
			for (i=0;i<n;i++)
				if (toupper(c)==toupper(*choice[i])) {
					exit=1;
					posn=i+1;
					break; }

		if (pos!=posn) {
			place_bar (x,y,pos,attrF,attrT,col,len);
			place_bar (x,y,posn,attrH,attrH,col,len);
			pos = posn;
			if (exitp!=NULL)
				exitp (pos-1); }
	if (exit) return (pos); }

Retpack:
	return((c<<7)|pos);
}

int wsave (x,y,dx,dy)
coor x,y,dx,dy;
{
	box *ptr;
	union REGS reg;

	if ((ptr=malloc(sizeof(box)))==NULL) return (false);
	ptr->link=head;
	head=ptr;
	head->ox=Win_x;
	head->oy=Win_y;
	head->odx=Win_dx;
	head->ody=Win_dy;
	if ((head->buffer=malloc(dx*dy*2))==NULL) return (false);
	wwindow (x,y,dx,dy);
	wgettext (x,y,dx,dy,head->buffer);
	wgettext (1,25,80,1,head->footn);
	head->x=x;
	head->dx=dx;
	head->y=y;
	head->dy=dy;
	reg.h.ah=0x0F;
	int86(0x10,&reg,&reg);
	reg.h.ah=0x03;
	int86(0x10,&reg,&reg);
	head->shape=reg.x.cx;
	head->posit=reg.x.dx;
	return (true);
}

int wrestore (void)

{
	box *ptr;
	union REGS reg;

	if (head==NULL) return (false);
	wputtext (1,25,80,1,head->footn);
	wputtext (head->x,head->y,head->dx,head->dy,head->buffer);
	wwindow (head->ox,head->oy,head->odx,head->ody);
	reg.h.ah=0x0F;
	int86(0x10,&reg,&reg);
	reg.h.ah=0x01;
	reg.x.cx=head->shape;
	int86(0x10,&reg,&reg);
	reg.h.ah=0x02;
	reg.x.dx=head->posit;
	int86(0x10,&reg,&reg);
	ptr=head;
	head=ptr->link;
	free (ptr->buffer);
	free (ptr);
	return (true);
}

int wavoid (void)

{
	box *ptr;

	if (head==NULL) return (false);
	ptr=head;
	head=ptr->link;
	free (ptr->buffer);
	free (ptr);
	return (true);
}

int wwidth (void)

{
	union REGS reg;
	reg.h.ah=0x0F;
	int86(0x10,&reg,&reg);
	return (reg.h.ah);
}

void wattr (attr)
int attr;

{
	wclrscr (attr);
}

void wputs (p,string)
char **p,*string;

{
	int i;

	i=0;
	while (string[i]!=0) {
		**p=string[i++];
		(*p)+=2; }
}

void wsputs (p,attr,string)
char **p;
int attr;
char *string;

{
	int i,j;

	j=*(wcolors+attr);
	i=0;
	while (string[i]!=0) {
		*(*p++)=string[i++];
		*(*p++)=j; }
}

void wfoot (string)
char *string;

{
	int i,j,attr;
	char *p;

	p=wgeta(1,25);
	attr=*(wcolors+10);
	*p++=' ';
	*p++=attr;

	for (i=0,j=0;i<wwidth()-1;) {
		switch (string[j]) {
			case '{':
				attr=*(wcolors+10);
				j++;
				break;
			case '}':
				attr=*(wcolors+11);
				j++;
				break;
			case '\0':
				*p++=' ';
				*p++=attr;
				i++;
            break;
			default:
				*p++=string[j++];
				*p++=attr;
				i++; } }
}

void wbell (void)

{
	int i;

	for (i=1;i<14;i++) { sound (100+250*i); delay (4); }
	nosound();
}

int winput (x,y,text,type,help,attrB,attrT,attrD,buf,len,check)
coor x,y;
char *buf;
char *text;
int type;
int help;
int len;
int attrB,attrT,attrD;
int check(char *s);

{
	char *p, *q, *r;
	int pos=0;
	int def=0;
	int just=0;
	int i,j,c;


	wblock(x,y,len+4,3,attrB,text,type);
	wattr (attrD);
	wfoot (WSERV_TEXT_7);
	wsetcursor (7,13);

	if((q=malloc(len))==NULL) {
		wrestore();
		return (0); }
	j=strlen(buf);
	strcpy (q,buf);
	p=wgeta (x+2,y+1);
	r=p;
	wputs (&p,q);

	for (;;) {
		if (def&&!just) draw_bar (x+1,y+1,len,attrT);
		wgotoxy (pos+2,1);
		c=wgetkey (ALL);
		if(isprint(c)) {
			if (!def&&!just) {
				j=0;
				*q=0;
				wattr(attrT);
				just=1; }
			if (j<len) {
				for (i=j;i>=pos;i--)
					r[i*2+2]=r[i*2];
				r[pos*2]=c;
				pos++;
				j++; }
			else
				wbell (); }
		else
			switch (c) {
				case F1:
					whelp(help);
					break;
				case Esc:
					wrestore();
					return (0);
				case Left:
					def=1;
					if (pos>0) pos--;
					break;
				case Right:
					def=1;
					if (pos<j) pos++;
					break;
				case Home:
					def=1;
					pos=0;
					break;
				case End:
					def=1;
					pos=j;
					break;
				case BS:
					if (pos==0)	break;
					pos--;
				case Del:
					def=1;
					if (pos<j) {
						for (i=pos;i<j;i++)
							r[i*2]=r[i*2+2];
						r[j*2]=' ';
						j--; }
					break;
				case Enter:
					for (i=0;i<j;i++)
						q[i]=r[i*2];
					q[j]=0;
					while (j>=1&&q[--j]==' ') q[j]=0;
					j++;
					if((pos=check(q)-1)<0) {
						strcpy (buf,q);
						wrestore ();
						return (strlen(buf)); }
					pos--;
					wbell (); } }
}

int wgint (x,y,text,help,attrB,attrT,attrD,init)
coor x,y;
char *text;
int help;
int attrB,attrT,attrD;
int init;

{
	int j;

	itoa(init,buffer,10);
	j=winput (x,y,text,0,help,attrB,attrT,attrD,buffer,25,__integ);
	if (j==0)
		return (init);
	else
		return (conv.i);
}

double wgfloat (x,y,text,help,attrB,attrT,attrD,init)
coor x,y;
char *text;
int help;
int attrB,attrT,attrD;
double init;

{
	int j;

	gcvt(init,7,buffer);
	j=winput (x,y,text,0,help,attrB,attrT,attrD,buffer,25,__float);
	if (j==0)
		return (init);
	else
		return (conv.f);
}

convert *wgetlast (void)

{
	return (&conv);
}

int wverify (x,y,str,help)
coor x,y;
char *str;
int help;

{
	char *p;

	wblock (x,y,strlen(str)+11,3,15,WSERV_TEXT_8,1);
	wattr (16);
	wfoot (WSERV_TEXT_9);

	p=wgeta(x+2,y+1);
	wputs (&p,str);
	wputs (&p,WSERV_TEXT_10);
	for (;;)
		switch (wgetkey(KEYONLY)) {
		case Esc:
			wrestore ();
			return (0);
		case 'Y':
		case 'y':
			wrestore ();
			return (1);
		case 'N':
		case 'n':
			wrestore ();
			return (-1);
		case F1:
			whelp(help); }
}

void wwindow (x,y,dx,dy)
coor x,y,dx,dy;

{
	Win_x=x;
	Win_y=y;
	Win_dx=dx;
	Win_dy=dy;
}

void wclrscr (attr)
int attr;

{
	wcls (Win_x,Win_y,Win_dx,Win_dy,wcolors[attr]);
}

void wgotoxy (x,y)
coor x,y;

{
	union REGS reg;

	reg.h.ah=0x0F;
	int86(0x10,&reg,&reg);
	reg.h.ah=0x02;
	reg.h.dl=x+Win_x-2;
	reg.h.dh=y+Win_y-2;
	int86(0x10,&reg,&reg);
}

void wsetupbar (x,y,len,total)
coor x,y;
int len;
long total;

{
	int i;
	char *p;

	bar_ptr=wgeta(x,y);
	bar_length=len;
	bar_scale=(total<32000l)?32000l:1;
	bar_mul=bar_scale*total/(len+1);
	bar_cur=0;

	for (p=bar_ptr,i=0;i<len;i++) {
		*p='°';
		p+=2; }
}

void wappendbar (long current)

{
	current*=bar_scale;
	while (bar_length>0&&current-bar_cur>bar_mul) {
		bar_cur+=bar_mul;
		wputs (&bar_ptr,"Û"); }
}

void video_mode (int mode)

{
	union REGS reg;

	reg.h.ah=0;
	reg.h.al=mode;
	int86(0x10,&reg,&reg);
	mouse_init();
}

