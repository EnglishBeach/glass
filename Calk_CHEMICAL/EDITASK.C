#include <stdlib.h>
#include <string.h>
//#include <keys.h>
#define Enter 13
#define Ins   338
#define Del   339
#include <alloc.h>
#include "chem.h"
#include "wserv.h"

#define onEnter if (key==Enter)
#define Mfatal(x) result=/* if (!(x)) fatal_nomem() */ x
#define arg(x,y) (jarg[y]=(x),(char*)&jarg[y])

/* Enumerated type for indexing form_control array */
enum { AMO, COND, ADINF, ION };

static float fi0,fi1;
static int nstep;
static lines[5] = { 0,0,0,0,0 };
static ice *the_ice;
static Bool result;
static int jarg[5];
static int current_group;

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ The following array controls a format of formulae in each group.   บ
ณ First line enables amount field after formula (true if enabled),   บ
ณ second enables condensed phase flag, third allows additional info  บ
ณ to appear after formula, fourth maintans ability of presence the   บ
ณ ionizaton flags (+ or -) in formula.                               บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool form_controls [][5] = {
 { true,  false, false, true,  true  },			/* [AMO]   */
 { false, true,  true,  true,  true  },			/* [COND]  */
 { false, true,  true,  true,  true  },			/* [ADINF] */
 { false, true,  false, true,  true  } };			/* [ION]   */

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Parse routine called from winput. Parses input for pressure and    บ
ณ temperature. Returns first and (if is) last points in fi0 and fi1  บ
ณ and number of steps nstep.                                         บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static int tp_check (char *s)

