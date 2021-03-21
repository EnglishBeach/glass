#include	<stdlib.h>
//#include <keys.h>
#define Alt_X 301
#include "chem.h"
#include "wserv.h"

/*
ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤท
ณ                                                                 บ
ณ Main  menu.  Draws  out  a  Copyright box, then loops in a main บ
ณ menu.                                                           บ
ณ                                                                 บ
ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

void main_menu (void)

{
	static char *copyright[] = {
		"  CHEMICAL",
		"Version 1.10",
		"",
		" Copyright",
		"     by",
		"  Green",
		"    Arrow","",
		"  (c) 1990",
		"  üý 1990" };

	static char *choices[] = { "",MAINM_TEXT_1,"",MAINM_TEXT_2,"",MAINM_TEXT_3,
								"",MAINM_TEXT_4,"",MAINM_TEXT_5,"" } ;
	static int disa[11] = {1,0,1,0,1,0,1,0,1,0,1} ;
	int j,k,init;
	char *p;

	wblock (67,14,14,11,6,NULL,1);
	wavoid ();
	wattr (7);
	for (j=0;j<9;j++) {
		p=wgeta(68,15+j);
		if (j==8&&video_card!=0) j++;
		wputs (&p,copyright[j]); }
	file_new ();
	edit_init ();
	file_new ();

	init = 4;

	for (;;) {
		j=wmenu (67,1,10,11,1,choices,disa,1,2,0,3,2,
			0,MAINM_TEXT_6,MF_avo|MF_esc|MF_akb,init,10,NULL,NULL);

		if ((k=(j>>7)&0x1FF)==0) {

			init=-j;
			switch (j) {
				case 2: file_service (); break;
				case 4: edit_service (); init=j; break;
				case 6: solve_service (); break;
				case 8: options_service (); break;
				case 10: base_service (); break; } }
		else {
			if (k==Alt_X)
				leave_Chemical();
			init=-(j&0x7F); } }
}
