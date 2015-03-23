/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal System/2  */
/*                                                                   */
/*             JLBINIT0 -  ** SYSTEM INITIALIZATION **               */
/*                                                                   */
/*             Copyright  J. L. Bell        March 1987               */
/*                                                                   */
/* 08/08/95 Implement High Resolution Timer                          */
/* 12/27/96 Clean up Control Data for Graphic Pushbuttons            */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
#include "tmr0_ioc.h"

PGBTNCDATA pMnBtn[8];                // Main pushbuttons

int main(int argc, char *argv[])
{
  char *devnam;
  static CHAR szTitle[22];

  APIRET rc;
  ULONG  oflag = 1, omode = 0x2012;               // MPU driver Open parms
  ULONG  INIoflgs = 0x11, INImode = 0x31a2;       // MLABPM.INI Open parms

  ULONG  ulHRTOFlgs = OPEN_ACTION_OPEN_IF_EXISTS;
  ULONG  ulHRTOMode = OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE;

  ULONG  ulResolution = 1;
  ULONG  ulSize1=sizeof(ulResolution);

  ULONG  * _Seg16 pulTimer16;
  ULONG  ulSize2=sizeof(pulTimer16);

  PSZ BtnNmes[8] = {"Sys Reset", "Notes Off", "Re-Take", "Overdub",
                    "Record", "Pause", "FastFwd", "Play"};
  HMTX   hmtx;         // System interlock semaphore
  QMSG   qmsg;         // Message queue
  PID    pid;          // Process identifier for adding name to switch list
  SHORT  i;
  extern HMTX  SemPutd;

/*********************************************************************/
/*    Initialize Presentation Manager and make sure there is not     */
/*    is not another MidiLab process somewhere in the system         */
/*********************************************************************/
  if((hAB = WinInitialize(0)) == 0)           /* Initialize PM       */
    return(1);

  if((hMQ = WinCreateMsgQueue(hAB, 100)) == 0)  /* Set up PM msg queue */
    return(1);

  rc = DosCreateMutexSem("\\SEM32\\MLABLOCK.SEM", &hmtx, 0, 0);
  if (rc == 285)
    bagit("MidiLab/2 is already active");

/*********************************************************************/
/* These are not initialized in dmgreset and must be initialized here*/
/*********************************************************************/
  filename = "(Untitled)";                /* Default file name       */
  mpu_hnd = 0xffff;                       /* Dev drvr handle         */
  sysflag2 = QSAVE;                       /* System flag byte #2     */
  clikmult = 1;                           /* Click multiplier 1/4:1/8*/
  mmode = 0x83;                           /* Metronome mode cmd      */
  songpath[0] = '\0';                     /* Set null string         */
  MlabCat = MLAB_CAT;                     /* DevIOCtl category code  */
  max_measures = 1;                       /* Reset measures in song  */
  ini.iniflag1 |= UARTMODE;               /* Force UART mode 8/12/95 */

/*********************************************************************/
/*      Load INI file from current directory.                        */
/*      If it does not exist, create it using system defaults        */
/*********************************************************************/
  ulFileAttribute = 0x20;
  rc = DosOpen("MLABPM.INI", &INIfile, &action, sizeof(ini),
                ulFileAttribute, INIoflgs, INImode, 0L);
  if (rc == 0)
  {
    if (action == 1)                     /* File existed            */
    {
      DosRead(INIfile, &ini, sizeof(ini), &BytesXfrd);
      if (ini.mlab_id != MLAB_FILE_VERNEW) // 3/17/94
      {
        sprintf(msgbfr, "Invalid MLABPM.INI file; erase it and restart");
        bagit(msgbfr);
      }
    }
    else if (action == 2)                /* File did not exist      */
    {
      WinMessageBox(HWND_DESKTOP, hWndClient, "MLABPM.INI not found; using defaults",
                      szAppName, 0, MB_OK | MB_INFORMATION);
      ini.mlab_id =  MLAB_FILE_VERNEW;   // MidiLab identifier
      DosWrite(INIfile, &ini, sizeof(ini), &BytesXfrd); // Write defaults
    }
  }
  else
  {
    sprintf(msgbfr, "DosOpen error on MLABPM.INI  RC:%d", rc);
    bagit(msgbfr);
  }

/*********************************************************************/
/*                       Process input parms                         */
/*********************************************************************/
  for (i = 1; i < argc; i++)               /* Parm scan (cmd line)   */
  {
    if (*argv[i] != '/' && *argv[i] != '-') goto parmerr;

    switch (toupper(*(argv[i]+1)))
    {
      case 'M':                           /* Save files in MIDI fmt */
                sysflag2 &= 255-QSAVE;
                break;
      case 'I':                           /* Enable private goodies */
                iuo = ON;                 /* Set 'internal use only'*/
                break;
      case 'L':                           /* Enable special testing */
                iuo = LEVEL2;
                break;
      case 'P':
                if (strlen(argv[i]+2) > 34) goto parmerr;
                strcpy(songpath, argv[i]+3);
                break;
      case 'X':                           /* Estab external control */
                sp.proflags |= EXTCNTRL;
                break;
      case 'Z':                           /* Simulated MPU          */
                sysflag2 |= NOMPU;
                break;

      default: parmerr:
                sprintf(msgbfr,"Incorrect input parm: %.12s\n",argv[i]);
                bagit(msgbfr);
    }
  }

/*********************************************************************/
/*        Open High-Resolution Timer device driver   8/8/95          */
/*********************************************************************/
  if (sysflag2 & NOMPU)        // If simulating MPU, point to phony timer value
    pulTimer = &MlabCat;
  else                         // Do the real thing
  {
    rc=DosOpen("TIMER0$  ", &HRT_hnd, &action, 0, 0, ulHRTOFlgs, ulHRTOMode, NULL);
    if (rc)
      bagit("Hi-Res Timer cannot be opened");

    rc=DosDevIOCtl(HRT_hnd, HRT_IOCTL_CATEGORY, HRT_SET_RESOLUTION,
                       &ulResolution, ulSize1, &ulSize1, NULL, 0, NULL);
    if (rc)
      bagit("Cannot set Hi-Res Timer resolution");

    rc=DosDevIOCtl(HRT_hnd, HRT_IOCTL_CATEGORY, HRT_GET_POINTER,
                       NULL, 0, NULL, (PULONG)&pulTimer16, ulSize2, &ulSize2);
    if (rc)
      bagit("Cannot get Timer pointer");
    pulTimer = pulTimer16;    // Converts 16:16 ptr to a 0:32 Timer Pointer
  }

/*********************************************************************/
/*                      Open our Device Driver                       */
/* If /Z is not specified, we'll automatically try the MPU; if that  */
/* fails, we'll try the simulated device. Else we'll force simulation*/
/*********************************************************************/
  devnam = (sysflag2 & NOMPU) ? "MPU.SIM" : "MIDILAB$";
  rc = DosOpen(devnam, &mpu_hnd, &action, 0, 0, oflag, omode, 0);
  if (rc)
  {
    sprintf(msgbfr, "Cannot open %.12s  RC:%d,\nwill use simulated MPU",devnam, rc);
    rc=WinMessageBox(HWND_DESKTOP, hWndClient, msgbfr, "MidiLab/2", 0, MB_OKCANCEL | MB_ERROR);
    if (rc == MBID_CANCEL) bagit(NULL);

    rc = DosOpen("MPU.SIM", &mpu_hnd, &action, 0, 0, oflag, omode, 0);
    if (rc)
    {
      sprintf(msgbfr, "Cannot open MPU.SIM, make sure there is a file by that name in the current directory");
      bagit(msgbfr);    // Can't even open the simulation device
    }
    else sysflag2 |= NOMPU;  // Indicate simulated MPU
  }
  else
  {
    if (mpucmd(MPU_RESET, 0) || mpucmd(MPU_UART, 0)) /* Set MPU-401 UART mode*/
      bagit("MIDI hardware is not responding; check for IRQ conflicts, etc.");

/*********************************************************************/
/*               Create all other MidiLab semaphores                 */
/*********************************************************************/
    rc = DosCreateMutexSem("\\SEM32\\MPUPUTD.SEM",  &SemPutd, 0, 0);
    if (rc) goto semerr;

    rc = DosCreateEventSem("\\SEM32\\BCLICK.SEM",   &SemClk,  0, 0);
    if (rc) goto semerr;

    rc = DosCreateEventSem("\\SEM32\\EDITSTEP.SEM", &SemStep, 0, 0);
    if (rc) goto semerr;

    rc = DosCreateEventSem("\\SEM32\\MAINEDIT.SEM", &SemEdit, 0, 0);
    if (rc) goto semerr;

    rc = DosCreateEventSem("\\SEM32\\SYSEX.SEM",    &SemSysx, 0, 0);
    if (rc) semerr:
      bagit("Cannot create MidiLab/2 semaphores");
  }

/*********************************************************************/
/*                 Allocate track data storage                       */
/*********************************************************************/
  for (i = 0; i < NUMTRKS; i++)
  {
    trk[i].trkaddr = malloc(ini.trksize[i]);
    if (trk[i].trkaddr == NULL)
    {
      sprintf(msgbfr,"Cannot allocate storage for track %d\n",i+1);
      bagit(msgbfr);
    }
  }

/*********************************************************************/
/*  Set MidiLab 1.0 file specification standards to external format  */
/*********************************************************************/
  rev_int((char *)&MThd.hdrlnth, 4);    /* Reverse format            */
  rev_int((char *)&MThd.hdrfmt, 2);     /* Reverse format            */
  rev_int((char *)&MThd.hdrntrks, 2);   /* Reverse format            */
  rev_int((char *)&t1.t1_lnth, 4);      /* Reverse format            */

/*********************************************************************/
/*                   Create MPU main loop thread                     */
/*********************************************************************/
  if (!(sysflag2 & NOMPU))               // Bypass if MPU simulation
  {
    DosCreateThread((PTID)&tid1, (PFNTHREAD)mainloop, (ULONG)0,
         (ULONG)0, (ULONG)STKSZ);
    DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, +7, tid1);
  }

