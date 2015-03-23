/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBEDIT0 -    ** MidiLab Track Editor dialog procedure (I)      */
/*                                                                   */
/*       Copyright  J. L. Bell        July 1992                      */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
extern SHORT quantize_amt;
extern LONG starting_index[];
extern HWND hWndMLABHelp;
LONG   AdjustedLnth[16];

MRESULT EXPENTRY EDITMsgProc(HWND hWndDlg,
                                 USHORT message, MPARAM mp1, MPARAM mp2)
{
  APIRET rc;
  ULONG PostCt;
  LONG j, NdxSave;
  static LONG current_index, szTrailerBufr;
  static PLONG TrailerBufr;
  static BYTE ptrack;                           /* Edit track              */
  SHORT i;
  static HWND hWndParent;
  char key, hitswt;
  EVPK *ClpPtr;
  static DPM *PrmBlk;

/**********************************************************************/
/*     Locate the track associated with this message procedure        */
/*          (This works after WM_INITDLG has been processed)          */
/**********************************************************************/
  for (i = 0; i < 16; ++i)
    if (hWndDlg == hWndETrk[i])
    {
      ptrack = i;
      break;
    }

  switch(message)
   {
    case WM_INITDLG:
      hWndParent = (HWND)mp1;
      cwCenter(hWndDlg, (HWND)hWndParent);
      PrmBlk = PVOIDFROMMP(mp2);            // Point to Main's parameter block
      ptrack = PrmBlk->ETrack;              // Set up local ptrack
      hWndETrk[ptrack] = hWndDlg;           // Assign handle for Main's use
      memcpy(&EVP, PrmBlk->EventPkt, sizeof(EVP)); // Copy MAIN's packet to Editor's

      current_index = PTI;                  // Remember current track location
      spptr = (SP *)PrmBlk->SongPrfl;       // Set ptr to song profile
      AdjustedLnth[ptrack] = spptr->trkdatalnth[ptrack]; // Init. trk adj lnth
      PlayBanner = OFF;                     // Initial state = OFF (2/06/93)
      WinSendMsg(hWndDlg, UM_DLGINIT, 0L, 0L);    // Signal init routine
      WinAssociateHelpInstance(hWndMLABHelp, hWndDlg); // Link to HELP 12/31/92
      break; /* End of WM_INITDLG */

/**********************************************************************/
/*           Initialize all controls within the Dialog Box            */
/**********************************************************************/
    case UM_DLGINIT:
      NdxSave = PTI;            // Preserve current track index during Edit
      sprintf(msgbfr, "Track Editor - %.12s: Trk %d (%.12s)  Chn %d",
                PrmBlk->SongName, ptrack+1, spptr->trkname[ptrack],
                 spptr->tcc[ptrack]);
      WinSetWindowText(hWndDlg, msgbfr);

      WinSendDlgItemMsg(hWndDlg, 124, BM_SETCHECK, MPFROMSHORT(PlayBanner), 0L);
      WinSendDlgItemMsg(hWndDlg, 125, BM_SETCHECK, MPFROMSHORT(Stepmode), 0L);
      XeqSwt |= 1 << ptrack;
      WinSendDlgItemMsg(hWndDlg, 135, BM_SETCHECK, MPFROMSHORT(ON), 0L);
      AutoOff |= 1 << ptrack;
      WinSendDlgItemMsg(hWndDlg, 136, BM_SETCHECK, MPFROMSHORT(ON), 0L);

      if (!WinIsWindow(hAB, hWndCLIP))      // If ClipBoard is not loaded
        hWndCLIP = WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP, // Load Clipboard box
                              (PFNWP)E2MsgProc, 0, IDLG_EDIT2, NULL);
      else WinSendDlgItemMsg(hWndDlg, 141, BM_SETCHECK, MPFROMSHORT(ClipShow), 0L);

      WinSetPresParam(WinWindowFromID(hWndDlg, 117),  // Set event box font
                       PP_FONTNAMESIZE, 12, "2.System VIO"); // 4/28/96

/**********************************************************************/
/*           Get storage for event array                              */
/**********************************************************************/
      PostCt = (spptr->trkdatalnth[ptrack] > 2048) ?  // Use min of 2048
                spptr->trkdatalnth[ptrack] : 2048;
      rc = DosAllocMem(&evaptr[ptrack], PostCt * sizeof(EVP), PAG_WRITE+PAG_COMMIT);
      if (rc)
      {
        sprintf(msgbfr, "DosAllocMem(event buffer) failed  RC:%d", rc);
        umsg('E', hWndETrk[ptrack], msgbfr);
      }
      EVPptr = (EVPK *)evaptr[ptrack];          // Set base event array pointer

      WinSendDlgItemMsg(hWndDlg, 118, SPBM_SETLIMITS,  // Timing Spin Button
        MPFROMLONG(239), MPFROMLONG(0));
      WinSendDlgItemMsg(hWndDlg, 120, SPBM_SETLIMITS,  // Data byte2 Spin B.
        MPFROMLONG(127), MPFROMLONG(0));

/**********************************************************************/
/*                 Load event list box                                */
/**********************************************************************/
    if (*PT == SYSX)
    {
     ; // Stay tuned...  (CWIN0 currently traps SysEx tracks)
    }
    else
    {
      bar_ctr = PrmBlk->EStrtMeas + 1;   // Set measure counter
      tmg_ctr = 0;                       // Reset timing count
      TRK.flags1 = TRACK;                // Indicate get track data
      PTI = starting_index[ptrack];      // Set index of first measure to edit

/* If we're sitting at a measure-end, skip over it since it should have time=0 */
      skiptrks |= 1 << ptrack;           // Force resend of status
      get_event(ptrack);                 // See if next event is MEAS_END
      if ((EVP.flags1 & MPUMARK) &&
           EVP.dbyte1 == MEASURE_END &&
           EVP.deltime == 0)
        starting_index[ptrack] = PTI;    // Yes, update starting index
      else PTI -= EVP.length;            // No, restore previous event

      j = PTI;                           // Save initial index
      PtrBusy(ON);                       // Set mouse ptr
      do
      {
        PTI = j;                         // Restore current index
        rc = get_event(ptrack);          // Get a track event
        if (rc)                          // Hang it up; something's haywire
        {
          sprintf(msgbfr, "Invalid data (T%d) at index %u", ptrack+1, PTI);
          umsg('E', hWndETrk[ptrack], msgbfr);
          goto errexit;
        }
        if (j == current_index)          // If this is the current track pos.,
          current_index = Lndx;          // save the List-Box index for display

        j = PTI;                         // Save current index
        PTI -= EVP.length;               // Set index for this event
        rc = decode_event(&EVP, ptrack); // This sets text in 'msgbfr'
        if (rc == 20) break;             // Something is haywire again
        if (rc == 2)                     // Measure End
        {
          additem(hWndDlg, ptrack, FALSE);  // Add Measure-End to list box
          strcpy(msgbfr, "");           // To write a blank line after
          additem(hWndDlg, ptrack, FALSE);
          continue;
        }
        additem(hWndDlg, ptrack, rc);    // Add item to list box
        if (rc == 4)
          additem(hWndDlg, ptrack, FALSE); // Add chn cmd event to list
      }
      while (bar_ctr <= PrmBlk->ENumMeas && !(EVP.flags1 & EOT));

      szTrailerBufr = sp.trkdatalnth[ptrack] - j; // Get size of trailer
      TrailerBufr = malloc(szTrailerBufr); // Get storage for trailer
      memcpy(TrailerBufr, PT+j, szTrailerBufr); // Save trailer in buffer
      EVPptr -= 1;                       // Back up one event
      EVPptr->flags2 |= LASTEVENT;       // Indicate last event being edited

      PtrBusy(OFF);
      if (OverRun & 1 << ptrack)         // We didn't make it
      {
        sprintf(msgbfr, "Track %d is too big; use Song Editor to mark a smaller section", ptrack+1);
        WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, msgbfr, "MidiLab/2", 0,
                              MB_OK | MB_ERROR);
      }
      /* Position list box to current place in track  10/08/93   */
      WinSendDlgItemMsg(hWndDlg, 117, LM_SETTOPINDEX,
                          MPFROMLONG(current_index), 0L);
    }
    PTI = NdxSave;                       // Restore index
    break; /* End of UM_UINIT */

    case WM_BUTTON1UP:
      if (BadStart & 1 << ptrack)        // We marked a space or header
      {
        WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,
        MPFROMSHORT(LIT_NONE), MPFROMSHORT(FALSE)); // De-delect all
        BadStart &= 0xffff - (1 << ptrack);   // Reset Bad first mark switch
      }
      break;

    case WM_CONTROL:
      // Go process event list box
      rc = EvListBox(hWndDlg, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1), ptrack);
      if (rc)
        return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
      else break; /* End of WM_CONTROL */

