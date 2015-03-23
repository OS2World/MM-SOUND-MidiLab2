/********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBEDIT2 -    ** MidiLab Track Editor subroutines **            */
/*                                                                   */
/*       Copyright  J. L. Bell        August 1992                    */
/*********************************************************************/
#define  EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
extern   USHORT InitTrks;
extern   LONG starting_index[], AdjustedLnth[];

/**********************************************************************/
/*        additem - Send event to load event list box                 */
/**********************************************************************/
VOID additem(HWND hWnd, SHORT ptrack, BOOL Space)
{
  extern LONG uindex;
  SHORT irc;
  if (Space || !PTI)                     // changed to next line on 1/10/93
  {
    if (!(OverRun & 1 << ptrack))
    {
      irc = (USHORT)WinSendDlgItemMsg(hWnd, 117, LM_INSERTITEM,
                            MPFROMSHORT(LIT_END), MPFROMP("") );
      if (irc == LIT_ERROR || irc == LIT_MEMERROR)
      {
        OverRun |= 1 << ptrack;       // Set overrun flag
        uindex = PTI;                 // Save last available index
      }
      else Lndx = irc;                // Save index of last added item
    }
    EVPptr->flags1 |= EVASPACE;       // Indicate space slot
    EVPptr->length = 0;
    EVPptr += 1;                      // Incr to next slot
  }

  if (!(OverRun & 1 << ptrack))
  {
    irc = (USHORT)WinSendDlgItemMsg(hWnd, 117, LM_INSERTITEM,
                          MPFROMSHORT(LIT_END),
                          MPFROMP((Space == 4) ? hdrbfr : msgbfr));
    if (irc == LIT_ERROR || irc == LIT_MEMERROR)
    {
      OverRun |= 1 << ptrack;         // Set overrun flag
      uindex = PTI;                   // Save last available index
    }
    else Lndx = irc;                  // Save index of last added item
  }
  memcpy((PVOID)EVPptr, &EVP, sizeof(EVP)); // Copy event packet to array slot

  /* If this is a 1.0 meta event, save the data portion */
  if ((EVP.flags1 & METAEVT) && !(EVP.flags1 & MLABMETA))
  {
    EVPptr->Meta_ptr = malloc(EVP.MetaLn); // Get stg for meta data
    memcpy(EVPptr->Meta_ptr, P_metadata, EVP.MetaLn);
  }

  if (Space == 4)
  {
    EVPptr->flags1 |= EVAHEADR;       // Indicate header (NOTE ON, CONTROLLER, etc)
    EVPptr->length = 0;
  }
  if (strlen(msgbfr) == 1)            // Space following measure-end
  {
    EVPptr->flags1 |= EVASPACE;       // Indicate space slot
    EVPptr->length = 0;
  }
  EVPptr += 1;                        // Incr to next slot
}

