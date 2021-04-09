#define SOLVETYPES 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <keys.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <ctype.h>
#include <dos.h>
#include "chem.h"
#include "wserv.h"

static XX *X;
static int Printer;
static int Page_no, Lines_pg, Line_no, Tab_no, Width;

extern char perio[];

static void build_solution_window (void)

{
	char *p;
	int i;
	int heigth=min(X->N0+X->J0+X->L0+X->F0+1,19)+5;

	wblock (10,1,61,heigth,49,NULL,1);
	wattr (49);
	wfoot (OUTPUT_TEXT_1);
	p=wgeta(31,1);            wputs(&p,"ÑÍÍÍÍÍÍÍÍÍÍÍÍÑÍÍÍÍÍÍÍÍÍÍÍÍÑ");
	p=wgeta(31,heigth); wputs(&p,"ÏÍÍÍÍÍÍÍÍÍÍÍÍÏÍÍÍÍÍÍÍÍÍÍÍÍÏ");
	for (i=2;i<heigth;i++) {
		p=wgeta(31,i);         wputs(&p,"³            ³            ³"); }
	p=wgeta(10,4);
	wputs(&p,"ÇÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ¶");
	p=wgeta(12,2); wputs(&p,OUTPUT_TEXT_2);
	p=wgeta(12,3); wputs(&p,OUTPUT_TEXT_3);
}

static void view_tp (int pnt)

{
	int i,x;
	char *p,buffer[40];

	for (i=0;i<3&&pnt<X->M0;i++,pnt++) {
		x=33+13*i;
		sprintf(buffer,"%10.4g",X->Tx[pnt]); p=wgeta(x,2); wputs(&p,buffer);
		sprintf(buffer,"%10.4g",X->Px[pnt]); p=wgeta(x,3); wputs(&p,buffer); }
}

static void view_row (int i, int y, int l, Bool comp, int ph)

{
	int k,j;
	char *p,buffer[50],buf[5];

	y+=5;
	j=-i-1;
	if (comp) {
		buffer[0]=' ';
		if (i>=0)
			print_form_matr (X->mask,X->A[i],X->J0,X->C[i].adinf,ph,buffer+1);
		else {
			if (j<X->J0) {
				strcpy(buffer+1,"\x0F(");
				strncat(buffer,perio+X->mask[j]*2-2,2);
				strcat(buffer,")"); }
			else if (j<X->J0+X->L0) {
				strcpy(buffer+1,"\x0F(");
				strcat(buffer,itoa(j-X->J0+1,buf,10));
				strcat(buffer,")"); }
			else if (j<X->J0+X->L0+X->F0) {
				if (j==X->J0+X->L0)
					strcpy(buffer+1,"Vapor");
				else {
					strcpy(buffer+1,"Solution #");
					strcat(buffer,itoa(j-X->J0-X->L0,buf,10)); } }
			else
				strcpy(buffer+1,"Convergency"); }
		strcat(buffer,"                    ");
		buffer[20]=0;
		p=wgeta(11,y); wputs (&p,buffer); p+=4; }
	else
		p=wgeta(33,y);

	for (k=0;k<3&&l<X->M0;k++,l++) {
		if (i>=0)
			sprintf (buffer,"%#10.4e",X->C[i].c2[X->ixJ[l]].g);
		else
			if (j<X->J0+X->L0)
				sprintf (buffer,"%#+9.3e",X->Mu[j][X->ixJ[l]]);
			else
				sprintf (buffer,"%#10.4e",X->Mu[j][X->ixJ[l]]);
		wputs(&p,buffer); p+=6; }
}

static void view_table (int io, int jo, Bool comp, Bool tp)

{
	int i,n,y,ph;

	if (tp)
		view_tp(jo);
	for (ph=0,y=0,i=io;i<X->N0+X->J0+X->L0+X->F0+1&&y<19;i++,y++) {
		while(X->ixN[i]>=X->M[ph]) ph++;
		if (ph>X->F0) ph-=X->F0+1;
		n=(i<X->N0)?X->ixN[i]:X->N0-i-1;
		view_row (n,y,jo,comp,ph); }
}

static void view_results (void)

