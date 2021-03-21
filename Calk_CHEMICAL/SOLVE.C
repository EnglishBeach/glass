#define SOLVETYPES 1
#define RECOVERY 1

#include "chem.h"
#include "wserv.h"
#include <string.h>
#include <alloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <values.h>
#include <fcntl.h>
#include <math.h>
//#include <keys.h>
#define F9 323
#define F10 324
#include <dos.h>

enum { PRIMARY, CONSISTENCY, GENERAL };
enum { REVIEWAL, ACCURATE };
enum { L_CONT=1, L_GOBAK, L_NOACC, L_ALACC, L_DFACC, L_ABANDON };

static int errata[22][3];
static int total_errors;
static Bool selected[50];
static Bool window_on_screen=false;

static XX X = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static float B[50],B1[50];
static double dnfv,**AL=NULL,*nx=NULL,*z=NULL;
static float *nmax=NULL,*g=NULL,*mu=NULL;
static char  *headln,*lowpane_addr[6];

static Bool break_is_pressed;

static float cvfloat (int cvf)

{
	return ((cvf>>7)+(float)(cvf&0x7F)/100.0);
}

static int break_handler(void)

{
	break_is_pressed=true;
	wfoot("");
	return(1);
}

static void cwremove (void)

{
	if (window_on_screen) {
		ctrlbrk(ignore_break);
		wrestore();
		window_on_screen=false; }
}

static void free_work (void)

{
	int i;

	for (i=0;i<X.I0;i++) {
		cvfree (X.A[i]);
		cvfree (X.C[i].c2); }
	for (i=0;i<X.J0+X.L0+X.F0+1;i++)
		cvfree (X.Mu[i]);
	cvfree (X.A);
	cvfree (X.C);
	cvfree (X.Mu);
	cvfree (nmax);
	cvfree (nx);
	cvfree (g);
	cvfree (mu);
	cvfree (z);
	for (i=0;i<X.J0+X.L0;i++) cvfree (AL[i]);
	cvfree (AL);
	cvfree (X.Tx);
	cvfree (X.Px);
	cvfree (X.ixJ);
	cvfree (X.ixN);

	cwremove();
}

static void store_errata (int group, int subgroup, int index)

{
	if (total_errors<22) {
		errata[total_errors][0]=group;
		errata[total_errors][1]=subgroup;
		errata[total_errors++][2]=index; }
}

static Bool display_errata (int set)

{
	ice **sim[] = { &ctask.source, &ctask.exclud, NULL, &ctask.fixed };
	cornice *com[] = { &ctask.solut, NULL, &ctask.cstrain };
	char *header[] = { SOLVE_TEXT_1,
							 SOLVE_TEXT_2,
							 SOLVE_TEXT_3 };
	char *errtext[] = {
		SOLVE_TEXT_4,
		SOLVE_TEXT_5,
		SOLVE_TEXT_6,
		SOLVE_TEXT_7,
		SOLVE_TEXT_8,
		SOLVE_TEXT_9,
		SOLVE_TEXT_10 };

	int i,gr,sg,ix;
	ice *v;
	char *p,*q,*r,buffer[40];
	char elm[6] = {0}, adinf[10] = "";
	int stoi[6] = { 128 };

	if (total_errors!=0) {
		wblock(35,2,43,total_errors+2,8,header[set],1);
		wattr(9);
		q=wgeta(37,3);
		for (i=0;i<total_errors;i++) {
			gr=errata[i][0];
			sg=errata[i][1];
			ix=errata[i][2];
			p=q;
			wputs(&p,errtext[gr-1]);
			switch(gr=errata[i][0]) {
			case 1:
			case 3:
				v=*sim[gr]+ix;
				r=buffer;
				*r++=' ';
				print_form(v->el,v->stoi,v->adinf,v->flag,r,false,40);
				break;
			case 2:
			case 4:
				itoa(sg+1,buffer,10);
				r=buffer+strlen(buffer);
				v=com[gr-2]->p[sg]+ix;
				*r++=' ';
				print_form(v->el,v->stoi,v->adinf,v->flag,r,false,40);
				break;
			case 5:
			case 6:
				p+=2;
				elm[0]=ix;
				print_form (elm,stoi,adinf,0,buffer,false,40);
				break;
			case 7:
				*buffer=0;
				break; }
			wputs(&p,buffer);
			wputs(&p,".");
			q+=160; }
		return (false); }
	else
		return (true);
}

