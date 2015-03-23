/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal System/2  */
/*                                                                   */
/*   JLBCWIN0 -    ** Main Client window procedure **                */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
#define     EXT extern
#include   "MLABPM.h"
#include   "MLABPM.rch"
#include   "JLBCSA00.C"

VOID  FastFwd(VOID);            /* Fast-Fwd thread                      */
SHORT SkipSave = 0;             /* Skip_bar_ctr status saver            */
RECTL rClient;                  /* Handle to rectangle formed by client area */
HPS   hPS;                      /* Handle for the Presentation Space    */
BYTE  ffswt = OFF;              /* Fast-Fwd indicator switch            */
BYTE  previous_func = OFF;      /* Re-take indicator                    */

/***************************************************************************/
/*          Main (Client) Window Procedure                                 */
/***************************************************************************/
MRESULT EXPENTRY WndProc(HWND hWnd, USHORT message, MPARAM mp1, MPARAM mp2)
{                        /* Beginning of message processor                 */
  APIRET rc;
  ULONG  DlgId;
  SHORT  i, j, x, y;
  BYTE   paint_path, key;

   switch(message)
     {
      case WM_CONTROL:                      // Check for Fast Forward
           x = SHORT1FROMMP(mp1);           // Get control id
           y = SHORT2FROMMP(mp1);           // Get notification code
           if (x == MN_B6)                  // FastFwd button
           {
             if (y == GBN_BUTTONHILITE)     // FF pressed
             {
               if (actvfunc == PLAY)        // Need to pause Play
                 WinSendMsg(hWnd, WM_COMMAND, MPFROM2SHORT(MN_B5,0), 0L);
               else if (!actvfunc)          // Need to prep Play
                 actv_trks = sp.playtracks;
               else break;                  // Forget it, other function active

               ffswt = ON;                  // Set Fast-forward mode
               WinSendMsg(WinWindowFromID(hWnd, MN_B6), GBM_ANIMATE,
                            MPFROMSHORT(TRUE), NULL); // Strt FFwd animation
               DosCreateThread((PTID)&tidFF, (PFNTHREAD)FastFwd, (ULONG)0,
                           (ULONG)0, (ULONG)STKSZ);
             }
             else if (y == GBN_BUTTONUP)    // FF released
               ffswt = OFF;                 // End the FastFwd thread
           }
           break;

      case WM_COMMAND:
           i = SHORT1FROMMP(mp1);
           switch(i)
            {
             case IDM_F_NEW:
                  goto Reset;

             case IDM_F_SAVE:
                  if (songname[0] != '\0')  // Name already specified
                  {
                    dmgrsave();
                    break;
                  }                         // else fall thru to save-as

             case IDM_F_OPEN:
                  if (!WinIsWindow(hAB, hWndFILE))
                  {
                    hWndFILE = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                       (PFNWP)FILEOPENMsgProc, 0, IDLG_FILEOPEN, NULL);
                  }
                  else
                  {
                    WinSetFocus(HWND_DESKTOP, hWndFILE);
                    WinSetDlgItemText(hWndFILE, IDD_FILENAME, ""); //Clear name
                  }
                  break;

             case IDM_F_EXIT:
                  WinPostMsg(hWnd, WM_CLOSE, 0L, 0L );
                  break;

             case IDM_F_SAVPRF:
                  DosSetFilePtr(INIfile, 0L, FILE_BEGIN, &action);
                  rc = DosWrite(INIfile, &ini, sizeof(ini), &BytesXfrd);
                  if (!rc)
                  {
                    DosBeep(75,40);          // Comforting assurance
                    chngdini = OFF;          // Everything's safe
                  }
                  break;

 /**************************************************************************/
 /*                      Create Song Editor Window                         */
 /**************************************************************************/
             case IDM_E_CREATE:
                  if (!WinIsWindow(hAB, hWndSongEditor))
                  {
                    CHAR szTitle[22];
                    ULONG CrFlgs =
                           FCF_TITLEBAR   | FCF_SYSMENU      | FCF_VERTSCROLL|
                           FCF_MINBUTTON  | FCF_MAXBUTTON    | FCF_HORZSCROLL|
                           FCF_SIZEBORDER | FCF_MENU         | FCF_ICON      |
                           FCF_ACCELTABLE | FCF_SHELLPOSITION;

                    sysflag2 &= 255 - EDITCMPL;  // Indicate window being created
                    hWndEDITFrame = WinCreateStdWindow(HWND_DESKTOP, 0L,
                           &CrFlgs, szMLEDITAppName, szTitle, 0L, 0, ID_MLEDIT,
                           &hWndSongEditor);     // Client handle
                    if (hWndEDITFrame == 0)
                      umsg('E', hWnd, "Cannot create Song Editor");
                    else set_attr(hWnd, IDM_E_CREATE, MIA_CHECKED, ON);
                  }
                  else WinSetFocus(HWND_DESKTOP, hWndSongEditor);
                  break;

             case IDM_E_CLOSE:                    // Close Song Edit window
                  WinDestroyWindow(hWndEDITFrame);
                  set_attr(hWnd, IDM_E_CREATE, MIA_CHECKED, OFF);
                  break;

             case IDM_C_MAIN:                     // Main Control Panel
                  if (!WinIsWindow(hAB, hWndCTLPANEL))
                  {
                    hWndCTLPANEL = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, // Change 2nd HWND_DESKTOP to hWnd to make Help work
                       (PFNWP)CTLPANELMsgProc, 0, IDLG_CTLPANEL, NULL);
                  }
                  else WinSetFocus(HWND_DESKTOP, hWndCTLPANEL);
                  break;

             case IDM_O_USERPREF:                 // User preferences
                  if (!WinIsWindow(hAB, hWndSPECFUNC))
                  {
                    DlgId = IDLG_SPECFUNC;        // Tell Dlg Proc who called it
                    hWndSPECFUNC = WinLoadDlg(HWND_DESKTOP, hWnd,
                       (PFNWP)SPECFUNCMsgProc, 0, IDLG_SPECFUNC, &DlgId);
                  }
                  else WinSetFocus(HWND_DESKTOP, hWndSPECFUNC);
                  break;

             case IDM_O_BULKDUMPRESTORE:          // Sysex utility
                  if (chkact() || standby) break; /* Chk if ok to do now*/
                  if (!WinIsWindow(hAB, hWndSYSEX))
                  {
                    hWndSYSEX = WinLoadDlg(HWND_DESKTOP, hWnd,
                       (PFNWP)SYSEXMsgProc, 0, IDLG_SYSEX, NULL);
                  }
                  else WinSetFocus(HWND_DESKTOP, hWndSYSEX);
                  break;

             case IDM_O_INDEXDISPLAY:       // Toggle index display
                  ini.indexdispl = ini.indexdispl ? OFF : ON;
                  set_attr(hWndClient, IDM_O_INDEXDISPLAY, MIA_CHECKED, ini.indexdispl);
                  chngdini = ON;            // Indicate initialization changed

                  break;

             case IDM_O_PGMCHNG:            // Send Program Chnage
                  ltime = IDLG_PGMCHNG;   // Tell Dlg Proc who called it
                  WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)UTILMsgProc,
                                  0, IDLG_PGMCHNG, &ltime);
                  break;

             case IDM_O_RMTCTL:             // Remote control
                  if (!(chkact()))
                  {
                    actvfunc = CAPTURE;
                    WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)RCTLMsgProc,
                                 0, IDLG_RMTCTL, NULL);
                  }
                  break;

             case IDM_O_BIGBARS:            // Display in-your-face measures
                  sysflag2 ^= BIGBARS;      // Flip display switch
                  WinSetDlgItemShort(hWndBIGBARS, 201, measurectr, TRUE);
                  WinSetDlgItemShort(hWndBIGBARS, 202, max_measures, TRUE);
                  WinShowWindow(hWndBIGBARS, sysflag2 & BIGBARS);
                  set_attr(hWndClient, IDM_O_BIGBARS, MIA_CHECKED, sysflag2 & BIGBARS);
                  if (sysflag2 & BIGBARS)
                    WinSetFocus(HWND_DESKTOP, hWndBIGBARS);
                  break;

             case IDM_O_TRACE_C1:
                  if (!WinIsWindow(hAB, hWndTRACE))
                  {
                    ltime = IDLG_TRACE;      // Tell Dlg Proc who called it
                    hWndTRACE = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
                       (PFNWP)UTILMsgProc, 0, IDLG_TRACE, &ltime);
                    tracefull = OFF;         // Clear 'full' switch
                  }
                  else WinSetFocus(HWND_DESKTOP, hWndTRACE);
                  break;

            case IDM_H_HELPFORHELP:
                 if(hWndMLABHelp)
                   WinSendMsg(hWndMLABHelp, HM_DISPLAY_HELP, 0L, 0L);
                 break;

            case IDM_O_TESTMODE:
                 break;

            case IDM_H_ABOUT:
                 ltime = IDLG_ABOUT;         // Tell Dlg Proc who called it
                 WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)UTILMsgProc,
                           0, IDLG_ABOUT, &ltime);
                 break;

            case IDM_METRO:
                 metro(mnswt ? OFF : ON);    // Toggle MPU-401 metronome
                 break;

            case IDM_MIDITHRU:
                 WinSendMsg(hWndSPECFUNC, WM_CONTROL, MPFROM2SHORT(304, 0), 0L);
                 break;