/*********************************************************************/
/*    Build Control Structures for Main Window Graphic Pushbuttons   */
/*********************************************************************/
  action = sizeof(GBTNCDATA) + sizeof(USHORT) * 8;
  for (i = 0; i < 8; ++i)                   // Initialize main pushbuttons
  {
    pMnBtn[i] = (PGBTNCDATA)malloc(action);
    pMnBtn[i]->usReserved = GB_STRUCTURE;   // Initialize flags
    pMnBtn[i]->pszText = BtnNmes[i];        // Move in text
    pMnBtn[i]->hmod = NULLHANDLE;           // Group handle (not used)
  }

  pMnBtn[0]->cBitmaps = 1;                  // Number of bitmaps
  pMnBtn[0]->aidBitmap[0] = ID_BMP_RESET;   // Sys Reset bitmap

  pMnBtn[1]->cBitmaps = 1;                  // Number of bitmaps
  pMnBtn[1]->aidBitmap[0] = ID_BMP_NOTESOFF;// Notes-off bitmap

  pMnBtn[2]->cBitmaps = 1;                  // Number of bitmaps
  pMnBtn[2]->aidBitmap[0] = ID_BMP_LOAD;    // Re-take bitmap

  pMnBtn[3]->cBitmaps = 2;                  // Number of bitmaps
  pMnBtn[3]->aidBitmap[0] = ID_BMP_REC1;    // Overdub bitmap
  pMnBtn[3]->aidBitmap[1] = ID_BMP_REC0;

  pMnBtn[4]->cBitmaps = 2;                  // Number of bitmaps
  pMnBtn[4]->aidBitmap[0] = ID_BMP_REC1;    // Record bitmap
  pMnBtn[4]->aidBitmap[1] = ID_BMP_REC0;

  pMnBtn[5]->cBitmaps = 1;                  // Number of bitmaps
  pMnBtn[5]->aidBitmap[0] = ID_BMP_PAUSE;   // Pause bitmap

  pMnBtn[6]->cBitmaps = 2;                  // Number of bitmaps
  pMnBtn[6]->aidBitmap[0] = ID_BMP_FASTF;   // FastFwd bitmap
  pMnBtn[6]->aidBitmap[1] = ID_BMP_FASTF1;

  pMnBtn[7]->cBitmaps = 5;                  // Number of bitmaps
  pMnBtn[7]->aidBitmap[0] = ID_BMP_PLAY0;   // Play bitmap
  pMnBtn[7]->aidBitmap[1] = ID_BMP_PLAY1;
  pMnBtn[7]->aidBitmap[2] = ID_BMP_PLAY2;
  pMnBtn[7]->aidBitmap[3] = ID_BMP_PLAY3;
  pMnBtn[7]->aidBitmap[4] = ID_BMP_PLAY4;