/**********************************************************************/
/*        EditDel - Delete events and list box items                  */
/**********************************************************************/
VOID EditDel(HWND hWndDlg, SHORT ptrack)
{
  PVOID  Xptr, Zptr;
  SHORT  i;
  BYTE   Xevtype = 0;
  EVPK   *saveptr;

  EVPptr = (EVPK *)evaptr[ptrack] + Xndx;  // Initialize EVP pointer

  saveptr = EVPptr;            // Issue Achtung if header will be deleted
  for (i = 0; i < Ict; ++i)
  {
    if (saveptr->flags1 & EVAHEADR)
    {
      if (umsg('C', hWndETrk[ptrack], "Warning! Delete Channel Status?") == MBID_YES)
        break;                 // Do it
      else return;             // Hang it up
    }
    saveptr += 1;              // look at next event to be deleted
  }

  saveptr = EVPptr + (Ict - 1);        // Point to last (or only) item
  if (saveptr->flags1 & MLABMETA && saveptr->evtype == TRKXFER)
    xferswt = ON; // Don't adjust track xfers in this case
  else adj_xfer('-', ptrack);          // Adjust track xfer if necessary

  Xevtype = EVPptr->evtype;            // Save evtype from Xndx event
  if (!(EVPptr->flags1 & RUNSTAT) &&   // If this event contains a chn cmd
      !(EVPptr->flags1 & MPUMARK) &&   // and is not an MPU Mark
      !(EVPptr->flags1 & METAEVT) &&   // and is not a meta event
      Xndx)                            // and we're away from the first event
  {
    EVPptr += Ict;                     // Look at item following the last marked one
    if (EVPptr->flags1 & RUNSTAT)      // If running status continues,
    {
      EVPptr->flags1 &= 255 - RUNSTAT; // propagate chn cmd status
      if (EVPptr->evtype != Xevtype)   // We need to generate a new header
      {
        saveptr = EVPptr;
        decode_event(EVPptr, ptrack);  // Go do it (set hdrbfr; should be rc4)
        EVPptr = (EVPK *)evaptr[ptrack] + Xndx -1;
        memcpy((PVOID)EVPptr, (PVOID)saveptr, sizeof(EVP));
        EVPptr->flags1 |= EVAHEADR;
        EVPptr->length = 0;
        WinSendDlgItemMsg(hWndDlg, 117, LM_SETITEMTEXT,
                             MPFROMSHORT(Xndx-1), MPFROMP(hdrbfr));
      }
    }
    else                // We're deleting an entire run; delete header also
    {
      ++Ict;                           // Increase item count
      --Xndx;                          // Back up to include header
    }
  }
  else if (Ict > 1)                    // Multiple selections
  {
    EVPptr += Ict;                     // Look at item following the last marked one
    if (EVPptr->evtype != Xevtype)     // We need to generate a new header
    {
      EVPptr->flags1 &= 255 - RUNSTAT; // Force channel cmd
      saveptr = EVPptr;
      decode_event(EVPptr, ptrack);    // Go do it (set msgbfr)
      WinSendDlgItemMsg(hWndDlg, 117, LM_SETITEMTEXT,
                          MPFROMSHORT(Xndx), MPFROMP(hdrbfr));
      EVPptr = (EVPK *)evaptr[ptrack] + Xndx;
      memcpy((PVOID)EVPptr, (PVOID)saveptr, sizeof(EVP));
      EVPptr->flags1 |= EVAHEADR;
    EVPptr->length = 0;
      ++Xndx;                            // Reserve a header slot
      --Ict;
    }
  }
  Xptr = (EVPK *)evaptr[ptrack] + Xndx;  // Point to 1st (or only) selected item
  Zptr = (EVPK *)evaptr[ptrack] + Zndx;  // Point to first unmarked item
  memmove(Xptr, Zptr, (Lndx - Zndx + 1) * sizeof(EVP));
  Lndx -= Ict;                           // Set new value for end of list

  for (i = 0; i < Ict; ++i)              // Unmark items in list box
  {
    WinSendDlgItemMsg(hWndDlg, 117, LM_DELETEITEM, MPFROMSHORT(Xndx), 0L);
  }
  WinSendDlgItemMsg(hWndDlg, 117,        // Make sure everything is deselected
                    LM_SELECTITEM, MPFROMSHORT(LIT_NONE), MPFROMSHORT(TRUE));
  AdjustedLnth[ptrack] -= Tlnth;         // Adj. overall trk lnth
  Re_Index(hWndDlg, ptrack);             // Re-index track
}

