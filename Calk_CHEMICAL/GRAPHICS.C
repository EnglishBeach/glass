#define SOLVETYPES 1

#include "chem.h"
#include "wserv.h"
#include <graphics.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <keys.h>
#define Esc 27
#define Enter 13
#define F9 323
#include <alloc.h>
#include <dos.h>
#include <ctype.h>

static int card,old_card;
static XX *X;

static int low_y[] = { 164, 290, 608, 256 };
static int msg_l[] = { 180, 322, 0, 0 };
static int box_h[] = { 9, 14, 34, 34 };
static int drv_v[] = { CGA, EGA };
static int mod_v[] = { CGAHI, EGAHI };
static int grf_l[] = { 39, 17 };

static int *color;
static int cga_c[] = { 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
static int ega_c[] = { 0x01, 0x03, 0x0F, 0x0E, 0x0B, 0x0A, 0x0F, 0x0C };
enum { Bkgd, Grid, Grf1, Grf2, Labl, Text, Hlit, Errr };
enum { Wleft=164, Wright = 615, Wtop=8 };

static int Max_lines=16;

static float xa,xt,x0;
static float ya,y0,ytlin=0;
static int ylog;

static char *P_buf;
static unsigned P_off;
static int   P_x0,P_y0,P_x1,P_y1,P_style,P_color;

static struct text {
	int x;
	int y;
	char mess[26];
} text_heap[88];  /* Size must be at least 4*Maxlines+|log(sigfcy)|+7 */

static int text_ptr,total_text,scan_line;


static void open_video (void)

{
	initgraph (drv_v+card, mod_v+card, "");
	color=(card==0)?cga_c:ega_c;

	setbkcolor (color[Bkgd]);
}

static void pixel (int x,int y)

{
	unsigned b,s;

	if ((x+=P_x0)<=P_x1&&(y+=P_y0)<=P_y1) {
		b=(((y&1)==0)?0:P_off)+(unsigned)y/16*640+(unsigned)x;
		s=(y>>1)&0x07;
		if (P_color)
			P_buf[b]|=0x0080>>s;
		else
			P_buf[b]&=(signed)0xFF7F>>s; }
}

static void out_text (int x,int y, char *text)

{
	if (x>0&&y>0) {
		if (card<2)
			outtextxy(x,y,text);
		else {
			x+=P_x0; y+=P_y0;
			if (islower(text[0])) y-=(x<10)?4:8;
			text_heap[text_ptr].x=x;
			text_heap[text_ptr].y=y+(y>>1);
			strncpy(text_heap[text_ptr].mess,text,25);
			text_heap[text_ptr++].mess[25]=0; } }
}

static void set_color (int color)

{
	if (card<2)
		setcolor(color);
	else
		P_color=(color!=0);
}

static void put_line (int x1, int y1, int x2, int y2)

{
	int dy,dx,x;
	#define y x

	if (y1+P_y0>P_y1) y1=P_y1-P_y0; if (y1<0) y1=0;
	if (y2+P_y0>P_y1) y2=P_y1-P_y0; if (y2<0) y2=0;
	if (x1+P_x0>P_x1) x1=P_x1-P_x0; if (x1<0) x1=0;
	if (x2+P_x0>P_x1) x2=P_x1-P_x0; if (x2<0) x2=0;

	if (y1==y2&&x1==x2) return;

	if (card<2)
		line(x1,y1,x2,y2);
	else {
		if (abs(y2-y1)<abs(x2-x1)) {
			if (x1>x2) { x=x1; x1=x2; x2=x; y=y1; y1=y2; y2=y; }
			dx=x2-x1; dy=y2-y1;
			for (x=0;x<=dx;x++)
				if (P_style||(x&2)==0) pixel(x1+x,y1+(long)dy*x/dx); }
		else {
			if (y1>y2) { x=x1; x1=x2; x2=x; y=y1; y1=y2; y2=y; }
			dx=x2-x1; dy=y2-y1;
			for (y=0;y<=dy;y++)
				if (P_style||(x&2)==0) pixel(x1+(long)dx*y/dy,y1+y); } }
	#undef y
}

static void line_style (int style)

{
	if (card<2)
		setlinestyle (style,0,0);
	else
		P_style=!style;
}

static void fill_bar (int x1, int y1, int x2, int y2)

{
	int x;

	if (card<2) {
		setfillstyle(SOLID_FILL,0);
		setcolor (0);
		bar(x1,y1,x2,y2); }
	else {
		P_style=1;
		P_color=0;
		for (x=x1;x<=x2;x++)
			put_line(x,y1,x,y2); }
}

static void view_port (x0,y0,x1,y1,clip)

{
	P_x0=x0; P_x1=x1; P_y0=y0; P_y1=y1;
	if (card<2)
		setviewport(x0,y0,x1,y1,clip);
}

static void clear_device (void)

{
	if (card<2) cleardevice();
}

static void draw_rectangle (x1,y1,x2,y2)

{
	put_line(x1,y1,x1,y2);
	put_line(x1,y2,x2,y2);
	put_line(x2,y1,x2,y2);
	put_line(x1,y1,x2,y1);
}

static Bool open_buffer (void)

{
	unsigned i,size;

	size=80u*(low_y[card]+16);
	P_buf=malloc(size);
	if (P_buf==NULL) {
		card=old_card;
		set_color(color[Errr]);
		out_text(100,msg_l[card]+9,"Not enough memory for printing!");
		delay(1500);
		return(false); }

	for (i=0;i<size;i++) P_buf[i]=0;
	P_off=size/2;
	P_x0=0; P_x1=639;
	P_y0=0; P_y1=10000;
	text_ptr=0;
	return (true);
}

static int compare_handler (const void *p1, const void *p2)

{
	struct text *t1 = (struct text*) p1;
	struct text *t2 = (struct text*) p2;

	return (2*((t1->y>t2->y)-(t1->y<t2->y))+(t1->x>t2->x)-(t1->x<t2->x));
}

static void advance_paper (int n)

{
	xprint ("\r",1);
	if (n>0) {
		scan_line+=n;
		xprint("\x1BJ",2);
		xprint((char*)&n,1); }
}

static void move_print_head (int x)

{
	if (x>0) {
		xprint ("\x1B*\x06",3);
		xprint ((char*)&x,2);
		for (;x>=0;x--)
			xprint ("\x0",1); }
}

static void print_top_items (int max_advance)

{
	int advance;

	for (;;) {
		if (text_ptr>=total_text) break;
		advance=text_heap[text_ptr].y-scan_line;
		if (max_advance>=0&&advance>=max_advance) break;
		max_advance-=advance;
		advance_paper(advance);
		move_print_head(text_heap[text_ptr].x);
		if (options.print.pitch==2)
			xprint("\x1Bx1",3);
		else
			xprint("\x1Bx0\x1B!\x08",6);
		if (isupper(text_heap[text_ptr].mess[0]))
			form_to_printer (text_heap[text_ptr].mess);
		else if (islower(text_heap[text_ptr].mess[0])) {
			xprint ("\x1BS\x01",3);
			xprint (text_heap[text_ptr].mess,0);
			xprint ("\x1BT\x1BH",4); }
		else
			xprint (text_heap[text_ptr].mess,0);
		text_ptr++; }
	advance_paper(max_advance);
}

static void print_buffer (void)

{
	char *pe,*po;
	int i;

	if (open_printer()) {
		qsort(text_heap,text_ptr,sizeof(struct text),compare_handler);
		total_text=text_ptr;
		text_ptr=0;
		scan_line=0;
		setcolor(color[Errr]);
		outtextxy(100,msg_l[old_card]+9,"Printing is in progress...");
		pe=P_buf; po=P_buf+P_off;
		xprint ("\x1B@\x1B""5\x00\x1B""3\x16",8);
		for (i=0;i<grf_l[card-2];i++) {
			xprint("\x1B*\x06\x80\x02",5); xprint(pe,640);
			print_top_items(2);
			xprint("\x1B*\x06\x80\x02",5); xprint(po,640);
			print_top_items(22);
			pe+=640; po+=640; }
		print_top_items(-1);
		xprint ("\n",1);
		close_printer(); }
	else {
		card=old_card;
		setcolor(color[Errr]);
		outtextxy(100,msg_l[card]+9,"Error while in printing!");
		delay(1500); }
	free (P_buf);
}

static float get_parm (int pnt)

{
	if (pnt<0) pnt=X->mpnt-1;
	if (ctask.p1==0)
		return (get_tp_1(ctask.t0,ctask.t1,pnt));
	else
		return (get_tp_1(ctask.p0,ctask.p1,pnt));
}

static int conv_x (double x)

{
	if (ctask.logstep)
		return (x0+xa*log(x/xt));
	else
		return (x0+xa*(x-xt));
}

static int conv_y (double y)

{
	if (ylog==0)
		return (y0+ya*(ytlin-y));
	else
		if (y!=1)
			return (y0+ya*log(y));
		else
			return (y0);
}

static void build_x_axis (void)

{
	int i,x,step;
	double tp;
	char buffer[40];

	view_port (0,0,639,low_y[card]+16,1);
	set_color(color[Grid]);
	line_style (DOTTED_LINE);
	draw_rectangle(Wleft,Wtop,Wright,low_y[card]);

	x0=Wleft+1;
	xt=get_parm(0);
	xa=ctask.logstep?
		((Wright-x0)/log(get_parm(-1)/xt)):
		((Wright-x0)/(get_parm(-1)-xt));

	step=X->mpnt/7+1;

	for (i=0;i<X->mpnt;i++) {
		tp=get_parm(i);
		x=conv_x(tp);
		if (i>0&&i<X->mpnt-1) {
			set_color(color[Grid]);
			put_line(x,Wtop,x,low_y[card]+3); }
		if (i%step==0||i==X->mpnt-1) {
			set_color(color[Text]);
			out_text(x-16,low_y[card]+6,general_5_float(tp,buffer)); } }
}

#pragma warn -aus

static void build_y_axis (int low)

{
	int y,i,j,n;
	double f,f0,f1;
	char buffer[40];
	double drainage;		/* It's needed because of bug in Turbo C */

	y0=Wtop+1;
	ylog=low;
	if (low==0) { 			/* Linear scale 0.0 to 1.0 */
		if (ytlin==0) {
			n=4;
			ytlin=0.02;
			for (i=0;i<X->I0&&n>0;i++)
				if (i<X->M[0]||i>=X->M[1])
					for (j=0;j<X->M0;j++)
						while (X->C[i].c2[j].g>ytlin)
							ytlin+=0.02; }
		ya=(low_y[card]-y0-1)/ytlin; }
   else
		ya=(low_y[card]-y0-1)/low/log(10);

	if (low!=0) {
		f0=pow10(low);
		f1=1; }
	else {
		f0=0;
		f1=ytlin; }
	for (f=f0;f<=f1;drainage=(low!=0)?(f=f*10):(f=f+ytlin/5)) {
		y=conv_y(f);
		if (f!=f0&&f!=f1) {
			set_color(color[Grid]);
			put_line(Wleft-3,y,Wright,y); }
		sprintf(buffer,(low!=0)?"%5.1E":"%#5.3f",f);
		set_color(color[Text]);
		out_text(Wleft-51,y-6,buffer); }
}

static void put_label (int x,int y,int label)

{
	if (x!=0) {
		fill_bar(x,y,x+8,y+8);
		set_color(color[Labl]);
		out_text (x,y,(char*)&label); }
}

#pragma warn +aus

static Bool build_graphic (Bool vapor)

{
	int i,j,k,l,ph,yt=0,dyt=box_h[card],label='a',c;
	int x1,x2,y1,y2,xl1,xl2,yl1,yl2;
	float tp1,tp2,n1,n2,f,fm;
	char buffer[50];

	if (!vapor&&X->ixN[0]>=X->M[0]&&ctask.total[2]==0)
		return (true);

	if (vapor) {
		f=options.sigfcy*10;
		if (f<1e-16) f=1e-16;
		while (X->N0>Max_lines) {
			f*=10;
			build_index_N(f); } }

	clear_device();
	build_x_axis ();
	if (vapor)
		build_y_axis(ceil(log10(f)-0.5));
	else
		build_y_axis (0);

	fm=(vapor)?f:options.accuracy;
	k=4;
	view_port(0,8,639,low_y[card]-1,1);

	for (ph=0,i=0;i<X->N0;i++) {
		l=X->ixN[i];
		while(l>=X->M[ph]) ph++;
		if (ph>X->F0) ph-=X->F0+1;  /********/
		if (vapor==(ph==1)) {
			yt+=dyt;
			set_color (color[Labl]);
			out_text(2,yt,(char*)&label);
			print_form_2(X->mask,X->A[l],X->J0,X->C[l].adinf,ph,buffer);
			switch (ph) {
			case 0:
				j=Grf1;
				break;
			case 1:
				j=(ctask.ion&&X->A[l][X->J0]!=0)?Grf2:Grf1;
				break;
			default:
				j=Grf2;
				break; }
			set_color (color[j]);
			out_text(14,yt,buffer);
			xl1=0;
			for (j=0;j<X->M0-1;j++) {
				if (ctask.p1==0) {
					tp1=X->Tx[j];
					tp2=X->Tx[j+1]; }
				else {
					tp1=X->Px[j];
					tp2=X->Px[j+1]; }
				n1=X->C[l].c2[X->ixJ[j]].g; n2=X->C[l].c2[X->ixJ[j+1]].g;
				if (n1>fm&&n2>fm) {
					x1=conv_x(tp1); y1=conv_y(n1)-8;
					x2=conv_x(tp2); y2=conv_y(n2)-8;
					k+=11*i+13*j; k=2+k%3;
					xl2=x2+k*(x1-x2)/5-4; yl2=y2+k*(y1-y2)/5-4;
					if (xl1==0) { xl1=xl2; yl1=yl2; }
					line_style (SOLID_LINE);
					put_line (x1,y1,x2,y2); } }
			put_label (xl1,yl1,label);
			put_label (xl2,yl2,label);
			label++; } }

	if (vapor) build_index_N(options.sigfcy);
	if (card>1) return(true);

	view_port(0,0,639,getmaxy(),1);

	for (;;) {

		fill_bar(0,msg_l[card]+9,639,msg_l[card]+17);
		set_color(color[Hlit]);
		out_text(5,msg_l[card],vapor?"VAPOR display":"SOLID display");
		set_color(color[Text]);
		out_text(125,msg_l[card]," Commands: ESC-exit  SPACE-toggle display  F9-print graphics");

		c=wgetkey(KEYONLY);
		switch(c) {
		case Esc:
			return (false);
		case ' ':
		case Enter:
			return (true);
		case F9:
			old_card=card;
			card=2+!vapor;
			if (open_buffer()) {
				build_graphic(vapor);
				print_buffer(); }
			card=old_card;
			break; } }
}

void grafix_out (XX *Y)

{
	X=Y;
	wsave (1,1,80,25);
	card=video_card!=0; /* 0=CGA, 1=EGA & VGA in EGA mode */
	open_video();
	ytlin=0;
	for (;;) {
		if (!build_graphic(false)) break;
		if (!build_graphic(true)) break; }
	closegraph();
	load_font(video_card);
	wrestore();
}
