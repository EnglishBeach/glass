#include "chem.h"
#include "wserv.h"
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#pragma warn -dup

extern char ch_exe[];

static void ask_accuracy (void)

{
	static char *choices[] = {
		OPTIONS_TEXT_1,
		OPTIONS_TEXT_2,
		OPTIONS_TEXT_3,
		OPTIONS_TEXT_4,
		OPTIONS_TEXT_5 };

	static float acc[] = { 1e-4,1e-5,1e-6,1e-7,1e-8 };
	static int   itr[] = { 50,75,100,150,150 };

	int j;

	j=wmenu(50,14,16,5,1,choices,NULL,18,19,19,21,0,0,
			OPTIONS_TEXT_6,MF_let,1,-133,NULL,NULL);

	if (j!=0) {
		options.accuracy=acc[j-1];
		options.max_iter=itr[j-1];
		wrestore (); }
}

static void solve_options (void)

{
	#define OFFSET 18

	static char *choices[] = {
		OPTIONS_TEXT_7,
		OPTIONS_TEXT_8,
		OPTIONS_TEXT_9,
		OPTIONS_TEXT_10,
		OPTIONS_TEXT_53 };

	int j=1,flg=0;
	char buffer[30];

	for (;;) {

		strcpy (choices[0]+OFFSET,gcvt(options.accuracy,3,buffer));
		strcpy (choices[1]+OFFSET,gcvt(options.sigfcy,3,buffer));
		strcpy (choices[2]+OFFSET,itoa(options.Cchain,buffer,10));
		strcpy (choices[3]+OFFSET,itoa(options.mpex,buffer,10));
		strcpy (choices[4]+OFFSET,gcvt(options.spread,3,buffer));

		j=wmenu (48,12,28,5,1,choices,NULL,18,19,20,21,0,0,
			OPTIONS_TEXT_11,flg,j,118,NULL,NULL);

		if (j==0) return;
		flg|=MF_blk;
		switch (j) {
		case 1:
			ask_accuracy();
			break;
		case 2:
			options.sigfcy=wgfloat(45,15,OPTIONS_TEXT_12,
					123,22,23,24,options.sigfcy);
			break;
		case 3:
			options.Cchain=wgint(48,16,OPTIONS_TEXT_13,
					124,22,23,24,options.Cchain);
			break;
		case 4:
			options.mpex=wgint(48,17,OPTIONS_TEXT_14,
					125,22,23,24,options.mpex);
			break;
		case 5:
			options.spread=wgfloat(45,18,OPTIONS_TEXT_54,
					134,22,23,24,options.spread);
			break; } }
}

void print_options (void)

{
	#define OFFSET 21

	static char *choices[] = {
		OPTIONS_TEXT_15,			/* 0 */
		OPTIONS_TEXT_16,			/* 1 */
		OPTIONS_TEXT_17,			/* 2 */
		OPTIONS_TEXT_18,			/* 3 */
		OPTIONS_TEXT_19,			/* 4 */
		OPTIONS_TEXT_20,			/* 5 */
		OPTIONS_TEXT_21, 			/* 6 */
		OPTIONS_TEXT_22, };		/* 7 */

	static char *pitch[]   = { OPTIONS_TEXT_23,OPTIONS_TEXT_24,OPTIONS_TEXT_25};
	static char *spacing[] = { OPTIONS_TEXT_26,OPTIONS_TEXT_27 };
	static char *width[]   = { OPTIONS_TEXT_28,OPTIONS_TEXT_29 };
	static char *heigth[]  = { OPTIONS_TEXT_30,OPTIONS_TEXT_31 };
	static char *paper[]   = { OPTIONS_TEXT_32,OPTIONS_TEXT_33 };
	static char *listcm[]  = { OPTIONS_TEXT_34,OPTIONS_TEXT_35 };
	static char *prtcom[]  = { OPTIONS_TEXT_36,OPTIONS_TEXT_37 };


	int j=1, flg=0;

	for (;;) {

		strcpy (choices[0]+OFFSET,pitch  [options.print.pitch]);
		strcpy (choices[1]+OFFSET,spacing[options.print.space]);
		strcpy (choices[2]+OFFSET,width  [options.print.width]);
		strcpy (choices[3]+OFFSET,heigth [options.print.heigth]);
		strcpy (choices[4]+OFFSET,paper  [options.print.wait]);
		strcpy (choices[5]+OFFSET,listcm [options.print.list]);
		strcpy (choices[6]+OFFSET,prtcom [options.print.all]);
		strcpy (choices[7]+OFFSET,options.prt_file);

		j=wmenu (40,13,36,8,1,choices,NULL,18,19,20,21,0,0,
			OPTIONS_TEXT_38,flg,j,109,NULL,NULL);

		if (j==0) return;
		flg|=MF_blk;
		switch (j) {
		case 1:
			options.print.pitch=++options.print.pitch%3;
			break;
		case 2:
			options.print.space++;
			break;
		case 3:
			options.print.width++;
			break;
		case 4:
			options.print.heigth++;
			break;
		case 5:
			options.print.wait++;
			break;
		case 6:
			options.print.list++;
			break;
		case 7:
			options.print.all++;
			break;
		case 8:
			winput(43,22,OPTIONS_TEXT_39,0,117,
							22,23,24,options.prt_file,32,wdummy_);
			strupr(options.prt_file);
			if (strlen(options.prt_file)==0)
				strcpy(options.prt_file,"PRN");
			break; } }
}