static Bool correct_errors (int set)

{
	int i;
	Bool check_ok;

	cwremove();
	check_ok=display_errata(set);
	if (!check_ok&&(wverify (3,3,SOLVE_TEXT_11,94+set))>0) {
		wrestore();
		enable_viewing (false);
		for (i=0;i<total_errors;i++)
			switch (errata[i][0]) {
			case 2:
			case 4:
				delete_member(errata[i][1],errata[i][2],errata[i][0]-2);
				break;
			case 1:
			case 3:
				delete_icicle(errata[i][1],errata[i][0]);
				break; }
		enable_viewing (true);
		view_screen (); }
	else
		wrestore();
	return (check_ok);
}

static Bool build_mask (void)

{
	int i,j,k;
	float b;
	char w;

	X.J0=0;
	for (i=0;i<ctask.total[0];i++)
		for (j=0;j<6&&(ctask.source+i)->el[j]!=0;j++) {
			for (k=0;k<X.J0;k++)
				if ((ctask.source+i)->el[j]==X.mask[k]) break;
			if (k==X.J0) {
				X.mask[X.J0]=(ctask.source+i)->el[j];
				B[X.J0]=0;
				if (X.J0++==48-ctask.total[4]) {
					tell_error (
						SOLVE_TEXT_12,99);
					return (false); } }
			B[k]+=cvfloat((ctask.source+i)->stoi[j])*(ctask.source+i)->amo; }

	if (ctask.ion) {
		X.mask[X.J0]=255;
		B[X.J0++]=0.0; }
	X.mask[X.J0]=0;

	for (j=0,b=0;j<X.J0;j++) b+=B[j];
	for (j=0;j<X.J0;j++) B[j]/=b;

	for (j=0;j<X.J0;j++) {
		i=j;
		for (k=j+1;k<X.J0;k++)
			if (X.mask[k]<X.mask[i]) i=k;
		if (i!=j) {
			w=X.mask[j]; X.mask[j]=X.mask[i]; X.mask[i]=w;
			b=B[j]; B[j]=B[i]; B[i]=b; } }

	X.L0=ctask.total[4];
	return (true);
}

static void fix_up (void)

{
	int i,j,k;
	unsigned char elm;
	float max_abundance,x;
	ice *p;

	for (j=0;j<X.J0;j++)
		B1[j]=B[j];
	dnfv=0;
	for (i=0;i<ctask.total[3];i++) {
		p=ctask.fixed+i;
		max_abundance=MAXFLOAT;
		for (j=0;j<6&&(elm=p->el[j])!=0;j++) {
			for (k=0;k<X.J0;k++)
				if (X.mask[k]==elm) break;
			if (max_abundance>(x=B[k]/cvfloat(p->stoi[j])))
				max_abundance=x; }
		for (j=0;j<6&&(elm=p->el[j])!=0;j++) {
			for (k=0;k<X.J0;k++)
				if (X.mask[k]==elm) break;
			B1[k]-=max_abundance*cvfloat(p->stoi[k])*p->amo/100.0; }
		if (!(p->flag&DB_sol)) dnfv+=max_abundance*p->amo/100.0; }

	for (j=0;j<X.J0-ctask.ion;j++)
		if (B1[j]<=0)
			store_errata(6,0,X.mask[j]);
}

static double get_tp (float tp0, float tp1,int i,int n)

{
	if (n==0||tp1==0||i==0) return (tp0);
	if (ctask.logstep)
		return (tp0*pow(tp1/tp0,i/(n-1.0)));
	else
		return (tp0+(tp1-tp0)*i/(n-1.0));
}

double get_tp_1 (float tp0, float tp1, int point)

{
	return (get_tp(tp0,tp1,point,ctask.nstep));
}

static double get_tp_2 (float tp0,float tp1, int intvl, int point)

{
	return(get_tp(get_tp_1(tp0,tp1,intvl),get_tp_1(tp0,tp1,intvl+1),
						point+1,X.mpex+2));
}

static double compute_g (float *f,float h,double t)

{
	t/=10000.;
	return(h/t-f[0]-f[1]*log(t)-f[2]/t/t-f[3]/t-f[4]*t-
				f[5]*t*t-f[6]*t*t*t);
}

static Bool start_reading_base (void)

{
	return (base_open(O_RDONLY,0));
}