{
	char *p=s;
	char *q=s;
	char c;
	int err;
	int pos=1;

	fi0=0; fi1=0; nstep=-1;

	while (*q!=0&&*q!=',') q++;
	c=*q; *q=0;
	if ((err=__float(p))!=0||(fi0=wgetlast()->f)<=0) return (pos+err);
	if (c==0) return (0);
	pos+=strlen(p); *q++=c; p=q;
	while (*q!=0&&*q!=',') q++;
	c=*q; *q=0;
	if ((err=__integ(p))!=0||(nstep=wgetlast()->i)<0) return (pos+err);
	if (c==0)
		if (nstep==0)
			return (0);
		else
			return (pos+strlen(p)+2);
	pos+=strlen(p); *q++=c; p=q;
	if ((err=__float(p))!=0||(fi1=wgetlast()->f)<=0) return (pos+err);
	return (0);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ The following routine enters new parameters (see above one) for    บ
ณ temperature or pressure. It displays an input box with title <s>   บ
ณ and eliminates it before return.                                   บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static void input_tp(int y,char *s, float *f0, float *f1, float *g1, int help)

{
	char buf[36],*p=buf;
	float f;

	gcvt(*f0,5,p);
	if (*f0>0) {
		if (ctask.nstep!=0&&*f1!=0) {
			p+=strlen(p);
			*p++=',';
			itoa(ctask.nstep,p,10);
			p+=strlen(p);
			*p++=',';
			gcvt(*f1,5,p); } }
	else
		*p=0;
	if (winput (10,y,s,0,help,22,23,24,buf,35,tp_check)<=0) return;
	if (fi1>0&&fi0>fi1) { f=fi0; fi0=fi1; fi1=f; }
	*f0=fi0;
	if (fi1>0) {
		*f1=fi1;
		*g1=0; }
	else
		nstep=0;
	if (nstep<=1)
		*f1=0;
	else
		ctask.nstep=nstep;
	store_temp();
	store_pres();
	store_step();
	refresh_screen (0);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Store line in format <startpoint> [ -> <npoints> -> <endpoint> ]   บ
ณ into edit buffer from current position                             บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static void store_tp (float f1,float f2)

{

	char buf[30],buf2[30];

	store_continued(42,gcvt(f1,4,buf),NULL);
	if (ctask.nstep>1&&f2!=0) {
		store_continued(42," \x1A ",itoa(ctask.nstep,buf,10)," \x1A ",
			gcvt(f2,4,buf2),NULL); }
	store_clear (42);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ The following 5 routines store temperature, pressure, scale type   บ
ณ (logarythmical or linear) and usage of ion forms (do or don't)     บ
ณ into edit buffer in position where they must be found.             บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void store_temp (void)

{
	store_initial (0,4,42,EDITASK_TEXT_1,NULL);
	store_tp (ctask.t0,ctask.t1);
}

void store_pres (void)

{
	store_initial(0,5,42,EDITASK_TEXT_2,NULL);
	store_tp (ctask.p0,ctask.p1);
}

void store_ion (void)

{
	store_initial (49,5,22,EDITASK_TEXT_3,
		ctask.ion?EDITASK_TEXT_4:EDITASK_TEXT_5,NULL);
}

void store_step (void)

{
	if (ctask.nstep!=0) {
		store_initial (49,4,22,EDITASK_TEXT_6,
			ctask.logstep?EDITASK_TEXT_7:EDITASK_TEXT_8,NULL); }
	else
		store_initial (49,4,22,"\x02",NULL);
}

static void store_extype (void)

{
	store_initial(EDITASK_INT_1,9+lines[0],1000,
		ctask.incex?EDITASK_TEXT_9:EDITASK_TEXT_10,NULL);
}

static void store_desc (void)

{
	store_initial(6,1,58,ctask.desc,NULL);
	store_clear(58);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Adjust (ie, insert needed or delete excess) amount of lines in     บ
ณ edit buffer for given <group> to <new_lines>.                      บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static int adjust_lines (int group,int new_lines)

{
	int i,first_line;

	for (first_line=8,i=0;i<group;i++)
		first_line+=lines[i]+2;
	do {
		if (new_lines>lines[group]) {insert_row(first_line); lines[group]++;}
		if (new_lines<lines[group]) {delete_row(first_line); lines[group]--;}
	} while (new_lines!=lines[group]);
	return (first_line);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Store array of formulae into edit buffer. The parameters are:      บ
ณ   p      - pointer to array                                        บ
ณ   group  - group number (0..4)                                     บ
ณ   y      - starting line number (from 0)                           บ
ณ   total  - size of array to store                                  บ
ณ   seq    - sequence code of the array. Used when the array is a    บ
ณ            part of group                                           บ
ณ   len    - maximal length of the formula                           บ
ณ   n      - number of columns which to place formulae in            บ
ณ   alone  - true if not part of group                               บ
ณ   skip_  - true if must leave unchanged first position in first    บ
ณ            line (used in storing linear constrains where that      บ
ณ            position is intended for right hand)                    บ
ณ                                                                    บ
ณ  Returns number of line just following last filled one             บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static int store_ice (ice *p,int group,int y,int total,int seq,int len,int n,
						Bool alone,Bool skip_first)

{
	int i,x,np;
	char buffer[30];

	if (alone) {
		total=ctask.total[group];
		y=adjust_lines (group,ctask.total[group]/n+(ctask.total[group]%n!=0)); }
	x=alone?0:(skip_first?len+8:8);
	np=alone?0:(skip_first?1:0);
	for (i=0;i<total;i++) {
		store_initial (x,y,len-1,"\x01",arg((seq<<4)|i+1,0),arg(len-5,1),
			arg(group+5,2),print_form(p[i].el,p[i].stoi,p[i].adinf,
			p[i].flag,buffer,false,len-5),NULL);
		if (form_controls[AMO][group])
			store_continued(len-1,":",gcvt(p[i].amo,3,buffer),NULL);
		store_clear (len);
		x+=len;
		if (++np>=n||i==total-1) store_continued (len,"\x02",NULL);
		if (np>=n) {
			y++;
			np=0;
			if (alone)
				x=0;
			else {
				store_initial (0,y,1000,"\x03\x03\x03\x03    ",NULL);
				x=8; } } }
	display_core();
	return (y+(np!=0));
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Store group number <group>. Also displays amount of free core.     บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static void store_group (int group)

{
	int y,new_lines,i,j;
	char buffer[30];
	cornice *src[3] = { &ctask.solut, NULL, &ctask.cstrain };


	for (new_lines=0,i=0;i<ctask.total[group];i++)
		new_lines+=(j=src[group-2]->n[i]+(group==4))/4+(j%4!=0);
	y=adjust_lines(group,new_lines);
	for (i=0;i<ctask.total[group];i++) {
		store_initial (0,y,10,"\x01",arg((i<<4)|0x0F,0),arg(i<9?2:3,1),
				arg(group+5,2),itoa(i+1,buffer,10),(i<9)?":  ":": ",NULL);
		if (group==4)
			store_initial (8,y,19,"\x01",arg((i<<4)|0x0D,0),"\x0E",
				arg(group+5,1),gcvt(src[group-2]->right[i],5,buffer),NULL);
			store_clear (19);
		y=store_ice (src[group-2]->p[i],group,y,src[group-2]->n[i],i,
				19,4,false,group==4); }
	display_core();
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Parser called from winput when entering formula. Performs parse    บ
ณ and syntactical check depending on form_controls array (see)       บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static int form_check (char *s)

{
	unsigned char *p;
	int j;

	p=NULL;
	if (form_controls[AMO][current_group]) {
		if ((p=strchr(s,':'))==NULL) return (strlen(s)+1);
		*p=0; }
	j=parse_form(the_ice->el,the_ice->stoi,the_ice->adinf,&the_ice->flag,s);
	if (p!=NULL) {
		*p++=':';
		if (__float(p)!=0) return (strlen(s)+1);
		the_ice->amo=wgetlast()->f; }
	if (j|=0) return (j);
	if (!form_controls[ION][current_group]&&strchr(the_ice->el,255)!=NULL)
		return(strcspn(s,"+-")+1);
	if (!form_controls[COND][current_group]&&(the_ice->flag!=0)) return(1);
	if (!form_controls[ADINF][current_group]&&(*the_ice->adinf!=0))
		return(strcspn(s,"[")+2);
	if (current_group==2) the_ice->flag|=DB_sol;  /* For solid sol only */
	return(0);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Ask user to enter formula and possibly amount (thru semicolon).    บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool edit_form (ice *entry,int group)

{
	char buffer[50],*p=buffer;
	int help;

	the_ice=entry;
	current_group=group;
	p+=strlen(print_form(entry->el,entry->stoi,entry->adinf,entry->flag,
					p,true,25));
	if (form_controls[AMO][group]) {
		*p++=':';
		gcvt (entry->amo,4,p); }
	help=form_controls[AMO][group]?73:74;
	return (winput(18,8,EDITASK_TEXT_11,0,help,22,23,24,buffer,25,form_check));
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                    บ
ณ Ask user to enter right hand of constrain. Returns 0, if input is  บ
ณ abandoned by pressing escape, 1 if right hand is entered and may   บ
ณ be accessed via wgetlast()->f, 2 if no right hand applies to this  บ
ณ group.                                                             บ
ณ                                                                    บ
ิออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static int right_hand(int group, double init)

{
	char buffer[30];

	gcvt (init,5,buffer);
	return ((group!=2)?2:
		winput (20,10,EDITASK_TEXT_12,0,76,22,23,24,buffer,25,__float)!=0);
}

static Bool insert_ice (ice **p,int nmax,int i,int *total,void rout(int),int group,
				char *err,int help)

{
	ice temp = { { 1,8,0,0,0,0 },{ 256,128,0,0,0,0 },0,"",10.0 };

	memset(temp.adinf,0,sizeof(temp.adinf));
	if (*total<nmax) {
		if (edit_form(&temp,group)) {
			Mfatal (ins_cell ((void **)p,(*total)++,i,sizeof(ice)));
			memcpy ((*p)+i,&temp,sizeof(ice));
			if (rout!=NULL) {
				rout(group);
				refresh_screen (0); }
			return (true); } }
	else
		tell_error (err,help);
	return (false);
}

static void delete_group (int is, int group, Bool refresh)

{
	int j;
	cornice *src[3] = { &ctask.solut, NULL, &ctask.cstrain };

	free (src[group]->p[is]);
	ctask.total[group+2]--;
	for (j=is;j<ctask.total[group+2];j++) {
		src[group]->p[j]=src[group]->p[j+1];
		src[group]->right[j]=src[group]->right[j+1];
		src[group]->n[j]=src[group]->n[j+1]; }
	if (refresh) {
		store_group(group+2);
		refresh_screen ((is==ctask.total[group+2])?-1:0); }
}

static void insert_group (int is, int group)

{
	int j,k;
	cornice *src[3] = { &ctask.solut, NULL, &ctask.cstrain };
	char *err[3] = {
		EDITASK_TEXT_13, NULL,
		EDITASK_TEXT_14 };
	int help[3] = { 99,0,99 };

	if (is==0x0F) is=0;
	if ((j=ctask.total[group+2])==14)
		tell_error (err[group],help[group]);
	else if ((k=right_hand(group,0.0))!=0) {
		for (;j>is;j--) {
			src[group]->p[j]=src[group]->p[j-1];
			src[group]->right[j]=src[group]->right[j-1];
			src[group]->n[j]=src[group]->n[j-1]; }
		ctask.total[group+2]++;
		Mfatal ((src[group]->p[is]=malloc(sizeof(ice)))!=NULL);
		src[group]->n[is]=0;
		if (k==1) src[group]->right[is]=wgetlast()->f;
		if (!insert_ice(src[group]->p+is,13,0,src[group]->n+is,
					store_group,group+2,NULL,0))
			delete_group (is,group,false); }
}

static void edit_ice (ice *p,int i,void rout(int),int group)

{
	ice temp;

	memcpy (&temp,p+i,sizeof(ice));
	if (edit_form(&temp,group))
		memcpy (p+i,&temp,sizeof(ice));
	if (rout!=NULL) {
		rout(group);
		refresh_line (0,1); }
}

void delete_ice (ice **p,int i,int *total,void rout(int),int group)

{

	Mfatal (del_cell ((void **)p,(*total)--,i,sizeof(ice)));
	if (rout!=NULL) {
	rout(group);
	refresh_screen((i==*total)?-1:0); }
}

static void store_icicle (int group)

{
	int len[5] = { 19,17,0,19,0 };
	int num[5] = { 4,5,0,4,0 };
	ice **src[5] = { &ctask.source,&ctask.exclud,NULL,&ctask.fixed,NULL };

	store_ice (*src[group],group,0,0,0,len[group],num[group],true,false);
}

static void insert_icicle (int id,int group)

{
	ice **src[5] = { &ctask.source, &ctask.exclud, NULL, &ctask.fixed, NULL };
	char *err[5] = {
		EDITASK_TEXT_15,
		EDITASK_TEXT_16,
		NULL,
		EDITASK_TEXT_17,
		NULL };
	int help[5] = { 99,99,0,99,0 };

	insert_ice (src[group],254,id,&ctask.total[group],
				store_icicle,group,err[group],help[group]);
}

static void insert_member (int is, int im, int group)

{
	cornice *src[3] = { &ctask.solut, NULL, &ctask.cstrain };
	char *err[3] = {
		EDITASK_TEXT_18, NULL,
		EDITASK_TEXT_19 };
	int help[3] = { 99,0,99 };

	insert_ice (src[group]->p+is,12,im,src[group]->n+is,
				store_group,group+2,err[group],help[group]);
}

static void edit_icicle (int id,int group)

{
	ice **src[5] = { &ctask.source,&ctask.exclud,NULL,&ctask.fixed,NULL };

	edit_ice (*src[group],id,store_icicle,group);
}

static void edit_member (int is, int im, int group)

{
	cornice *src[3] = { &ctask.solut, NULL, &ctask.cstrain };

	if (im!=0x0C)
		edit_ice (src[group]->p[is],im,store_group,group+2);
	else if (right_hand(group,src[group]->right[is])==1) {
		src[group]->right[is]=wgetlast()->f;
		store_group(group+2);
		refresh_line (0,1); }
}

void delete_icicle (int id,int group)

{
	ice **src[5] = { &ctask.source,&ctask.exclud,NULL,&ctask.fixed,NULL };

	delete_ice (src[group],id,&ctask.total[group],store_icicle,group);
}

void delete_member (int is, int im, int group)

{
	cornice *src[3] = { &ctask.solut, NULL, &ctask.cstrain };

	delete_ice (src[group]->p+is,im,src[group]->n+is,NULL,0);
	if (src[group]->n[is]==0)
		delete_group(is,group,false);
	store_group(group+2);
	refresh_screen ((is==ctask.total[group+2]?-1-(group==2):0)+
						(im==src[group]->n[is]?-1:0));
}

void change_task (int id, int act, int key)

{
	int is,im;

	switch (act) {
	case 1:								/* Temperature */
		onEnter input_tp(6,EDITASK_TEXT_20,&ctask.t0,&ctask.t1,&ctask.p1,71);
		break;
	case 2:								/* Pressure */
		onEnter input_tp(9,EDITASK_TEXT_21,&ctask.p0,&ctask.p1,&ctask.t1,72);
		break;
	case 3:								/* Scale */
		onEnter {
			ctask.logstep=!ctask.logstep;
			store_step ();
			refresh_line (0,1); }
		break;
	case 4:								/* Ion forms */
		onEnter {
			ctask.ion=!ctask.ion;
			store_ion ();
			refresh_line (0,1); }
		break;
	case 5:								/* Source */
	case 6:
	case 8:
		id--;
		act-=5;
		switch (key) {
		case Enter:
			if (id!=0xFE)
				edit_icicle(id,act);
			else if (act==1) {
				ctask.incex=!ctask.incex;
				store_extype();
				refresh_line (0,1); }
			break;
		case Ins:
			if (id==0xFE) id=0;
			insert_icicle(id,act);
			break;
		case Del:
			if (id!=0xFE) delete_icicle(id,act);
			break; }
		break;
	case 7:								/* Solid solutions */
	case 9:
		act-=7;
		is=(id&0x00F0)>>4;
		im=(id&0x000F)-1;
		switch (key) {
		case Enter:
			if (im!=0x0E)
				edit_member(is,im,act);
			break;
		case Ins:
			if (im==0x0E)
				insert_group(is,act);
			else
				insert_member(is,im,act);
			break;
		case Del:
			if (im==0x0E) {
				if (is!=0x0F) delete_group(is,act,true); }
			else
				delete_member(is,im,act);
			break; }
		break;
	case 10:
		onEnter {
			winput(3,3,EDITASK_TEXT_22,0,75,
							22,23,24,ctask.desc,58,wdummy_);
			store_desc();
			refresh_line (0,1); }
	break; }
}

void store_task (void)

{
	int i;

	store_temp ();
	store_pres ();
	store_ion ();
	store_step ();
	store_desc ();
	store_extype ();
	for (i=0;i<5;i++)
		if (i==2||i==4)
			store_group(i);
		else
			store_icicle(i);
}