/**********************************************************************/
/*        ClipWrite - Save marked items in clipboard                  */
/**********************************************************************/
VOID ClipWrite(SHORT Clipndx, SHORT Clipct, SHORT ptrack)
{
  APIRET rc;
  PVOID Xptr;
  SHORT i, irc;
  EVPK *ClpPtr;

  ClpPtr = (EVPK *)evaptr[ptrack] + Clipndx;   // Point to start of marked area

  if (Xndx)                                    // If not at very top
  {
    ClpPtr -= 1;                               // Back up a moment
    if (ClpPtr->flags1 & EVAHEADR)             // If event header,
      ++Clipct;                                // include in clipbrd
    else ClpPtr += 1;                          // Else restore pointer
  }

  ClipSize = Clipct * sizeof(EVP);             // Set size of clipboard data
  ClipPtr = realloc(ClipPtr, ClipSize);        // Get storage for marked area
  if (ClipPtr == NULL)
  {
    sprintf(msgbfr, "Cannot get storage for clipboard\n (need %d bytes)", ClipSize);
    umsg('E', hWndETrk[ptrack], msgbfr);
  }
  else
  {
    Xptr = ClpPtr;                       // Need PVOID for memcpy
    memcpy(ClipPtr, Xptr, Clipct * sizeof(EVP)); // Copy to clipboard

    /*  Load the Clipboard list box with a representation of the events */
    WinSendDlgItemMsg(hWndCLIP, 148, LM_DELETEALL, NULL, NULL);
    bar_ctr = 1;                         // Reset measure counter
    tmg_ctr = 0;                         // Reset timing counter
    for (i = 0; i < Clipct; ++i)
    {
      rc = decode_event(ClpPtr, ptrack); // This sets text in 'msgbfr/hdrbfr'
      if (rc == 20) break;               // Something is haywire
      if (rc == 8)                       // Space/Header slot
      {
        if (ClpPtr->flags1 & EVASPACE)   // Space slot
          irc = (USHORT)WinSendDlgItemMsg(hWndCLIP, 148, LM_INSERTITEM,
                            MPFROMSHORT(LIT_END), MPFROMP("") );
        else
        {
          ClpPtr += 1;                   // Point to next packet
          continue;                      // Keep going
        }
      }
      else
      {
        if (rc == 4)
          irc = (SHORT)WinSendDlgItemMsg(hWndCLIP, 148, LM_INSERTITEM,
                             MPFROMSHORT(LIT_END), MPFROMP(hdrbfr));
        irc = (SHORT)WinSendDlgItemMsg(hWndCLIP, 148, LM_INSERTITEM,
                           MPFROMSHORT(LIT_END), MPFROMP(msgbfr));
      }
      if (irc == LIT_ERROR || irc == LIT_MEMERROR)
      {
        umsg('E', hWndETrk[ptrack], "INSERTITEM err on Clipboard");
//      break;
      }
      ClpPtr += 1;          // Point to next packet
    }
  }
  Clpln = Tlnth;            // Set aggregate length of clipboard data
}

/**********************************************************************/
/*       EditPaste - Paste new data from clipboard or Insert          */
/**********************************************************************/
VOID EditPaste(HWND hWndDlg, PVOID Pdata, ULONG Psize, SHORT ptrack)
{
  APIRET rc;
  PVOID tbuffer, Xptr;
  SHORT i, irc;
  EVPK *ClpPtr;
  ULONG RegionSize, MemUtil, szTrail;

  szTrail = (Lndx - Zndx + 1) * sizeof(EVP); // Set length of trailer piece
  tbuffer = malloc(szTrail);                 // Get storage for trailer
  if (tbuffer == NULL)
  {
    sprintf(msgbfr, "Cannot get storage for Paste\n (need %d bytes)", szTrail);
    umsg('E', hWndETrk[ptrack], msgbfr);
    return;
  }

  RegionSize = 8000000;           // Go for a preposterous storage size
  DosQueryMem(evaptr[ptrack], &RegionSize, &MemUtil); // Get the actual size
  MemUtil = Lndx * sizeof(EVP) + Psize;      // Calculate delta for this paste
  if (MemUtil >= RegionSize)                 // Will it fit?
  {
    umsg('W', hWndETrk[ptrack], "The work space is exhausted; please save and re-edit the track");
    return;
  }

  if (Xndx)                                 // If not inserting at beginning,
    adj_xfer('+', ptrack);                  // adjust track xfers as necessary
  Xptr = (EVPK *)evaptr[ptrack] + Xndx+1;   // Point to Paste mark
  memcpy(tbuffer, Xptr, szTrail);           // Save trailing end
  memcpy(Xptr, Pdata, Psize);               // Move in new data
  memcpy((PCHAR)Xptr+Psize, tbuffer, szTrail);  // Tack trailer behind new data
  free(tbuffer);                            // All done with buffer

  /* Now do the list box */
  ClpPtr = (EVPK *)Xptr;
  szTrail = Xndx;                  // Borrow szTrail variable to preserve Xndx
  for (i = 0; i < (Psize/sizeof(EVP)); ++i)
  {
    rc = decode_event(ClpPtr, ptrack); // This sets text in 'msgbfr/hdrbfr'
    if (rc == 20) break;               // Something is haywire
    if (rc == 8)                       // Space/Header slot
    {
      if (ClpPtr->flags1 & EVASPACE)   // Space slot
        irc = (USHORT)WinSendDlgItemMsg(hWndDlg, 117, LM_INSERTITEM,
                          MPFROMSHORT(++szTrail), MPFROMP("") );
      else
      {
        ClpPtr += 1;                   // Point to next packet
        continue;                      // Keep going
      }
    }
    else
    {
      if (rc == 4)
        irc = (SHORT)WinSendDlgItemMsg(hWndDlg, 117, LM_INSERTITEM,
                           MPFROMSHORT(++szTrail), MPFROMP(hdrbfr));
      irc = (SHORT)WinSendDlgItemMsg(hWndDlg, 117, LM_INSERTITEM,
                         MPFROMSHORT(++szTrail), MPFROMP(msgbfr));
    }
    if (irc == LIT_ERROR || irc == LIT_MEMERROR)
    {
      umsg('E', hWndETrk[ptrack], "INSERTITEM err on Clipboard");
      break;
    }
    ClpPtr += 1;                       // Point to next packet
  }
  Lndx += Psize / sizeof(EVP);         // Incr last index to include pasted stuff
  AdjustedLnth[ptrack] += Tlnth;       // Adj. overall trk lnth
  Re_Index(hWndDlg, ptrack);           // Re-index track
}