static void end_reading_base (void)

{
	base_close ();
}

static Bool find_in_ice (db_entry *entry, ice *p, int n)

{
	int i;

	for (i=0;i<n;i++) {
		if (memcmp(entry->el,p[i].el,6)==0&&
			memcmp(entry->stoi,p[i].stoi,12)==0&&
			strcmp(entry->adinf,p[i].adinf)==0&&
			(entry->flag&DB_sol)==(p[i].flag&DB_sol)) return (true); }
	return (false);
}

static Bool read_base_entry (db_entry *entry)

{
	Bool more=true;

	do {
		while (!base_search(entry,false,X.mask,&more))
			if (!more) return (false); }
	while (find_in_ice (entry,ctask.fixed,ctask.total[3])||
		(find_in_ice (entry,ctask.exclud,ctask.total[1])!=ctask.incex));
	return (true);
}

int find_in_mattrix (char *el,int *stoi,char *adinf,char flag,int start)

{
	int i,j,k,l,m,mt;

	for (mt=0;mt<6&&el[mt]!=0;mt++);

	for (i=start;i<X.I0;i++) {
		if (strcmp(adinf,X.C[i].adinf)!=0||(flag&DB_sol)!=(X.C[i].flag&DB_sol))
			continue;
		m=mt;
		l=mt;
		for (j=0;j<X.J0;j++)
			if (X.A[i][j]!=0) {
				l--;
				for (k=0;k<6&&el[k]!=0;k++)
					if (X.mask[j]==el[k]&&X.A[i][j]==cvfloat(stoi[k])) m--; }
		if (m==0&&l==0) return (i); }
	return (-1);
}

void add_row_to_mattrix(char *el,int *stoi,char *adinf,char flag,char def_d)

{
	int i,j,k;

	Realloc (X.A,X.I0+1);
	X.A[X.I0]=NULL;
	Realloc (X.C,X.I0+1);
	X.C[X.I0].c2=NULL;
	i=X.I0++;

	Alloc (X.A[i],X.J0+X.L0);
	Alloc (X.C[i].c2,X.mpnt);

	for (j=0;j<X.J0+X.L0;j++) X.A[i][j]=0.0;
	for (j=0;j<X.mpnt;j++)  X.C[i].c2[j].d=def_d;
	strcpy (X.C[i].adinf,adinf);
	X.C[i].flag=flag&DB_sol;

	for (k=0;k<6&&el[k]!=0;k++) {
		for (j=0;j<X.J0;j++)
			if (X.mask[j]==el[k]) break;
		X.A[i][j]=cvfloat(stoi[k]); }
}

static void expand_mattrix (void)

{
	int i,j;

	for (i=0;i<X.K0;i++) {
		Realloc (X.C[i].c2,X.mpnt+X.mint*X.mpex);
		display_core();
		for (j=X.mpnt;j<X.mpnt+X.mint*X.mpex;j++)
			X.C[i].c2[j].d=255; }
	for (j=0;j<X.J0+X.L0+X.F0+1;j++)
		Realloc (X.Mu[j],X.mpnt+X.mint*X.mpex);
	display_core();
}

static char defect (float t0, float t1, float t)

{
	float dt;

	if (t0<t&&t<t1)
		return (0);
	else {
		dt=t<t0?t0-t:t-t1;
		return(dt>5000.0?251:dt/20.0+1); }
}

static int sort_mattrix (int from, int to)

{
	int l,r,j;
	float w;
	char c,tmp[12];
	C2 y;

	l=from; r=to-1;
	do {
      while (X.C[l].flag&DB_sol&&l<=r) l++;
      while (!(X.C[r].flag&DB_sol)&&l<=r) r--;
      if (l<r) {
			for (j=0;j<X.J0+X.L0;j++) { w=X.A[l][j]; X.A[l][j]=X.A[r][j];
															X.A[r][j]=w; }
         strcpy (tmp,X.C[l].adinf); strcpy (X.C[l].adinf,X.C[r].adinf);
         strcpy(X.C[r].adinf,tmp);
         c=X.C[l].flag; X.C[l].flag=X.C[r].flag; X.C[r].flag=c;
         for (j=0;j<X.mpnt;j++) {
				y=X.C[l].c2[j]; X.C[l].c2[j]=X.C[r].c2[j]; X.C[r].c2[j]=y; }
			l++; r--; } }
	while (l<r);
	return (l);
}

