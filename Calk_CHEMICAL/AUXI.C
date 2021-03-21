#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <keys.h>
#define Esc 27
#define F1 315
#include <math.h>
#include <dos.h>
#include "chem.h"
#include "wserv.h"

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Auxillary routines used in many different points of CHEMICAL.   บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

extern char perio[];

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Print convientional float number.                               บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void print_float (buf,f,n,ones)

char **buf;
int f;
int n;
Bool ones;
{

	char *q=*buf;
	int i,j;

	if (f<0) {
		f=-f;
		*q++='-'; }
	if (ones||abs(f)!=128) {
		itoa(f>>7,q,10);

		q+=strlen(q);
		if (n>0&&(f&0x007F)>0) {
			*q++='.';
			j=1;
			for (i=0;i<n-1;i++) j*=10;
			while ((f&0x007F)<j&&j>1) {
				*q++='0';
				j/=10; }
			itoa(f&0x007F,q,10);
			q+=strlen(q);
			while (*--q=='0');
			q++; } }
   *buf=q;
	return;
}

void print_form_gen (el,stoi,adinf,flag,p,ones,len)
int *stoi,len;
char *adinf;
unsigned char *el,flag;
Bool ones;
char *p;

{
	int i,f;
	char buf[120];
	char *q=buf;

	if (DB_sol&flag) *q++='*';
	for (i=0;i<6;i++) {
		if (el[i]==0) break;
		f=stoi[i];
		if (el[i]!=255) {
			*q++=perio[2*el[i]-2];
			*q=perio[2*el[i]-1];
			if (*q!=' ') q++; }
		else
			if (f>0) *q++='+';
		print_float (&q,f,2,ones); }
	if (*adinf!=0) {
		*q++='[';
		strcpy (q,adinf);
		q+=strlen(adinf);
		*q++=']'; }
	*q=0;
	if (strlen(buf)>len) {
		buf[len-1]=0x07;
		buf[len]=0; }
   strcpy (p,buf);
}


/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Print  out  the formula of a compound into buffer. When current บ
ณ video card is not CGA then upper  and  lower  chemical  indices บ
ณ recoded  according  to font loaded into card hardware character บ
ณ generator.                                                      บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

char *print_form (el,stoi,adinf,flag,p,ones,len)
int *stoi,len;
char *adinf;
unsigned char *el,flag;
Bool ones;
char *p;

