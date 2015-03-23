/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBRCRD0 -    ** Record/Overdub Setup Routines **               */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

MRESULT EXPENTRY RECRDMsgProc(HWND hWndDlg,
                              USHORT message, MPARAM mp1, MPARAM mp2)
{
//static HWND hWndParent;
  char *charptr;
  SHORT i;
  static BYTE acc_allswt = 1, save_hi, save_lo, chng_acc;

  switch(message)
   {
    case WM_INITDLG:
         save_lo = ini.acc_chn_lo;     // Save these in case user cancels
         save_hi = ini.acc_chn_hi;     // after diddling with chn settings
         chng_acc = OFF;

/**********************************************************************/
/*       Initialize the controls within the Dialog Box                */
/**********************************************************************/
         WinSetDlgItemShort(hWndDlg, 257, rtrack + 1, TRUE);
         WinSetDlgItemShort(hWndDlg, 261, rec_measures, TRUE);
         WinSetDlgItemShort(hWndDlg, 262, rec_loop, TRUE);
         WinSendDlgItemMsg(hWndDlg, 284+rtrack, BM_SETCHECK, MPFROMSHORT(1), 0L);
         for (i = 0; i < 8; ++i)  // Set Acceptable Tracks
         {
           if (ini.acc_chn_lo & 1 << i)
             WinSendDlgItemMsg(hWndDlg, 268+i, BM_SETCHECK, MPFROMSHORT(1), 0L);
           if (ini.acc_chn_hi & 1 << i)
             WinSendDlgItemMsg(hWndDlg, 276+i, BM_SETCHECK, MPFROMSHORT(1), 0L);
         }
         charptr = actvfunc == RECORD ? "Record":"Overdub";
         sprintf(msgbfr, "%s setup", charptr);  // Set window title
         WinSetWindowText(hWndDlg, msgbfr);
         break; /* End of WM_INITDLG */

    case WM_CONTROL:
        i = SHORT1FROMMP(mp1);

        if (i >= 268 && i <= 275)       // Acceptable channels 1 - 8
        {
          i -= 268;
          ini.acc_chn_lo ^= (1 << i);
          chng_acc = ON;                // Indicate initialization changed
        }
        else if (i >= 276 && i <= 283)  // Acceptable channels 9 - 16
        {
          i -= 276;
          ini.acc_chn_hi ^= (1 << i);
          chng_acc = ON;                // Indicate initialization changed
        }
        else if (i >= 284 && i <= 299)  // Track Radio buttons
          rtrack = i - 284;             // Set record track

        break; /* End of WM_CONTROL */

    case WM_COMMAND:
         i = SHORT1FROMMP(mp1);
         switch( i )
         {
          case 263: /* Button text: "OK"                                 */
               WinQueryWindowText(WinWindowFromID(hWndDlg, 261), 6, szChars);
               i = atoi(szChars);
               if (!chk_range(i, "Measures", 0, 9999)) break;
               rec_measures = max_measures = i;

               WinQueryWindowText(WinWindowFromID(hWndDlg, 262), 4, szChars);
               i = atoi(szChars);
               if (!chk_range(i, "Repeats", 0, 254)) break;
               rec_loop = i;

               if (chng_acc == ON)             // Need to set accptbl chns
               {
                 mpucmd(0xee, ini.acc_chn_lo); // Channels 1-8  (6/3/93)
                 mpucmd(0xef, ini.acc_chn_hi); // Channels 9-16 (6/3/93)
                 chngdini = ON;                // Set master switch
               }
               WinDismissDlg(hWndDlg, TRUE);
               rec_prep();                     // Go do final preparations
               break;

          case 264: /* Button text: "Cancel"                         */
               ini.acc_chn_lo = save_lo;       // Restore these settings
               ini.acc_chn_hi = save_hi;
               Esc_cancel();
               standby = actvfunc = OFF;
               WinDismissDlg(hWndDlg, FALSE);
               break;

          case 267: // Button text: "Input Chns" toggle
               acc_allswt ^= 1;
               ini.acc_chn_lo = (acc_allswt == 1) ? 255 : 0;
               ini.acc_chn_hi = (acc_allswt == 1) ? 255 : 0;
               for (i = 0; i < 16; ++i)
               WinSendDlgItemMsg(hWndDlg, 268+i, BM_SETCHECK,
                                 MPFROMSHORT(acc_allswt), 0L);
               chng_acc = ON;          // Indicate initialization changed
               break;
         }
         break; /* End of WM_COMMAND */

    case WM_CLOSE:
         WinPostMsg(hWndDlg, WM_COMMAND, MPFROMSHORT(DID_CANCEL), 0L);
         break; /* End of WM_CLOSE */

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
  return FALSE;
} /* End of RECRDMsgProc */

/**********************************************************************/
/*       Message procedure for the Standby Dialog Box                 */
/**********************************************************************/
MRESULT EXPENTRY STDBYMsgProc(HWND hWndDlg,
                              USHORT message, MPARAM mp1, MPARAM mp2)
{
  SHORT i;
  HWND  hWndParent;
  char *charptr;

  switch (message)
   {
/**********************************************************************/
/*       Initialize the controls within the Dialog Box                */
/**********************************************************************/
    case WM_INITDLG:
         hWndSTDBY = hWndDlg;
         hWndParent = (HWND)mp1;
         cwCenter(hWndDlg, (HWND)hWndParent);
         sprintf(msgbfr, "%d Measures, %d Repeats", rec_measures, rec_loop);
         WinSetDlgItemText(hWndDlg, 266, msgbfr);

         charptr = actvfunc == RECORD ? "Record":"Overdub";
         sprintf(msgbfr, "%s Standby  %d bar count-off", charptr,
           mnswt ? ini.leadin_bars/2 : 0); // Set leadin bars for window title
         WinSetWindowText(hWndDlg, msgbfr);
         break;  /* End of WM_INITDLG */

    case WM_COMMAND:
         i = SHORT1FROMMP(mp1);
         switch (i)
         {
          case 260: // Button text: "Start Recording"
           if (standby)                       /* Getting ready for record*/
           {
             if (mnswt && ini.leadin_bars)    /* If metronome is running */
             {
               leadin_ctr = sp.tsign * (ini.leadin_bars / 2) + 1;
               standby = LEVEL2;
             }
             else
             {
               standby = OFF;
               rec_control(ON);               /* Prepare for start record*/
               mpucmd(reccmd,0);              /* Issue MPU START command */
             }
           }
           if (leadin_ctr)
             WinSetDlgItemText(hWndDlg, 260, "¯ Counting ®");
           break;

          case 264: /* Button text: "Stop Recording"                     */
           Esc_cancel();
           standby = actvfunc = OFF;
           WinDismissDlg(hWndDlg, FALSE);
           break;

         }
         break; /* End of WM_COMMAND */

    case WM_CLOSE:
         WinPostMsg(hWndDlg, WM_COMMAND, MPFROMSHORT(DID_CANCEL), 0L);
         break; /* End of WM_CLOSE */

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
  return FALSE;
} /* End of STDBYMsgProc */

/*********************************************************************/
/*   Process Remote-control events    Called from JLBRMIDI  3/20/93  */
/*   Return codes:                                                   */
/*                 0 - Keep event                                    */
/*                 8 - Discard event                                 */
/*********************************************************************/
HWND hWndRCTL;          // These 2 are shared by the next two routines
PBYTE ctl_ev_ptr;

BYTE Remote_ctl(PVOID p_EVPrc)
{
  EVPK *EVPrc;
  BYTE hitswt = OFF, i;

  EVPrc = (EVPK *)p_EVPrc;                   // Set up event packet

  if (ini.RmtCtlS1 & CAPTURE_ARMED)          // We're waiting to capture
  {
    if (ini.RmtCtlS2 == CAPTURE_PLAY)
      ctl_ev_ptr = ini.play_ctlev;
    else if (ini.RmtCtlS2 == CAPTURE_RETAKE)
      ctl_ev_ptr = ini.retake_ctlev;
    else if (ini.RmtCtlS2 == CAPTURE_MULTI)
      ctl_ev_ptr = ini.multi_ctlev;

    *(ctl_ev_ptr)   = EVPrc->evcode;         // Set new control event
    *(ctl_ev_ptr+1) = EVPrc->dbyte1;
    *(ctl_ev_ptr+2) = EVPrc->dbyte2;

    decode_event(EVPrc, 0);                  // Get text of captured event
    ctl_ev_ptr = ini.play_disp + (ini.RmtCtlS2 * sizeof(ini.play_disp));
    strcpy(ctl_ev_ptr, msgbfr+11);           // Put text in INI file

    WinSetDlgItemText(hWndRCTL, 132 + ini.RmtCtlS2, msgbfr+11);
    WinSendMsg(hWndRCTL, WM_COMMAND, MPFROMSHORT(126), 0L); // "Capture"
    chngdini = ON;                           // Indicate INI file has changed
    return(8);                               // Discard this event
  }

/*********************************************************************/
/*    Check to see if the incoming event matches one of the three    */
/*    Remote Control events.                                         */
/*      (Historical note: This routine broke my balls on 3/27/93)    */
/*********************************************************************/
  ctl_ev_ptr = ini.play_ctlev;               // Point to first control event
  for (i = 0; i < 3; ++i)
  {
    if (EVPrc->evcode == *(ctl_ev_ptr))      // Chk for match on channel code
    {
      if (EVPrc->dbyte1 == *(ctl_ev_ptr+1))  // Chk for match on 1st data byte
      {
        if (EVPrc->flags1 & DBYTES2)         // Need to chk 2nd data byte
        {
          if (EVPrc->evcode == NOTE_ON)      // Some real screwing around now
          {
            if ((*(ctl_ev_ptr+2) &&  EVPrc->dbyte2) ||  // Hit- note-on
               (!*(ctl_ev_ptr+2) && !EVPrc->dbyte2))    // Hit- note-off
            {
              hitswt = ON;                   // Got one... (note on/off)
              break;
            }
            else if (*(ctl_ev_ptr+2) && !EVPrc->dbyte2)
               return(8);      // Discard the note-off of a captured note-on
          }
          else
          if (EVPrc->dbyte2 == *(ctl_ev_ptr+2)) // Chk for match on 2nd data byte
          {
            hitswt = ON;                      // Got one... (2 data byte type)
            break;
          }
          else
          {
            ctl_ev_ptr += sizeof(ini.play_ctlev); // Point to next control event
            continue;                         // Keep looking
          }
        }
        else
        {
          hitswt = ON;                        // Got one... (Pgm chg/Aft tch)
          break;
        }
      }
    }
    ctl_ev_ptr += sizeof(ini.play_ctlev);     // Point to next control event
  }

  if (hitswt && !(ini.RmtCtlS1 & RMT_CTL_DISABLD))
  {
    switch (i)
    {
     case 0:
       WinPostMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B7,0), 0L); // Play
       break;

     case 1:
       WinPostMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B2,0), 0L); // Re-take
       break;

     case 2:
       if (standby && !leadin_ctr)
         WinPostMsg(hWndSTDBY, WM_COMMAND, MPFROMSHORT(260), 0L); // Start rcd
       else if (actvfunc == RECORD || actvfunc == OVERDUB)
         WinPostMsg(hWndSTDBY, WM_COMMAND, MPFROMSHORT(264), 0L); // Stop rcd
       else
         WinPostMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B5,0), 0L); // Pause
       break;
    }
  }
  if ((actvfunc != RECORD && actvfunc != OVERDUB) ||
       (standby && leadin_ctr != 1) || hitswt)  // Allow anticipated notes $$
    return(8);                     // Tell RMIDI to discard this event
  else return(0);
}

