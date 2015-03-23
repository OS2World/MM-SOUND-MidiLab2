# Created by IBM WorkFrame/2 MakeMake at 10:14:12 on 03/06/95
#
# The actions included in this makefile are:
#   BIND::Resource Bind
#   COMPILE::C Set ++ Compile
#   COMPILE::Resource Compile
#   LINK::Link386

!IF "$(GEN)" == "NDBUG"
copt = /Ti- /O+
lopt =
!ELSE
copt = /Ti+ /O-
lopt = /DEBUG
!ENDIF

.all: \
  .\MLABPM.EXE

.SUFFIXES:

.SUFFIXES: .C .OBJ .RC .RES

.RC.res:
      @echo WF::COMPILE::Resource Compile
      rc.exe -r -X2 %s %|fF.RES

.C.OBJ:
      @echo WF::COMPILE::C Set ++ Compile
      icc.exe /Weffgenlanobsparprorearetuse $(copt) /Sms /Q /N10 /Gm+ /Gd- /Ge+ /Gf+ /Gh- /G4 /Gs+ /Gn+ /C %s

.\MLABPM.EXE: \
    .\JLBCSA00.OBJ \
    .\JLBINIT0.OBJ \
    .\JLBMAIN0.OBJ \
    .\JLBSUBS0.OBJ \
    .\JLBSUBS1.OBJ \
    .\JLBSUBS2.OBJ \
    .\JLBCWIN0.OBJ \
    .\JLBDMGR0.OBJ \
    .\JLBDMGR1.OBJ \
    .\JLBPMIDI.OBJ \
    .\JLBPMID1.OBJ \
    .\JLBRMIDI.OBJ \
    .\JLBGTEV0.OBJ \
    .\JLBFILE0.OBJ \
    .\JLBCTLP0.OBJ \
    .\JLBSYSX0.OBJ \
    .\JLBSPEC0.OBJ \
    .\JLBRCRD0.OBJ \
    .\JLBSCRN0.OBJ \
    .\JLBVIEW0.OBJ \
    .\JLBVIEW1.OBJ \
    .\JLBEDIT0.OBJ \
    .\JLBEDIT1.OBJ \
    .\JLBEDIT2.OBJ \
    .\JLBEDIT3.OBJ \
    .\JLBMPU00.OBJ \
    .\JLBUART0.OBJ \
    .\MLABPM.RES   \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}MMPM2.LIB  \
    {$(LIB)}MLABPM.DEF \
    .\MLABPM.MAK
      @echo WF::LINK::Link386
      link386.exe @<<
       $(lopt) /NOE /NOI /NOD: /W /NOLOGO /FAR /ALIGN:2 /EXEPACK:2 /PACKC /PACKD /BASE:0x10000 +
       .\JLBINIT0.OBJ +
       .\JLBMAIN0.OBJ +
       .\JLBSUBS0.OBJ +
       .\JLBSUBS1.OBJ +
       .\JLBSUBS2.OBJ +
       .\JLBCWIN0.OBJ +
       .\JLBDMGR0.OBJ +
       .\JLBDMGR1.OBJ +
       .\JLBPMIDI.OBJ +
       .\JLBPMID1.OBJ +
       .\JLBRMIDI.OBJ +
       .\JLBGTEV0.OBJ +
       .\JLBFILE0.OBJ +
       .\JLBCTLP0.OBJ +
       .\JLBSYSX0.OBJ +
       .\JLBSPEC0.OBJ +
       .\JLBRCRD0.OBJ +
       .\JLBSCRN0.OBJ +
       .\JLBVIEW0.OBJ +
       .\JLBVIEW1.OBJ +
       .\JLBEDIT0.OBJ +
       .\JLBEDIT1.OBJ +
       .\JLBEDIT2.OBJ +
       .\JLBEDIT3.OBJ +
       .\JLBMPU00.OBJ +
       .\JLBUART0.OBJ +
       .\JLBCSA00.OBJ
        MLABPM.EXE
        MLABPM.MAP
        DDE4MBS +
        OS2386  +
        MMPM2
        MLABPM.DEF;
<<
      @echo WF::BIND::Resource Bind
      rc.exe -X2 .\MLABPM.RES MLABPM.EXE

!include mlabpm.Dep