/***************************************************************************/
/*          Main (Client) Window Push Buttons                              */
/***************************************************************************/
             case MN_B7:         //Top Client pushbutton  (Play)
                 if (!(chkact() || standby)) /* Chk if ok to do now     */
                 {
                   if (!sp.playtracks)
                   {
                     umsg('W', hWnd,  "No tracks are active");
                     break;
                   }
                   Locate_SPP(-1);           /* Set maximum measures    */

                   if (!playprep(PLAY))      /* No tracks are active    */
                     break;
                   actvfunc = PLAY;          /* Set active function     */
                   start_time = *pulTimer;   /* Get start time          */
                   beatctr=beatswt=0;        /* Reset beat counters     */
//                 mpucmd(0x95,0);           /* Clock-to-host: on       */
                   if (ExtCtlSwt == LEVEL2)  /* STRT/CONT issued xtrnaly*/
                   {
                     WinSetDlgItemText(hWndClient, MN_B7, "..waiting");
                     break;
                   }
                   if (sysflag1 & POSACTV)   /* If positioning is active*/
                     mpucmd(0x0b,0);         /* issue Continue play     */
                   else mpucmd(0x0a,0);      /* Start normal play       */
                   WinSendMsg(WinWindowFromID(hWndClient, MN_B7), GBM_ANIMATE,
                              MPFROMSHORT(TRUE), NULL); // Strt Play animation
                   WinPostMsg(hWndClient, UM_SET_HEAD,
                               (MPARAM)(0x04+0x10), NULL); // Set measctr & ndxs
                   WinPostMsg(hWndSongEditor, UM_MOVE_BAR, NULL, NULL);

                   if (SkipSave)             /* We just did a FastFwd   */
                   {
                     skip_bar_ctr = SkipSave; /* Restore starting measure*/
                     if (SkipSave > 1)
                       start_bar_swt = ON;    /* Tell SUBS0 to position  */
                     sysflag1 &= 255-POSACTV; /* Positioning now invalid */
                     SkipSave = 0;
                   }
                 }
                 break;

             case MN_B6:         //Client pushbutton  (Fast Forward released)
                break;

             case MN_B5:         //Client pushbutton  (Pause)
                space_bar();     // Go do space-bar schtick
                break;

             case MN_B4:         //Client pushbutton  (Record)
             case MN_B3:         //Client pushbutton  (Overdub)
                  if (!(chkact() || standby))      /* Chk if ok to do now*/
                  {
                    standby = ON;
                    actvfunc = (i == MN_B4) ? RECORD : OVERDUB;
                    previous_func = actvfunc;  // For repeat
                    measurectr = 1;            // Set initial value
                    WinSendMsg(hWndClient, UM_SET_HEAD, (MPARAM)4, NULL); // Set measure ctr
                    WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)RECRDMsgProc,
                               0, IDLG_RECRD, NULL);
                  }
                  break;

             case MN_B2:         //Client pushbutton  (Repeat Record/Overdub)
                  if (chkact() || standby) break; /* Chk if ok to do now*/
                  if (previous_func)
                  {
                    actvfunc = previous_func;
                    standby = ON;
                    WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)4, NULL); // Set measure ctr
                    rec_prep();    // Go re-do with existing settings
                  }
                  else umsg('W', hWnd,  "You must set up a Record or Overdub first...");
                  break;

             case MN_B1:         //Client pushbutton  (All notes off)
                  mpucmd(0xb9,0);       // Clear Play Map
                  fsustain = ON;        // Force sustain indicator
                  Rel_sustain(TRUE);    // Release any active sustains

