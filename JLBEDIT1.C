/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBEDIT1 -    ** MidiLab Track Editor dialog procedures (II)    */
/*                                                                   */
/*       Copyright  J. L. Bell        July 1992                      */
/*********************************************************************/
#define  EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

/**********************************************************************/
/*            Clipboard message processing routine                    */
/**********************************************************************/
MRESULT EXPENTRY E2MsgProc(HWND hWndDlg,
                              USHORT message, MPARAM mp1, MPARAM mp2)
{
  SHORT i;
  BYTE ptrack;

  for (i = 0; i < 16; ++i)
    if (hWndETrk[i])           // Find the active edit handle
    {
      ptrack = i;
      break;
    }

  switch(message)
  {
    case WM_COMMAND:

      switch(SHORT1FROMMP(mp1))
      {
        case 115:                   // Hide Clipboard
          if (emode)                // At least one edit is active
            WinSendMsg(hWndETrk[ptrack], WM_CONTROL, MPFROM2SHORT(141,0), 0L); // Simulate clipbrd checkbox
          else
          {
            WinShowWindow(hWndDlg, FALSE);  // Hide the clipboard
            ClipShow = OFF;
          }
          break;
      }
      break;

    case WM_CONTROL:
      switch(SHORT1FROMMP(mp1))
      {
        case 148:         // ClipBoard list box
          break;
      }
    default:
      return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
  }
  return(0);
}