static void build_constrains_columns (void)

{
	int i,j,l;
	ice *v;

	for (l=X.L0-1;l>=0;l--) {
		B1[l+X.J0]=B[l+X.J0]=ctask.cstrain.right[l];
		for (j=ctask.cstrain.n[l]-1;j>=0;j--) {
			v=ctask.cstrain.p[l]+j;
			i=find_in_mattrix (v->el,v->stoi,v->adinf,v->flag,0);
			if (i<0)
				store_errata (4,l,j);
			else
				X.A[i][l+X.J0]=v->amo; } }
}

static void build_solution_rows (void)

{
	int i,i1,j,k,l;
	ice *v;

	for (l=ctask.total[2]-1;l>=0;l--) {
		X.M[X.F0++]=X.I0;
		for (j=ctask.solut.n[l]-1;j>=0;j--) {
			v=ctask.solut.p[l]+j;
			i=find_in_mattrix (v->el,v->stoi,v->adinf,v->flag,0);
			if (i<0)
				store_errata (2,l,j);
			else {
				i1=X.I0;
				add_row_to_mattrix (v->el,v->stoi,v->adinf,v->flag,0);
				for (k=0;k<X.J0+X.L0;k++) X.A[i1][k]=X.A[i][k];
				for (k=0;k<X.mpnt;k++) X.C[i1].c2[k].g=X.C[i].c2[k].g;
				strcpy (X.C[i1].adinf,X.C[i].adinf); } } }
}

static void allocate_working_storage (void)

{
	int j;

	Alloc (nx,X.I0);
	Alloc (nmax,X.K0);
	Alloc (g,X.I0);
	Alloc (mu,X.I0);
	Alloc (z,X.I0);
	Alloc (AL,X.J0+X.L0);
	for (j=0;j<X.J0+X.L0;j++) Alloc (AL[j],X.J0+X.L0);
	Alloc (X.Mu,X.J0+X.L0+X.F0+1);
	for (j=0;j<X.J0+X.L0+X.F0+1;j++) Alloc (X.Mu[j],X.mpnt);
}

static void compute_nmax ()

{
	int i,j;
	double t;

	for (i=0;i<X.K0;i++) {
		nmax[i]=MAXFLOAT;
		for (j=0;j<X.J0;j++)
			if (X.A[i][j]!=0&&nmax[i]>(t=B[j]/X.A[i][j]))
				nmax[i]=t;
		if (nmax[i]>MAXFLOAT/10.0)
			nmax[i]=1e-5; }
}

static void draw_searching_base_window (int heigth, char *mess)

{
	break_is_pressed=false;
	ctrlbrk(break_handler);
	window_on_screen=true;
	wblock (16,8,40,heigth,42,mess,1);
	wattr (43);
	wfoot (SOLVE_TEXT_13);
	wsetupbar (20,10,32,base_size());
}

static void append_fixed_species (void)

{
	int i,i0,j;
	ice *p;

	X.K0=X.I0;
	j=X.F0;
	if (ctask.total[3]>0) {
		i0=X.I0;
		for (i=0;i<ctask.total[3];i++) {
			p=ctask.fixed+i;
			add_row_to_mattrix (p->el,p->stoi,p->adinf,p->flag,0); }
		X.K0=X.I0;
		X.I0=i0;
		X.M[j++]=X.I0;
		X.M[j++]=sort_mattrix(X.I0,X.K0); }
	X.M[j]=X.K0;
}

static void correct_for_fixed (void)

{
	int i,j;

	for (j=0;j<X.L0;j++) B1[j]=B[j];

	for (i=X.I0;i<X.K0;i++)
		for (j=0;j<X.L0;j++)
			B1[j]-=X.A[i][j]*nmax[i]*ctask.fixed[i-X.I0].amo/100.0;
}

static Bool build_mattrix (void)

