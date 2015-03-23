/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBSUBS1 -      ** GENERAL SUBROUTINES   Volume II **           */
/*                                                                   */
/*   - chkact                   - rec_prep                           */
/*   - rec_control              - metro                              */
/*   - trkxfer                  - settbase                           */
/*   - space_bar                - Rel_sustain                        */
/*   - ndxwrt                   - Trk_Edit                           */
/*   - Wrt_trace                                                     */
/*                                                                   */
/*              Copyright  J. L. Bell        March 1987              */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

/*********************************************************************/
/*          CHKACT - if active function, disallow request            */
/*********************************************************************/
  int chkact( void )
  {
      if ((actvfunc && !stopplay))      // 10/5/92
      {
        umsg('W', hWndClient, "Sorry, I can't do this right now...");
        return(8);
      }
      else return(0);
    }
/*********************************************************************/
/*           Get Ready for RECORD and OVERDUB                        */
/*********************************************************************/
void rec_prep( void )
{
  RTI = 0;                                /* Reset track index       */
  actv_trks = 0;                          /* Reset this (7/28/94)    */
  if ((sp.transadj || (sp.proflags & VELLEVEL))
      && sp.trkmask & 1 << rtrack)
        umsg('W', hWndClient, " !WARNING - TRANS/VELOC ON!");
  if (actvfunc == RECORD)
    mpucmd(0xec,0);                       /* De-activate play tracks */
  if (actvfunc == OVERDUB)
    playprep(OVERDUB);                    /* Get ready for playback  */

  ndxwrt(rtrack, CLR_RED);                /* Go display index        */
  measurectr = 1;                         /* Set initial value       */
  stopplay = OFF;                         /* In case play interrupted*/
  if (rec_measures || quantize_amt)
  {
    metro(ON);                            /* Start metro if necessary*/
    mpucmd(0x95,0);                       /* Issue Clk-to-Host ON    */
  }
  beatctr = beatswt = 0;                  /* Reset beat counters     */
  mpucmd(0x20,0);                         /* Issue Record-Standby    */

  WinSetDlgItemShort(hWndBIGBARS, 202, rec_measures, TRUE);
  WinSendMsg(WinWindowFromID(hWndClient, actvfunc == OVERDUB ? MN_B3 : MN_B4),
              GBM_ANIMATE, MPFROMSHORT(TRUE), NULL);  // Strt animation
  WinSendMsg(WinWindowFromID(hWndClient, MN_B5), (ULONG)GBM_SETSTATE,
              MPFROMSHORT(GB_UP), (MPARAM)TRUE);  /* Set Pause button 'up'   */
  WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, (PFNWP)STDBYMsgProc,
              0, IDLG_STNDBY, NULL); // Display stand-by dlg box
}
/*********************************************************************/
/*           Issue Record and Overdub START/STOP commands            */
/*********************************************************************/
void rec_control(char cntl)
{
  SHORT i;

  switch (cntl)
  {
  case ON:
    if (actvfunc == RECORD)
      reccmd = 0x22;                      /* Start Record            */
    else if (actvfunc == OVERDUB)
    {
      reccmd = 0x2a;                      /* Start Overdub           */
      WinSendMsg(WinWindowFromID(hWndClient, MN_B7),
                  GBM_ANIMATE, MPFROMSHORT(TRUE), NULL);  // Strt Play animation
    }
    start_time = *pulTimer;               /* Get start time          */
    WinSetDlgItemText(hWndSTDBY, 260, "¯ Rolling ®");
    break;

  case OFF:
    reccmd = (actvfunc == RECORD) ? 0x11 : 0x15; // Set stop command
    mpucmd(reccmd,0);                     /* Stop record/overdub     */
    for (i = 1; i < 128; ++i)             /* Turn off active note-ons*/
      if (note_array[i])                  /* A note's been turned on */
      {
        *(RT + RTI++) = 0;                /* Set timing = 0          */
        if (note_array[0] != LEVEL2)      /* If first time           */
          *(RT + RTI++) = NOTE_OFF;       /* Set status = NOTE OFF   */
        *(RT + RTI++) = i;                /* Set note number         */
        *(RT + RTI++) = 64;               /* Set bogus velocity      */
        note_array[0] = LEVEL2;           /* To force running status */
        note_array[i] = OFF;              /* Reset array indicator   */
      }
    note_array[0] = OFF;                  /* Reset running-status ind*/
    stoprec = OFF;                        /* Reset stop-switch       */
    if (actvfunc == OVERDUB)
      WinSendMsg(WinWindowFromID(hWndClient, MN_B7),
                GBM_ANIMATE, MPFROMSHORT(FALSE), NULL);  // Stop Play animation
    break;
  }
}

