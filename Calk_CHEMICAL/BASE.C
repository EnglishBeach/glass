#include <alloc.h>
#include <dos.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
//#include <keys.h>
#define Del 339
#define PgUp 329
#define PgDn 337
#include <stdlib.h>
#include <string.h>
#include "chem.h"
#include "wserv.h"


static int handle;				/* Base file handle	*/
static db_entry entry;			/* Base entry        */
static char buffer[50];			/* Formula buffer		*/
static unsigned char mask[7]; /* Mask for search	*/
static int C_chain;

char perio[] = {			/* Periodical System	*/
 "H HeLiBeB C N O F NeNaMgAlSiP S ClArK CaScTiV CrMnFeCoNiCuZn\
GaGeAsSeBrKrRbSrY ZrNbMoTcRuRhRdArCdInSnSbTeI XeCsBaLaCePrNd\
PmSmEuGdTbDyHoErTmYbLuHfTaW ReOsIrPtAuHgTlPbBiPoAtRnFrRaAcTh\
PaU NpPuAmCmBkCfEsFmMdNoLrKuNs" };

/*----------------------------------------------------------------*/
/*							Database file primitives							*/
/*----------------------------------------------------------------*/

/*
	Generate database file name
*/

static char *base_file (void)

{
	static char buffer[40];
	extern char chem_dir[];

	strcpy (buffer,chem_dir);
	strcat (buffer+strlen(buffer),"CHEMICAL.DBS");
	return (buffer);
}


/*
	Open base file with desired access
*/

int base_open (int access, int Cchn)

{
	handle=open(base_file(),access);
	if (handle<0) {
		tell_ioerr (BASE_TEXT_1,errno);
		return(false); }
	C_chain=(Cchn==0)?options.Cchain:Cchn;
	return (true);
}


/*
	Read one entry from current base file position
*/

static int base_read (db_entry *entry)

{
	#define ratio 12
	static int now=0,i,j,c,ierr;

   ierr = _read(handle,entry,sizeof(db_entry));
	if (ierr<0) {
		tell_ioerr (BASE_TEXT_2,errno);
		return(false); }
   if (ierr<sizeof(db_entry))
   	return(false);
	else {
		if (now++>ratio) {
			now=0;
			wappendbar (tell(handle)); }
		for (i=0;i<6&&entry->el[i]!=0;i++)
			for (j=0;j<5&&entry->el[j+1]!=0;j++)
				if (entry->el[j]>entry->el[j+1]) {
					c=entry->el[j];
					entry->el[j]=entry->el[j+1];
					entry->el[j+1]=c;
					c=entry->stoi[j];
					entry->stoi[j]=entry->stoi[j+1];
					entry->stoi[j+1]=c; }
		return (true); }
}

/*
	Write one entry fron current file position
*/

static int base_write (db_entry *entry)

{
	if (_write(handle,entry,sizeof(db_entry))<0) {
		tell_ioerr (BASE_TEXT_3,errno);
		return(false); }
	return (true);
}


/*
	Immediate write all base file buffers
*/

static int base_flush (void)

{
	int j;

	if ((j=dup(handle))<0||close(j)<0) {
		tell_ioerr (BASE_TEXT_4,errno);
		return(false); }
	return (true);
}

/*
	Step <i> entries forward or back in base file
*/

static int base_set (int i)

{
	if (lseek(handle,tell(handle)+(long)i*sizeof(db_entry),SEEK_SET)<0) {
		tell_ioerr (BASE_TEXT_5,errno);
		return(false); }
	return (true);
}

int base_close (void)

{
	wsetupbar (0,0,0,0);
	if (close(handle)<0) {
		tell_ioerr (BASE_TEXT_6,errno);
		return(false); }
	return (true);
}

long base_size (void)

{
	return (filelength(handle));
}

long base_tell (void)

{
	return (tell(handle));
}

/*
	Add new entry from <entry> to the base
*/

static int base_newcomp (void)

{
	db_entry en2;

	lseek(handle,0l,SEEK_SET);
	entry.flag|=DB_exi;
	do
		if (!base_read(&en2)) return(false);
	while (!(DB_las&en2.flag)&&(DB_exi&en2.flag));
	if (!base_set(-1)) return(false);
	if (DB_exi&en2.flag) {
		en2.flag&=~DB_las;
		if (!base_write(&en2)) return(false);
		entry.flag|=DB_las;
		if (!base_write(&entry)) return(false); }
	else {
		if (DB_las&en2.flag) entry.flag|=DB_las;
		if (!base_write(&entry)) return(false); }
	tell_short (BASE_TEXT_7);
	return (true);
}

/*----------------------------------------------------------------*/
/*								Encoders & decoders								*/
/*----------------------------------------------------------------*/