//                mpucmd(0x33, 0);      // Emergency reset 8/16/95
                  break;

             case MN_B0:         //Bottom Client pushbutton  (Reset)
             Reset:
                 if (actvfunc || ((safequit | editchng) &&
                     umsg('C', hWnd, "Clear and reset MidiLab/2?") != MBID_YES))
                   break;
                 else
                 {
                   actvfunc = OFF;           /* Clear active functions  */
                   previous_func = OFF;      /* Rec/Odub no longer valid*/
                   songname[0] = '\0';       /* Indicate no file loaded */
                   DLLReset();               /* Reset DLL side          */
                   dmgreset(7,0);            /* Reset everything in Main*/
                   max_measures = 1;         /* Reset measures in song  */
                   paint_path = 255;         /* Paint everything        */

                   WinSetWindowText(hWndFrame, "MidiLab/2  (Untitled)");
                   WinSetDlgItemText(hWndClient, MN_B7, " Play");
                   WinInvalidateRect(hWnd, &rClient, FALSE);
                   for (i = 0; i < NUMTRKS; ++i)
                     trk[i].num_measures = 0;/* Reset bars per track    */

                   WinPostMsg(hWndSongEditor, WM_COMMAND,
                                MPFROM2SHORT(IDM_E_CLEAR,0), 0L);

                   if (WinIsWindow(hAB, hWndFILE))
                     WinPostMsg(hWndFILE, WM_INITDLG, 0L, 0L); // Init file window

                   safequit = OFF;
                   DosBeep(270,36);
                 }
                 break;

             default:
                  break; /* End of default case for switch(mp1) */
            }
           break; /* End of WM_COMMAND */

      case HM_QUERY_KEYS_HELP:
           /* If the user requests Keys Help from the help pulldown,       */
           /* IPF sends the HM_QUERY_KEYS_HELP message to the application, */
           /* which should return the panel id of the Keys Help panel      */
           return((MRESULT)999);

      case HM_ERROR:
           /* If an IPF error occurs, an HM_ERROR message will be sent to  */
           /* the application.                                             */
           if(hWndMLABHelp && ((ULONG)mp1) == HMERR_NO_MEMORY)
             {
               char HelpBuf[80];
               char HelpBufTitle[80];

               WinLoadString(hAB, 0, IDS_HELP_TERM, 80, HelpBuf);
               WinLoadString(hAB, 0, IDS_HELP_TERM_TITLE, 80, HelpBufTitle);
               WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, HelpBuf, HelpBufTitle,
                             1, MB_OK | MB_APPLMODAL | MB_MOVEABLE);
               WinDestroyHelpInstance(hWndMLABHelp);
             }
            else
             {
               char HelpBuf[80];
               char HelpBufTitle[80];

               WinLoadString(hAB, 0, IDS_HELP_OCCRD, 80, HelpBuf);
               WinLoadString(hAB, 0, IDS_HELP_TERM_TITLE, 80, HelpBufTitle);
               WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, HelpBuf, HelpBufTitle,
                             1, MB_OK | MB_APPLMODAL | MB_MOVEABLE);
             }
           break;

      case WM_CREATE:
           hiMLABHelp.cb = sizeof(HELPINIT);  /* size of init structure   */
           hiMLABHelp.ulReturnCode = 0;
           hiMLABHelp.pszTutorialName = 0; /* no tutorial                 */
           hiMLABHelp.phtHelpTable = (PVOID)(0xffff0000 | ID_MLAB);
           hiMLABHelp.hmodAccelActionBarModule = 0;
           hiMLABHelp.idAccelTable = 0;
           hiMLABHelp.idActionBar = 0;
           hiMLABHelp.pszHelpWindowTitle = "MidiLab/2";
           hiMLABHelp.hmodHelpTableModule = 0;
           hiMLABHelp.fShowPanelId = 0;
           hiMLABHelp.pszHelpLibraryName = "MLABPM.HLP";

 /**************************************************************************/
 /* Create Instance of IPF                                                 */
 /**************************************************************************/
           hWndMLABHelp = WinCreateHelpInstance(hAB, &hiMLABHelp);
           if(!hWndMLABHelp || hiMLABHelp.ulReturnCode)
           {
             char HelpBuffer[80];

             WinLoadString(hAB, 0, IDS_NO_HELP, 80, HelpBuffer);
             WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                           (PSZ) HelpBuffer,
                           (PSZ) szAppName,
                           1,
                           MB_OK | MB_APPLMODAL | MB_MOVEABLE);
             WinDestroyHelpInstance(hWndMLABHelp);
           }
           break; /* End of WM_CREATE */

 /**************************************************************************/
 /*         Process mouse detection, hit-testing, etc.                     */
 /**************************************************************************/
      case WM_MOUSEMOVE:
           return(WinDefWindowProc(hWnd, message, mp1, mp2));

      case WM_BUTTON1DBLCLK:
           x = MOUSEMSG(&message)->x;
           y = MOUSEMSG(&message)->y;
           for (i = 1; i < 17; ++i)
           {
             j = cyClient - ((cyChar+cyDesc) * i);
             if ((y <= j)  &&  (y >= j - (cyChar+cyDesc)))
             {
               xtrack = --i;           // To pass to dlg proc

 /**************************************************************************/
 /*                     Assign Track to Channel                            */
 /**************************************************************************/
               if (x >= grid_chn && x <= grid_index)
               {
                 ltime = IDLG_ASSIGN;       // Tell Dlg Proc who called it
                 WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)UTILMsgProc,
                                0, IDLG_ASSIGN, &ltime);
               }

 /**************************************************************************/
 /*                     Assign Name to Track                               */
 /**************************************************************************/
               else if (x >= grid_trkname && x <= cxClient)
               {
                 ltime = IDLG_NAMETRK;      // Tell Dlg Proc who called it
                 WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)UTILMsgProc,
                                0, IDLG_NAMETRK, &ltime);
               }

 /**************************************************************************/
 /*                       Set Track Size                                   */
 /**************************************************************************/
               else if (x >= grid_size && x <= grid_content)
               {
                 ltime = IDLG_TRKSIZE;      // Tell Dlg Proc who called it
                 WinDlgBox(HWND_DESKTOP, hWnd, (PFNWP)UTILMsgProc,
                                0, IDLG_TRKSIZE, &ltime);
               }

 /**************************************************************************/
 /*                       Erase Track                                      */
 /**************************************************************************/
               else if (!actvfunc &&
                        x >= grid_index && x <= grid_trkname)
               {
                 sprintf(msgbfr, "Erase track %d (%s) ?",
                             xtrack+1, sp.trkname[xtrack]);
                 if (umsg('C', hWnd, msgbfr) == MBID_YES)
                 {
                   trk[xtrack].index = 0;      // Set track index to zero
                   *trk[xtrack].trkaddr = 0;   // Set track-end timing byte
                   *(trk[xtrack].trkaddr+1) = END_OF_TRK;
                   sp.trkdatalnth[xtrack] = 2; // Indicate 2 bytes long
                   sp.tcc[xtrack] = 0;         // Reset Track/Channel corr.
                   memset(sp.trkname[xtrack], ' ', TNML);   // Clear name field
                   WinPostMsg(hWnd, UM_SET_HEAD,          // Update track info
                              MPFROM2SHORT(0x78, NULL), NULL);
                   safequit = ON;
                 }
               }

 /**************************************************************************/
 /*                       Invoke Track Edit                                */
 /**************************************************************************/
               else if (x >= grid_content && x <= grid_chn)
               {
                 starting_index[xtrack] = 0;      // Set beginning of track
                 Trk_Edit(xtrack, 0, 9999, hWnd); // Go to call up track editor
               }
               break;
             }
           }
           return(WinDefWindowProc(hWnd, message, mp1, mp2));

      case WM_BUTTON2CLICK:
           space_bar();                     // Simulate space bar
           break;

      case WM_CHAR:
           if (CHARMSG(&message)->fs & KC_CHAR)
           {
             key = CHARMSG(&message)->chr;
             if (key > '0' && key < '9')    // Calling Edit from keys
             {
               xtrack = (key & 0x0f) - 1;   // Develop track for edit
               Trk_Edit(xtrack, 0, 9999, hWnd); // Go to call up track editor
             }
           }
           break;

 /**************************************************************************/
 /* Initialize all client coordinates,load bit maps,Re-draw Client windows */
 /**************************************************************************/
      case WM_SIZE:                         /* Code for sizing client area */
           cxClient = SHORT1FROMMP(mp2);
           cyClient = SHORT2FROMMP(mp2);

           hPS = WinGetPS(hWnd);
           GpiQueryFontMetrics(hPS, (LONG)sizeof(FONTMETRICS), &ini.fm);
           cxChar = (INT)ini.fm.lAveCharWidth;
           cyChar = (INT)ini.fm.lMaxBaselineExt;
           cxCaps = (INT)ini.fm.lEmInc;
           cyDesc = (INT)ini.fm.lMaxDescender;
           hbm_metro1 = GpiLoadBitmap(hPS, NULLHANDLE, ID_BMP_METRO1, 0L, 0L);
           hbm_metro2 = GpiLoadBitmap(hPS, NULLHANDLE, ID_BMP_METRO2, 0L, 0L);
           WinReleasePS(hPS);

           cyClient -= cyChar;   // To position grid closer to center
           cxClient -= cxChar;
           grid_bottom = cyClient - ((cyChar+cyDesc) * 17);
           grid_left = (cxClient / 3) - (cxChar * 6 + 2); // Establish left side
           grid_size = grid_left + cxChar * 2 + 6;
           grid_content = grid_size + cxChar * 9;
           grid_chn = grid_content + cxChar * 9;
           grid_index = grid_chn + cxChar * 4;
           grid_trkname= grid_index + cxChar * 9;

           for (i = 0; i < NUMTRKS; ++i) // Initialize trk/chn, trkname, & index rcl's
           {
             ndxrcl[i].xLeft  = grid_index+4;     // This never changes
             ndxrcl[i].xRight = grid_trkname-2;   // Nor does this
             tccrcl[i].xLeft  = grid_chn+3;       // This never changes
             tccrcl[i].xRight = grid_index-2;     // Nor does this
             namrcl[i].xLeft  = grid_trkname+3;   // This never changes
             namrcl[i].xRight = cxClient-4;       // Nor does this

             ndxrcl[i].yBottom = tccrcl[i].yBottom = namrcl[i].yBottom =
                                  ((cyClient + 2 - (cyChar+cyDesc) * 2)) -
                                  (cyChar + cyDesc) * i;

             ndxrcl[i].yTop = ndxrcl[i].yBottom + cyChar;
             tccrcl[i].yTop = tccrcl[i].yBottom + cyChar;
             namrcl[i].yTop = namrcl[i].yBottom + cyChar;
           }
           for (i = 0; i < 8; ++i)
             WinDestroyWindow(hWndButt[i]);    // Get rid of current pushbuttons
           WinQueryWindowRect(hWnd, &rClient); // Get size of client window
           rClient.xLeft = (cxClient / 5);
           Set_Client();                       // Draw new windows in Client
           break;  // End of WM_SIZE

      case WM_PAINT:
           hPS = WinBeginPaint(hWnd, 0, 0);

           /* Set the foreground & background colors from the init. file   */
           GpiErase(hPS);                      // Clear window
           WinFillRect(hPS, &rClient, ini.MwndClr);
           GpiSetColor(hPS, ini.MfgndClr);     // Set foreground color

           GpiCreateLogFont(hPS, NULL, 200L, &ini.fontDlg.fAttrs);
           GpiSetCharSet(hPS, 200L);
           paint(hPS, 255);         // Go draw the main display grid
           WinEndPaint(hPS);
           break; /* End of WM_PAINT */

      case UM_SET_HEAD:
           if (actvfunc == SYSX)
             Esc_cancel();                   // End Sysx mode
           paint_path = SHORT1FROMMP(mp1);   // Get requested heading
           if (paint_path == 255)            // Complete re-paint
             WinInvalidateRect(hWnd, &rClient, FALSE);
           else                              // Selective re-paint
           {
             ml_hps = WinGetPS(hWnd);
             paint(ml_hps, paint_path);      // Go set the requested headings
             WinReleasePS(ml_hps);
           }
           break;

      case WM_MOVE:
           if (sysflag2 & INITCMPL)
           { // Client window was moved after initialization completed
             WinQueryWindowPos(hWndFrame, &ini.MainPos); // Get new position
             chngdini = ON;            // Indicate init file has changed
           }
           break;

      case WM_CLOSE:
           /**************************************************************************/
           /*                  We're hanging it up...                                */
           /**************************************************************************/
           if (chkact() || standby) break; /* Chk if ok to do now      */
           if ((safequit | editchng) && umsg('C', hWnd, "Do you really want to quit?") != MBID_YES)
                break;                     /* Abort termination        */

           if (chngdini)                   /* Initialization prms chgd */
           {
             rc = umsg('C', hWnd, "MLABPM.INI has changed; save changes?");
             if (rc == MBID_YES)           /* Save INI file            */
               chngdini = LEVEL2;          /* Tell termination routine */
             else if (rc == MBID_CANCEL)
               break;                      /* Abort termination        */
           }

           DosPostEventSem(SemClk);        /* Insure Click Thread ends */
           sysflag1 |= SHUTDOWN;           /* Indicate System Shutdown */
           mpucmd(MPU_RESET,0);            /* Leave UART mode          */
           mpucmd(MPU_RESET,0);            /* One more to send ack     */

           if(hWndMLABHelp)
             WinDestroyHelpInstance(hWndMLABHelp);
           if(hWnd != hWndClient) break;
           GpiDeleteBitmap(hbm_metro1);    /* Delete metronome bitmap1 */
           GpiDeleteBitmap(hbm_metro2);    /* Delete metronome bitmap2 */

      default:
           return(WinDefWindowProc(hWnd, message, mp1, mp2));
     }
   return(0L);
} /* End of WndProc */