{
	db_entry entry;
	int i,j;
	double t;
	unsigned char dt;
	char *p,*p_com,buffer[20];

	if (!start_reading_base()) return (false);
	X.I0=0;
	X.mpnt=(ctask.nstep==0||(ctask.t1==0&&ctask.p1==0))?1:ctask.nstep;
	Alloc (X.A,1);
	Alloc (X.C,1);

	draw_searching_base_window (7,SOLVE_TEXT_14);
	p=wgeta(20,12); wputs(&p,SOLVE_TEXT_15); p_com=p-2;

	while (read_base_entry(&entry)) {
		if (break_is_pressed) Immediate_break;
		if	((i=find_in_mattrix(entry.el,entry.stoi,entry.adinf,entry.flag,0))
					==-1) {
			i=X.I0;
			add_row_to_mattrix(entry.el,entry.stoi,entry.adinf,entry.flag,255);
			p=p_com; wputs(&p,itoa(X.I0,buffer,10)); display_core(); }
		for (j=0;j<X.mpnt;j++) {
			t=get_tp_1(ctask.t0,ctask.t1,j);
			dt=defect(entry.t[0],entry.t[1],t);
			if (dt<X.C[i].c2[j].d) {
				X.C[i].c2[j].d=dt;
				X.C[i].c2[j].g=compute_g(entry.f,entry.h,t); } } }
	end_reading_base();

	cwremove();

	total_errors=0;
	for (j=0;j<X.J0;j++) {
		for (i=0;i<X.I0;i++)
			if (X.A[i][j]!=0) break;
		if (i==X.I0)
			store_errata ((j==X.J0-ctask.ion)?7:5,0,X.mask[j]); }

	build_constrains_columns();

	X.J0-=ctask.ion;
	X.L0+=ctask.ion;

	X.F0=1;
	X.M[0]=sort_mattrix (0,X.I0);
	build_solution_rows();
	append_fixed_species();
	allocate_working_storage();
	compute_nmax();
	correct_for_fixed();
	display_core ();

	return (correct_errors(GENERAL));
}

static void expand_solutions (db_entry *entry, int j0, int j1)

{
	int i,i1,j;

	i1=find_in_mattrix(entry->el,entry->stoi,entry->adinf,entry->flag,0);
	i=i1;

	while	((i=find_in_mattrix
		(entry->el,entry->stoi,entry->adinf,entry->flag,i+1))>0)
			for (j=j0;j<j1;j++)
				X.C[i].c2[j]=X.C[i1].c2[j];
}

static void fill_in_expanded_columns (void)

{
	db_entry entry;
	int i,j,k,l;
	double t;
	unsigned char dt;

	if (!start_reading_base()) Immediate_break;
	draw_searching_base_window (6,SOLVE_TEXT_16);

	X.J0+=ctask.ion;

	while (read_base_entry(&entry)) {
		i=find_in_mattrix(entry.el,entry.stoi,entry.adinf,entry.flag,0);
		for (j=0,l=X.mpnt;j<X.mpnt-1;j++) {
			if (selected[j])
				for (k=0;k<X.mpex;k++,l++) {
					t=get_tp_2(ctask.t0,ctask.t1,j,k);
					dt=defect(entry.t[0],entry.t[1],t);
					if (dt<X.C[i].c2[l].d) {
						X.C[i].c2[l].d=dt;
						X.C[i].c2[l].g=compute_g(entry.f,entry.h,t); } } }
		if (ctask.total[2]>0)
			expand_solutions (&entry,X.mpnt,l); }
	end_reading_base();

	X.J0-=ctask.ion;

	cwremove();
	return;
}

static void check_dups (ice *p, int n, int group, int subgroup)

{
	int i,j;
	Bool marked[255] = { 0 };

	for (i=0;i<n;i++)
		for (j=n-1;j>i;j--)
			if (!marked[j]&&memcmp(p+i,p+j,sizeof(ice))==0) {
				marked[j]=true;
				store_errata (group,subgroup,j); }
}

static void consistency_check (ice *p, int n, int group, int subgroup)

{
	int i,j,k;
	Bool marked[255] = { 0 };

	for (i=n-1;i>=0;i--)
		if (!marked[i])
			for (j=0;j<6&&(p+i)->el[j]!=0;j++) {
				for (k=0;k<X.J0;k++)
					if (X.mask[k]==(p+i)->el[j]) break;
				if (k==X.J0) {
					marked[i]=true;
					store_errata (group,subgroup,i);
					break; } }
}

static Bool primary_build (void)

