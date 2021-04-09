#define MF_let 0x01
#define MF_fn  0x02
#define MF_sfn 0x04
#define MF_afn 0x08
#define MF_cfn 0x10
#define MF_xfn 0x1E
#define MF_esc 0x20
#define MF_akb 0x40
#define MF_ckb 0x80
#define MF_pgx 0x100
#define MF_cky 0x200
#define MF_hlp 0x400
#define MF_avo 0x800
#define MF_blk 0x1000
#define MF_res 0x2000

#define KEYONLY 0
#define ALL 1
#define MENU 2

typedef int coor;

typedef union {
	int i;
	float f;
} convert;

char *wgeta(coor x,coor y);
int wsave (coor x,coor y,coor dx,coor dy);
int wrestore (void);
int wavoid (void);
int wwidth (void);
void wsetcursor (int from,int to);
int wgetkey (int mkey);
unsigned wmenu (coor x,coor y,int len,int n,int col,char **choice,
				int *dsabl,int atrrB,int attrT,int attrF,int attrH,
				int attrD, int type,char *text,
				int flags,int init,int help,char *footn,void (*exitp)(int));
void wblock (coor x,coor y,coor dx,coor dy,int attr,
				char *text,int type);
void wsethelp (char *file,coor x,coor y,int ab,int at);
void whelp(int i);
void wattr(int i);
void wputs(char **p, char *string);
void wsputs(char **p,int attr, char *string);
void wfoot (char *string);
void wbell (void);
int winput (coor x,coor y,char *text,int type,int help,int attrB,
			int attrT,int attrD,char *buf,int len,int check(char *s));
int wgint (coor x,coor y,char *text,int help,int attrB,
			int attrT,int attrD,int init);
double wgfloat (coor x,coor y,char *text,int help,int attrB,
			int attrT,int attrD,double init);
int wverify (coor x,coor y,char *str,int help);
void wcls (coor x, coor y, coor dx, coor dy, int attr);
void wputtext (coor x, coor y, coor dx, coor dy, char *s);
void wgettext (coor x, coor y, coor dx, coor dy, char *s);
void wbkgnd (coor x, coor y, coor dx, coor dy, int attr);
void wclrscr (int attr);
void wwindow (coor x, coor y, coor dx, coor dy);
int wdummy_ (char *s);
int __float (char *s);
int __integ (char *s);
convert *wgetlast (void);
void wgotoxy (coor x,coor y);
void wsetupbar (coor x, coor y, int len, long total);
void wappendbar (long current);
void video_mode (int mode);
void mouse_init (void);