/*********************************************************************/
/*         Thread to Fast Forward while >> is held down              */
/*********************************************************************/
VOID FastFwd (VOID)
{
  while (ffswt && measurectr < max_measures)   // Fast-Fwd is held down
  {
    ++measurectr;                  // Increment measure counter
    Locate_SPP(((measurectr-1) * sp.tsign)*4); // Position to this measure
    DosSleep(100);                 // Slow things down a bit
    WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)0x04, NULL); // Update measctr
    if (WinIsWindow(hAB, hWndSongEditor))
      WinPostMsg(hWndSongEditor, UM_MOVE_BAR, NULL, NULL);   // Update bar
  }

  if (measurectr < max_measures)   // If we didn't roll up to the very end
  {
    SkipSave = skip_bar_ctr;       // Save current starting measure
    skip_bar_ctr = measurectr;     // Set to FastFwd'd measure
    start_bar_swt = ON;            // Indicate to PLAYPREP
    WinPostMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B7,0), 0L); // Start Play
  }
  else {DosBeep(900,50); DosBeep(500,50);}   // End of the line
  WinPostMsg(WinWindowFromID(hWndClient, MN_B6), (ULONG)GBM_ANIMATE,
               MPFROMSHORT(FALSE), NULL);    // Stop FFwd animation
}