/*********************************************************************/
/*                 Register window classes                           */
/*********************************************************************/
  WinLoadString(hAB, 0, IDS_APP_NAME, 80, szAppName); /* Program name */
  rc = WinRegisterClass(hAB,             /* Anchor block handle             */
                       (PCH)szAppName,   /* Name of class being registered  */
                       (PFNWP)WndProc,   /* Window procedure for class      */
                       CS_SIZEREDRAW | CS_MOVENOTIFY, 1*sizeof(char FAR *));
  if (rc == FALSE) return(FALSE);

  WinLoadString(hAB, 0, IDS_APP_C1_NAME, 80, szMLEDITAppName); /* Program name */
  rc = WinRegisterClass(hAB, (PCH)szMLEDITAppName, (PFNWP)EDITWndProc,
                       CS_SIZEREDRAW | CS_MOVENOTIFY, 0);
  if (rc == FALSE) return(FALSE);

/*********************************************************************/
/*                     Create Main client window                     */
/*********************************************************************/
 /* The CreateWindow function creates a frame window for this application's*/
 /* top window, and set the window's size and location as appropriate.     */

  WinLoadString(hAB, 0, IDS_TITLE, 22, szTitle);
  hWndFrame = cwCreateWindow( (HWND)HWND_DESKTOP,
          FCF_TITLEBAR     | FCF_SYSMENU      | FCF_MINBUTTON    |
          FCF_MAXBUTTON    | FCF_SIZEBORDER   | FCF_MENU         |
          FCF_ICON         | FCF_SHELLPOSITION| FCF_ACCELTABLE,
          szAppName, szTitle, ID_MLAB,
          0,             // Horizontal position - left (x) edge
          0,             // Vertical position   - bottom (y) edge (changed 8/18)
          70,            // Width in chars
          29,            // Height in lines + 3 (approx.)
          &hWndClient, 0L, SWP_SHOW);
  if (hWndFrame == 0) return(1);

 /**************************************************************************/
 /*        Load great big measure indicator and set its position           */
 /**************************************************************************/
  ltime = IDM_O_BIGBARS;     // Tell msg proc who called it
  hWndBIGBARS = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
          (PFNWP)UTILMsgProc, 0, IDLG_BIGMEASURE, &ltime);
  if (!ini.bigbars.cx)       // First time, set SWP
    WinQueryWindowPos(hWndBIGBARS, &ini.bigbars);
  WinSetWindowPos(hWndBIGBARS, HWND_TOP, ini.bigbars.x, ini.bigbars.y,
                  ini.bigbars.cx, ini.bigbars.cy,
                  SWP_MOVE | SWP_SIZE | SWP_ACTIVATE);
  WinSetFocus(HWND_DESKTOP, hWndClient);   // Re-focus on client window

 /**************************************************************************/
 /* Associate Instance of IPF                                              */
 /**************************************************************************/
 if(hWndMLABHelp)
   WinAssociateHelpInstance(hWndMLABHelp, hWndFrame);

 /* The following inline routine fills out the application's switch control*/
 /* structure with the appropriate information to add the application's    */
 /* name to the OS/2 Task Manager List, a list of the jobs currently       */
 /* running on the computer.                                               */

  WinQueryWindowProcess(hWndFrame, &pid, &tid);
  Swctl.hwnd = hWndFrame;                        /* Frame window handle    */
  Swctl.idProcess = pid;                         /* Process identifier     */
  Swctl.uchVisibility = SWL_VISIBLE;             /* visibility             */
  Swctl.fbJump = SWL_JUMPABLE;                   /* Jump indicator         */
  strcpy(Swctl.szSwtitle, "Main Display");       /* Frame window title     */
  hSwitch = WinAddSwitchEntry(&Swctl);

  if (strlen(songpath))                          /* If /P arg supplied     */
  {
    if (songpath[1] == ':')                      /* Was drive specified?   */
    {
      rc = DosSetDefaultDisk(toupper(songpath[0]) - '@'); /* Convert to numeric eq  */
      if (rc)
        bagit("Invalid drive specified for song path");
    }
    rc = DosSetCurrentDir(songpath);             /* Set song file path     */
    if (rc)
    {
      sprintf(msgbfr, "Error setting requested songfile directory  RC:%d", rc);
      bagit(msgbfr);
    }
  }

  dmgreset(255, 0);                              /* Reset everything       */
  sysflag2 |= INITCMPL;                          /* Initialization complete*/

 /**************************************************************************/
 /*     Loop here until the cows come home  (or until termination)         */
 /**************************************************************************/
 while (WinGetMsg(hAB, &qmsg, (HWND)NULL, 0, 0))
 {
   WinDispatchMsg(hAB, &qmsg);
 }