/**********************************************************************/
/*    Message Processing routine for Remote Control Facility 3/20/93  */
/**********************************************************************/
MRESULT EXPENTRY RCTLMsgProc(HWND hWndDlg,
                             USHORT message, MPARAM mp1, MPARAM mp2)
{
  SHORT i;

  switch(message)
  {
   case WM_INITDLG:
        hWndRCTL = hWndDlg;         // Save for use by RmtCtl function call
        if (ini.RmtCtlS1 & RMT_CTL_DISABLD)         // Remote ctrl disabled
        {
          WinEnableControl(hWndDlg, 126, FALSE);    // Grey out Capture button
          WinSendDlgItemMsg(hWndDlg, 131, BM_SETCHECK, MPFROMSHORT(ON), 0L);
        }
        WinSendDlgItemMsg(hWndDlg, 128+ini.RmtCtlS2,
            BM_SETCHECK, MPFROMSHORT(1), 0L);       // Set active radio button

        ctl_ev_ptr = ini.play_disp;                 // Point to first text
        for (i = 132; i < 135; ++i)                 // Do 3 descriptions
        {
          WinSetDlgItemText(hWndDlg, i, ctl_ev_ptr);// Set dlg desc. field
          ctl_ev_ptr += sizeof(ini.play_disp);      // Point to next desc
        }
        break; /* End of WM_INITDLG */

   case WM_CONTROL:
        i = SHORT1FROMMP(mp1);
        switch(i)
          {
           case 128:        /* Radio Button: Play                   */
                ini.RmtCtlS2 = CAPTURE_PLAY;
                break;

           case 129:        /* Radio Button: Re-take                */
                ini.RmtCtlS2 = CAPTURE_RETAKE;
                break;

           case 130:        /* Radio Button: Multi-function         */
                ini.RmtCtlS2 = CAPTURE_MULTI;
                break;

           case 131:        /* Chkbox:       Disable rmt cntrl      */
                if (ini.RmtCtlS1 & RMT_CTL_DISABLD)
                {
                  WinEnableControl(hWndDlg, 126, TRUE);
                  mpucmd(0x8b,0);             /* Data-in-stop: ON   */
                }
                else                          /* Currently enabled  */
                {
                  WinEnableControl(hWndDlg, 126, FALSE);
                  mpucmd(0x8a,0);             /* Data-in-stop:OFF   */
                }
                ini.RmtCtlS1 ^= RMT_CTL_DISABLD;   /* Flip switch   */
                chngdini = ON;            /* Indicate INI file has changed */
                break;
          }
          break; /* End of WM_CONTROL */

   case WM_COMMAND:
        i = SHORT1FROMMP(mp1);
        switch(i)
          {
           case 124:        /* Button text: "Done"          */
                WinPostMsg(hWndDlg, WM_CLOSE, 0L, 0L); // Signal Close
                break;

           case 126:        /* Button text: "Capture"       */
                if (WinIsControlEnabled(hWndDlg, 124)) // Is capture armed
                {                                      // No
                  WinSetDlgItemText(hWndDlg, 126, "¯ waiting for MIDI event ®");
                  WinEnableControl(hWndDlg, 124, FALSE);// Disable "Done"
                  ini.RmtCtlS1 |= CAPTURE_ARMED;       // Indicate we're waiting
                }
                else
                {
                  WinSetDlgItemText(hWndRCTL, 126, "Capture");
                  WinEnableControl(hWndDlg, 124, TRUE);    // Enable "Done"
                  ini.RmtCtlS1 &= 255-CAPTURE_ARMED;       // Reset Capture flag
                }
                break;
          }
        break; /* End of WM_COMMAND */

   case WM_CLOSE:
        actvfunc = OFF;
        WinDismissDlg(hWndDlg, TRUE);
        break; /* End of WM_CLOSE */

   default:
        return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
  }
  return FALSE;
}  /*   End of RCTLMsgProc */