/**********************************************************************/
/*            Process push buttons in main Edit window                */
/**********************************************************************/
    case WM_COMMAND:
      PtrBusy(ON);
      switch(SHORT1FROMMP(mp1))
      {
       case 114:          // Save -  Update the track from event array
         if (OverRun & 1 << ptrack)       // We didn't make it
           goto eQuit;
         if (chkact()) break;
         PTI = starting_index[ptrack];    // Set index of first edited measure
         ClpPtr = EVPptr;                 // Save EVP pointer (9/26/93)
         EVPptr = (EVPK *)evaptr[ptrack]; // Set base event array pointer
         do
         {
           j = TrkBld(EVPptr, ptrack);    // Move packet data to track
           if (EVPptr->flags2 & LASTEVENT)
             break;                       // All done
           else EVPptr += 1;              // Incr to next event
         }
         while (j != -1);                 // Unless track overrun
         EVPptr = ClpPtr;                 // Restore EVP pointer (9/26/93)

         memcpy(PT+PTI, TrailerBufr, szTrailerBufr); // Move in trailer data
         free(TrailerBufr);               // Release buffer storage
         spptr->trkdatalnth[ptrack] = AdjustedLnth[ptrack]; // Set trk lnth

         Locate_SPP(-1);                  // Go reset max measures
         WinSendMsg(hWndClient, UM_SET_HEAD, (MPARAM)0x40, NULL);
         dmgreset(1,0);                   // In case we were playing something
         safequit = ON;                   // Set main track change indicator
         editchng &= 0xffff - (1 << ptrack); // Reset EDIT change indicator
         DosBeep(70,30);                  // Comforting assurance
         break;

       case 115: /* Done            */
        eQuit:
        if (editchng & 1 << ptrack)         // We've changed something
        {
          if (umsg('C', hWndETrk[ptrack],"Track data has changed; Quit anyway?") != MBID_YES)
            break;
          else editchng &= 0xffff - (1 << ptrack);  // Reset EDIT change indicator
        }
/**********************************************************************/
/* Free storage that was used to hold meta event data, unless the     */
/* event has been moved to the clipboard.                             */
/* Note: this routine has some holes in it; e.g., if events are moved */
/* to the clipboard, edit is ended, and the clipboard is subsequently */
/* overlaid with new events, the storage owned by the overlaid events */
/* will not be freed.  9/29/92                                        */
/**********************************************************************/
            EVPptr = (EVPK *)evaptr[ptrack]; // Set base event array pointer
            j = ClipSize / sizeof(EVP);      // j = count of clipbrd items
            while (!(EVPptr->flags2 & LASTEVENT))
            {
              if ((EVPptr->flags1 & METAEVT) && !(EVPptr->flags1 & MLABMETA))
              {
                hitswt = OFF;                   // Reset hit switch
                if (ClipPtr != NULL)            // Something in Clipboard
                {
                  ClpPtr = (EVPK *)ClipPtr;     // Set up working pointer
                  for (i = 0; i < j; ++i)
                  {
                    if ((ClpPtr->flags1 & METAEVT) &&
                       !(ClpPtr->flags1 & MLABMETA))
                         if (EVPptr->Meta_ptr == ClpPtr->Meta_ptr)
                           hitswt = ON;
                    ClpPtr += 1;                // Incr to next clipbrd event
                  }
                }
                if (!hitswt)                    // No match in Clipboard
                  free(EVPptr->Meta_ptr);       // Free meta storage
              }
              EVPptr += 1;                      // Incr to next event
            }

            if (SeqPlay)
            {
              if (Stepmode || PlayBanner)
              {
                umsg('W', hWndETrk[ptrack], "Please Stop-Play first");
                break;
              }
              DosResetEventSem(SemEdit, &PostCt);     // Set initial block
              SeqPlay = LEVEL2;   // Kill any active sequence that's playing
              DosWaitEventSem(SemEdit, -1); // Wait till Play thread is done
              DosResetEventSem(SemEdit, &PostCt);     // Re-set Semaphore
            }
         errexit:
            OverRun &= 0xffff - (1 << ptrack);  // Reset ListBox overrun swt
            emode &= 0xffff - (1 << ptrack);    // Remove edit interlock
            WinPostMsg(hWndClient, UM_SET_HEAD, // Remove HiLite & updt
                       MPFROM2SHORT(0xc0, NULL), NULL);  // contents
            hWndETrk[ptrack] = 0;
            rc = DosFreeMem(evaptr[ptrack]);
            if (rc)
            {
              sprintf(msgbfr, "DosFreeMem failed  RC:%d", rc);
              umsg('E', hWndETrk[ptrack], msgbfr);
            }
            WinDismissDlg(hWndDlg, TRUE);
            break;

       case 126: /* Start-Block    */
       selected:
            SBlk = Xndx;                  // Remember start index
            WinSetDlgItemText(hWndDlg, 126, "STRT BLK"); // Change text
            break;

       case 127: /* Insert         */
            if (Ict > 1)
              umsg('W', hWndETrk[ptrack], "Please select only one item to insert after");
            else WinDlgBox(HWND_DESKTOP, hWndDlg, (PFNWP)E1MsgProc,
                              0, IDLG_INS_MENU, &ptrack);
            break;

       case 128: /* Delete         */
            EditDel(hWndDlg, ptrack);
            for (i = 118; i < 138; ++i)
              WinEnableControl(hWndDlg, i, FALSE); // Disable all
            editchng |= 1 << ptrack;          // Indicate change
            break;

       case 129: /* Cut            */
            ClipWrite(Xndx, Ict, ptrack);     // Write marked items to clipbrd
            EditDel(hWndDlg, ptrack);
            for (i = 118; i < 138; ++i)
               WinEnableControl(hWndDlg, i, FALSE);  // Disable all
            editchng |= 1 << ptrack;          // Indicate change
            break;

       case 130: /* Copy           */
            ClipWrite(Xndx, Ict, ptrack);  // Write marked items to clipbrd
            WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,
            MPFROMSHORT(LIT_NONE), MPFROMSHORT(FALSE)); // De-delect all
            for (i = 118; i < 138; ++i)
              WinEnableControl(hWndDlg, i, FALSE); // Disable all
            break;

       case 131: /* Mark to End - all items from Xindex to end of track    */
            if (Lndx > 3)                    // If trk is not empty
            {
              EVPptr = (EVPK *)evaptr[ptrack]; // Point to first event
              if (Xndx == 0 && EVPptr->flags1 & (EVASPACE+EVAHEADR)) ++Xndx;
              Zndx = Lndx - 2;               // Set Zindex to remainder of trk
              Ict = Zndx - Xndx;             // Set number of marked items

              EVPptr += Xndx;                // Point to start of mark(s)
              Tlnth = 0;                     // Initialize total length
              for (i = 0; i < Ict; ++i)
                Tlnth += (EVPptr+i)->length; // Set track length of marked items

              SeqPlay = ON;                  // To improve speed in EDIT1
              WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,
              MPFROMSHORT(LIT_NONE), MPFROMSHORT(FALSE)); // Clear any previous marks
              for (i = Xndx; i < Lndx-2; ++i)
                WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,
                  MPFROMSHORT(i), MPFROMSHORT(TRUE)); // Mark all selected items
              SeqPlay = OFF;                 // Back to normal
              for (i = 121; i < 135; ++i)
                WinEnableControl(hWndDlg, i, TRUE); // Enable all
            }
            break;

       case 134: /* Paste          */
            if (ClipPtr == NULL)
              umsg('W', hWndETrk[ptrack], "The Clipboard is empty");
            else if (Ict > 1)
              umsg('W', hWndETrk[ptrack], "Please select only one item to paste after");
            else
            {
              Tlnth = Clpln;                 // For adj_xfer (2/28/93)
              EditPaste(hWndDlg, ClipPtr, ClipSize, ptrack);
              editchng |= 1 << ptrack;       // Indicate change
            }
            break;

       case 137: /* Modify         */
            EVPptr = (EVPK *)evaptr[ptrack]; // Set base event array pointer
            EVPptr += Xndx;                  // Point to first marked item
            while (Xndx < Zndx)              // Modify marked block
            {
              if ((EVPptr->evcode == NOTE_ON || EVPptr->evcode == NOTE_OFF) &&
                   !(EVPptr->flags1 & MPUMARK))
              {
                EVPptr->dbyte1 = (EVPptr->dbyte1 + sp.transadj > 127) ?
                  127 : EVPptr->dbyte1 + spptr->transadj;
                if ((EVPptr->evcode == NOTE_ON) && EVPptr->dbyte2) // note-on
                {
                  if (spptr->proflags & VELLEVEL) //Velocity leveling on,
                    EVPptr->dbyte2 = 64;          // force uniform velocity
                  EVPptr->dbyte2 = (EVPptr->dbyte2 + spptr->velocadj > 127) ?
                    127 : EVPptr->dbyte2 + spptr->velocadj;
                }
              }

              if (quantize_amt && EVPptr->deltime < 240) // Quantization active
              {
                EVPptr->deltime = quantize(EVPptr->deltime);
                if (EVPptr->deltime == 240)    // Need to insert an overflow
                {
                  ClpPtr = (EVPK *)malloc(sizeof(EVP)); // Get stg for 1 event
                  Build_EVP(ClpPtr, 240, TIMING_OVFLO, MPUMARK, 0);
                  EditPaste(hWndDlg, ClpPtr, sizeof(EVP), ptrack);
                  free(ClpPtr);
                }
              }

              if (Ict < 100)        // To avoid long delays
              {
                rc = decode_event(EVPptr, ptrack); // Update list box
                if (rc != 8)        // Not space or header
                  WinSendDlgItemMsg(hWndDlg, 117, LM_SETITEMTEXT,
                                     MPFROMSHORT(Xndx), MPFROMP(msgbfr));
              }
              EVPptr += 1;          // Increment to next event
              ++Xndx;               // Increment index
            }
            WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,
              MPFROMSHORT(LIT_NONE), MPFROMSHORT(FALSE)); // De-delect all
            for (i = 118; i < 138; ++i)
              WinEnableControl(hWndDlg, i, FALSE); // Disable all
            editchng |= 1 << ptrack;       // Indicate change
            break;

       case 139: /* Play  ()     */
            if (SeqPlay)
              DosBeep(70,60);     // Only one edit session at a time
            else
            {
              SeqPlay = ON;       // Level 1
              PSt.ptrack = ptrack;
              PSt.hWnd = hWndDlg;
              PStptr = &PSt;
              WinEnableControl(hWndDlg, 140, TRUE); // Enable STOP
              WinSendDlgItemMsg(hWndDlg, 139, BM_SETHILITE, (MPARAM)ON, 0L);
              DosCreateThread((PTID)&PlayTid, (PFNTHREAD)PlayEvnt,
                             (LONG)0, (ULONG)0, (ULONG)4096);
              DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, +5, PlayTid);
            }
            break;

       case 140: /* Stop/Pause Play   */
            SeqPlay = LEVEL2;
            if (Stepmode)
            {
              Stepmode = OFF;
              WinSendDlgItemMsg(hWndDlg, 125, BM_SETCHECK, MPFROMSHORT(OFF), 0L);
              DosPostEventSem(SemStep);   // Allow final step
            }
            WinEnableControl(hWndDlg, 140, FALSE);// Disable STOP
            break;

       case 150: /* All Notes Off  */
            mpucmd(0xb9,0);              // Clear Play Map
            Rel_sustain(TRUE);           // Release any active sustains
            break;

       case 151: /* Clear Track    */
            if (editchng & 1 << ptrack)
              if (umsg('C', hWndETrk[ptrack],"Clear entire track?") != MBID_YES)
                break;
            WinSendDlgItemMsg(hWndDlg, 117, LM_DELETEALL, NULL, NULL);
            EVPptr = (EVPK *)evaptr[ptrack]; // Set base event array pointer
            Build_EVP(EVPptr, 0, 0, EVASPACE, 0);
            EVPptr += 1;
            Build_EVP(EVPptr, 0, END_OF_TRK, MPUMARK+EOT, 0);
            WinSendDlgItemMsg(hWndDlg, 117, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                               MPFROMP(""));
            WinSendDlgItemMsg(hWndDlg, 117, LM_INSERTITEM, MPFROMSHORT(LIT_END),
                               MPFROMP("0    T:0  ** END OF TRACK **"));
            AdjustedLnth[ptrack] = 2;           // Set trk content
            for (i = 118; i < 138; ++i)
              WinEnableControl(hWndDlg, i, FALSE); // Disable all
            editchng &= 0xffff - (1 << ptrack); // Reset EDIT change indicator
            break;

       case IDLG_EDIT1:    /* Button text: "Help"        */
            break;
      }
      PtrBusy(OFF);
      break; /* End of WM_COMMAND */

    case WM_CLOSE:
         goto eQuit;

    case WM_CHAR:
         if (CHARMSG(&message)->fs & KC_CHAR)
         {
           key = CHARMSG(&message)->chr;
           if (key == BIGPLUS)
    case WM_BUTTON2DBLCLK:      // This looks funny, but it's right
             if (Stepmode)
               DosPostEventSem(SemStep);   // Step to next sequence event
         }
         break;

    case WM_BUTTON2DOWN:
         if (WinIsControlEnabled(hWndDlg, 126)) // If Start-Block button active
           goto selected;
         else break;

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
 return FALSE;
}   /* End of EDITMsgProc */
