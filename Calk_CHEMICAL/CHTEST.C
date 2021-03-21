#define MAIN

#include "chem.h"
#include "wserv.h"
#include <alloc.h>
#include <dos.h>
#include <string.h>
#include <graphics.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>

main_menu();

int wcolors[] =   {  0x0F,0x0F,0x0B,0x5F,0x0C,   /*  0 */
							0x0B,0x0F,0x09,0x4F,0x4F,
							0x0F,0x0A,0x0B,0x0C,0x0A,   /* 10 */
							0x0B,0x0E,0x0F,0x0F,0x0E,
							0x0D,0x0B,0x0B,0x0E,0x0D,   /* 20 */
							0x1E,0x1F,0x0E,0x11,0x0F,
							0x09,0x0A,0x1F,0x71,0x17,   /* 30 */
							0x07,0x0F,0x0B,0x0B,0x0D,
							0x00,0x70,0x0F,0x09,0x4F,   /* 40 */
							0x4F,0x0F,0x44,0x4B,0x1E,
							0x4F,0x4F,0x0F,0x1E };

static int bwcol[]={ 0x0F,0x07,0x07,0x70,0x0F,
							0x0F,0x07,0x07,0x0F,0x0F,
							0x0F,0x07,0x70,0x70,0x70,
							0x0F,0x0F,0x07,0x70,0x70,
							0x70,0x0F,0x07,0x0F,0x07,
							0x07,0x0F,0x70,0x00,0x0F,
							0x0F,0x07,0x0F,0x70,0x07,
							0x07,0x70,0x70,0x70,0x07,
							0x77,0x07,0x70,0x70,0x70,
							0x70,0x0F,0x77,0x0F,0x70,
							0x07,0x07,0x70,0x0F };

static int nul_handle;
task ctask;
optn options = {

				/* Solve options */

	1e-6,					/* Concentration significancy level */
	1e-6,					/* Defaul accuracy in solving system */
	5, 					/* Default initial estimate spread factor */
	35,					/* Maximum iteration may be made */
	6,						/* Maximal length of C-chain for CnHm compounds */
	6,						/* Number of additional points */

				/* Environment options */

	{ 1 					/* Sound on */
	},

				/* Print options */
	{
		0,					/* 0=Elite 10 cpi, 1=Pica 12 cpi, 2=NLQ 10 cpi */
		0,					/* 0=8 inches, 1=13 inches */
		0,					/* 0=print significant, 1=print all */
		0,					/* 0=do print list of comps, 1=don't */
		0,					/* 0=12 inches, 1=11 inches */
		0,					/* 0=don't wait after page, 1=do */
		0					/* 0=6 lpi, 1=8 lpi */
	},
	"PRN"					/* Default print file */


};


char chem_dir[82];
char ch_exe[82];
int video_card = 1;

static void logo_screen (void)