/**********************************************************************/
/*            Process Event List Box control                          */
/**********************************************************************/
BYTE EvListBox(HWND hWndDlg, SHORT cntl, SHORT ncode, SHORT ptrack)
{
  USHORT i;
  SHORT Mrc;
  LONG llong, lHi, lLo;

  switch (cntl)
  {
    case 117: // Event List box
    switch(ncode)                         // Switch on Notification Code
   {
     case LN_SELECT:                      // Item is being selected
      if (SeqPlay) break;                 // Moving banner; get out quick
      if ((BadStart & 1 << ptrack) && Ict == 1) break; // We started off with a space/hdr

      Xndx = SHORT1FROMMR(WinSendDlgItemMsg(hWndDlg, 117, LM_QUERYSELECTION,
                           MPFROMSHORT(LIT_FIRST), 0L));
      Zndx = Xndx;                        // Initialize Zndx

      // These next two lines added 9/22 to avoid problems with deleting 1st item
      mresReply = WinSendDlgItemMsg(hWndDlg, 117, LM_QUERYITEMCOUNT, NULL, NULL);
      Lndx = (SHORT)mresReply;            // Set last index

      if (SBlk)                           // Start-Block func added 3/15/93
      {
        PtrBusy(ON);                      // Mouse ptr = clock
        SeqPlay = ON;                     // To improve speed
        if (Xndx < SBlk)                  // Ending mark is ahead of start
        {
          Zndx = SBlk;                    // SBlk is last item in this case
          WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,
            MPFROMSHORT(Zndx), MPFROMSHORT(TRUE)); // Mark last item in block
        }
        else Xndx = SBlk;                 // Set Xindex to start of block
        SBlk = OFF;                       // Reset until next time
        for (i = Xndx; i < Zndx; ++i)
          WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,
            MPFROMSHORT(i), MPFROMSHORT(TRUE)); // Mark all items in block
        WinSetDlgItemText(hWndDlg, 126, "Start Block"); // Set button back
        SeqPlay = OFF;                    // Back to normal
        PtrBusy(OFF);
      }
      else while (TRUE)                   // Loop while dragging mouse
      {
        Mrc = SHORT1FROMMR(WinSendDlgItemMsg(hWndDlg, 117,
                          LM_QUERYSELECTION, MPFROMSHORT(Zndx), 0L));

        if (Mrc == LIT_NONE) break;
        else Zndx = Mrc;                  // Save ending index
      }

      ++Zndx;                             // Bump up for subsequent calculations
      Ict = Zndx - Xndx;                  // Save number of marked items
      EVPptr = (EVPK *)evaptr[ptrack];    // Point to start of EVP array

      Ndx_of_Mark = 0;                    // Initialize Index of marked item
      for (i = 0; i < Xndx; ++i)
        Ndx_of_Mark += (EVPptr+i)->length;// Develop index of 1st marked item

      EVPptr += Xndx;                     // Point to start of mark(s) 2/28/93
      Tlnth = 0;                          // Initialize total length
      for (i = 0; i < Ict; ++i)
        Tlnth += (EVPptr+i)->length;      // Set track length of marked items

      EVPptr = (EVPK *)evaptr[ptrack];    // Point to start of EVP array
      EVPptr += Zndx - 1;                 // Point to selected slot 2/8/93
      for (i = 121; i < 124; ++i)
        WinSetDlgItemText(hWndDlg, i, "");       // Clear field headings

      if (EVPptr->flags1 & (EVASPACE+EVAHEADR) ||// If space or header slot
         (EVPptr->flags1 & EOT))                 // or end-of-track
      {
        DosBeep(700,36);                         // Give gentle indication
        for (i = 118; i < 138; ++i)              // Include Modify  2/6/93
          WinEnableControl(hWndDlg, i, FALSE);   // Disable all
        if (EVPptr->flags1 & EOT || Xndx)        // If end-of-track or not at top
        {
          if (Ict == 1)                          // We can't start marking here
            BadStart |= 1 << ptrack;             // Ignore multiple add'l marks
          return(8);                             // just go away
        }
        else
        {
          WinEnableControl(hWndDlg, 127, TRUE);    // Enable Insert
          WinEnableControl(hWndDlg, 131, TRUE);    // Enable Mark-to-End
          if (ClipSize)                            // If something in clipboard,
            WinEnableControl(hWndDlg, 134, TRUE);  // enable Paste
        }
        break;
      }

      if (EVPptr->deltime == 240)                // Timing Overflow
        for (i = 121; i < 140; ++i)
          WinEnableControl(hWndDlg, i, TRUE);    // Enable all except spin btns
      else
      {
        WinSetDlgItemText(hWndDlg, 121, "Timing");
        WinSendDlgItemMsg(hWndDlg, 118, SPBM_SETCURRENTVALUE,
          MPFROMLONG(EVPptr->deltime), 0);

        llong = EVPptr->dbyte1;                  // Move dbyte1 to long
        WinSendDlgItemMsg(hWndDlg, 119, SPBM_SETLIMITS,  // Data byte1 Spin B.
          MPFROMLONG(EVPptr->flags1 & MLABMETA ? 255 : 127), MPFROMLONG(0));
        EVPptr->dbyte1 = llong;                  // Restore dbyte1
        WinSendDlgItemMsg(hWndDlg, 119, SPBM_SETCURRENTVALUE,
          MPFROMLONG(llong), 0);

        for (i = 118; i < 140; ++i)
          WinEnableControl(hWndDlg, i, TRUE);   // Enable all controls
      }

      if (EVPptr->flags1 & MLABMETA)            // MidiLab track order
      {
        switch (EVPptr->evtype)
        {
         case TRKXFER:
         case TRKREST:
          WinEnableControl(hWndDlg, 120, FALSE);  // Disable dbyte2 spin button
          WinSetDlgItemText(hWndDlg, 122, "Repeats");
          break;

         case TRKINIT:
          WinSetDlgItemText(hWndDlg, 122, "Trk Map");
          WinEnableControl(hWndDlg, 120, FALSE);  // Disable dbyte2 spin button
          break;

         case TRKCMDX:
          if (EVPptr->in_chan & TCMTRN)
          {
            llong = (CHAR)EVPptr->dbyte2;          // Move dbyte2 to long
            lHi = 36; lLo = -36;                   // Set limits for xpose
            WinSetDlgItemText(hWndDlg, 123, "Trnspse ");
            WinEnableControl(hWndDlg, 119, FALSE); // Disable dbyte1 spin button
          }
          else if (EVPptr->in_chan & TCMVEL)
          {
            llong = (CHAR)EVPptr->dbyte2;          // Move dbyte2 to long
            lHi = 50; lLo = -15;                   // Set limits for velocity
            WinSetDlgItemText(hWndDlg, 123, "Velocity");
            WinEnableControl(hWndDlg, 119, FALSE); // Disable dbyte1 spin button
          }
          else
          {
            llong = EVPptr->dbyte2;                // Move dbyte2 to long
            lHi = 255; lLo = 0;                    // Set limits for cmd byte
            WinSetDlgItemText(hWndDlg, 122, "Command");
            WinSetDlgItemText(hWndDlg, 123, " Data ");
          }

          WinSendDlgItemMsg(hWndDlg, 120, SPBM_SETLIMITS,  // Data byte2 Spin B.
            MPFROMLONG(lHi), MPFROMLONG(lLo));
          EVPptr->dbyte2 = llong;                  // Restore dbyte2
          WinSendDlgItemMsg(hWndDlg, 120, SPBM_SETCURRENTVALUE,
          MPFROMLONG(llong), 0);
          break;
        }
        break;
      }
      else if (EVPptr->flags1 & METAEVT)        // 1.0 Meta event
      {
        memcpy(msgbfr, EVPptr->Meta_ptr, EVPptr->MetaLn); // Move text to msgbfr
        *(msgbfr + EVPptr->MetaLn) ='\0';       // Set terminator
        WinMessageBox(HWND_DESKTOP, hWndDlg, msgbfr,
                  "Meta Event", 0, MB_OK | MB_ICONASTERISK);
        goto meta_disable;                      // Disable Spins 2 & 3
      }

      if (!(EVPptr->flags1 & MPUMARK) )         // Not an MPU mark
      {
        switch (EVPptr->evcode)
        {
         case NOTE_ON:
         case NOTE_OFF:
          sprintf(msgbfr, "Note: %s", Resolve_Note(EVPptr->dbyte1, LEVEL1));
          WinSetDlgItemText(hWndDlg, 122, msgbfr); // Show the note
          WinSetDlgItemText(hWndDlg, 123, "Velocity");
          break;

         case PGM_CHANGE:
          WinSetDlgItemText(hWndDlg, 122, "Program");
          break;

         case CONTROLLER:
          WinSetDlgItemText(hWndDlg, 122, "Control");
          WinSetDlgItemText(hWndDlg, 123, " Value");
          break;

         case PITCH_BEND:
          WinSetDlgItemText(hWndDlg, 122, "  LSB");
          WinSetDlgItemText(hWndDlg, 123, "  MSB");
          break;

         default:
          break;
        } // End switch(evcode)

        if (EVPptr->flags1 & DBYTES2)
        {
          llong = EVPptr->dbyte2;               // Move dbyte2 to long
          WinSendDlgItemMsg(hWndDlg, 120, SPBM_SETCURRENTVALUE,
            MPFROMLONG(llong), 0);
        }
        else WinEnableControl(hWndDlg, 120, FALSE);
        if ((XeqSwt & 1 << ptrack) && !SeqPlay) // Go to play/execute this event
          XeqEvent(ptrack, EVPptr);
      }                            //End If-not-Mark
      else if (!(EVPptr->flags1 & MLABMETA))      // Disable spin buttons unless trk order
      {
      meta_disable:
        WinEnableControl(hWndDlg, 119, FALSE);    // Disable dbyte1 if MPU Mark
        WinEnableControl(hWndDlg, 120, FALSE);    // Disable dbyte2 if MPU Mark
        if (EVPptr->deltime == 240)               // Timing Overflow
          WinEnableControl(hWndDlg, 118, FALSE);  // Disable timing
      }
      break;

     default:
      return(4);
    }
    break;

    case 118:                // Delta time Spin Button
    switch(ncode)
    {
      case SPBN_ENDSPIN:
        decode_event(EVPptr, ptrack);
        WinSendDlgItemMsg(hWndDlg, 117, LM_SETITEMTEXT, MPFROMSHORT(Xndx), MPFROMP(msgbfr));
        break;

      case SPBN_UPARROW:
      case SPBN_DOWNARROW:
        editchng |= 1 << ptrack;       // Indicate change
        break;

      case SPBN_CHANGE:
        WinSendDlgItemMsg(hWndDlg, cntl, SPBM_QUERYVALUE,
                    &EVPptr->deltime, 0);
        break;
    }
    break;

    case 119:                // Data byte1 Spin Button
    switch(ncode)
    {
      case SPBN_ENDSPIN:
        decode_event(EVPptr, ptrack);
        WinSendDlgItemMsg(hWndDlg, 117, LM_SETITEMTEXT, MPFROMSHORT(Xndx), MPFROMP(msgbfr));
        break;

      case SPBN_UPARROW:
      case SPBN_DOWNARROW:
        editchng |= 1 << ptrack;       // Indicate change
        break;

      case SPBN_CHANGE:
        WinSendDlgItemMsg(hWndDlg, cntl, SPBM_QUERYVALUE, &llong, 0);
        EVPptr->dbyte1 = (BYTE)llong;
        if (EVPptr->evcode == NOTE_ON || EVPptr->evcode == NOTE_OFF)
        {
          sprintf(msgbfr, "Note: %s", Resolve_Note(EVPptr->dbyte1, LEVEL1));
          WinSetDlgItemText(hWndDlg, 122, msgbfr); // Show the note
        }
        break;
    }
    break;

    case 120:                // Data byte2 Spin Button
    switch(ncode)
    {
      case SPBN_ENDSPIN:
        decode_event(EVPptr, ptrack);
        WinSendDlgItemMsg(hWndDlg, 117, LM_SETITEMTEXT, MPFROMSHORT(Xndx), MPFROMP(msgbfr));
        break;

      case SPBN_UPARROW:
      case SPBN_DOWNARROW:
        editchng |= 1 << ptrack;       // Indicate change
        break;

      case SPBN_CHANGE:
        WinSendDlgItemMsg(hWndDlg, cntl, SPBM_QUERYVALUE, &llong, 0);
        EVPptr->dbyte2 = (BYTE)llong;
        break;
    }
    break;

    case 124:         // Toggle Play banner move
     PlayBanner = (PlayBanner)? OFF : ON;
     for (i = 0; i < 16; ++i)
       if (hWndETrk[i])  // Set banner checkbox for all active Edit windows
         WinSendDlgItemMsg(hWndETrk[i], 124, BM_SETCHECK, MPFROMSHORT(PlayBanner), 0L);
     break;

    case 125:         // Toggle Step mode
     if (Stepmode)
     {
       Stepmode = OFF;
       DosPostEventSem(SemStep);   // Unblock Play thread and turn it loose
     }
     else
    {
       Stepmode = ON;
       AutoOff |= 1 << ptrack;         // Force Auto Note-Off
       WinSendDlgItemMsg(hWndDlg, 136, BM_SETCHECK, MPFROMSHORT(ON), 0L);
     }
     for (i = 0; i < 16; ++i)
       if (hWndETrk[i])  // Set stepmode checkbox for all active Edit windows
         WinSendDlgItemMsg(hWndETrk[i], 125, BM_SETCHECK, MPFROMSHORT(Stepmode), 0L);
     break;

    case 135:         // Toggle Play selected event
     XeqSwt ^= 1 << ptrack;
     break;

    case 136:         // Toggle Automatic Note-Off
     AutoOff ^= 1 << ptrack;
     break;

    case 141:         // Toggle Show Clipboard
     ClipShow = (ClipShow) ? FALSE : TRUE;
     for (i = 0; i < 16; ++i)
       if (hWndETrk[i])  // Set Clipbrd checkbox for all active Edit windows
         WinSendDlgItemMsg(hWndETrk[i], 141, BM_SETCHECK, MPFROMSHORT(ClipShow), 0L);
     WinShowWindow(hWndCLIP, ClipShow);
     if (ClipShow)
       WinSetFocus(HWND_DESKTOP, hWndCLIP);
     break;
  }
  return(0);
}