/*********************************************************************/
/*             Terminate MidiLab and return to OS/2                  */
/*********************************************************************/
  if (chngdini == LEVEL2)         /* Save INI file                   */
  {
    DosSetFilePtr(INIfile, 0L, FILE_BEGIN, &action);
    DosWrite(INIfile, &ini, sizeof(ini), &BytesXfrd);
  }

  DosClose(INIfile);              /* Close Init file                 */
  WinDestroyWindow(hWndFrame);    /* Destroy the frame window        */
  WinDestroyMsgQueue(hMQ);        /* Destroy this application's msgq */
  WinTerminate(hAB);              /* Terminate this app's use of PM  */

  for (i = 0; i < NUMTRKS; i++)
    free(trk[i].trkaddr);         /* Return track storage            */
  DosClose(mpu_hnd);              /* Close MPU driver                */
  DosClose(HRT_hnd);              /* Close Hi-Res Timer              */

  DosCloseMutexSem(hmtx);         /* Remove system interlock         */
  DosCloseEventSem(SemStep);      /* Close semaphore                 */
  DosCloseEventSem(SemEdit);      /* Close semaphore                 */
  DosCloseEventSem(SemSysx);      /* Close semaphore                 */
  return(0);                      /* Sayonara, exit MidiLab          */
}