/**********************************************************************/
/*                 Metronome Click Thread     8/21/95                 */
/**********************************************************************/
VOID MetroClk(VOID)
{
  ULONG PostCt;
  static HMQ QMetro;
  extern SHORT ClickVel, BeepValue;

  QMetro = WinCreateMsgQueue(hAB, 0);     // Set up msg queue
  DosResetEventSem(SemClk, &PostCt);      // Initialize semaphore
  while (mnswt)
  {
    DosWaitEventSem(SemClk, -1);          // Wait for post from MAIN0
    if (ini.iniflag1 & METROMID)          // Sending clicks to MIDI OUT
    {
      mpuputd(NOTE_ON | ini.MetChn-1, 0); // Send NOTE_ON
      mpuputd(ini.MetNote, 0);
      mpuputd(ClickVel, LAST);
      skiptrks |= 1 << ptrack;            // Force resend of status

      DosSleep(ini.MetDur);               // 'Delay' IOCtl is not reentrant!
//    Delay(ini.MetDur);                  // User-specified duration

      mpuputd(NOTE_OFF | ini.MetChn-1, 0);// Send NOTE_OFF
      mpuputd(0x45, 0);
      mpuputd(ClickVel, LAST);
      skiptrks |= 1 << ptrack;            // Force resend of status
    }
    else DosBeep(BeepValue, ini.MetDur);  // Use PC speaker for clicks
    DosResetEventSem(SemClk, &PostCt);    // Reset for next click
  }
  WinDestroyMsgQueue(QMetro);
}
