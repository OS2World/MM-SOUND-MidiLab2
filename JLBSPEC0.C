/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*      JLBSPEC0 -    ** User Preference and misc functions **       */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

LONG CallerId;
MRESULT EXPENTRY SPECFUNCMsgProc(HWND hWndDlg,
                                 USHORT message, MPARAM mp1, MPARAM mp2)
{
 ULONG  ulUtility, cntl;
 static HWND  hWndParent;
 static BYTE initSwt;
 BYTE   result;
 SHORT  i;
 static PLONG ClrPtr;
 CHAR   szFamilyname[FACESIZE];
 PULONG caller_id;
 HPS    hPS;
 extern RECTL rClient;         /* Handle to rectangle formed by client area */

 switch(message)
   {
    case WM_INITDLG:
         initSwt = ON;
         hWndParent = (HWND)mp2;
         cwCenter(hWndDlg, (HWND)hWndParent);
         caller_id = PVOIDFROMMP(mp2);               // Get id of active dialog
         CallerId = *caller_id;
         WinPostMsg(hWndDlg, UM_DLGINIT, 0L, 0L);    // Signal init routine
         break; /* End of WM_INITDLG */

/**********************************************************************/
/*       Initialize the controls within the Dialog Box                */
/**********************************************************************/
    case UM_DLGINIT:
      if (CallerId == IDLG_SPECFUNC)          // Processing main dialog
      {
        WinSendDlgItemMsg(hWndSPECFUNC, 263, BM_SETCHECK,   // Set Disable Timing
              MPFROMSHORT(sp.proflags & MTERROFF ? ON : OFF), 0L);

        WinSendDlgItemMsg(hWndSPECFUNC, 304, BM_SETCHECK,
              MPFROMSHORT(ini.iniflag1 & MTHRU ? ON:OFF),0L); //Set MIDI-THRU

        WinSetDlgItemShort(hWndSPECFUNC,266, ini.leadin_bars >> 1, TRUE); // Lead-in

        i = (ini.iniflag1 & METROMID) ? MET_MIDI : MET_SPKR;
        WinSendDlgItemMsg(hWndDlg, i, BM_SETCHECK, MPFROMSHORT(ON),0L);

        WinEnableControl(hWndDlg, MET_NOTE, ini.iniflag1&METROMID? TRUE: FALSE);
        WinEnableControl(hWndDlg, MET_CHN,  ini.iniflag1&METROMID? TRUE: FALSE);

        WinSendDlgItemMsg(hWndDlg, MET_NOTE, SPBM_SETLIMITS,
                           MPFROMLONG(127), MPFROMLONG(0));
        WinSendDlgItemMsg(hWndDlg, MET_NOTE, SPBM_SETCURRENTVALUE,
          MPFROMCHAR(ini.MetNote), 0);
        WinSetDlgItemText(hWndDlg, MET_NOTETXT, Resolve_Note(ini.MetNote, LEVEL1));

        WinSendDlgItemMsg(hWndDlg, MET_CHN, SPBM_SETLIMITS,
                           MPFROMLONG(16), MPFROMLONG(1));
        WinSendDlgItemMsg(hWndDlg, MET_CHN, SPBM_SETCURRENTVALUE,
          MPFROMCHAR(ini.MetChn), 0);

        WinSendDlgItemMsg(hWndDlg, MET_DUR, SPBM_SETLIMITS,
                           MPFROMLONG(250), MPFROMLONG(1));
        WinSendDlgItemMsg(hWndDlg, MET_DUR, SPBM_SETCURRENTVALUE,
          MPFROMCHAR(ini.MetDur), 0);
      }

/**********************************************************************/
/*               Load color options list box                          */
/**********************************************************************/
      if (CallerId == IDLG_COLOR)             // Processing COLOR dialog
      {
        ClrPtr = 0;                           // Init. cell pointer
        for ( i = 0; i < 19; i++ )
        {
          WinLoadString(hAB, 0, C_DEFAULT + i, 12, (PSZ)szChars);
          WinSendDlgItemMsg(hWndDlg, 127, LM_INSERTITEM,
                             MPFROM2SHORT(LIT_END, 0), MPFROMP(szChars));
        }
      }
      initSwt = OFF;
      break; /* End of UM_DLGINIT */

     case WM_CONTROL:
      switch(SHORT1FROMMP(mp1))
      {
       case 263: /* Disable measure timing                              */
         sp.proflags ^= MTERROFF;   /* Flip switch in profile */
         break;

       case 304: /* Checkbox text: "MIDI-Thru"                          */
          if (chkact()) break;      /* Insure ok to do this now*/
          if (!(ini.iniflag1 & MTHRU))
          {
            result = 0x7d;            /* OMNI: on                */
            ini.iniflag1 |= MTHRU;
            mpucmd(0x89,0);           /* MPU MIDI THRU: on       */
          }
          else
          {
            result = 0x7c;            /* OMNI: off               */
            ini.iniflag1 &= 255-MTHRU;
            mpucmd(0x88,0);           /* MPU MIDI THRU: off      */
          }
          for (i = 0; i < 16; ++i)
          {
            mpucmd(0xd7,0);           /* Want to send data       */
            mpuputd((BYTE)(0xb0 | i),0); /* Send status to each chn */
            mpuputd(result,0);        /* OMNI on/off             */
            mpuputd(0, LAST);         /* V=0                     */
          }
          mpucmd(0xb9,0);             /* Clear Play Map          */
          WinCheckButton(hWndSPECFUNC, 304, ini.iniflag1 & MTHRU ? ON:OFF);
          chngdini = ON;
          break;

        case MET_SPKR:
          ini.iniflag1 &= 255-METROMID; // Set normal (speaker) click mode
          WinEnableControl(hWndDlg, MET_NOTE, FALSE); // Grey this out
          WinEnableControl(hWndDlg, MET_CHN,  FALSE);
          chngdini = ON;
          break;

        case MET_MIDI:
          ini.iniflag1 |= METROMID;     // Set MIDI click mode
          WinEnableControl(hWndDlg, MET_NOTE, TRUE); // Activate these
          WinEnableControl(hWndDlg, MET_CHN,  TRUE);
          chngdini = ON;
          break;

        case MET_NOTE:                  // Note spin button
        case MET_CHN:                   // Channel spin button
        case MET_DUR:                   // Duration spin button
          if (initSwt) break;           // Screws up values during DLGINIT
          cntl = SHORT1FROMMP(mp1);     // Save control id
          switch(SHORT2FROMMP(mp1))     // Switch on Notification Code
          {
           case SPBN_ENDSPIN:
           case SPBN_CHANGE:
            WinSendDlgItemMsg(hWndDlg, cntl, SPBM_QUERYVALUE, // Get value
                               &ulUtility, 0);
            if (cntl == MET_NOTE)
            {
              ini.MetNote = ulUtility;
              WinSetDlgItemText(hWndDlg, MET_NOTETXT,
                Resolve_Note(ulUtility, LEVEL1));
            }
            else if (cntl == MET_CHN) ini.MetChn = ulUtility;
            else if (cntl == MET_DUR) ini.MetDur = ulUtility;
            chngdini = ON;
            break;
          }
          break;

        case 127: /* Color list box                                 */
          switch(SHORT2FROMMP(mp1)) /* switch on Notification Code  */
          {
           case LN_SELECT:    /* Field is being selected    */
             i = SHORT1FROMMR(WinSendDlgItemMsg(hWndDlg,
                                127, LM_QUERYSELECTION, 0L, 0L ) );
             if (i != LIT_NONE && ClrPtr) // We have a good selection
             {
               *ClrPtr = i - 3;  // To relate to system color definitions
               WinInvalidateRect(hWndSongEditor, &Edrcl, FALSE); // Force re-paint
               WinInvalidateRect(hWndClient, &rClient, FALSE);
             }
             break;

           default: /* Default other messages                     */
             return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
          }
          break;

        case 128:        /* Radio btn: "S.E. Foregrnd Color"    */
          ClrPtr = &ini.EfgndClr;
          WinSendDlgItemMsg(hWndDlg, 127, LM_SELECTITEM,
                            MPFROMSHORT(ini.EfgndClr+3),  MPFROMSHORT(TRUE));
          break;

        case 129:        /* Radio btn: "S.E. Backgrnd Color"    */
          ClrPtr = &ini.EwndClr;
          WinSendDlgItemMsg(hWndDlg, 127, LM_SELECTITEM,
                            MPFROMSHORT(ini.EwndClr+3),  MPFROMSHORT(TRUE));
          break;

        case 130:        /* Radio btn: "S.E. MeasLine Color"    */
          ClrPtr = &ini.EbarClr;
          WinSendDlgItemMsg(hWndDlg, 127, LM_SELECTITEM,
                            MPFROMSHORT(ini.EbarClr+3),  MPFROMSHORT(TRUE));
          break;

        case 132:        /* Radio btn: "Main foregrnd Color"    */
          ClrPtr = &ini.MfgndClr;
          WinSendDlgItemMsg(hWndDlg, 127, LM_SELECTITEM,
                            MPFROMSHORT(ini.MfgndClr+3),  MPFROMSHORT(TRUE));
          break;

        case 133:        /* Radio btn: "Main Background color"  */
          ClrPtr = &ini.MwndClr;
          WinSendDlgItemMsg(hWndDlg, 127, LM_SELECTITEM,
                            MPFROMSHORT(ini.MwndClr+3),  MPFROMSHORT(TRUE));
          break;

        default: /* Default other messages                     */
          return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
      }
     break; /* End of WM_CONTROL */

    case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
          case 125:        /* Button text: "Fonts"     */
            hPS = WinGetPS(hWndClient); // Get handle to pres space
 /***************************************************************/
 /* Initialize those fields in the FONTDLG structure that are   */
 /* used by the appl; then display the font dlg and set the font*/
 /***************************************************************/
   strcpy(szFamilyname, ini.fm.szFamilyname); // Family name of font
   ini.fontDlg.fAttrs.usRecordLength = sizeof(FATTRS);
   ini.fontDlg.cbSize = sizeof(FONTDLG);      // Size of structure
   ini.fontDlg.hpsScreen = hPS;               // Screen presentation space
   ini.fontDlg.pszTitle = "MidiLab/2 Font Selection";
   ini.fontDlg.pszFamilyname = szFamilyname;  // point to Family name of font
   ini.fontDlg.fl = FNTS_CENTER | FNTS_INITFROMFATTRS; // Font type option bits
   ini.fontDlg.clrFore = CLR_NEUTRAL;         // Selected foreground color
   ini.fontDlg.clrBack = SYSCLR_WINDOW;       // Selected background color
   ini.fontDlg.usFamilyBufLen = sizeof(szFamilyname); //Length of family name buffer

           hWndFontDlg = WinFontDlg(HWND_DESKTOP, hWndDlg, &ini.fontDlg);
           if (hWndFontDlg && (ini.fontDlg.lReturn == DID_OK))
           {
             GpiQueryFontMetrics(hPS, (LONG)sizeof(FONTMETRICS), &ini.fm);
             WinInvalidateRect(hWndClient, &rClient, FALSE); // Force re-paint
           }
           WinReleasePS(hPS);
           break;  // End of Font processing

      case 126:        /* Button text: "Colors"     */
           ltime = IDLG_COLOR;              // Identify to initialization rtn
           WinDlgBox(HWND_DESKTOP, hWndDlg, (PFNWP)SPECFUNCMsgProc,
                              0, IDLG_COLOR, &ltime);
           break;

      case 139:        /* Button text: "Keep"   (Color)  */
           chngdini = ON;                   // Indicate initialization changed
      case 136:        /* Button text: "Done"   (Color)  */
      case 137:        /* Button text: "Cancel" (Color)  */
           WinDismissDlg(hWndDlg, TRUE);    // Close color box
           break;

      case DID_OK:     /* Button text: "Ok"        */
           WinQueryWindowText(WinWindowFromID(hWndSPECFUNC, 266), 4, szChars);
           ini.leadin_bars = atoi(szChars) << 1;
           i = atoi(szChars);
           if (!chk_range(i, "Lead-in bars", 0, 8)) break;
           ini.leadin_bars = i << 1;
           chngdini = ON;              // Indicate initialization changed

      case DID_CANCEL: /* Button text: "Cancel"    */
           WinDestroyWindow(hWndDlg);
           hWndSPECFUNC = 0;
           break;
         }
         break; /* End of WM_COMMAND */

    case WM_CLOSE:
         WinPostMsg(hWndDlg, WM_COMMAND, MPFROMSHORT(DID_CANCEL), 0L);
         break; /* End of WM_CLOSE */

    case WM_FAILEDVALIDATE:
         WinAlarm(HWND_DESKTOP, WA_ERROR);
         WinSetFocus(HWND_DESKTOP, WinWindowFromID(hWndDlg, SHORT1FROMMP(mp1)));
         return((MRESULT)TRUE); /* End of WM_FAILEDVALIDATE */

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
 return FALSE;
} /* End of SPECFUNCMsgProc */