/**********************************************************************/
/*        decode_event - Prepare a text string for list box item      */
/*        rc = FALSE: ordinary event                                  */
/*        rc = TRUE : Need a space                                    */
/*        rc = 2    : Need 2 spaces  (Measure End)                    */
/*        rc = 4    : Channel command found, header is in hdrbfr      */
/*        rc = 8    : Space or existing header slot; ignored          */
/*        rc = 20   : Cannot decode the event                         */
/**********************************************************************/
BYTE decode_event(PVOID pevent, SHORT ptrack)
{
  SHORT i, j;
  EVPK *event;
  char *utilpntr;
  BYTE hdrswt = OFF;

  event = pevent;                    // Get pointer to event

  if (event->flags1 & (EVASPACE+EVAHEADR)) return(8);

  tmg_ctr += event->deltime;         // Increment timing counter
  if (event->deltime == 240)
  {
    sprintf(msgbfr, "%-4u T:240 * NULL TIME *", PTI);
    return(FALSE);
  }

  if (event->flags1 & METAEVT)       // Meta event
  {
    if (event->flags1 & MLABMETA)    // MidiLab track order
    {
      if (event->evtype == TRKXFER)
        sprintf(msgbfr, "%-4u T:%-3u * XFER to %u; %u repeat(s) *",
         PTI, event->deltime, PTI + 7 - event->trkcmddata, event->dbyte1);

      else if (event->evtype == TRKREST)
      {
        sprintf(msgbfr, "%-4u T:%-3u * REST - %u Bar(s) *",
              PTI, event->deltime, event->dbyte1+1);
        bar_ctr += event->dbyte1;          // Increment measure counter
      }

      else if (event->evtype == TRKINIT)
      {
        InitTrks = event->dbyte2;          // Set tracks 9 - 16
        InitTrks <<= 8;                    // Shift to hi order
        InitTrks |= event->dbyte1;         // Set tracks 1 - 8
        sprintf(msgbfr, "%-4u T:%-3u * TRK INIT - Trk Map:$%4.4X  Start:%u *",
              PTI, event->deltime, InitTrks, event->trkcmddata);
      }

      else if (event->evtype == TRKCMDX)
      {
        if (event->in_chan == TCMTRN)
          sprintf(msgbfr, "%-4u T:%-3u * ADJ. TRANSPOSE: %+d *",
              PTI, event->deltime, (signed char)event->dbyte2);
        else if (event->in_chan == TCMVEL)
          sprintf(msgbfr, "%-4u T:%-3u * ADJ. VELOCITY: %+d *",
              PTI, event->deltime, (signed char)event->dbyte2);
        else  sprintf(msgbfr, "%-4u T:%-3u * MPU CMD:$%X  Data:%u *",
              PTI, event->deltime, event->dbyte1, event->dbyte2);
      }
    }
    else                               // Standard 1.0 Meta event
    {
      i = event->evcode;               // Get meta event code
      j = event->MetaLn;               // Get length of event
      sprintf(msgbfr, "%-4u T:%-3u * META EVENT X'%02X' Length:%ld *",
                 PTI,  event->deltime, i, j);
    }
    return(FALSE);
  }

  if (event->flags1 & MLABMETA)      /* Non-meta track order      */
  {
    sprintf(msgbfr, "%-4u T:%-3u * OLD-STYLE TRK ORDER *",
              PTI, event->deltime);
    return(FALSE);
  }

  if (event->flags1 & MPUMARK)       /* MPU mark                  */
  {
    switch (event->dbyte1)
    {
      case MEASURE_END:
        sprintf(msgbfr, "%-4u T:%-3u * END MEASURE %d (%d) *",
            PTI, event->deltime, bar_ctr, tmg_ctr);
        ++bar_ctr;                   // Increment measure counter
        tmg_ctr = 0;                 // Reset timing count
        return(2);

      case NOP:
        sprintf(msgbfr, "%-4u T:%-3u * NO-OP *", PTI , event->deltime);
        return(FALSE);

      case END_OF_TRK:
        sprintf(msgbfr, "%-4u T:%-3u * END OF TRACK *",
                  PTI, event->deltime);
        return(TRUE);
    }
  }
  switch (event->evtype & 0xf0)
  {
    case NOTE_ON:
    case NOTE_OFF:
      if (!(event->flags1 & RUNSTAT))
      {
        utilpntr = (event->evcode == NOTE_ON) ? "ON" : "OFF";
        sprintf(hdrbfr, "* NOTE %s - Ch%d *", utilpntr, event->in_chan+1);
        hdrswt = ON;         // Indicate header created
      }
      sprintf(msgbfr, "%-4u T:%-3u %-6s V%u",
              PTI, event->deltime, Resolve_Note(event->dbyte1,0), event->dbyte2);
      return(hdrswt ? 4 : FALSE);

    case CONTROLLER:
      if (!(event->flags1 & RUNSTAT))
      {
        sprintf(hdrbfr, "* CONTROLLER CHANGE - Ch%d *", event->in_chan+1);
        hdrswt = ON;         // Indicate header created
      }
      utilpntr = cntrl_type(event->dbyte1); /* Go look up control type */
      sprintf(msgbfr, "%-4u T:%-3u %s: %-3u",
              PTI, event->deltime, utilpntr, event->dbyte2);
      return(hdrswt ? 4 : FALSE);

    case PITCH_BEND:
      if (!(event->flags1 & RUNSTAT))
      {
        sprintf(hdrbfr, "* PITCH BEND - Ch%d *", event->in_chan+1);
        hdrswt = ON;         // Indicate header created
      }
      sprintf(msgbfr, "%-4u T:%-3u LSB:%-3u MSB:%-3u",
              PTI, event->deltime, event->dbyte1, event->dbyte2);
      return(hdrswt ? 4 : FALSE);

    case POLY_AFTER:
      if (!(event->flags1 & RUNSTAT))
      {
        sprintf(hdrbfr, "* POLY AFTER-TOUCH - Ch%d *",event->in_chan+1);
        hdrswt = ON;         // Indicate header created
      }
      sprintf(msgbfr, "%-4u T:%-3u Note:%-3u PV:%-3u",
              PTI, event->deltime, event->dbyte1, event->dbyte2);
      return(hdrswt ? 4 : FALSE);

    case PGM_CHANGE:
      if (!(event->flags1 & RUNSTAT))
      {
        sprintf(hdrbfr, "* PROGRAM CHANGE - Ch%d *", event->in_chan+1);
        hdrswt = ON;         // Indicate header created
      }
      sprintf(msgbfr, "%-4u T:%-3u Pgm:%-2u",
              PTI, event->deltime, event->dbyte1);
      return(hdrswt ? 4 : FALSE);

    case AFTER_TOUCH:
      if (!(event->flags1 & RUNSTAT))
      {
        sprintf(hdrbfr, "* CHANNEL AFTER-TOUCH - Ch%d *", event->in_chan+1);
        hdrswt = ON;         // Indicate header created
      }
      sprintf(msgbfr, "%-4u T:%-3u PV:%-3u",
              PTI, event->deltime, event->dbyte1);
      return(hdrswt ? 4 : FALSE);

    default:
      sprintf(msgbfr, "Unknown event type at Index %d", PTI);
      umsg('E', hWndETrk[ptrack], msgbfr);
      return(20);
  }
}