static void environment_options (void)

{
	#define OFFSET 14

	static char *choices[] = {
		OPTIONS_TEXT_40 };

	static char *snd[] = { OPTIONS_TEXT_41,OPTIONS_TEXT_42 };

	int j=1,flg=0;

	for (;;) {

		strcpy (choices[0]+OFFSET,snd[options.env.sound]);

		j=wmenu (50,14,22,1,1,choices,NULL,18,19,20,21,0,0,
			OPTIONS_TEXT_43,flg,j,131,NULL,NULL);

		if (j==0) return;
		flg|=MF_blk;
		switch (j) {
		case 1:
			options.env.sound++;
			break; } }
}

static int open_options (void)

{
	int handle,pages_no,page_len;

	handle=_open(ch_exe,O_RDWR);
	if (handle>0) {
		lseek(handle,2l,SEEK_SET);
		_read(handle,&page_len,2);
		_read(handle,&pages_no,2);
		lseek(handle,512l*(pages_no-1)+page_len,SEEK_SET); }
	else
		tell_error(OPTIONS_TEXT_44,130);
	return(handle);
}

void read_options (Bool err)

{
	int handle;
	unsigned long sign;
	char buffer[200];

	if ((handle=open_options())>0)
		if (_read(handle,&sign,4)<4||sign!=0xACCE55ED||
			_read(handle,buffer,sizeof(options))<sizeof(options)) {
				if (err) tell_error(OPTIONS_TEXT_45,128); }
		else
			memcpy(&options,buffer,sizeof(options));
	_close(handle);
}

static void write_options (void)

{
	int handle;
	unsigned long sign = 0xACCE55ED;

	if ((handle=open_options())>0)
		if (_write(handle,&sign,4)<4||
			_write(handle,&options,sizeof(options))<sizeof(options))
				tell_error(OPTIONS_TEXT_46,129);
	_close (handle);
}

void options_service (void)

{
	static char *choices[] = {
		OPTIONS_TEXT_47,
		OPTIONS_TEXT_48,
		OPTIONS_TEXT_49,
		OPTIONS_TEXT_50,
		OPTIONS_TEXT_51 };

	int j;
	static int pos=1;

	if (pos<0) pos=-pos;
	for (;;) {
		j=wmenu (64,10,OPTIONS_INT_1,5,1,choices,NULL,12,13,14,15,0,0,OPTIONS_TEXT_52,
					0,pos,104,NULL,NULL);

		if (j==0) return;
		pos=-j;

		switch (j) {
			case 1: solve_options ();       break;
			case 2: print_options ();       break;
			case 3: environment_options (); break;
			case 4: write_options ();       break;
			case 5: read_options (true);    break; } }
}