{
	char *p,*q;
	int i;
	static char logo[] = {

"ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"
"³                ÚÄ¿ÚÄ¿ÚÄ¿ÚÄ¿Ú¿ Ú        ÚÄ¿ÚÄ¿ÚÄ¿ÚÄ¿¿  Ú                      ³"
"³                ³ ¿ÃÂÙÃ´ Ã´ ³À¿³        ÃÄ´ÃÂÙÃÂÙ³ ³³Ú¿³                      ³"
"³                ÀÄÙÙÀÙÀÄÙÀÄÙÙ ÀÙ        Ù ÀÙÀÙÙÀÙÀÄÙÀÙÀÙ                      ³"
"³                                                                              ³"
"³                             P R E S E N T S                                  ³"
"³                                                                              ³"
"³              ËÍÍ» »   É ËÍÍÍ» ËÍ» ÉÍË ÍËÍ ËÍÍ» ËÍÍÍË Ë   TM                  ³"
"³              º    º   º º     º º º º  º  º    º   º º                       ³"
"³              º    ÌÍÍÍ¹ ÌÍ¹   º º º º  º  º    ÌÍÍÍ¹ º                       ³"
"³              º    º   º º     º ÈÍ¼ º  º  º    º   º º   »                   ³"
"³              ÊÍÍ¼ ¼   È ÊÍÍÍ¼ ¼     È ÍÊÍ ÊÍÍ¼ ¼   È ÊÍÍÍ¼                   ³"
"³                                                                              ³"
"³ ÚÂ¿¿ ÚÚÄ¿ÚÄ¿Ú¿Ú¿ÚÄ¿ÂÄ¿¿ ÚÚ¿ ÚÚÄ¿Ú¿Ú¿ÂÚÄ¿ÚÄ¿   ÚÄ¿ÚÄ¿Â  ÚÄ¿¿ ÚÂ  ÚÄ¿ÚÂ¿ÚÄ¿ÚÄ¿ ³"
"³  ³ ÃÄ´Ã´ ÃÂÙ³ÀÙ³³ ³³ ³ÀÂÙ³À¿³ÃÄ´³ÀÙ³³³  ÀÄ¿   ³  ÃÄ´³  ³  ³ ³³  ÃÄ´ ³ ³ ³ÃÂÙ ³"
"³  Á Ù ÀÀÄÙÙÀÙÙ  ÀÀÄÙÁÄÙ Á Ù ÀÙÙ ÀÙ  ÀÁÀÄÙÀÄÙ   ÀÄÙÙ ÀÀÄÙÀÄÙÀÄÙÀÄÙÙ À Á ÀÄÙÙÀÙ ³"
"³                                                                              ³"
"³         Project manager &      Programmings &        Extensive               ³"
"³         algorithmization       documentation          testing                ³"
"³                                                                              ³"
"³           Kutyin A.M.          Katsnelson K.M      Medvetskaya W.Y.          ³"
"³                                                                              ³"
"³                                                                              ³"
"³                             Version 1.10 (C) 1990                            ³"
"ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ"
	};


	p=wgeta(1,1); q=logo;

	for (i=0;i<2000;i++) { *p++=*q++; *p++=wcolors[53]; }
	wgetkey(KEYONLY);
}

static void check_video_card (void)

{
	int drv=DETECT,mod;

	registerbgidriver(EGAVGA_driver);
	registerbgidriver(CGA_driver);

	detectgraph(&drv,&mod);

	switch (drv) {
	case CGA:
		video_card = 0;
		break;
	case EGA:
		video_card = 1;
		break;
	case VGA:
		video_card = 2;
		break;
	default:
		exit(12); }
}

static int get_video_mode (void)

{
	union REGS reg;

	reg.h.ah=0x0F;
	int86(0x10,&reg,&reg);
	return (reg.h.al);
}

void redirect_stdout (Bool to_nul /*else to stderr, ie always CON*/)

{
	union REGS reg;

	reg.h.ah=0x46;
	reg.x.bx=to_nul?nul_handle:2;
	reg.x.cx=1;
	int86(0x21,&reg,&reg);
}

#pragma warn -par

void main (int argc, char **argv)

{
	int i;
	char *p,hlp[65];
	static char notice_text[] = NOTICE_TEXT;

	nul_handle=_open("NUL",O_WRONLY);
	redirect_stdout(true);

	save_break_state();
	ctrlbrk(ignore_break);

	strcpy(ch_exe,argv[0]);
	strcpy(chem_dir,argv[0]);
	p=chem_dir+strlen(chem_dir);
	while(*--p!='\\');
	*++p=0;

	check_video_card();

	i=get_video_mode();
	if (i==0||i==2)
		memcpy(wcolors,bwcol,sizeof(wcolors));

	delay (100);
	_write(2,notice_text,strlen(notice_text));
	mouse_init();
	wgetkey(KEYONLY);
	video_mode (3);
	load_font(video_card);
	wsetcursor(0x20,0);

	logo_screen();

	for (i=0;i<5;i++)
		ctask.total[i]=0;
	ctask.source=malloc(sizeof(ice));
	ctask.fixed =malloc(sizeof(ice));
	ctask.exclud=malloc(sizeof(ice));

	strcpy(hlp,chem_dir);
	strcat(hlp,HELP_FILE);
	wsethelp (hlp,0,0,4,5);

	read_options (false);

	main_menu();
}