/*
	Parse symbol of chemical element. Begins parse from s[*t].
	Return sequential number or 0 when fails.
*/

static int parse_sym (char *s,int *t)

{
	int i;

	if(s[*t]=='+') { (*t)++; return (255); }
	if(s[*t]=='-') { (*t)++; return (254); }

	if (isalpha(s[*t])) {
		if (isalpha(s[*t+1]))
			for (i=0;i<sizeof(perio);i+=2)
				if (toupper(s[*t])==toupper(perio[i])&&
					 toupper(s[*t+1])==toupper(perio[i+1])) {
					*t+=2;
					return(i/2+1); }
		for (i=0;i<sizeof(perio);i+=2)
			if (toupper(s[*t])==toupper(perio[i])&&perio[i+1]==' ') {
				(*t)++;
				return(i/2+1); } }
	return (0);
}


/*
	Parse stoichiometrical rate. Return rate or 0 when fails
*/

static int parse_stoi (char *s, int *t)

{
	char c;
	int i,j,f1,f2=0;

	i=*t;
	while (isdigit(s[(*t)++]));
	c=s[--(*t)];
	s[*t]=0;
	f1=atoi(s+i);
	s[*t]=c;
	j=*t+1;
	if (s[*t]=='.') {
		while (isdigit(s[++(*t)]));
		f2=1; }
	if (i==*t) return (128);
	if (f2>0) {
		i=*t;
		while (s[--i]=='0');
		i++;
		if (i-j>2) i=j+2;
		c=s[i];
		s[i]=0;
		f2=atoi(s+j);
		s[i]=c;
		if (i-j<2) f2*=10; }
	if (abs(f1)>256||f2<0||f2>99)
		return (0);
	else
		return ((f1<<7)|f2);
}


/*
	Parse chemical formula from s into entry. Return 0 when
	successful or syntax error offset in s (beginning from 1).
*/

int parse_form (el,stoi,adinf,flag,s)

unsigned char *el,*flag;
char *adinf,*s;
int *stoi;

{
	int i=0;
	int j=0;
	int k,l,st;
	unsigned char e;
	char *p;

	*flag=0;

	if (s[0]=='*') {
		*flag=DB_sol;
		j++; }
	do {
		if ((el[i]=parse_sym(s,&j))==0) return (j+1);
		if ((stoi[i]=parse_stoi(s,&j))==0) return (j+1);
		if (el[i]==254) {
			el[i]=255;
			stoi[i]=-stoi[i]; }
		for (k=0;k<i;k++)
			if (el[i]==el[k]) return (j+1);
		if (++i>6) return (j+1);
	} while (s[j]&&s[j]!='[');
	if (s[j]=='[') {
		strcpy(adinf,s+j+1);
		if (*(p=adinf+strlen(adinf)-1)==']') *p=0; }
	else
		*adinf=0;
	for (k=i;k<6;k++) {
		el[k]=0;
		stoi[k]=0; }
	for (k=0;k<i-1;k++)
		for (l=0;l<i-1;l++)
			if (el[l]>el[l+1]) {
				e=el[l];
				el[l]=el[l+1];
				el[l+1]=e;
				st=stoi[l];
				stoi[l]=stoi[l+1];
				stoi[l+1]=st; }
	return (0);
}


/*
	Parser routine called from winput
*/

static int base_synt (char *s)

{
	return parse_form (entry.el,entry.stoi,entry.adinf,&entry.flag,s);
}


/*
	Parser for element list called from winput
*/

static int mask_check (char *s)

{
	int i,t=0;

	mask[6]=0;
	if (s[0]==0) return(1);
	for (i=0;i<6;i++) {
		if ((mask[i]=parse_sym(s,&t))==0) return (t+1);
		if (mask[i]==254) mask[i]=255;
		if (s[t++]==',') continue;
		if (s[--t]==0) {
			if (i<5) mask[i+1]=0;
			return(0); }
		else return(t+1); }
	return (t+1);
}


/*
	Enter entry from keyboard
*/

static void base_comp (void)