/*********************************************************************/
/*      Errors detected during initialization; go away               */
/*********************************************************************/
void bagit(PSZ message)
{
  if (message != NULL)
  {
    DosBeep(200,100);   // Bronx cheer
    WinMessageBox(HWND_DESKTOP, hWndClient, message,
                     szAppName, 0, MB_CANCEL | MB_ERROR);
  }
  WinDestroyMsgQueue(hMQ);        /* Destroy this application's msgq */
  WinTerminate(hAB);              /* Terminate this app's use of PM  */
  DosExit(EXIT_PROCESS, 8);
}

/*********************************************************************/
/*       Place pushbuttons in client window and Initialize fields    */
/*********************************************************************/
BYTE Set_Client(VOID)
{
  RECTL rcl;
  SHORT i, j;

  ULONG Button_Styles[8] =
  {
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_TWOSTATE  ,// Reset
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_TWOSTATE  ,// Panic
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_TWOSTATE  ,// ReTake
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_ANIMATION ,// Overdub
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_ANIMATION ,// Record
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_TWOSTATE  ,// Pause
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_ANIMATION ,// FstFwd
   WS_TABSTOP | WS_VISIBLE| GBS_3D_TEXTRECESSED | GBS_ANIMATION  // Play
  };

  WinQueryWindowRect(hWndClient, &rcl);
  j = rcl.yTop / 8;                   // Divide height into 8ths
  WinRegisterGraphicButton();         // MMPM/2 goodies

  for (i = 0; i < 8; ++i)             // Build the main window buttons
  {
    hWndButt[i] = WinCreateWindow(hWndClient, WC_GRAPHICBUTTON, "",
      Button_Styles[i], 0, j * i, (SHORT)rcl.xRight / 5, j,   // used to be 8
      hWndClient, HWND_BOTTOM, MN_B0 + i, MPFROMP(pMnBtn[i]), NULL);

      WinSetPresParam(hWndButt[i], PP_FONTNAMESIZE, 22, "10.System Proportional");

/*  if (ini.fontDlg.fAttrs.usRecordLength)  // If we've saved an ini file
      WinSetPresParam(hWndButt[i], PP_FONTNAMESIZE, 22, "10.System Proportional");;
*/
  }
  WinSendMsg(WinWindowFromID(hWndClient, MN_B7), GBM_SETANIMATIONRATE,
              MPFROMLONG(125L), NULL);        // Set 'Play' animation rate
  WinSendMsg(WinWindowFromID(hWndClient, MN_B6), GBM_SETANIMATIONRATE,
              MPFROMLONG(100L), NULL);        // Set 'FFwd' animation rate
  WinSendMsg(WinWindowFromID(hWndClient, MN_B4), GBM_SETANIMATIONRATE,
              MPFROMLONG(325L), NULL);        // Set 'Record' animation rate
  WinSendMsg(WinWindowFromID(hWndClient, MN_B3), GBM_SETANIMATIONRATE,
              MPFROMLONG(350L), NULL);        // Set 'Overdub' animation rate

 /**************************************************************************/
 /*    Place visual indicators across bottom of client window              */
 /**************************************************************************/
  i = rcl.yTop / 15;                       // Divide height into 15ths
  j = rcl.xRight / 8;                      // Divide width into 8ths

  WinSetRect(hAB, &beatrcl,  j*2, 4, j*2+j, i);
  WinSetRect(hAB, &temporcl, j*4, 4, j*4+j, i);
  WinSetRect(hAB, &mctrrcl,  j*6, 4, j*6+j, i);

  WinInflateRect(hAB, &beatrcl,  -cyDesc, -cyDesc);  //Reduce size for border
  WinInflateRect(hAB, &temporcl, -cyDesc, -cyDesc);
  WinInflateRect(hAB, &mctrrcl,  -cyDesc, -cyDesc);
  return(0);
}
