.c.obj:
 @msg compiling $< ...
 @tcc $<

.asm.obj:
 @msg Assembling $< ...
 @tasm /zi $<

ch.exe: graphics.obj wserv.obj chtest.obj mainm.obj base.obj \
        auxi.obj edvideo.obj editask.obj file.obj solve.obj \
        kernel.obj output.obj options.obj \
        wserva.obj loadfnt.obj cgadrv.obj egadrv.obj

 @msg Linking CH.EXE ...
 @tlink /c/v/x @chem.lnk,ch.exe,,c:\tc\lib\emu+c:\tc\lib\mathl+c:\tc\lib\cl+c:\tc\lib\graphics

graphics.obj: graphics.c
wserv.obj: wserv.c
chtest.obj: chtest.c
mainm.obj: mainm.c
base.obj: base.c
auxi.obj: auxi.c
edvideo.obj: edvideo.c
editask.obj: editask.c
file.obj: file.c
solve.obj: solve.c
kernel.obj: kernel.c
output.obj: output.c
options.obj: options.c

wserva.obj: wserva.asm
loadfnt.obj: loadfnt.asm

cgadrv.obj: c:\tc\bgi\cga.bgi
 @msg Converting CGA.BGI ...
 @bgiobj c:\tc\bgi\cga.bgi cgadrv _CGA_driver CGADRV_TEXT

egadrv.obj: c:\tc\bgi\egavga.bgi
 @msg Converting EGA.BGI ...
 @bgiobj c:\tc\bgi\egavga.bgi egadrv _EGAVGA_driver EGADRV_TEXT