/*********************************************************************/
/*          Routine to resolve a note number into a real note        */
/* LEVEL0 produces: C3/60                                            */
/* LEVEL1 produces: C3                                               */
/*********************************************************************/
PCHAR Resolve_Note(BYTE note, BYTE lvl)
{
  SHORT i, j;
  static char noteFmt[8];
  char *note_resolution_table[12] = {"C","C#","D","Eb","E","F",
                                     "F#","G","Ab","A","Bb","B"};
  i = note / 12 - 2;                /* Note registration/octave  */
  j = note % 12;                    /* Index to note resolution  */
  if (lvl)
    sprintf(noteFmt, "%s%d", note_resolution_table[j], i); //Just show note
  else sprintf(noteFmt, "%s%d/%u", note_resolution_table[j], i, note); //Show note & num
  return(noteFmt);
}

/*********************************************************************/
/*   Routine to re-index events after a Delete, Insert, or Modify    */
/*   If mode = ON, update the List Box.   (this takes some time)     */
/*********************************************************************/
VOID Re_Index(HWND hWndDlg, SHORT ptrack)
{
  USHORT EVndx = starting_index[ptrack], Lstndx = 0;
  EVPK *event;
  BYTE refreshSwt = ON;

  PtrBusy(ON);
  event = (EVPK *)evaptr[ptrack];        // Point to first event in track
  bar_ctr = 1;                           // Reset measure counter
  tmg_ctr = 0;                           // Reset timing counter

  if (Lndx > 500 && umsg('C', hWndDlg, "Refresh Event List?") != MBID_YES)
    refreshSwt = OFF;                    // Might take a while in this case

  while (TRUE)
  {
    if (!(event->flags1 & EVASPACE+EVAHEADR))
    {
      if (refreshSwt)
      {
        PTI = EVndx;
        decode_event(event, ptrack);       // Update list box
        WinSendDlgItemMsg(hWndDlg, 117, LM_SETITEMTEXT,
                          MPFROMSHORT(Lstndx), MPFROMP(msgbfr));
      }
      PTI = EVndx;                         // Set index of this event
      EVndx += event->length;              // Increment index counter
    }
    if ((event->flags2 & LASTEVENT) ||     // All done
        (event->flags1 & EOT))
      break;
    event += 1;                            // Point to next event
    ++Lstndx;                              // Increment list index
  }
  PtrBusy(OFF);
}

