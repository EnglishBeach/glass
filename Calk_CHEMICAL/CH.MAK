#		*Translator Definitions*
CC = bcc +CH.CFG
TASM = TASM
TLINK = tlink


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
  chtest.obj \

#		*Explicit Rules*
ch.exe: ch.cfg $(EXE_dependencies)
  $(TLINK) /v/x/c/P-/LC:\BORLANDC\LIB @&&|
c0l.obj+
auxi.obj+
base.obj+
chtest.obj+
editask.obj+
edvideo.obj+
file.obj+
graphics.obj+
kernel.obj+
mainm.obj+
options.obj+
output.obj+
solve.obj+
wserv.obj+
cgadrv.obj+
egadrv.obj+
wserva.obj+
loadfnt.obj
ch
		# no map file
emu.lib+
mathl.lib+
cl.lib+
graphics.lib
|


#		*Individual File Dependencies*
auxi.obj: auxi.c 

base.obj: base.c 

chtest.obj: chtest.c 

editask.obj: editask.c 

edvideo.obj: edvideo.c 

file.obj: file.c 

graphics.obj: graphics.c 

kernel.obj: kernel.c 

mainm.obj: mainm.c 

options.obj: options.c 

output.obj: output.c 

solve.obj: solve.c 

wserv.obj: wserv.c 

#		*Compiler Configuration File*
ch.cfg: ch.mak
  copy &&|
-ml
-1
-K
-v
-y
-IC:\BORLANDC\INCLUDE;.
-LC:\BORLANDC\LIB
-P-.C
| ch.cfg