{
	int corr=192;
	char *q;

	print_form_gen (el,stoi,adinf,flag,p,ones,len);

	if (video_card!=0&&!ones) {
		q=p;
		while (*q!=0&&*q!='[') {
			if (*q=='(') corr=0;
			if (*q==')') corr=192;
			if (*q=='+') {
				*q='\x0E';
				corr=-32; }
			else if (*q=='-') {
				*q='\x0D';
				corr=-32; }
			if (isdigit(*q)) *q+=corr;
			q++; } }
	return(p);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Print  formula  from  string  of  stoi  mattrix.  Used  by  two บ
ณ following routines.                                             บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void print_form_2 (char *mask,float *a,int j0,char *adinf,
							int phase,char *buffer)

{
	int i;
	char *q=buffer;

	if (phase==0)
		*q++='*';
	else if (phase!=1) {
		*q++='(';
		itoa(phase-1,q,10);
		q+=strlen(q);
		*q++=')'; }

	for (i=0;i<j0;i++)
		if (a[i]!=0) {
			*q++=perio[2*mask[i]-2];
			*q=perio[2*mask[i]-1];
			if (*q!=' ') q++;
			if (a[i]!=1.0) {
				gcvt(a[i],2,q);
				q+=strlen(q); } }
	if (ctask.ion&&a[j0]!=0) {
		if (a[j0]>0) *q++='+';
		if (fabs(a[j0])!=1.0) {
			gcvt(a[j0],2,q);
			q+=strlen(q); }
		else if (a[j0]<0)
			*q++='-'; }
	if (*adinf!=0) {
		*q++='[';
		strcpy (q,adinf);
		q+=strlen(adinf);
		*q++=']'; }
	*q=0;
	if (strcmp(buffer,"-")==0)
		strcpy(buffer,"E.gas-");
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Print   formula   into  buffer  according  current  video  card บ
ณ character set (see comments to print_form)                      บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void print_form_matr (char *mask,float *a,int j0,char *adinf,
										int phase,char *buffer)

{
	int corr=192;
	char *q;

	print_form_2 (mask,a,j0,adinf,phase,buffer);

	if (video_card!=0) {
		q=buffer;
		while (*q!=0&&*q!='[') {
			if (*q=='(') corr=0;
			if (*q==')') corr=192;
			if (*q=='+') {
				*q='\x0E';
				corr=-32; }
			else if (*q=='-') {
				*q='\x0D';
				corr=-32; }
			if (isdigit(*q)) *q+=corr;
			q++; } }
}



/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Convientional free routine. Frees memory only if pointer passed บ
ณ non-NULL; NULLs pointer after freeing.                          บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void conv_free (void **p)

{
	if (*p!=NULL) {
		free(*p);
		*p=NULL; }
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Convert double to string no more than 5 charactes long.         บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

char *general_5_float (double val,char *p)

{
	char *q,*r;

	if (val<0.09) {
		sprintf (p,"%.2e",val);
		q=p; r=p;
		do {
			if (*r=='e') r++;
			if (*r=='-') { *q++=*r++; r++; }
			*q++=*r++; }
		while (*r);
		*q=0; }
	else {
		sprintf (p,"%.5f",val);
		q=p+5;
		if (*q=='.')
			*q=0;
		else {
			*q--=0;
			while (*q=='0') *q--=0;
			if (*q=='.') *q=0; } }
	return (p);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Put box with error message.                                     บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void tell_ioerr(char *s, int n)

{
	static char *errors[] = {
		AUXI_TEXT_1,
		AUXI_TEXT_2,
		AUXI_TEXT_3,
		AUXI_TEXT_4,
		AUXI_TEXT_5,
		AUXI_TEXT_6,
		AUXI_TEXT_7,
		AUXI_TEXT_8,
		AUXI_TEXT_9,
		AUXI_TEXT_10,
		AUXI_TEXT_11,
		AUXI_TEXT_12,
		AUXI_TEXT_13,
		AUXI_TEXT_14,
		"",
		AUXI_TEXT_15,
		AUXI_TEXT_16,
		AUXI_TEXT_17,
		AUXI_TEXT_18,
		AUXI_TEXT_19,
		AUXI_TEXT_20,
		AUXI_TEXT_21,
		AUXI_TEXT_22,
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		AUXI_TEXT_23,
		AUXI_TEXT_24,
		AUXI_TEXT_25 };

	int len;
	char *p;

	len = strlen(s)+strlen(errors[n])+20;
	wblock (2,4,len,3,8,AUXI_TEXT_26,1);
	wfoot(AUXI_TEXT_27);
	wattr (9);
	p=wgeta (4,5);
	wputs (&p,s);
	wputs (&p," : ");
	wputs (&p,errors[n]);
	wputs (&p,AUXI_TEXT_28);

	for (;;)
		switch (wgetkey(KEYONLY)) {
			case Esc:
				wrestore();
				return;
			case F1:
				whelp (77);
				break; }
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Put box with an error message.                                  บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void tell_error (char *s, int help)

{
	int len;
	char *p;

	len = strlen(s)+17;
	wblock (2,4,len,3,8,AUXI_TEXT_29,1);
	wfoot(AUXI_TEXT_30);
	wattr (9);
	p=wgeta (4,5);
	wputs (&p,s);
	wputs (&p,AUXI_TEXT_31);

	for (;;)
		switch (wgetkey(KEYONLY)) {
			case Esc:
				wrestore();
				return;
			case F1:
				whelp (help); }
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Tell that there's not enough memory.                            บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void tell_nomem (void)

{
	tell_error (AUXI_TEXT_32,78);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Tell non-erratic message.                                       บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void tell_short (char *s)

{
	int len;
	char *p;

	len = strlen(s)+4;
	wblock (15,7,len,3,29,NULL,0);
	wfoot("");
	wattr (30);
	p=wgeta (17,8);
	wputs (&p,s);
	wgetkey(KEYONLY);
	wrestore();
	return;
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Tell non-erratic message and don't remove its box from screen.  บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void tell_wait (char *s)

{
	int len;
	char *p;

	len = strlen(s)+14;
	wblock (15,7,len,3,29,NULL,0);
	wfoot(AUXI_TEXT_33);
	wattr (30);
	p=wgeta (17,8);
	wputs (&p,s);
	wputs (&p,AUXI_TEXT_34);
	return;
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Exit when not enough memory for volatile arrays.                บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void exit_nomem (void)

{
	tell_error (AUXI_TEXT_35,99);
	exit (4);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Exit  when internal error is detected and CHEMICAL is unable to บ
ณ continue.                                                       บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void exit_interr (void)

{
	tell_error (AUXI_TEXT_36,99);
	whelp (99);
	exit (4);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Delete cell <j> from array. Array is pointed by <*p> and have n บ
ณ celss <size> bytes each.                                        บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

Bool del_cell (void **p,int n,int j,int size)

{
	int i;
	char *q;

	if (n>1) {
		q=*p;
		for (i=j;i<n-1;i++)
			memcpy (q+i*size,q+(i+1)*size,size);
		q=realloc(*p,(n-1)*size);
		if (q==NULL) return (false);
		*p=q; }
	return (true);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Insert cell into array. Cell is inserted before j'th unit.      บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

Bool ins_cell (void **p,int n,int j,int size)

{
	int i;
	char *q;

	q=realloc(*p,(n+1)*size);
	if (q==NULL) return (false);
	if (n>0)
		for (i=n-1;i>=j;i--)
			memcpy (q+(i+1)*size,q+i*size,size);
	*p=q;
	return (true);
}

static int break_state;

int ignore_break (void)

{
	return(1);
}

void save_break_state (void)

{
	break_state=getcbrk();
	setcbrk(1);
}

void restore_break_state (void)

{
	setcbrk(break_state);
}

void check_for_break (void)

{
	union REGS reg;

	reg.h.ah=0x0B;
	int86(0x21,&reg,&reg);
}

void leave_Chemical (void)

{
	video_mode (3);
	restore_break_state();
	exit(0);
}