{
	int i;
	Bool check_ok;

	if (ctask.total[0]==0) {
		tell_error (SOLVE_TEXT_17,100);
		return (false); }
	total_errors=0;
	check_dups(ctask.exclud,ctask.total[1],1,0);
	for (i=0;i<ctask.total[2];i++)
		check_dups(ctask.solut.p[i],ctask.solut.n[i],2,i);
	for (i=0;i<ctask.total[4];i++)
		check_dups(ctask.cstrain.p[i],ctask.cstrain.n[i],2,i);

	if (!build_mask()) return (false);
	check_ok=correct_errors(PRIMARY);

	total_errors=0;
	consistency_check(ctask.exclud,ctask.total[1],1,0);
	consistency_check(ctask.fixed,ctask.total[3],3,0);
	for (i=0;i<ctask.total[2];i++)
		consistency_check(ctask.solut.p[i],ctask.solut.n[i],2,i);
	for (i=0;i<ctask.total[4];i++)
		consistency_check(ctask.cstrain.p[i],ctask.cstrain.n[i],4,i);

	fix_up ();
	return (correct_errors(CONSISTENCY)&&check_ok);
}

static void solve_point (int m, double P,char *p_itr, char *p_err)

{
	int i;
	double l[50],nf[16],s;

	check_for_break();
	if (break_is_pressed) Immediate_break;
	for (i=0;i<X.I0;i++) g[i]=X.C[i].c2[m].g;
	s=kernel (P,X.A,B1,g,mu,nx,nmax,z,AL,l,nf,dnfv,
				X.I0,X.J0,X.L0,X.F0,X.M,p_itr,p_err);
	for (i=0;i<X.I0;i++)
		X.C[i].c2[m].g=nx[i]>1e-36?nx[i]:0.0;
	for (i=X.I0;i<X.K0;i++)
		X.C[i].c2[m].g=nmax[i]*ctask.fixed[i-X.I0].amo/100.0;
	for (i=0;i<X.J0+X.L0;i++)
		X.Mu[i][m]=l[i];
	for (i=0;i<X.F0;i++)
		X.Mu[i+X.J0+X.L0][m]=nf[i];
	X.Mu[X.F0+X.J0+X.L0][m]=s;
	return;
}

static void solve_system (int pass)

{
	double P;
	int i,j,l;
	char *p,*p_pnt,*p_itr,*p_err,buffer[20];

	break_is_pressed=false;
	ctrlbrk(break_handler);
	window_on_screen=true;
	wblock (16,8,40,10,42,(pass==REVIEWAL)?SOLVE_TEXT_18:SOLVE_TEXT_19,1);
	wattr (43);
	wfoot (SOLVE_TEXT_20);
	p=wgeta(22,12); wputs(&p,SOLVE_TEXT_21); p_pnt=p-2;
	p=wgeta(40,12); wputs(&p,SOLVE_TEXT_22);
		wputs(&p,itoa(X.mpnt+((pass==ACCURATE)?X.mpex*X.mint:0),buffer,10));
	p=wgeta(22,14); wputs(&p,SOLVE_TEXT_23); p_itr=p;
	p=wgeta(22,15); wputs(&p,SOLVE_TEXT_24); p_err=p;

	switch (pass) {
	case REVIEWAL:
		wsetupbar (20,10,32,X.mpnt);
		for (i=0;i<X.mpnt;i++) {
			P=get_tp_1(ctask.p0,ctask.p1,i);
			p=p_pnt; wputs(&p,itoa(i+1,buffer,10));
			wappendbar(i+1);
			solve_point(i,P,p_itr,p_err); }
		break;
	case ACCURATE:
		if (X.mint==0) break;
		wsetupbar (20,10,32,X.mpex*X.mint+X.mpnt);
		for (i=0,l=X.mpnt;i<X.mpnt-1;i++)
			if (selected[i])
				for (j=0;j<X.mpex;j++,l++) {
					P=get_tp_2(ctask.p0,ctask.p1,i,j);
					p=p_pnt; wputs(&p,itoa(l+1,buffer,10));
					wappendbar(l+1);
					solve_point(l,P,p_itr,p_err); }
		break; }
	wsetupbar(0,0,0,0);
	cwremove();
}

static Bool check_row (int i,int j)

{
	float n1,n2;

	n1=X.C[i].c2[j].g;
	n2=X.C[i].c2[j+1].g;
	return ((n1<=options.accuracy)!=(n2<=options.accuracy));
}

static Bool check_range (int j)

{
	Bool Ok=false;
	int i,k;

	for (i=0;i<X.M[0];i++)
		Ok|=check_row (i,j);
	for (k=1;k<X.F0;k++)
		for (i=X.M[k];i<X.M[k+1];i++)
			Ok|=check_row (i,j);
	return (Ok);
}

