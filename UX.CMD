/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal System/2  */
/*                                                                   */
/*    **  Utility functions and development support routine  **      */
/*    (for use with C-Set2)                                          */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
Trace Err
Arg func arg2 arg3
Address CMD

If func = 'CV' Then Do
  'ipmd mlabpm 35 20 15 10 10 10 10 10' arg2 '/P=D:\MLABPM\SONGLIB'
End

Else If func = 'D' Then Do
  'ICC /Q /C /Sms /Gd- /W3 /Gm /Gs- /Ge /Kb /Ti  JLB'arg2'.C'
  If rc = 0 Then Do
    'LINK386 /DEBUG /NOD /NOE /NOI /ALIGN:4 /BASE:0x10000 @MLABPM.LNK'
    'rc mlabpm.res'
  End
End

Else If func = 'DD' Then Do
  'ICC /Q /C /Ss /Gd+ /W1 /Gm+ /Gs+ /Se /Ge- /Re /Kf+ /Ti JLB'arg2'.C'
  If rc = 0 Then Do
    'LINK386 /NOLOGO /DEBUG /NOE /NOD /NOI /ALIGN:4 /BASE:0x10000 /EXEPACK @MLABDLL0.LNK'
  End
End

Else If func = 'M' Then Do
  Status = Stream('MLABPM.INI','C','Open Write')
  if Status <> "READY:" Then Do
    Call Beep 300, 200
    Say "** Take MidiLab/2 down first **"
    Exit
  End
  Else xx = Stream('MLABPM.INI','C','Close')

  if arg2 = 'D' Then
    'nmake /nologo /f mlabpm.mak'
  Else
    'nmake /nologo /f mlabpm.mak GEN=NDBUG'
End

Else If func = 'E' Then Do
  if length(arg2) <5 Then
  'e MLABPM.'arg2
  Else 'e JLB'arg2'.c'
End

Else If func = 'B' Then Do
  'fvp JLB'arg2'.c'
End

Else If func = 'C' Then Do
  'icc.exe /Weffgenlanobsparprorearetuse /Ti+ /O- /Sms /Q /N10 /Gm+ /Gd- /Ge+ /Gf+ /Gh- /G4 /Gs+ /Gn+ /C JLB'arg2'.c'
End

Else If func = 'L' Then Do
  'LINK386 'arg2 arg3' /DE /NOLOGO /M:FULL /NOD /NOE /NOI /ALIGN:4 /BASE:0x10000 /EXEPACK @MLABPM.LNK'
  'rc mlabpm.res'
End

Else If func = 'LD' Then Do
  'LINK386 'arg2 arg3' /NOLOGO /NOE /NOD /NOI /ALIGN:4 /BASE:0x10000 /EXEPACK @MLABDLL0.LNK'
End

Else If func = 'F' Then Do
   'GREP -p "'arg2'" *.C *.H *.RC *.RCH *.DLG'
End

Else If func = 'MD' Then Do
  'NMAKE -NOLOGO -F ' ARG2 'MLABDLL0.MAK'
End

Else If func = 'RC' Then Do
  'RC -R MLABPM'
  'RC MLABPM.RES'
End

Else If func = 'CNT' Then Do
  'DIR /F *.* | SLOCNT > SRCCNT.TXT'
  Say "Data stored in SRCCNT.TXT"
End

Else If func = 'S' Then Do
  swt = mlab   /* Force it this way */
  If swt = mlab Then Do
    "E c:\config.sys '/DEVICE=\MPUDRVR\MPU401DD.SYS I=5/"
    "E c:\windows\system.ini '/MIDI1=rdmpu401.drv/"
     Exit
  End
  Else if swt = win Then Do
    "E c:\config.sys 'c /DEVICE=\MLABPM\MPU401DD.SYS I=5/rem DEVICE=\MLABPM\MPU401DD.SYS I=5/*'       "
    "E c:\os2\mdos\winos2\system.ini 'c /;MIDI1=rdmpu401.drv/MIDI1=rdmpu401.drv/*'       "
  End
End

Else Do
  Say "C: Compile   D: Debug EXE   E: Edit   L: Link       M: Make EXE"
  Say "B: Browse    F: Grep       CV: Debug MD: Make DLL  RC: Resource Compile"
  Say "CNT: Count source lines  S: Edit configs"
End

Exit
