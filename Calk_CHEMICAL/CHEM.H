#define false 0
#define true 1
#ifndef Bool
#define Bool int
#endif

#include "text_e.h"

/*
	Definition for task structure
*/

typedef struct {
	unsigned char el[6];
	int stoi[6];
	unsigned char flag;
	char adinf[12];
	float amo;
} ice;

typedef struct {
	ice *p[14];
	int n[14];
	float right[14];
} cornice;

typedef struct {
	float t0,t1,p0,p1;		/* T & P */
	int nstep;					/* Number of points */
	Bool logstep;				/* T=log.stepping; F=linear */
	Bool ion;					/* F=ion forms off */
	Bool incex;					/* T=include selected comps; F=exclude */
	char desc[62];

	int total[5];
	ice *source;
	ice *exclud;
	cornice solut;
	ice *fixed;
	cornice cstrain;
} task;

typedef struct {
	double sigfcy;
	double accuracy;
	double spread;
	int max_iter;
	int Cchain;
	int mpex;
	struct {
		unsigned int sound :1;
	} env;
	struct {
		unsigned pitch  :2;		/* 0=Elite 10 cpi, 1=Pica 12 cpi, 2=NLQ 10 cpi */
		unsigned width  :1;		/* 0=8 inches, 1=13 inches */
		unsigned all    :1;		/* 0=print significant, 1=print all */
		unsigned list   :1;		/* 0=do print list of comps, 1=don't */
		unsigned heigth :1;     /* 0=12 inches, 1=11 inches */
		unsigned wait   :1;		/* 0=don't wait after page, 1=do */
		unsigned space  :1;		/* 0=6 lpi, 1=8 lpi */
	} print;
	char prt_file[42];
} optn;


/*
	Database entry definition
*/

typedef struct {
	unsigned char el[6];		/* Element code */
	int stoi[6];				/* Stoichiometrical rate */
	char adinf[10];			/* Additional info */
	unsigned char flag;		/* Flag byte */
	float t[2];					/* Temperatural range */
	float f[7];					/* Thermo polynom coefficients */
	float h;						/* Standard entalphy */
} db_entry;

/*
	Defines for base entry flags byte
*/

#define DB_exi	0x80			/* Entry filled in */
#define DB_las 0x40			/* Entry is last */
#define DB_sol	0x01			/* Solid phase */


char *general_5_float (double val,char *p);
void conv_free (void **p);
void load_font (int video_card);
void tell_ioerr (char *s,int n);
void tell_error (char *s,int help);
void tell_nomem (void);
void tell_short (char *s);
void tell_wait (char *s);
void exit_nomem (void);
void exit_interr (void);
char *print_form (unsigned char *el,int *stoi,char *adinf,
					unsigned char flag,char *p,Bool ones,int len);
void print_form_gen (unsigned char *el,int *stoi,char *adinf,
					unsigned char flag,char *p,Bool ones,int len);
void print_form_2 (char *mask,float *a,int j0,char *adinf,
							int phase,char *buffer);
void print_form_matr (char *mask,float *a,int j0,char *adinf,
										int phase,char *buffer);
int  parse_form (unsigned char *el,int *stoi,char *adinf,
					unsigned char *flag,char *s);
Bool base_search (db_entry *entry,Bool mask_or,
						unsigned char *mask,Bool *more);
void base_service (void);
void base_init (void);
void base_look (void);
void base_delete (void);
void base_add (void);
Bool base_open (int access, int Cchn);
Bool base_close (void);
long base_size (void);
long base_tell (void);
void edit_service (void);
void display_core (void);
void file_service (void);
void file_load (void);
void file_save (Bool i);
void file_new (void);
void edit_init (void);
void store_temp (void);
void store_pres (void);
void store_ion (void);
void store_step (void);
void change_task (int id, int act, int key);
void store_initial (int x, int y, int len, ...);
void store_continued (int len, ...);
void store_clear (int w);
int  store_getx (void);
void refresh_line (int from, int count);
void refresh_screen (int shift);
void view_screen (void);
void insert_row (int i);
void delete_row (int i);
void enable_viewing (Bool enable);
void store_task (void);
void solve_service (void);
Bool del_cell (void **p,int n,int j,int size);
Bool ins_cell (void **p,int n,int j,int size);
double kernel (double P, float **A, float *B1, float *g, float *mu, double *n,
				float *nmax, double *z, double **AL, double *l,double *nf,
				double dnfv,int I0, int J0, int L0, int F0, int *M,
				char *p_itr, char *p_err);
void delete_member (int sg, int ix, int gr);
void delete_icicle (int ix, int gr);
void options_service (void);
void print_options(void);
Bool open_printer (void);
void close_printer (void);
void xprint (char *s,unsigned len);
int ignore_break (void);
void save_break_state (void);
void restore_break_state (void);
void leave_Chemical (void);
void check_for_break (void);
void redirect_stdout (Bool to_nul);
void read_options (Bool err);


/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ·
³                                                                    º
³ This section allocates variables and creates macros to automatic   º
³ recovery after out-of-memory error. Recovery section must begin    º
³ before any dynamic memory related functions in OUTERMOST (head)    º
³ procedure in the module. It must free all dynamic allocated me-    º
³ mory blocks using cvfree (not free!) function and must not include º
³ return or other flow-control code. Preprocessor variable RECOVERY  º
³ must be defined to include these features in compile.              º
³                                                                    º
ÔÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/


#ifdef RECOVERY

#include <setjmp.h>

#define cvfree(p) conv_free(&(void*)(p));
#define Alloc(p,n) if ((VOIDS=malloc(sizeof(*p)*(n)))==NULL) longjmp(ABEXIT,1); else p=VOIDS
#define Realloc(p,n) if ((VOIDS=realloc(p,sizeof(*p)*(n)))==NULL) longjmp(ABEXIT,1); else p=VOIDS
#define Immediate_break longjmp(ABEXIT,2);
#define Recovery_section int CAUSE; if ((CAUSE=setjmp(ABEXIT))!=0) { CAUSE--;
#define End_of_recovery_section tell_nomem(); display_core(); return; }

static jmp_buf ABEXIT;
static void *VOIDS;

#endif

#ifndef MAIN

extern task ctask;
extern optn options;
extern int video_card;

#endif

#ifdef SOLVETYPES

typedef struct {
	float g;
	unsigned char d;
} C2;

typedef struct {
	char adinf[12];
	char flag;
	C2   *c2;
} C1;

typedef struct {
	float **A,*Tx,*Px,**Mu;
	int   *ixJ,*ixN;
	C1    *C;
	int   M0,N0;
	char  mask[50];
	int   M[17];
	int   I0,J0,K0,F0,L0,mpnt,mpex,mint;
} XX;

void output_results (XX *Y);
void build_index_N (float f);
void grafix_out (XX *X);
void form_to_printer (char *s);
double get_tp_1 (float tp0, float tp1, int point);

#endif