/*********************************************************************/
/*                 Metronome controller                              */
/*********************************************************************/
void metro(char func)
{
  switch (func)
  {
    case ON:
     if (!mnswt)                          /* If not already running  */
     {
       mpucmd(mmode,0);                   /* Start metronome         */
       mnswt = ON;
       DosCreateThread((PTID)&tidMet, (PFNTHREAD)MetroClk, (ULONG)0,
                       (ULONG)0, (ULONG)STKSZ); // Attach Click thread
       DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, +9, tidMet);
       if (actvfunc != PLAY)
         mpucmd(0x95,0);                  /* Start clks if necessary */
     }
     break;

    case OFF:
     if (mnswt)                          /* If running               */
     {
       mpucmd(0x84,0);                   /* Stop metronome           */
       if (!actvfunc && !standby)        /* If idle,                 */
         mpucmd(0x94,0);                 /* stop clk2host            */
       mnswt = OFF;
     }
     break;
  }
  WinSendDlgItemMsg(hWndCTLPANEL, 305, BM_SETCHECK,
                    MPFROMSHORT(mnswt), 0L);    //Set Metro box
}

/*********************************************************************/
/* Routine to set a Track Transfer.  The target field represents the */
/* difference between the track command header and the target index. */
/* Input parameters:  disp - Displacement into track for xfer cmd    */
/*                  target - Target index                            */
/*                  repeat - Repeat counts for xfer cmd              */
/*                 cmdtype - Normal xfer or measure rest             */
/* Note- The trk cmd meta prefix is inserted separately to remain    */
/*       compatible with the old style (non-meta) trk transfer.      */
/*********************************************************************/
int trkxfer(char *disp, USHORT target, BYTE repeat, long cmdtype)
{
  *(disp++) = 0;                           /* Set timing of zero      */
  memcpy(disp, &trkcmd_meta, sizeof(trkcmd_meta)); // Move in meta prefix
  disp +=sizeof(trkcmd_meta);              /* Incr beyond meta prefix */
  ttptr = (TT *)disp;                      /* Map transfer area       */
  ttptr->trkcmdhdr = TRKCMDHDR;            /* Set trk cmd header      */
  ttptr->trkcmdtype = cmdtype;             /* Set track order code    */
  ttptr->ttarget = target+sizeof(trkcmd_meta)+1; /* Set target        */
  ttptr->ttrepeat = repeat;                /* Set repeat counter      */
  ttptr->tteox = EOX;                      /* Set exclusive EOX       */
  return(sizeof(TT)+sizeof(trkcmd_meta)+1);  /* Return with size of TT*/
}

/*********************************************************************/
/*                 Set and execute timebase controls                 */
/*********************************************************************/
void settbase(BYTE path)
{
  path = 0;  //unused
  if (path) return;                         /* To avoid compiler msg   */

  clocks_per_bar = sp.tsign * sp.tbase;     /* Develop timing total/bar*/
  ms_per_beat =  60000 / sp.tempo;          /* Calc. millisecs/beat    */
  ms_per_tick = ms_per_beat / sp.tbase;     /* Calc. millisecs/tick    */
  mpucmd(0xe0, sp.tempo);                   /* Set tempo cmd           */
  ClkIncr = sp.tbase / 2;                   /* Develop clk-to-host incr*/
}

/*********************************************************************/
/*                   Start/Stop/Pause                                */
/*  The space bar serves many functions depending on the setting of  */
/*  various status switches and the current state of things. If any  */
/*  MIDI real-time msgs come in, they will cause a fake sp-bar hit.  */
/*********************************************************************/
void space_bar( void )
{
  switch (actvfunc)
  {
/********************************************************************/
/* For RECORD/OVERDUB, trackfull condition causes stop to be issued */
/* from DMGR0. Otherwise, if the metronome is running, a switch will*/
/* be set to cause RMIDI to issue STOP at the end of the current    */
/* measure. If none of the above, STOP will be issued right here.   */
/********************************************************************/
  case RECORD:
  case OVERDUB:
    if (!trackfull && !leadin_ctr)
    {
      if (!mnswt)
        rec_control(OFF);                /* Issue STOP command      */
      else stoprec = ON;
    }
    return;

  case PLAY:
    if (!stopplay)                       /* Play in progress        */
    {
      mpucmd(0x05,0);                    /* Stop play               */
      mpucmd(0x94,0);                    /* Clock-to-host: off      */
      stopplay = ON;                     // Set stop indicator      */
      WinSendMsg(WinWindowFromID(hWndClient, MN_B7), GBM_ANIMATE,
                        MPFROMSHORT(FALSE), NULL);  // Stop animation
      WinSetDlgItemText(hWndClient, MN_B7, " Play");
      WinSendMsg(WinWindowFromID(hWndClient, MN_B5), (ULONG)GBM_SETSTATE,
                       MPFROMSHORT(GB_DOWN), (MPARAM)TRUE); // Set Pause button 'down'
      return;
    }

  default:
    if (stopplay)                        /* Play has been interruptd*/
    {
      stopplay = OFF;                    /* Reset stopplay swt      */
      perror_flag = OFF;                 /* Reset event error flag  */
      actvfunc = PLAY;                   /* Set active function     */
      WinSendMsg(WinWindowFromID(hWndClient, MN_B7), GBM_ANIMATE,
                       MPFROMSHORT(TRUE), NULL);  // Strt animation
      WinSendMsg(WinWindowFromID(hWndClient, MN_B5), (ULONG)GBM_SETSTATE,
                       MPFROMSHORT(GB_UP), (MPARAM)TRUE); // Set Pause button 'up'
      mpucmd(0x95,0);                    /* Clock-to-host: on       */
      mpucmd(0x0b,0);                    /* Issue continue_play     */
    }
  }
}