static void display_events (int k)

{
	char *p,buffer[50];
	char spaces_20[21] = "                   ";
	int i,j,f;

	p=wgeta(3,17); wputs (&p,"Ã");
	p=wgeta(71,17); wputs (&p,"´");
	p=wgeta(5,4); wputs (&p,headln);
	p=wgeta(52,4); wputs (&p,SOLVE_TEXT_25); wputs (&p,itoa(X.mint,buffer,10));
		wputs (&p," Ä");

	j=0;
	for (i=0;i<X.M[0]&&j<6;i++)
		if (check_row(i,k)) {
			print_form_matr (X.mask,X.A[i],X.J0,X.C[i].adinf,0,buffer);
			strcat(buffer,(X.C[i].c2[k].g<options.accuracy)?" \x1E":" \x1F");
			strcat(buffer,spaces_20);
			buffer[20]=0;
			p=lowpane_addr[j++]; wputs(&p,buffer); }
	for (f=1;f<X.F0;f++)
		for (i=X.M[f];i<X.M[f+1]&&j<6;i++)
			if (check_row(i,k)) {
				print_form_matr (X.mask,X.A[i],X.J0,X.C[i].adinf,f,buffer);
				strcat(buffer,(X.C[i].c2[k].g<options.accuracy)?" \x1E":" \x1F");
				strcat(buffer,spaces_20);
				buffer[20]=0;
				p=lowpane_addr[j++]; wputs(&p,buffer); }
	for (;j<6;j++) {
		p=lowpane_addr[j];
		wputs(&p,spaces_20); }
}


static void prepare_panes_video (char *p)

{
	float f;
	int i,j,k;

	if (ctask.t1==0) {
		strcpy (p," T=");
		f=ctask.t0; }
	else {
		strcpy (p," P=");
		f=ctask.p0; }
	p+=strlen(p);
	general_5_float (f,p);
	strcat (p," ");

	for (k=0,i=18;i<20;i++)
		for (j=5;j<60;j+=19)
			lowpane_addr[k++]=wgeta(j,i);

}

static void range_to_ascii (int i,char *p)

{
	double f0,f1;
	char buffer [20];

	if (ctask.t1==0) {
		f0=ctask.p0;
		f1=ctask.p1; }
	else {
		f0=ctask.t0;
		f1=ctask.t1; }

	memset (p,' ',14);
	p[14]=0;
	p+=2;
	general_5_float (get_tp_1(f0,f1,i),buffer);
	strcpy (p,buffer);
	strcat (p,",");
	general_5_float (get_tp_1(f0,f1,i+1),buffer);
	strcat (p,buffer);
	*(p+strlen(p))=' ';
}

static int local_menu (void)

{
	static char *choices[] = {
		SOLVE_TEXT_26,
		SOLVE_TEXT_27,
		SOLVE_TEXT_28,
		SOLVE_TEXT_29,
		SOLVE_TEXT_30,
		SOLVE_TEXT_31 };

	int j=1;

	j=wmenu (58,6,17,6,1,choices,NULL,18,19,20,21,0,0,NULL,0,j,81,NULL,NULL);
	if (j>0)
		wrestore();
	else
		j=L_GOBAK;
	return (j);
}

static Bool search_enaccuratement (void)