{
	static char *list[] = {
		BASE_TEXT_8,
		BASE_TEXT_9,
		BASE_TEXT_10,
		BASE_TEXT_11,
		BASE_TEXT_12,
		BASE_TEXT_13,
		BASE_TEXT_14,
		BASE_TEXT_15,
		BASE_TEXT_16,
		BASE_TEXT_17,
		BASE_TEXT_18 };

	static char *desc[] = {
		BASE_TEXT_19,
		BASE_TEXT_20,
		BASE_TEXT_21,
		BASE_TEXT_22,
		BASE_TEXT_23,
		BASE_TEXT_24,
		BASE_TEXT_25,
		BASE_TEXT_26,
		BASE_TEXT_27,
		BASE_TEXT_28 };

#define rel 10			/* Relative location the values start from */

		static j=0;
		int i;
		int flg=MF_res;
      char *p;

	if (entry.el[0]==0) {
		entry.el[0]=1;
		entry.el[1]=8;
		entry.stoi[0]=256;
		entry.stoi[1]=128;
		for (i=2;i<6;i++) {
			entry.stoi[i]=0;
			entry.el[i]=0; }
		for (i=0;i<7;i++) entry.f[i]=0;
		entry.h=0;
		entry.t[0]=273.15;
		entry.t[1]=1000;
		entry.flag=0; }
	else
		entry.flag=entry.flag&~DB_las;
	if (j<=0) j=1;
	for (;;) {
		p=list[0]+rel;
		print_form (entry.el,entry.stoi,entry.adinf,entry.flag,p,false,35);
		for (i=0;i<10;i++)
			gcvt (entry.t[i],7,list[i+1]+rel);
		j=wmenu (10,4,45,11,1,list,NULL,18,19,20,21,0,0,
			BASE_TEXT_29,flg,j,32,NULL,NULL);
		if (j==0) break;
		flg|=MF_blk;
		switch (j) {
		case 1:
			print_form (entry.el,entry.stoi,entry.adinf,entry.flag,
				buffer,true,50);
			winput (24,6,BASE_TEXT_30,0,43,22,23,24,buffer,50,base_synt);
			break;
		default:
			do {
				entry.t[j-2]=
					wgfloat(24,j+5,desc[j-2],j+42,22,23,24,entry.t[j-2]);
				if (entry.t[0]>entry.t[1]||entry.t[0]<0)
					tell_error (BASE_TEXT_31,9);
			} while (entry.t[0]>entry.t[1]||entry.t[0]<0);
			break; } }
}

/*----------------------------------------------------------------*/
/*							Database menu services								*/
/*----------------------------------------------------------------*/

Bool base_search (db_entry *entry,Bool mask_or,
						unsigned char *mask,Bool *more)

{
	int j,k,l;
	Bool mask_ok;

	if (!base_read(entry)) {
   	*more = false;
		return (false); }
	if ((DB_las&entry->flag)) *more=false;
	if (!(DB_exi&entry->flag)) return (false);
	if (!mask[0])
		mask_ok=true;
	else {
		mask_ok=!mask_or;
		for (l=0;mask[l];l++);
		for (k=0;k<6&&entry->el[k];k++) {
			if (mask_or)
				mask_ok=mask_ok||memchr(mask,entry->el[k],l)!=NULL;
			else
				mask_ok=mask_ok&&memchr(mask,entry->el[k],l)!=NULL; } }

	for (j=0;mask_ok&&j<6;j++) {
		l=j;
		for (k=j+1;k<6;k++)
			if (entry->el[k]<entry->el[l])
				l=k;
		if (l==j) {
			k=entry->el[l]; entry->el[l]=entry->el[j]; entry->el[j]=k;
			k=entry->stoi[l];entry->stoi[l]=entry->stoi[j];entry->stoi[j]=k; }}

	mask_ok=mask_ok&&(strcmp(entry->el,"\x01\x06")!=0||
					(entry->stoi[1]>>7)<=C_chain);
	return (mask_ok);
}


/*
	Lookup choice
*/

static void base_look (void)