{
	int beg_point=0, beg_row=0;
	Bool tp, changed;

	build_solution_window();
	view_table (0,0,true,true);

	for (;;) {
		changed=false;
		tp=false;
		switch (wgetkey(ALL)) {
		case F1:
			whelp(98);
			break;
		case Esc:
			wrestore();
			return;
		case Up:
			if (beg_row>0) {
				changed=true;
				beg_row--; }
			break;
		case Down:
			if (beg_row<X->N0+X->J0+X->L0+X->F0+1-19) {
				changed=true;
				beg_row++; }
			break;
		case Left:
			if (beg_point>0) {
				changed=true;
				beg_point--;
				tp=true; }
			break;
		case Right:
			if (beg_point<X->M0-3) {
				changed=true;
				beg_point++;
				tp=true; }
			break; }

		if (changed)
			view_table (beg_row,beg_point,!tp,tp); }
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/

Bool open_printer (void)

{
	union REGS reg;

	Printer=open(options.prt_file,O_CREAT|O_BINARY,S_IWRITE);
	if (Printer>0) {
		reg.x.ax=0x4400;
		reg.x.bx=Printer;
		int86(0x21,&reg,&reg);
		if (reg.x.dx&0x80) {
			reg.x.ax=0x4401;
			reg.x.bx=Printer;
			reg.h.dl|=0x20;
			reg.h.dh=0;
			int86(0x21,&reg,&reg); } }
	return (Printer>0);
}

void close_printer (void)

{
	close (Printer);
}

void xprint (char *s,unsigned len)

{
	union REGS reg;

	if (len==0) len=strlen(s);
		reg.x.ax=0x4400;
		reg.x.bx=Printer;
		int86(0x21,&reg,&reg);
	if (len>0) _write (Printer,s,len);
}

static void init_printer (void)

{
	static char *s_init = "\x1B@\x1BO\x1B~1";
	static char *s_pitch[3] = { "\x1B!\x08", "\x1B!\x09",
										 "\x1Bx1" };
	static char *s_wait[2] = { "","\x1B" "8" };
	static char *s_space[2] = { "\x1B" "2", "\x1B" "0" };
	static char *s_margin[2][2] = {  { "\x1BX\x00\x50", "\x1BX\x00\x88" },
												{ "\x1BX\x00\x60", "\x1BX\x00\xA3" } };
	int pwidth[2][2] = { { 70,135 },{ 85,162 } };



	xprint (s_init,0);
	xprint (s_pitch[options.print.pitch],0);
	xprint (s_wait[options.print.wait],0);
	xprint (s_space[options.print.space],0);
	xprint (s_margin[options.print.pitch&1][options.print.width],4);

	Lines_pg=(10+options.print.heigth)*(6+2*options.print.space);
	Width=pwidth[options.print.pitch&1][options.print.width]-1;
}

static void page_header (void)

{
	char buffer[5];
	int j;

	xprint("É",1);for(j=0;j<Width-10;j++)xprint ("Í",1);xprint("ÑÍÍÍÍÍÍÍÍÍ»\n",0);
	xprint("º",1);for(j=0;j<Width-10;j++)xprint (" ",1);xprint("³ Page ",0);
		itoa(Page_no++,buffer,10); if (buffer[1]==0) strcpy(buffer+1," ");
		xprint(buffer,2); xprint(" º\n",3);
	xprint("º ",2); xprint(ctask.desc,0);
		for (j=0;j<Width-strlen(ctask.desc)-11;j++) xprint(" ",1);
      xprint("ÀÄÄÄÄÄÄÄÄÄ¶\n",0);
	xprint("º",1);for(j=0;j<Width;j++)xprint (" ",1);xprint("º\n",2);

	Line_no=4;
}

static void break_page (void)

{
	xprint("\x0C",1);
	if (options.print.wait) {
		wbell();
		wbell();
		tell_short(OUTPUT_TEXT_4); }
	page_header();
}

void form_to_printer (char *s)

{
	int index=1;
	char cansub[] = "\x1BT\x1BH";

	for (;;) {
		while(isalpha(*s)||*s=='*') xprint(s++,1);
		if(*s=='(') {
			do
				xprint(s++,1);
			while(*s!=')');
			xprint(s++,1); }
		if (*s==0||*s=='['||*s==':') break;
		if (*s=='+'||*s=='-') index=0;
		xprint("\x1BS",2); xprint((char*)&index,1);
		while(isdigit(*s)||*s=='.'||*s=='+'||*s=='-') {
			xprint(s++,1);
			if (*s=='+'||*s=='-') {
				index=0;
				xprint("\x1BS",2); xprint((char*)&index,1); } }
		xprint(cansub,0); }
	if (*s!=0) xprint(s,0);
	xprint("\t",1);
}

static void prt_list (void)

{
	char buffer[50];
	int i,j;

	j=0;
	for (i=0;i<X->M[1];i++) {
		if (i!=0&&j==0) xprint ("º\t\t",3);
		print_form_2 (X->mask,X->A[i],X->J0,X->C[i].adinf,i>=X->M[0],buffer);
		form_to_printer(buffer);
		if (++j>=Tab_no) {
			xprint("º\n",2);
			Line_no++;
			j=0; } }
	if (j>0) {
		for (;j<=Tab_no;j++) xprint("\t",1);
		xprint("º\n",2);
		Line_no++; }
}

static void prt_ice (ice *p, int n, int group)

{
	char buffer[50],buf2[10];
	Bool amos[5] = { true, false, false, true, true };
	int i,j;

	j=0;
	for (i=0;i<n;i++) {
		if (i!=0&&j==0) xprint ("º\t\t",3);
		print_form_gen(p[i].el,p[i].stoi,p[i].adinf,p[i].flag,buffer,0,100);
		if (amos[group]) {
			strcat (buffer,":");
			strcat (buffer,gcvt(p[i].amo,4,buf2)); }
		form_to_printer(buffer);
		if (++j>=Tab_no) {
			xprint("º\n",2);
			Line_no++;
			j=0; } }
	if (j>0) {
		for (;j<=Tab_no;j++) xprint("\t",1);
		xprint("º\n",2);
		Line_no++; }
}

static void prt_cor (cornice *p,int group)

{
	int i;
	char buffer[40];

	for (i=0;i<ctask.total[group];i++) {
		if (i!=0) xprint("º\t",2);
		xprint (itoa(i+1,buffer,10),0); xprint(":\t",2);
		if (group==4) {
			xprint(gcvt(p->right[i],5,buffer),0); xprint("\t",1); }
		prt_ice(p->p[i],p->n[i],group); }
}

static void print_header (void)

{
	int j;
	char *tabs[2][2] = { {
		"\x1B""D\x10\x14\x24\x34\x46",							  /* 10 cpi, 8"  */
		"\x1B""D\x10\x14\x24\x34\x44\x54\x64\x74\x87" },{	  /* 10 cpi, 13" */
		"\x1B""D\x10\x14\x24\x34\x44\x55",						  /* 12 cpi, 8"  */
		"\x1B""D\x10\x14\x25\x36\x47\x58\x69\x7A\x8B\xA2" }};/* 12 cpi, 13" */
	char *s;

	page_header();

	s=tabs[options.print.pitch&1][options.print.width];
	Tab_no=strlen(s)-4;
	xprint(s,0);

	xprint ("\rº Initial mix:\t\t",0);
		prt_ice(ctask.source,ctask.total[0],0);
	if (ctask.total[1]!=0) {
		xprint ("\rº Excludden  :\t\t",0);
			prt_ice(ctask.exclud,ctask.total[1],1); }
	if (ctask.total[2]!=0) {
		xprint ("\rº Solutions  :\t",0); prt_cor(&ctask.solut,2); }
	if (ctask.total[3]!=0) {
		xprint ("\rº Fixed      :\t\t",0);
			prt_ice(ctask.fixed,ctask.total[3],3); }
	if (ctask.total[4]!=0) {
		xprint ("\rº Constraints:\t",0); prt_cor(&ctask.cstrain,4); }
	xprint("º",1); for (j=0;j<Width;j++) xprint (" ",1); xprint("º\n",2);
	if (options.print.list) {
		xprint ("\rº Retrieved  :\t\t",0); prt_list();
		xprint("º",1); for (j=0;j<Width;j++) xprint (" ",1); xprint("º\n",2);}

	Line_no++;

	if (X->N0>Lines_pg-Line_no-7) {
		xprint("È",1);for(j=0;j<Width;j++)xprint ("Í",1);xprint("¼\n",2);
		break_page(); }

}

static void end_up_line (void)

{
	xprint ("\tº\n",3);
	Line_no++;
}

static void break_line (char *pse, int n)

{
	int i,j,k;

	xprint (pse,1);
	for (i=0;i<20;i++) xprint(pse+1,1);
	j=20;
	for (k=0;k<n;k++) {
		xprint(pse+2,1);
		for (i=0;i<12;i++) xprint (pse+1,1);
		j+=13; }
	for (;j<Width;j++) xprint (pse+1,1);
	xprint(pse+3,1); xprint("\n",1);
	Line_no++;
}

static void print_table (void)

{
	Bool just_broken=true;
	int p_remain=X->M0;
	int p_max,max_p[2][2]= { { 3,8 },{ 4,10 } };
	int i,j,k,l,t,pts,ph,headln=0;
	char buffer[50];
	char *head_line[2] = { "ÌÍÑ¹", "ÉÍÑ»" };

	p_max=max_p[options.print.pitch&1][options.print.width];
	xprint ("\x1B~0\x1B""D\x15",6); t=Width; t++; xprint((char*)&t,2);

	for (j=0;j<X->M0;j+=p_max) {
		pts=min(p_max,p_remain);
		p_remain-=p_max;

		if (X->N0>Lines_pg-Line_no-7) {
			break_page();
			headln=0; }

		for (ph=0,i=0;i<(options.print.all?X->I0:X->N0);i++) {
			l=options.print.all?i:X->ixN[i];
			if (just_broken) {
				just_broken=false;
				break_line (head_line[headln],pts);
				xprint("º Temperature T,K\t",0);
					for (k=0;k<pts;k++) {
						sprintf(buffer,"³ %10.4g ",X->Tx[k+j]); xprint(buffer,0);}
					end_up_line();
				xprint("º Pressure    P,at\t",0);
					for (k=0;k<pts;k++) {
						sprintf(buffer,"³ %10.4g ",X->Px[k+j]); xprint(buffer,0);}
					end_up_line();
				break_line ("ÇÄÅ¶",pts); }

			while(l>=X->M[ph]) ph++;
			if (ph>X->F0) ph-=X->F0+1;
			xprint("º ",2);
			print_form_2 (X->mask,X->A[l],X->J0,X->C[l].adinf,ph,buffer);
			form_to_printer(buffer);
			for (k=0;k<pts;k++) {
				sprintf (buffer,"³ %#10.5e ",X->C[l].c2[j+k].g);
				xprint (buffer,0); }
			end_up_line();
			if (Line_no==Lines_pg-2) {
				break_line("ÈÍÏ¼",pts);
				if (i!=X->N0-1) {
					break_page();
					headln=0;
					just_broken=true; } } }
		break_line("ÈÍÏ¼",pts);
		headln=1;
		just_broken=true; }

	xprint("\x0C",1);
}

static void print_solution (void)

{
	Page_no=1;
	init_printer();
	print_header();
	print_table();
}


static void print_results (void)

{
	static char *choices[] = {
		OUTPUT_TEXT_5,
		OUTPUT_TEXT_6 };

	int j=1;

	for (;;) {
		j=wmenu (60,5,15,2,1,choices,NULL,18,19,20,21,0,0,
			OUTPUT_TEXT_7,0,j,126,NULL,NULL);

		if (j==0) return;

		switch (j) {
		case 1:
			if (open_printer()) {
				tell_wait (OUTPUT_TEXT_8);
				print_solution();
				close_printer();
				wrestore(); }
			else
				tell_ioerr (OUTPUT_TEXT_9,errno);
			wrestore();
			return;
		case 2:
			print_options();
			break; }
		j=-j; }
}

static void grafix_shop (void)

{
	if (X->mpnt==1)
		tell_error (OUTPUT_TEXT_10,133);
	else
		grafix_out(X);
}

void output_results (XX *Y)

{
	static char *choices[] = {
		OUTPUT_TEXT_11,
		OUTPUT_TEXT_12,
		OUTPUT_TEXT_13,
		OUTPUT_TEXT_14,
		OUTPUT_TEXT_15 };

	int j=1;

	X=Y;

	for (;;) {
		j=wmenu (64,2,12,5,1,choices,NULL,50,51,48,52,0,0,OUTPUT_TEXT_16,
					MF_esc,j,88,NULL,NULL);

		switch (j) {
			case 1: view_results (); break;
			case 2: print_results (); break;
			case 3: grafix_shop (); break;
			case 4:
				options.sigfcy=wgfloat(45,8,OUTPUT_TEXT_17,
					123,22,23,24,options.sigfcy);
				build_index_N (options.sigfcy);
				break;
			case 5:
				if (wverify(61,8,OUTPUT_TEXT_18,132)>0) {
					wrestore();
					return; } }
		j=-j; }
}