{
	char heap[720],*list[48],headline[20];
	int disa[48],flg=MF_res|MF_fn,i,j;
	Bool cont;

	X.mint=0;
	if (X.mpnt==1) return (true);
	headln=headline;

	for (i=0;i<48;i++) {
		list[i]=heap+i*15;
		disa[i]=1; }

	prepare_panes_video(headline);
	for (i=0;i<X.mpnt-1;i++) {
		range_to_ascii(i,list[i]);
		selected[i]=check_range(i);
		if (selected[i]) {
			list[i][0]='û';
			X.mint++; }
		disa[i]=0; }


	wblock (3,17,69,4,44,NULL,0);
	wattr (45);

	j=1;
	cont=false;
	do {
		j=wmenu (3,4,14,48,4,list,disa,44,45,45,46,47,0,
			SOLVE_TEXT_32,flg,j,-80,SOLVE_TEXT_33,
			display_events);
		flg|=MF_blk;

		if ((j&0xFF80)!=0||j==0)
			switch ((j&0xFF80)>>7) {
			case 0:
			case F10:
				switch(local_menu()) {
				case L_ABANDON:
					if (wverify(25,1,SOLVE_TEXT_34,87)>0) {
						wrestore();
						wrestore();
						return (false); }
					break;
				case L_NOACC:
					X.mint=0;
					for (i=0;i<X.mpnt-1;i++) {
						selected[i]=false;
						list[i][0]=' '; }
					break;
				case L_ALACC:
					X.mint=X.mpnt-1;
					for (i=0;i<X.mpnt-1;i++) {
						selected[i]=true;
						list[i][0]='û'; }
					break;
				case L_DFACC:
					X.mint=0;
					for (i=0;i<X.mpnt-1;i++) {
						selected[i]=check_range(i);
						if (selected[i]) {
							list[i][0]='û';
							X.mint++; }
						else
							list[i][0]=' '; }
					break;
				case L_CONT:
					cont=true;
				case L_GOBAK:
					break; }
				break;
			case F9:
				cont=true;
				break; }
		else {
			selected[j-1]=!selected[j-1];
			list[j-1][0]=selected[j-1]?'û':' ';
			X.mint+=selected[j-1]?1:-1; }
		j&=0x007F; }
   while (!cont);

	if (X.mint>0)
		X.mpex=wgint(30,8,SOLVE_TEXT_35,97,22,23,24,options.mpex);
	wrestore();
	wrestore();
	return (true);
}


static void build_indices (void)

{
	int i,j,k,m0;

	m0=X.mpnt+X.mpex*X.mint;
	Alloc (X.Tx,m0);
	Alloc (X.Px,m0);
	Alloc (X.ixJ,m0);

	X.M0=0; k=X.mpnt;

	for (i=0;i<X.mpnt;i++) {
		X.Tx[X.M0]=get_tp_1 (ctask.t0,ctask.t1,i);
		X.Px[X.M0]=get_tp_1 (ctask.p0,ctask.p1,i);
		X.ixJ[X.M0++]=i;
		if (i<X.mpnt-1&&selected[i])
			for (j=0;j<X.mpex;j++,k++) {
				X.Tx[X.M0]=get_tp_2 (ctask.t0,ctask.t1,i,j);
				X.Px[X.M0]=get_tp_2 (ctask.p0,ctask.p1,i,j);
				X.ixJ[X.M0++]=k; } }

	Alloc (X.ixN,X.I0);

	build_index_N (options.sigfcy);
}

void build_index_N (float bound)

{
	int i,j,m0=X.mpnt+X.mpex*X.mint;

	X.N0=0;
	for (i=0;i<X.I0;i++) {
		for (j=0;j<m0;j++)
			if (X.C[i].c2[j].g>bound)
				break;
		if (j!=m0)
			X.ixN[X.N0++]=i; }
	for (i=X.I0;i<X.K0;i++)
		X.ixN[X.N0++]=i;
}

#pragma warn -pia

static Bool phase_1 (void)

{
	Bool Ok;
	if ((Ok=build_mattrix ()))
		solve_system (REVIEWAL);
	return (Ok);
}

static Bool phase_2 (void)

{
	Bool Ok;

	if ((Ok=search_enaccuratement())) {
		if (X.mint!=0) {
			expand_mattrix ();
			fill_in_expanded_columns();
			solve_system (ACCURATE); }
		build_indices(); }
	return (Ok);
}

#pragma warn +pia

void solve_service ()

{
	Recovery_section
		free_work();
	End_of_recovery_section

	X.mpnt=0;
	X.mint=0;
	if (!primary_build()) return;
	if (phase_1() && phase_2())
		output_results (&X);
	free_work();
	display_core();
/*

Dump:
	{
		#include <conio.h>
		#include <stdio.h>

		int i,j;

		textattr(0x1F);
		clrscr();
		cprintf ("\n\rJ0=%d  L0=%d  F0=%d    M: ",X.J0,X.L0,X.F0);
		for (j=0;j<X.J0;j++) cprintf ("%d  ",X.mask[j]);
		cprintf ("\n\n\r");
		for (i=0;i<X.M0;i++) {
			cprintf ("%3d³ %5.1g %5.1g ³",i,X.Tx[i],X.Px[i]);
			for (j=0;j<X.J0+X.L0+X.F0+1;j++) cprintf ("%6.1g",X.Mu[j][i]);
			cprintf (" ³\n\r"); }
	}
*/

}