/*********************************************************************/
/*             Thread to play a pre-selected sequence                */
/*********************************************************************/
VOID PlayEvnt(VOID)
{
  BYTE ptrack;
  HWND hWndDlg;
  EVPK *PlayEv;
  SHORT TopNdx, PlayNdx;
  ULONG PostCt, ulDelay;

  hMQPSt = WinCreateMsgQueue(hAB, 0);         // Set up a msg queue
  ptrack = PStptr->ptrack;                    // Set current track number
  hWndDlg = PStptr->hWnd;                     // Point to active Edit window
  PlayNdx = Xndx;             // Use a special index
  PlayEv = evaptr[ptrack];    // Use special EVP ptr so as not to disturb other edits
  PlayEv += PlayNdx;          // Make sure event slot matches index

  while (SeqPlay != LEVEL2 && PlayNdx < Zndx)
  {
    if (Stepmode)                             // Waiting for key-stroke
    {
      DosResetEventSem(SemStep, &PostCt);     // Re-set Semaphore
      DosWaitEventSem(SemStep, -1);           // Wait for post from WM_CHAR
    }
    else if (PlayEv->deltime)
    {
      ulDelay = ms_per_tick * PlayEv->deltime;// Set msecs for HRT block
      Delay(ulDelay);
    }
    XeqEvent(ptrack, PlayEv);                 // Send event to MPU
    PlayEv += 1;                              // Incr to next event
    if (PlayBanner)                           // If moving Select Bar
    {
      TopNdx = SHORT1FROMMR(WinSendDlgItemMsg(hWndDlg, 117, LM_QUERYTOPINDEX, 0, 0));
      if (PlayNdx > (TopNdx + 18))  // If select bar is at bottom (was +14)
        WinSendDlgItemMsg(hWndDlg, 117, LM_SETTOPINDEX,  // Move to top
          MPFROMSHORT(PlayNdx), 0L);
      WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,     // Turn off old position
        MPFROMSHORT(PlayNdx), MPFROMSHORT(FALSE));
    }
    ++PlayNdx;                   // Increment List Box index
    if (PlayBanner)
      WinSendDlgItemMsg(hWndDlg, 117, LM_SELECTITEM,     // Turn on new position
        MPFROMSHORT(PlayNdx), MPFROMSHORT(TRUE));
  }
  SeqPlay = OFF;
  mpucmd(0xb9,0);                       // Clear Play Map
  DosPostEventSem(SemEdit);             // In case Edit is terminating
  WinEnableControl(hWndDlg, 140, FALSE);// Disable STOP
  WinSendDlgItemMsg(hWndDlg, 139, BM_SETHILITE, (MPARAM)OFF, 0L); // Play ->up
  WinDestroyMsgQueue(hMQPSt);           // Destroy this thread's msgq
  DosExit(EXIT_THREAD, 0);              // Evaporate
}