{
	Bool more;					/* True if not a last page							*/
	Bool leave;					/* True if must to refresh window				*/
	Bool first;					/* True if first page								*/
	Bool masked_or;			/* True if OR-masking; else AND					*/
	Bool disa[88];				/* True if no formula must appear on screen	*/
	int flg=MF_cky|MF_pgx;	/* Initial flagword for menu						*/
	int i,k;						/* Misc-use integers									*/
	int j2;
	long pos;					/* Temp storage for file position				*/
	long posit[88];			/* File position for each entry on screen		*/
	char *list[88];			/* List of ASCII formulae							*/
	char heap[1500];			/* Heap of ASCII formulae							*/
	char select[20] = "";	/* Text-form list for masking						*/
	db_entry en2;				/* Intermediate ops entry							*/

	static int j1=1;
	static char *choice[]={BASE_TEXT_32,BASE_TEXT_33,BASE_TEXT_34};

	k=j1;
	masked_or=false;
	k=wmenu (64,14,12,3,1,choice,NULL,18,19,20,21,0,0,
		BASE_TEXT_35,0,k,27,NULL,NULL);
	if (k!=0) j1=k;
	switch (k) {
	case 0:
		return;
	case 1:
		mask[0]=0;
		break;
	case 2:
		masked_or=true;
	case 3:
		if (winput(46,15+k,BASE_TEXT_36,0,54+k,22,23,24,
			select,28,mask_check)==0) {
			wrestore();
			return; } }
	wrestore ();
	first=true;
	if (!base_open(O_RDWR,1000)) return;
	*list=heap;
	for (i=1;i<88;i++)
		list[i]=list[i-1]+17;
	for (;;) {
		more=true;
		leave=false;
		for (i=0;i<88;i++) disa[i]=1;
		for (i=0;i<88;) {
			pos=tell(handle);
			if (base_search(&entry,masked_or,mask,&more)) {
				disa[i]=0;
				posit[i]=pos;
				print_form(entry.el,entry.stoi,entry.adinf,entry.flag,
					list[i++],false,16); }
			if (!more) break; }
		if (i==88&&!more) {
			if (!base_set(-1)) return;
			more=1; }
		if (more) {
			strcpy (list[87],BASE_TEXT_37);
			disa[87]=0; }
		pos=tell(handle);
		if (i==0) {
			if (first)
				if (mask[0])
					tell_short (BASE_TEXT_38);
				else
					tell_short (BASE_TEXT_39);
			else
				tell_short (BASE_TEXT_40);
			if (flg&MF_blk) wrestore();
			return; }
		j2=1;
		for (;;) {
			j2=wmenu (1,1,16,88,4,list,disa,25,26,26,27,28,1,
				BASE_TEXT_41,flg,j2,-31,
				BASE_TEXT_42,NULL);
			if (j2==0) return;
			flg|=MF_blk;
			first=false;
			if ((j2&0xFF80)!=0)
				switch ((j2&0xFF80)>>7) {
				case Del:
					if (wverify(20,20,BASE_TEXT_43,30)>0) {
						lseek (handle,posit[(j2&0x007F)-1],SEEK_SET);
						if (!base_read(&entry)) return;
						entry.flag&=~DB_exi;
						if (!base_set(-1)||!base_write(&entry)||!base_flush())
							return;
						pos=posit[0];
						leave=true; }
					break;
				case PgUp:
					pos=0;
					leave=true;
					first=true;
					break;
				case PgDn:
					leave=more;
					break; }
			else {
				if (more&&j2==88) break;
				lseek(handle,posit[j2-1],SEEK_SET);
				if(!base_read(&entry)) return;
				memcpy(&en2,&entry,sizeof(entry));
				base_comp ();
				if((memcmp(entry.el,en2.el,
					sizeof(entry.el)+sizeof(entry.stoi)+sizeof(entry.adinf))!=0||
					(DB_sol&entry.flag)!=(DB_sol&en2.flag))&&
					wverify(15,20,
					BASE_TEXT_44,55)>0) {
						wrestore();
						if (!base_newcomp()||!base_flush()) return;
						pos=tell(handle)-sizeof(db_entry);
						leave=true; }
				else if(memcmp(entry.t,en2.t,10*sizeof(float))!=0 &&
						wverify(25,20,BASE_TEXT_45,54)>0) {
					wrestore();
					entry.flag|=en2.flag&(DB_las|DB_exi);
					if(!base_set(-1)||!base_write(&entry)||!base_flush())
						return; }
				else wrestore(); }
			j2=-(j2&0x7F);
			if (leave) break; }
		lseek(handle,pos,SEEK_SET); }
}


/*
	Append service
*/

static void base_add (void)

{
	int j;

	if (!base_open(O_RDWR,1000)) return;
	base_comp ();
	j=wverify (22,20,BASE_TEXT_46,58);
	wrestore ();
	if (j<=0) return;

	base_newcomp();
}


/*
	Initialize base file
*/

static void base_init (void)

{
	if (wverify(12,6,BASE_TEXT_47,59)<1) return;
	unlink (base_file());
	handle=open(base_file(),O_CREAT);
	if (handle<0) {
		tell_ioerr (BASE_TEXT_48,errno);
		return; }
	_chmod(base_file(),1,FA_ARCH);
	entry.flag=DB_las;
	if (_write(handle,&entry,sizeof(db_entry))<0) {
		tell_ioerr (BASE_TEXT_49,errno);
		return; }
	_close (handle);
	tell_short (BASE_TEXT_50);
	return;
}

/*----------------------------------------------------------------*/
/*							Routines called elsewhere							*/
/*----------------------------------------------------------------*/


/*
	Main base services menu
*/

void base_service(void)

{
	static int j=1;
	static char *choices[]= { BASE_TEXT_51,BASE_TEXT_52,BASE_TEXT_53 };

	if (j<0) j=-j;
	for (;;) {
		j=wmenu (62,12,13,3,1,choices,NULL,12,13,14,15,0,0,
			BASE_TEXT_54,0,j,101,NULL,NULL);
		if (j==0) return;
		switch (j) {
			case 1:
				base_look();
				close(handle);
				break;
			case 2:
				base_add();
				close(handle);
				break;
			case 3:
				base_init();
				break; }
		j=-j; }
}
