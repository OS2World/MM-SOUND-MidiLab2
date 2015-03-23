/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBSYSX0 -    ** Bulk Dump/Restore routine **                   */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
VOID sysxchk (VOID);

MRESULT EXPENTRY SYSEXMsgProc(HWND hWndDlg,
                              USHORT message, MPARAM mp1, MPARAM mp2)
{
  APIRET rc;
  static SHORT dumpreq;
  static HWND  hWndParent;
  LONG   i, j;
  ULONG  PostCt;
  ULONG ParmLnth, DataLnth;

/**********************************************************************/
/*   Device-specific dump request sequences for a few products (mine) */
/**********************************************************************/
  static BYTE
    Ids_NONE[1]    = {EOX},
    Ids_MX_8[7]    = {0xf0,0x00,0x00,0x07,0x09,0x03,EOX},
    Ids_SYNCMAN[6] = {0xf0,0x00,0x00,0x2f,0x01,EOX},
    Ids_DX7[1]     = {EOX},

    Ids_TX816[5]   = {0xf0,0x43,0x20,0x09,EOX}, // 0x20 - set TF1 to chan 1
  // To dump a TF1, set the unit to channel 1, select an out-slot, and make sure
  // the MIDI OUT can reach the computer. Otherwise it won't work.

    Ids_HR16[1]    = {EOX},
    Ids_TX81Z[1]   = {EOX},
    Ids_SPX90[1]   = {EOX},
    Ids_U220[1]    = {EOX},
    dbyte;

  PBYTE DevReq[] = {Ids_NONE, Ids_MX_8, Ids_SYNCMAN, Ids_DX7, Ids_TX816,
                    Ids_HR16, Ids_TX81Z, Ids_SPX90,  Ids_U220};

  switch(message)
   {
    case WM_INITDLG:
         hWndParent = (HWND)mp2;
         cwCenter(hWndDlg, (HWND)hWndParent);
         WinPostMsg(hWndDlg, UM_DLGINIT, 0L, 0L);    // Signal init routine
         break; /* End of WM_INITDLG */

/**********************************************************************/
/*       Initialize the controls within the Dialog Box                */
/**********************************************************************/
    case UM_DLGINIT:
/***********************************************************************/
/*          Load SysEx dump request list box                           */
/***********************************************************************/
      mresReply = WinSendDlgItemMsg(hWndSYSEX, 256,
                         LM_QUERYITEMCOUNT, NULL, NULL);
      if (mresReply == NULL)
      {
        for ( i = 0; i < 9; i++ )
        {
          WinLoadString( hAB,            // Load device type for dump req
                         0,
                         IDS_NONE + i,
                         8,              //Length of box entry
                         (PSZ)szChars );

          WinSendDlgItemMsg( hWndSYSEX,   // Add entry
                             256,
                             LM_INSERTITEM,
                             MPFROM2SHORT( LIT_END, 0 ),
                             MPFROMP(szChars) );
        }
      }
      WinSendDlgItemMsg(hWndSYSEX, 256, LM_SELECTITEM, MPFROMSHORT(0),  MPFROMSHORT(TRUE)); //Preselect none

         break; /* End of WM_UINIT */

    case WM_CONTROL:
         switch(SHORT1FROMMP(mp1))
           {
            case 256: /* Dump request List box                                          */
                 switch(SHORT2FROMMP(mp1)) /* switch on Notification Code  */
                   {
                    case LN_SELECT:    /* Field is being selected    */
                         dumpreq = SHORT1FROMMR(WinSendDlgItemMsg(hWndSYSEX,
                                    256, LM_QUERYSELECTION, 0L, 0L ) );
                         break;

                    default: /* Default other messages                     */
                         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
                   }
                 break;
           }
         break; /* End of WM_CONTROL */

    case WM_COMMAND:
                WinQueryWindowText(WinWindowFromID(hWndDlg, 260), 4, szChars);
                i = atoi(szChars);
                if (!chk_range(i, "Track", 1, 16)) break;
                else rtrack = --i;

         switch(SHORT1FROMMP(mp1))
           {
            case 257: /* Button text: "Send"                               */
                if ((BYTE)*(RT) != SYSX)
                  umsg('E', hWndDlg, "Track does not contain SysEx data");
                else
                {
                  WinSetWindowText(hWndSYSEX, "...sending");
                  PtrBusy(ON);
                  j = 0;
                  while (j < RTL-2)         // Send one or more sysx msgs
                  {
                    for (i=0; (BYTE)*(RT+j+i) != EOX; ++i)  // Find lnth-1
                      ;
                    ++i;                    // Incr to actual length
                    mpucmd(0xdf,0);         // Issue want-to-send-sysx msg

                    IOCParms.ulBufrLn = i;  // Store data length
                    ParmLnth = sizeof(IOCPARMS);
                    DataLnth = i;
                    rc = DosDevIOCtl(mpu_hnd, MlabCat, SEND_SYSX, &IOCParms,
                                     sizeof(IOCPARMS), &ParmLnth,
                                     RT + j,  i, &DataLnth);
                    if (rc)
                    {
                      sprintf(msgbfr, "SEND_SYSX Error  RC:%X", rc);
                      umsg('E', hWndClient, msgbfr);
                    }

                    j += i;                 // Point to next msg, if any
                    Delay(5);               // i486 delay (4/8/94)
                  }
                  sprintf(msgbfr,"%u bytes of SysEx data sent", j);
                  WinSetWindowText(hWndSYSEX, msgbfr);
                  DosBeep(75,35);           // Comforting assurance
                  PtrBusy(OFF);
                }
                break;

            case 258: /* Button text: "Recv"                               */
                 if (chkact() || standby) break; /* Chk if ok to do now*/
                 standby = ON;
                 actvfunc = SYSX;
                 RTI = 0;                  // Reset track index
                 ndxwrt(rtrack, CLR_RED);  // Go display index
                 mpucmd(0x97,0);           // MPU Sysx-to-host: on
                 DosResetEventSem(SemSysx, &PostCt); // Set initial block
                 DosCreateThread((PTID)&tid2, (PFNTHREAD)sysxchk, (ULONG)0,
                                     (ULONG)0, (ULONG)STKSZ);

        /*  Send dump request from preset sequences if available  */
                 if (*DevReq[dumpreq] != EOX)   // Is request sequence available?
                 {
                   i = 0;                       // Increment index
                   mpucmd(0xdf,0);              // MPU Want_to_send_Sysx
                   IOCParms.ulBufrLn = 1;       // Store data length
                   ParmLnth = sizeof(IOCPARMS);
                   DataLnth = 1;
                   do
                   {
                     dbyte = *(DevReq[dumpreq] + i);
                     rc = DosDevIOCtl(mpu_hnd, MlabCat, SEND_SYSX, &IOCParms,
                                      sizeof(IOCPARMS), &ParmLnth,
                                      &dbyte, 1, &DataLnth);
                     if (rc)
                     {
                       sprintf(msgbfr, "SEND_SYSX Error  RC:%X", rc);
                       umsg('E', hWndClient, msgbfr);
                     }
                     ++i;
                   }
                   while (dbyte != EOX);
                 }
                 WinSetWindowText(hWndSYSEX, "waiting for SysEx data...");
                 break;

            case DID_CANCEL: /* Button text: "Cancel"                      */
                 Esc_cancel();
                 WinDestroyWindow(hWndDlg);
                 hWndSYSEX = 0;
                 break;
           }
         break; /* End of WM_COMMAND */

    case WM_CLOSE:
         WinPostMsg(hWndDlg, WM_COMMAND, MPFROMSHORT(DID_CANCEL), 0L);
         break; /* End of WM_CLOSE */

    case WM_FAILEDVALIDATE:
         WinAlarm(HWND_DESKTOP, WA_ERROR);
         WinSetFocus(HWND_DESKTOP, WinWindowFromID(hWndDlg, SHORT1FROMMP(mp1)));
         return((MRESULT)TRUE);

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
 return FALSE;
} /* End of SYSEXMsgProc */

/*********************************************************************/
/*      Generic routine to receive bulk sysx transmissions           */
/*         Function:                                                 */
/*           LEVEL1 = Receive SYSX data (called from MAIN0)          */
/*********************************************************************/
void proc_sysx(BYTE function)
{
  if (function == LEVEL1)
  {
    WinSetWindowText(hWndSYSEX, "...receiving");
    *(RT + RTI++) = SYSX;                 /* Set $F0 as 1st byte     */

    do
    {
      indata = mpugetd();                 /* Read input byte from MPU*/
      if (RTI < (ini.trksize[rtrack]-8))
        *(RT + RTI++) = indata;           /* Store in track          */
      else trackfull = ON;                /* Out of space in track   */
    }
    while (indata != EOX);                /* If EOX, end of this one */

    if (!trackfull)                       /* No track overrun        */
    {
      RTL = RTI;                          /* Set track data length   */
      ndxwrt(rtrack, CLR_RED);            /* Display current index   */
    }
    safequit = ON;                        /* To guard against loss   */
    DosPostEventSem(SemSysx);             /* Let thread checker go   */
  }
  else do indata = mpugetd();             /* Discard sysx data       */
    while (indata != EOX);                /*                         */
}

/*********************************************************************/
/*         Thread to check for and process SYSX completion           */
/*********************************************************************/
VOID sysxchk (VOID)
{
  LONG sysxndx = -1;
  ULONG PostCt;

  while (sysxndx < RTI)                     // If equal, no more incoming
  {
    DosWaitEventSem(SemSysx, -1);           // Wait for post from EOX
    DosResetEventSem(SemSysx, &PostCt);     // Re-set Semaphore
    sysxndx = RTI;                          // Get current value of RTI
    Delay(500);                             // Wait 1/2 second
  }
  WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)255, NULL);
}