/**********************************************************************/
/*       Execute selected MIDI event  (this is real good stuff)       */
/**********************************************************************/
VOID XeqEvent(SHORT ptrack, PVOID Event)
{
  BYTE ChnCmd;
  EVPK *PlayEv;

  PlayEv = Event;
  if (PlayEv->flags1 & MPUMARK+METAEVT  || // Ignore MPU marks, meta events,
      PlayEv->flags1 & EVAHEADR+EVASPACE)  // headers, and spaces
      return;

  if (PlayEv->evcode == PGM_CHANGE &&    // Ignore Pgm changes
      sp.ccswt == LEVEL3) return;        // if filter is on

  ChnCmd = (spptr->tcc[ptrack]) ?        // Are we remapping the channel
    PlayEv->evcode | (spptr->tcc[ptrack]-1) : // Yes, force assigned channel
    PlayEv->evtype;                           // No, play it as it lies
  mpucmd(0xd7, 0);                       // Issue want-to-send-data
  mpuputd(ChnCmd,0);

  if (PlayEv->flags1 & DBYTES2)          // 2-byte event
  {
    mpuputd(PlayEv->dbyte1, 0);
    mpuputd(PlayEv->dbyte2, LAST);
  }
  else
    mpuputd(PlayEv->dbyte1, LAST);       // Send single data byte

  if ((PlayEv->evcode == NOTE_ON &&       // We just turned a note on
      (AutoOff & 1 << ptrack)   &&       // We want automatic note-off
      PlayEv->dbyte2) && !SeqPlay)       // Velocity > 0, not playing seq
  {
    DosSleep(175);                       // 175 ms note duration
    mpucmd(0xd7, 0);                     // Issue want-to-send-data
    mpuputd(PlayEv->dbyte1, 0);          // Send note
    mpuputd(0, LAST);                    // Force NOTE_OFF (velocity = 0)
  }
}
