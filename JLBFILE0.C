/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBFILE0 -    ** File Manager routine **                        */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
#define EXT extern
#define INCL_WINMESSAGEMGR
#define INCL_WINMENUS
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

/***************************************************************************/
/*              File Open/Save routine                                    */
/***************************************************************************/
MRESULT EXPENTRY FILEOPENMsgProc(HWND hWndDlg,
                                 USHORT message, MPARAM mp1, MPARAM mp2)
{
  APIRET rc;
  HWND  hWndParent;
  SHORT i;
  static USHORT sSelectIndex;
  char *fptr, mbfr[20];       // Need mbfr to avoid conflicts with msgbfr
  PSZ fileext = "*.M*";       // Hide *some* non-MIDI files  (2/6/94)

  switch(message)
   {
    case WM_INITDLG:
         hWndParent = (HWND)mp2;
         cwCenter(hWndDlg, (HWND)hWndParent);

         /* Initialize entry field control: filename                       */
         WinSendDlgItemMsg(hWndDlg, IDD_FILENAME, EM_SETTEXTLIMIT, MPFROMSHORT(13), 0L);
         WinSetDlgItemText(hWndDlg, IDD_FILENAME, songname);

         /* Initialize list box control:                                   */
         cwFillFileListBox(hWndDlg, fileext, 302);
         /* Initialize list box control:                                   */
         cwFillDirListBox(hWndDlg, 303, IDD_DIRNAME);

         rc = (sysflag2 & QSAVE) ? OFF : ON;     /* Initialize file format */
         WinSendDlgItemMsg(hWndDlg, 102, BM_SETCHECK, MPFROMSHORT(rc), 0L);
         rc = (ini.iniflag1 & MTEXT) ? ON : OFF; /* Initialize text display*/
         WinSendDlgItemMsg(hWndDlg, 124, BM_SETCHECK, MPFROMSHORT(rc), 0L);
         WinAssociateHelpInstance(hWndMLABHelp, hWndDlg); // Link to HELP 12/31/92
         break; /* End of WM_INITDLG */

    case WM_CONTROL:

         switch(SHORT1FROMMP(mp1))
           {
            case 102:          /* Save files in MIDI 1.0 format            */
                 sysflag2 ^= QSAVE;        /* Flip mode                    */
                 break;

            case 124:          /* Display meta text during load            */
                 ini.iniflag1 ^= MTEXT;    /* Flip text display switch     */
                 chngdini = ON;            /* Indicate INI file has changed*/
                 break;

            case IDD_FILENAME: /* Entry field variable: "filename"         */
                 switch(SHORT2FROMMP(mp1)) /* switch on Notification Code  */
                   {
                    case EN_SETFOCUS:/* Entry field is receiving the focus */
                         break;

                    case EN_KILLFOCUS: /* Entry field is losing the focus  */
                         break;

                    default: /* Default other messages                     */
                         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
                   }
                 break;

            case 302: /* List box   (files)                                */
                 {
                   char listname[14];

                   i = SHORT2FROMMP(mp1);
                   if (i == LN_ENTER || i == LN_SELECT)  // Select item
                   {
                     sSelectIndex = (USHORT)(ULONG)WinSendDlgItemMsg(hWndDlg,
                          SHORT1FROMMP(mp1), LM_QUERYSELECTION, MPFROMSHORT((SHORT)LIT_FIRST), 0L);

                     WinSendDlgItemMsg(hWndDlg, SHORT1FROMMP(mp1), LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(sSelectIndex, sizeof(listname)),
                                   MPFROMP(listname));

                     WinSetDlgItemText(hWndDlg, IDD_FILENAME, listname);
                   }
                   if (i == LN_ENTER)  // ENTER or DBL click
                   {
                     strcpy(songname, listname);
                     filename = &songname[0];   //Set up file name
                     goto loadsong;
                   }
                 }
                 break;

            case 303: /* List box   (directories)                          */
                 {
                  USHORT sSelect;       /* index to selected listbox item  */
                  ULONG  CurDisk;       /* number of the current disk      */
                  ULONG  DriveMap;      /* logical drive map               */
                  CHAR   szBuffer[100];

                  /* If the user has not pressed enter or double clicked   */
                  /* the mouse on an item, then break out of this case     */
                  if(SHORT2FROMMP(mp1) != LN_ENTER)
                     break;

                   /* Determine the index to the selected item             */
                   sSelect = (USHORT)(ULONG)WinSendDlgItemMsg(hWndDlg,
                                                       SHORT1FROMMP(mp1),
                                                       LM_QUERYSELECTION,
                                                       MPFROMSHORT((SHORT)LIT_FIRST), 0L);

                   /* Query the text of the selected item                  */
                   WinSendDlgItemMsg(hWndDlg, SHORT1FROMMP(mp1),
                                     LM_QUERYITEMTEXT,
                                     MPFROM2SHORT(sSelect, sizeof(szBuffer)),
                                     MPFROMP(szBuffer));

                   /* Is the selected item a drive or directory            */
                   if(szBuffer[0] == '[' &&
                      szBuffer[2] == ':' &&
                      szBuffer[3] == ']')
                     {
                      /* Save current drive in case of error               */
                      DosQueryCurrentDisk(&CurDisk, &DriveMap);
                      /* Log to new drive, on error restore to previous    */
                      if(DosSetDefaultDisk(szBuffer[1] - '@') != 0)
                         DosSetDefaultDisk(CurDisk);
                      /* Fill the listbox with new directory information   */
                      /* if an error occurs, restore to previous drive     */
                      if(cwFillDirListBox(hWndDlg, SHORT1FROMMP(mp1), IDD_DIRNAME) == -1)
                        {
                         WinLoadString(hAB, 0, IDS_DISK_ERR,
                                       sizeof(szBuffer), szBuffer);
                         WinMessageBox(HWND_DESKTOP, hWndDlg,
                                       szBuffer, szAppName,
                                       0, MB_OK | MB_INFORMATION);
                         DosSetDefaultDisk(CurDisk);
                         cwFillDirListBox(hWndDlg, SHORT1FROMMP(mp1), IDD_DIRNAME);
                        }
                      cwFillFileListBox(hWndDlg, fileext, 302);
                     }
                   else /* A directory was selected                        */
                     {
                      /* Change to new directory                           */
                      DosSetCurrentDir(szBuffer);
                      /*  Fill the listbox with new directory information  */
                      cwFillDirListBox(hWndDlg, SHORT1FROMMP(mp1), IDD_DIRNAME);
                      cwFillFileListBox(hWndDlg, fileext, 302);
                     }
                 }
                 break;

           }
         break; /* End of WM_CONTROL */

    case WM_COMMAND:
         i = SHORT1FROMMP(mp1);
         if (i != DID_OK && i != 257 && i != 259)
           break;     // 9/26/95
         WinQueryDlgItemText(hWndDlg, IDD_FILENAME, 14, songname);
         if (strlen(songname))
           filename = &songname[0];   // Set up file name pointer
         else
         {
           umsg('E', hWndDlg, "Type or select a file name first");
           break;                  // No file name, can't do anything
         }

         switch (i)
           {
          loadsong:
            case DID_OK: /* Button text: "Open/Load"                    */
                 if (chkact()) break;      /* Insure ok to do this now*/
                 if (safequit && umsg('C', hWndDlg, "Overlay current tracks?") != MBID_YES)
                   break;
                 fptr = filename;              /* Save file name        */
                 if (!dmgopen())               /* Open the selected file*/
                 {
                   rc = dmgrload();            /* Go to load disk file  */
                   if (!rc)
                   {
                     safequit = OFF;           /* If ok, we're safe     */
                     sprintf(msgbfr, "MidiLab/2 - %.12s", fptr);
                     WinSetWindowText(hWndFrame, msgbfr);
                   }
                 }
                 break;

            case 257:  // Button text "Save"
                rc = dmgrsave();
                if (rc < 4)                    /* Saved ok                  */
                {
                  if (rc == 0)      // File just saved for the first time
                    WinSendDlgItemMsg(hWndDlg, 302, LM_INSERTITEM,
                        MPFROM2SHORT(LIT_SORTASCENDING, 0), MPFROMP(filename));
                  WinSetDlgItemText(hWndDlg, IDD_FILENAME, filename);
                  sprintf(msgbfr, "MidiLab/2 - %.12s", filename);
                  WinSetWindowText(hWndFrame, msgbfr);
                }
                break;

            case 259:  // Button text "Delete"
                 sprintf(mbfr, "Delete %.12s?",filename);
                 if (umsg('C', hWndDlg,  mbfr) == MBID_YES)
                 {
                   rc = DosDelete(filename);         /* Erase file         */
                   if (!rc)
                   {
                     WinSendDlgItemMsg(hWndDlg, 302, LM_DELETEITEM, (MPARAM)sSelectIndex, NULL);
                     WinSetDlgItemText(hWndDlg, IDD_FILENAME, "");
                   }
                   else
                   {
                     sprintf(mbfr, "DosDelete failed  RC: %d", rc);
                     umsg('E', hWndDlg, mbfr);
                   }
                 }
                 break;
           }
         break; /* End of WM_COMMAND */

    case WM_CLOSE:
         WinDestroyWindow(hWndDlg);
         hWndFILE = 0;
         break; /* End of WM_CLOSE */

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
 return FALSE;
} /* End of FILEOPENMsgProc */

INT cwFillFileListBox(HWND hWnd, PSZ szFileSpec, INT ListID)
{
 APIRET rc;
 FILEFINDBUF3 findbuf ;
 HDIR         hDir = 1 ;
 ULONG        ulSearchCount = 1;

 WinSendDlgItemMsg(hWnd, ListID, LM_DELETEALL, 0, 0) ;
 rc = DosFindFirst(szFileSpec, &hDir, 0, (PVOID)&findbuf, sizeof(findbuf),
                              &ulSearchCount, FIL_STANDARD);
 if (rc)
   return(0);         // Probably no files found, but return in any case

 while(!rc)
   {
    WinSendDlgItemMsg(hWnd, ListID, LM_INSERTITEM,
                      MPFROM2SHORT(LIT_SORTASCENDING, 0),
                      MPFROMP(findbuf.achName));

    rc = DosFindNext(hDir, (PVOID)&findbuf, sizeof(findbuf), &ulSearchCount) ;
   }
 return(1);
}

INT cwFillDirListBox(HWND hWndDlg, USHORT id, USHORT targetid)
{
 APIRET rc;
 static CHAR buffer[10];
 FILEFINDBUF3 findbuf;
 HDIR         hDir = 1;
 SHORT        sDrive;
 ULONG        ulDriveMap, ulDriveNum, ulSearchCount = 1, ulPathLen;
 CHAR         szBuffer[128];

 WinSendDlgItemMsg(hWndDlg, id, LM_DELETEALL, 0, 0) ;

 rc = DosFindFirst("*.*", &hDir, 0x0017, (PVOID)&findbuf,
                   sizeof(findbuf), &ulSearchCount, FIL_STANDARD);

 if( (rc != 0) && (rc != 18) ) /* 18 == ERROR_NO_MORE_FILES */
   return( -1 );

 rc = 0;
 while(!rc)
    {
     if(findbuf.attrFile & 0x0010 &&
        (findbuf.achName [0] != '.' || findbuf.achName [1]))
               WinSendDlgItemMsg(hWndDlg, id, LM_INSERTITEM,
                                 MPFROM2SHORT(LIT_SORTASCENDING, 0),
                                 MPFROMP(findbuf.achName));
     rc = DosFindNext(hDir, &findbuf, sizeof(findbuf), &ulSearchCount) ;
    }

 rc = DosQueryCurrentDisk(&ulDriveNum, &ulDriveMap);
 for(sDrive = 0; sDrive < 26; sDrive++)
   if(ulDriveMap & 1L << sDrive)
     {
      sprintf(buffer,"[%c:]", (CHAR)sDrive + 'A');
      WinSendDlgItemMsg(hWndDlg, id, LM_INSERTITEM,
                        MPFROM2SHORT(LIT_END, 0),
                        MPFROMP(buffer)) ;
     }
 ulPathLen = sizeof(szBuffer);
 DosQueryCurrentDir(ulDriveNum, &szBuffer[3], &ulPathLen);
 szBuffer[0] = (char)('@' + ulDriveNum);
 szBuffer[1] = ':';
 szBuffer[2] = '\\';
 WinSetDlgItemText(hWndDlg, targetid, szBuffer);
 return(0);
}