/*********************************************************************/
/* Adjust any following xfers to compensate for deleted/inserted data*/
/* If mode = '+' data was inserted; '-' means data was deleted       */
/*********************************************************************/
VOID adj_xfer(BYTE mode, SHORT ptrack)
{
  LONG CurNdx = 0, target;
  SHORT i;
  EVPK *EVPadj;

  if (xferswt)                        // if we just inserted a transfer
  {
    xferswt = OFF;                    // Reset transfer insert swt
    return;                           // Don't do anything
  }

  EVPadj = (EVPK *)evaptr[ptrack];    // Point to start of EVP array
  for (i = 0; i < Xndx; ++i)
    CurNdx += (EVPadj+i)->length;     // Develop index up to the marked item

  EVPadj += Xndx;                     // Point to marked item
  if (mode == '+')                    // If Paste or Insert
  {
    EVPadj += 1;                      // skip to next event
    Ndx_of_Mark += EVPptr->length;    // Update index of 1st marked item
  }

  while (!(EVPadj->flags2 & LASTEVENT))
  {
    if (EVPadj->flags1 & MLABMETA && EVPadj->evtype == TRKXFER)
    {
      target = CurNdx + 7 - EVPadj->trkcmddata;
      if (Ndx_of_Mark >= target)         // Within scope of xfer
      {
        if (mode == '+')                 // Data inserted
          EVPadj->trkcmddata += Tlnth;   // Adj target by inserted amount
        else                             // Data deleted
          EVPadj->trkcmddata -= Tlnth;   // Adj target by deleted amount
      }
      break;                             // Quit after the first one
    }
    CurNdx += EVPadj->length;            // Update current index
    EVPadj+= 1;                          // Incr to next event
  }
}
