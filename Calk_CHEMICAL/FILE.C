#include <alloc.h>
#include <dir.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
//#include <keys.h>
#define PgUp 329
#define PgDn 337
#include <errno.h>
#include <stdio.h>
#include <process.h>
#include "wserv.h"
#include "chem.h"

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Saving and restoring CHEMICAL task files.                       บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/


#define STATIC_LENGTH 4*sizeof(float)+4*sizeof(int)+62

static char mask[50] = "*";
static char fname[50] = "";
static char signature[64] =
	"CHEMICAL V1.0 (c) Green Arrow Inc, 1990\r\nTask image file.\x1A";
static int handle;
static void *dest[5]= { &ctask.source, &ctask.exclud, &ctask.solut,
								&ctask.fixed, &ctask.cstrain };

void file_rename (void);

static void DOS_shell (void)

{
	char *p;

	restore_break_state();
	wrestore();
	wsave (1,1,80,25);
	video_mode (3);
	p=wgeta(1,1); wputs(&p,FILE_TEXT_1);
	redirect_stdout(false);
	system("");
	redirect_stdout(true);
	video_mode (3);
	load_font(video_card);
	wrestore();
	save_break_state();
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Displays  current  active  filename  in  status  line of "Task" บ
ณ window.                                                         บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void view_filename ()

{
	char *p=wgeta (11,2), *q=p;
	int i;

	for (i=0;i<9;i++) wputs (&q,"    "); /* 4x9 spaces */
	wputs(&p,(*fname==0)?FILE_TEXT_2:fname);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Close a file.                                                   บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static void file_close (void)

{
	view_filename();
	close (handle);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Erase task (options File/New).                                  บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void file_new (void)

{
	int i;

	ctask.t0=298.15; ctask.t1=2000;
	ctask.p0=1; ctask.p1=0;
	ctask.nstep=6;
	ctask.logstep=false;
	ctask.ion=false;
	ctask.incex=false;
	ctask.source=realloc(ctask.source,sizeof(ice));
	ctask.fixed=realloc(ctask.fixed,sizeof(ice));
	ctask.exclud=realloc(ctask.exclud,sizeof(ice));
	for (i=0;i<ctask.total[2];i++) free (ctask.solut.p[i]);
	for (i=0;i<ctask.total[4];i++) free (ctask.cstrain.p[i]);
	for (i=0;i<5;i++) ctask.total[i]=0;
	*ctask.desc=0;
	*fname=0;
	view_filename ();
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Ask user to and enter filename via Input box or if he/she wants บ
ณ via Directory box.                                              บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static char *file_name (void)

{
	static char name[50] = "*.CHM";
	char heap[660];
	char *list[44];
	int disa[44];
	char pat[50];
	char dsk[50];
	char nam[14];
	char ext[5];
	int i,j,more,done,drv,err,cnt;
	int flg=MF_pgx|MF_let;
	struct ffblk blk;

	if (winput(43,6,FILE_TEXT_3,0,12,22,23,24,name,24,wdummy_)>0) {
		strupr (name);
		j=fnsplit(name,dsk,pat,nam,ext);
		if (!(j&DRIVE)) {
			strcpy (dsk,"A:");
			*dsk+=getdisk ();
			strcpy (name,""); }
		else
			strcpy (name,dsk);
		drv=*dsk-'A'+1;
		*pat='\\';
		if (!(j&DIRECTORY))
			getcurdir (drv,pat+1);
		else
			strcat (name,pat);
		if (!(j&FILENAME)) {
			strcpy (nam,"*");
			j|=WILDCARDS; }
		strcat (name,nam);
		if (!(j&EXTENSION))
			strcpy (ext,".CHM");
		strcat (name,ext);
		fnmerge(mask,dsk,pat,nam,ext);
		if (j&WILDCARDS) {
			*list=heap;
			for (i=1;i<44;i++)
				list[i]=list[i-1]+15;
First:		err=findfirst(mask,&blk,0);
			cnt=-1;
			for (;;) {
				cnt++;
				for (i=0;i<44;i++) disa[i]=1;
				for (i=0;i<43;i++) {
					if (err<0) break;
					strcpy (list[i],blk.ff_name);
					disa[i]=0;
					err=findnext(&blk); }
				if ((more=(i==43&&err==0))!=0) {
					strcpy (list[43],FILE_TEXT_4);
					disa[43]=0; }
				do {
					j=wmenu(10,7,12,44,4,list,disa,36,37,38,39,40,0,mask,
						flg,1,-14,NULL,NULL);
					if (j==0) return(NULL);
					done=true;
					flg|=MF_blk;
					if ((j&0xFF80)!=0)
						switch ((j&0xFF80)>>7) {
						case PgUp:
							goto First;
						case PgDn:
							done=more;
							continue;
						default:
							done=false; }
					else
						if (j==44)
							continue;
						else {
							err=findfirst(mask,&blk,0);
							for (i=0;i<cnt*43+j-1;i++)
								err=findnext(&blk);
							strcpy (nam,blk.ff_name);
							*ext=0;
							fnmerge(mask,dsk,pat,nam,ext);
							wrestore ();
							return (mask); }
				} while (!done); } }
		return (mask); }
	else
		return (NULL);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Check  if file <name> is already exists; if there is and <warn> บ
ณ flag is true ask confirmation to overwrite  flie,  then  rename บ
ณ file to have ".BAK" extension.                                  บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool file_overwr (char *name,Bool warn)

{
	struct ffblk blk;
	char text[50] = FILE_TEXT_5;
	char bak[50];

	if (findfirst(name,&blk,0)==0) {
		strcat (text,name);
		if (!warn||wverify(8,5,text,16)>0) {
			strcpy (bak,name);
			strcpy (strchr(bak,'.'),".BAK");
			unlink(bak);
			if (rename(name,bak)<0) {
				tell_ioerr (FILE_TEXT_6,errno);
				return (true); }
			else
				return (false); }
		else
			return (true); }
	else
		return(false);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Return true if signature read from opened file is legal.        บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool check_signature (void)

{
	char buf[65];

	return (_read(handle,buf,64)==64&&memcmp(signature,buf,64)==0);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Read header immediately following the signature.                บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool read_header (void)

{
	return (_read(handle,&ctask,STATIC_LENGTH)==STATIC_LENGTH);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Write header and signature.                                     บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool write_header (void)

{
	return (_write(handle,signature,64)==64&&
		_write(handle,&ctask,STATIC_LENGTH)==STATIC_LENGTH);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Read  or write the ice. Parameter <iort> must point to _read or บ
ณ _write C library routines.                                      บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool rdwr_ice (int *total,ice **p,int iort(int,void*,unsigned))

{
	int j;

	if (iort(handle,total,2)!=2) return(false);
	j=*total*sizeof(ice);
	if (iort==_read)
		*p=realloc(*p,(j==0)?sizeof(ice):j);
	return (iort(handle,*p,j)==j);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Read or write cornice.                                          บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool rdwr_cor (int *total,cornice *p,int iort(int,void*,unsigned))

{
	int j;

	if (iort(handle,total,2)!=2) return(false);
	for (j=0;j<*total;j++) {
		if (iort(handle,p->right,sizeof(p->right))!=sizeof(p->right))
			return (false);
		if (iort==_read) p->p[j]=malloc(sizeof(ice));
		if (!rdwr_ice (p->n+j,p->p+j,iort)) return (false); }
	return (true);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Read or write proper task unit.                                 บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static Bool rd_wr (int group,void *s_d,int iort(int,void*,unsigned))

{
	if (group==2||group==4)
		return (rdwr_cor (&ctask.total[group],(cornice*)s_d,iort));
	else
		return (rdwr_ice (&ctask.total[group],(ice**)s_d,iort));
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Option File/Load. Also invoked from Edit service via F3 key.    บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void file_load (void)

{
	char *nam;
	int i;
	void *dest[5] = { &ctask.source, &ctask.exclud, &ctask.solut,
							&ctask.fixed, &ctask.cstrain };

	if ((nam=file_name())==NULL) return;
	file_new();
	if ((handle=open(nam,O_RDONLY))<0) {
		tell_ioerr(FILE_TEXT_7,errno);
		*fname=0;
		file_close ();
		return; }
	else if (!check_signature()) {
		tell_error(FILE_TEXT_8,18);
		*fname=0;
		file_close ();
		return; }
	else
		for (i=-1;i<5;i++)
			if (!(i==-1?read_header():rd_wr(i,dest[i],_read))) {
				tell_ioerr(FILE_TEXT_9,errno);
				*fname=0;
				file_close ();
				return; }
	strcpy (fname,nam);
	file_close ();
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Save file. Warn flag is the same as in <file_overwr>            บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void file_save (Bool warn)

{
	int i;

	if (*fname==0) {
		file_rename ();
		return; }
	if (file_overwr(fname,warn)) return;
	if ((handle=open(fname,O_CREAT,S_IWRITE))<0) {
		tell_ioerr(FILE_TEXT_10,errno);
		*fname=0;
		return; }
	else
		for (i=-1;i<5;i++)
			if (!(i==-1?write_header():rd_wr(i,dest[i],_write))) {
				tell_ioerr(FILE_TEXT_11,errno);
				file_close ();
				unlink (fname);
				*fname=0;
				return; }
	file_close();
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ File/Rename service.                                            บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

static void file_rename (void)

{
	char *nam;

	if ((nam=file_name())==NULL) return;
	strcpy (fname,nam);
	file_save (true);
}

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ File service submenu called from Main menu.                     บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void file_service(void)

{
	static int j=1;
	static char *choices[]= { FILE_TEXT_12,FILE_TEXT_13,
		FILE_TEXT_14,FILE_TEXT_15,FILE_TEXT_16,FILE_TEXT_17 };

	j=wmenu (62,4,13,6,1,choices,NULL,12,13,14,15,0,0,
		FILE_TEXT_18,0,j,20,NULL,NULL);
	switch (j) {
		case 0:
			j=1;
			break;
		case 1:
			file_load();
			wrestore ();
			store_task ();
			view_screen ();
			break;
		case 2:
			file_save (false);
			wrestore ();
			break;
		case 3:
			file_new ();
			wrestore ();
			store_task ();
			view_screen ();
			break;
		case 4:
			file_rename();
			wrestore ();
			break;
		case 5:
			DOS_shell();
			break;
		case 6:
			leave_Chemical(); }
}
