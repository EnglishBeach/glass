#include "chem.h"
#include "wserv.h"
#include <math.h>
#include <values.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ This  routine solves a linear system with rotation method. a is บ
ณ a system mattrix, b - right hand vector, c -  vector  to  store บ
ณ solution, n - system dimension                                  บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static void linear_system (double **a, double *b, double *c, int n)

{
	double m,l,r;
	int i,j,k;
	for (i=0;i<n-1;i++) for (k=i+1;k<n;k++) {
		if (a[i][i]==0&&a[k][i]==0) {
			m=1;
			l=0; }
		else {
			m=sqrt(a[i][i]*a[i][i]+a[k][i]*a[k][i]);
			l=-a[k][i]/m; m=a[i][i]/m; }
		for (j=0;j<n;j++) {
			r=m*a[i][j]-l*a[k][j];
			a[k][j]=l*a[i][j]+m*a[k][j];
			a[i][j]=r; }
		r=m*b[i]-l*b[k];
		b[k]=l*b[i]+m*b[k];
		b[i]=r; }
	for (i=n-1;i>=0;i--) {
		m=0;
		for (k=n-1;k>i;k--)
			m+=c[k]*a[i][k];
		c[i]=(b[i]-m)/a[i][i]; }
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ This  is     error  handler  for  C  library  math routines. It บ
ณ supersedes  standard  reaction  for  three  errors:   logarithm บ
ณ argument  singularity  returninig  a  minimal double value, and บ
ณ exponentiaton over and underflow.                               บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

int matherr (struct exception *e)

{
	if (strcmp(e->name,"log")==0&&e->type==SING) {
		e->retval=LN_MINDOUBLE;
		return (1); }
	else if (strcmp(e->name,"exp")==0&&e->type==UNDERFLOW) {
		e->retval=0;
		return (1); }
	else if (strcmp(e->name,"exp")==0&&e->type==OVERFLOW) {
		e->retval=MAXDOUBLE;
		return (1); }
	else
		return (0);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ This routine embounds argument to fit it in float range.        บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static float embound (long double arg)

{
	return (arg<1e-100?1e-100:(arg>1e100?1e100:arg));
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ The  kernel of solving method. Refer a project documentation fo บ
ณ its description.                                                บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

double kernel (P,A,B1,g,mu,n,nmax,z,AL,l,nf,dnfv,I0,J0,L0,F0,M,p_itr,p_err)

double P,dnfv,*l,*nf,**AL,*z,*n;
float **A,*B1,*g,*nmax,*mu;
int I0,J0,L0,F0,*M;
char *p_itr,*p_err;

{

int	i,j,k,f;
int	J_L,iter;
double G[50],DB[50],BL[50],w,r,s;
float mu_min=MAXFLOAT;
char *p,buffer[20];

	P=log(P);
	for (i=M[0];i<M[1];i++)
		g[i]+=P;

	J_L=J0+L0;

	for (i=0;i<I0;i++) {
		mu[i]=nmax[i]*g[i];
		if (mu[i]<mu_min)
			mu_min=mu[i]; }

	for (i=0;i<I0;i++)
		n[i]=nmax[i]*exp((mu_min-mu[i])/options.spread);

	s=0;
	for (j=0;j<J0;j++) {
		r=0;
		for (i=0;i<I0;i++)
			r+=n[i]*A[i][j];
		s+=B1[j]/r; }
	s/=J0;

	for (i=0;i<I0;i++)
		n[i]*=s;

	iter=1;
	for (j=0;j<J_L;j++) l[j]=0;
	do {
		w=1-exp(-0.497614*(iter++)+0.487563);

		for (i=0;i<M[0];i++)
			z[i]=0.0;
		for (f=0;f<F0;f++) {
			s=(f==0)?dnfv:0.0;
			for (i=M[f];i<M[f+1];i++)
				s+=n[i];
			nf[f]=s;
			s=log(s);
			for (i=M[f];i<M[f+1];i++)
				z[i]=log(n[i])-s; }
		for (j=0;j<J_L;j++) {
			G[j]=0.0;
			DB[j]=B1[j];
			for (k=0;k<J_L;k++)
				AL[j][k]=0.0; }
		for (i=0;i<I0;i++) {
			r=g[i]+z[i];
			for (j=0;j<J_L;j++) {
				s=A[i][j]*n[i];
				DB[j]-=s;
				G[j]+=s*r;
				for (k=0;k<=j;k++)
					AL[j][k]+=s*A[i][k]; } }
		for (j=0;j<J_L;j++) {
			BL[j]=G[j]+DB[j];
			for (k=0;k<j;k++)
				AL[k][j]=AL[j][k]; }

		if (options.env.sound) sound(5000);
		linear_system (AL,BL,l,J_L);
		if (options.env.sound) nosound();

		for (i=0;i<M[0];i++) {
			s=-g[i];
			for (j=0;j<J_L;j++)
				s+=A[i][j]*l[j];
			n[i]=embound((long double)n[i]*exp(s*w));
			if (n[i]>nmax[i]) n[i]=nmax[i]; }
      for (f=0;f<F0;f++)
			for (i=M[f];i<M[f+1];i++) {
				s=-g[i];
				for (j=0;j<J_L;j++)
					s+=A[i][j]*l[j];
				n[i]=embound((long double)nf[f]*exp(z[i]*(1.0-w)+s*w));
				if (n[i]>nmax[i]) n[i]=nmax[i]; }

		s=fabs(DB[0]);
		for (j=1;j<J0;j++)
			if (s<fabs(DB[j]))
				s=fabs(DB[j]);

	p=p_itr; wputs(&p,itoa(iter,buffer,10)); wputs(&p,"  ");
	p=p_err; wputs(&p,gcvt(s,2,buffer)); wputs(&p,"     ");

	} while (s>options.accuracy&&iter<options.max_iter);
	return (s);
}