/*********************************************************************/
/*       Rel_sustain: Function to release active sustains            */
/*         Force = TRUE: Send release to all channels                */
/*         Force = FALSE: Send only to channels with active sustain  */
/*********************************************************************/
void Rel_sustain(BOOL force)
{
  BYTE i, chnctl;

  if (force)                              /* Force sustain-release   */
    fsustain = -1;                        /* on all channels         */

  if (fsustain)                           /* Any sustain active      */
  {
    for (i = 0; i < 16; ++i)              /* Check for active sustain*/
      if (fsustain & 1 << i)              /* Pedal is down           */
      {
        chnctl = CONTROLLER | i;          /* Set status for this chn */
        mpuputd(chnctl, 0);               /* Send cntrl status       */
        mpuputd(64,0);                    /* Code for sustain pedal  */
        mpuputd(0, LAST);                 /* Release sustain         */
        skiptrks |= 1 << i;               /* Force resend of status  */
      }
    fsustain = OFF;                       /* Indicate pedals now up  */
  }
}

/*********************************************************************/
/*        Routine to write track index(s) during Play/Record         */
/*********************************************************************/
void ndxwrt(SHORT track, LONG bColor)
{
  char NdxBfr[7];
  LONG fColor = CLR_YELLOW;

  if (ini.indexdispl)                     /* If displaying indices   */
  {
    ltime = trk[track].index;             /* Move to long int        */
    _ltoa(ltime,NdxBfr,10);
    ml_hps = WinGetPS(hWndClient);
    WinDrawText(ml_hps, -1, &NdxBfr[0], &ndxrcl[track], fColor, bColor,
                DT_RIGHT | DT_BOTTOM | DT_ERASERECT);
    WinReleasePS(ml_hps);
  }
}

/*********************************************************************/
/*        Routine to write trace items to trace list box             */
/*********************************************************************/
VOID Wrt_Trace(char *text)
{
  SHORT lastitem = 0;
  MRESULT ndx;

  if (!WinIsWindow(hAB, hWndTRACE) || tracefull)
  return;                         // Exit if trace is not active

  ndx = WinSendDlgItemMsg(hWndTRACE, 125, LM_INSERTITEM,
                            MPFROMSHORT(LIT_END), MPFROMP(text));
  if ((SHORT)ndx == LIT_ERROR || (SHORT)ndx == LIT_MEMERROR)
  {
    WinSendDlgItemMsg(hWndTRACE, 125, LM_DELETEITEM, (MPARAM)lastitem, NULL);
    ndx = WinSendDlgItemMsg(hWndTRACE, 125, LM_INSERTITEM,
                     MPFROMSHORT(LIT_END),
                     MPFROMP("** TRACE FULL ** Use Clear to reset"));
    WinSendDlgItemMsg(hWndTRACE, 125, LM_SETTOPINDEX, ndx, NULL);
    DosBeep(840, 100);            // Sound warning
    tracefull = ON;
  }
  else lastitem = (SHORT)ndx;
}

/**************************************************************************/
/*                       Invoke Track Edit                                */
/**************************************************************************/
VOID Trk_Edit(BYTE track, SHORT measure, SHORT numMeas, HWND hWndC)
{
  if (chkact()) return;                   // No can do now
  if (emode & (1 << track))               // Already being edited
    WinSetFocus(HWND_DESKTOP, hWndETrk[track]);
  else if (*trk[track].trkaddr == SYSX)
    umsg('E', hWndC,  "This track contains SysEx data");
  else
  {
    emode |= (1 << track);                // Set interlock
    DPm.EventPkt = (PVOID)&evp[track];    // Set prmblk evp ptr
    DPm.SongPrfl = (PVOID)&sp;            // Set prmblk profile ptr
    DPm.ETrack   = track;                 // Set prmblk trk number
    DPm.EStrtMeas = measure;              // Set starting measure
    DPm.ENumMeas = numMeas;               // Set ending measure
    DPm.SongName = songname;              // Set prmblk song name
    WinPostMsg(hWndClient, UM_SET_HEAD,   // HiLite edited track
               MPFROM2SHORT(0x80, NULL), NULL);
    WinLoadDlg(HWND_DESKTOP, HWND_DESKTOP,
              (PFNWP)EDITMsgProc, 0, IDLG_EDIT1, &DPm);
  }
}