/**********************************************************************/
/*       Message Processing routine for misc utility dialogs          */
/**********************************************************************/
MRESULT EXPENTRY UTILMsgProc(HWND hWndDlg,
                             USHORT message, MPARAM mp1, MPARAM mp2)
{
 static HWND  hWndParent;
 SHORT  i;
 BYTE   pgm, chn, path;
 PULONG caller_id;

 switch(message)
   {
    case WM_INITDLG:
//       hWndParent = (HWND)mp2;
//       cwCenter(hWndDlg, (HWND)hWndParent);
         caller_id = PVOIDFROMMP(mp2);           // Get id of active dialog
         if (*caller_id == IDLG_ASSIGN)          // Processing ASSIGN dialog
         {
           sprintf(msgbfr, "Assign channel to track %d", xtrack+1);
           WinSetWindowText(hWndDlg, msgbfr);    // Set window title
         }
         else if (*caller_id == IDLG_NAMETRK)    // Processing NAME TRACK
         {
           sprintf(msgbfr, "Assign name to track %d", xtrack+1);
           WinSetWindowText(hWndDlg, msgbfr);    // Set window title
           WinSetDlgItemText(hWndDlg, 263, sp.trkname[xtrack]);
           WinSendDlgItemMsg(hWndDlg, 263, EM_SETTEXTLIMIT, // Name length limit
                              MPFROMSHORT(TNML), 0);
           WinSendDlgItemMsg(hWndDlg, 263, EM_SETINSERTMODE, // No Insert mode
                              MPFROMSHORT(FALSE), 0);
         }
         else if (*caller_id == IDLG_TRKSIZE)    // Processing TRACK SIZE
         {
           sprintf(msgbfr, "Set size for track %d", xtrack+1);
           WinSetWindowText(hWndDlg, msgbfr);    // Set window title
           _ltoa(ini.trksize[xtrack], szChars, 10);
           WinSetDlgItemText(hWndDlg, 126, szChars);
         }
         else if (*caller_id == IDLG_TRACE)      // Processing TRACE
         {
           cwCenter(hWndDlg, (HWND)hWndParent);
           for (i = 0; i < 5; ++i)
           {
             WinSendDlgItemMsg(hWndDlg, 264 + i, BM_SETCHECK,
               MPFROMSHORT(trace & 1 << i ? ON : OFF), 0L); //Set trace chkboxes
             if (trace & TRC_INDATA) pgm = 1;
             else if (trace & TRC_READBUFR) pgm = 2;
             else pgm = 0;
             WinSendDlgItemMsg(hWndDlg, 266, BM_SETCHECK,
               MPFROMSHORT(pgm), 0L);          //Set 3-stage input data chkbox
           }
         }
         WinAssociateHelpInstance(hWndMLABHelp, hWndDlg); // Link to HELP 12/31/92
         break; /* End of WM_INITDLG */

    case WM_CONTROL:
         i = SHORT1FROMMP(mp1);
         switch(i)
           {
/*********************************************************************/
/*                   TRACE options                                   */
/*********************************************************************/
            case 264:        /* Button text: "Trace cmds"      */
                 trace ^= TRC_CMDS;           // Flip switch
                 break;

            case 268:        /* Button text: "Trace clk to host*/
                 trace ^= TRC_CLK2HOST;       // Flip switch
                 break;

            case 265:        /* Button text: "Trace outdata"   */
                 trace ^= TRC_OUTDATA;        // Flip switch
                 break;

            case 266:        /* Button text: "Trace indata" (3-stage) */
                 trace &= 255 - TRC_INDATA - TRC_READBUFR;  // Set both off
                 i = WinQueryButtonCheckstate(hWndDlg, 266);
                 if (i == 1) trace |= TRC_INDATA;         // Trace mpugetd
                 else if (i == 2) trace |= TRC_READBUFR;  // Trace DosRead
                 break;

            default: /* Default other messages                     */
                 return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
           }
           break; /* End of WM_CONTROL */

    case WM_COMMAND:
         i = SHORT1FROMMP(mp1);
         switch(i)
           {
/*********************************************************************/
/*            Send MIDI program changes to the network               */
/* 3/27/94 - Changed to eliminate need to have an inactive trk avail.*/
/*********************************************************************/
            case 258:        /* Button text: "Send" (pgm change)      */
                 WinQueryWindowText(WinWindowFromID(hWndDlg, 257), 4, szChars);
                 i = atoi(szChars);
                 if (!chk_range(i, "Program#", 1, 128)) break;
                 pgm = i;

                 WinQueryWindowText(WinWindowFromID(hWndDlg, 261), 4, szChars);
                 i = atoi(szChars);
                 if (!chk_range(i, "MIDI channel", 1, 16)) break;
                 chn = i;

                 if (!(ini.iniflag1 & MTHRU))      // 3/27/94
                 {
                   mpucmd(0xd1, 0);          /* Want_to_send_data (Trk2)*/
                   mpuputd((BYTE)(PGM_CHANGE | --chn),0); /* Send chnl cmd  */
                   mpuputd(--pgm, LAST);     /* Send program number     */
                 }
                 else umsg('I', hWndDlg, "Please turn off MIDI-THRU first");
                 break;

/*********************************************************************/
/*              Track/channel correlation assignment                 */
/*********************************************************************/
            case 256:        /* Button text: "Assign"    */
                 WinQueryWindowText(WinWindowFromID(hWndDlg, 259), 3, szChars);
                 i = atoi(szChars);
                 if (chk_range(i, "MIDI channel", 0, 16))
                 {
                   sp.tcc[xtrack] = i;               // Assign new value
                   path = 0x20;                      // SCRN0 path to reset
                   goto cancel;
                 }
                 else break;

/*********************************************************************/
/*                   Enter name of track                             */
/*********************************************************************/
            case 260:        /* Button text: "Set trk name"    */
                 WinQueryWindowText(WinWindowFromID(hWndDlg, 263), 17, szChars);
                 memset(sp.trkname[xtrack], ' ', TNML);// Clear name field
                 strcpy(sp.trkname[xtrack], szChars);  // Set new name
                 path = 0x08;                          // SCRN0 path to reset
                 goto cancel;

/*********************************************************************/
/*                   Set Track Size                                  */
/*********************************************************************/
            case 127:        /* Button text: "Set trk size"    */
                 WinQueryWindowText(WinWindowFromID(hWndDlg, 126), 9, szChars);
                 ltime = atol(szChars);
                 if (chk_range(ltime, "Track Size", 1000, 999999))
                 {
                   ini.trksize[xtrack] = ltime;      // Assign new value
 //                path = 0x80;                      // SCRN0 path to reset
                   chngdini = ON;                    // Indicate INI changed
                   umsg('I', hWndDlg,
             "Ok - MidiLab/2 must be restarted for this change to take effect");
                   goto cancel;
                 }
                 else break;

/*********************************************************************/
/*        General cancel for all dlgs that come to this proc.        */
/* If necessary, the SET_HEAD msg will cause a repaint of the client */
/*********************************************************************/
            cancel:
                 WinPostMsg(hWndClient, UM_SET_HEAD,
                            MPFROM2SHORT(path, NULL), NULL);
            case 262:        /* Button text: "Cancel"    */
                 WinDismissDlg(hWndDlg, TRUE);
                 break;

            case 270:        /* Button text: "Trace Done" */
                 goto trace_close;

            case 271:        /* Button text: "Clear Trace Box */
                 WinSendDlgItemMsg(hWndDlg, 125, LM_DELETEALL, NULL, NULL);
                 tracefull = OFF;           // In case it was full
                 break;
           }
         break; /* End of WM_COMMAND */


    case WM_CLOSE:
      trace_close:
         WinDestroyWindow(hWndTRACE);
         hWndTRACE = 0;
         tracefull = OFF;           // In case it was full
         trace = OFF;               // Disable all traces
         WinPostMsg(hWndDlg, WM_COMMAND, MPFROMSHORT(DID_CANCEL), 0L);
         break; /* End of WM_CLOSE */

    case WM_MOVE:
         if (hWndDlg == hWndBIGBARS && (sysflag2 & INITCMPL))
         { // Big measures window was moved after initialization completed
           WinQueryWindowPos(hWndBIGBARS, &ini.bigbars); // Get new position
           chngdini = ON;           // Indicate init file has changed
         }
         break;

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
 return FALSE;
} /* End of UTILMsgProc */
