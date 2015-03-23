/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBSUBS0 -      ** GENERAL SUBROUTINES   Volume I **            */
/*                                                                   */
/*   - playprep                 - Esc_cancel                         */
/*   - dmgreset                 - Delay                              */
/*                                                                   */
/*       Copyright  J. L. Bell        March 1987                     */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
#include "tmr0_ioc.h"
extern SHORT editchng;

/*********************************************************************/
/*    Playprep - Function to prepare for playback operations         */
/*********************************************************************/
int playprep(BYTE function)
{
  APIRET rc;
  int spp;
  BYTE i;

  if (function == OVERDUB)
  {
    for (i = 0; i < 16; ++i)
    {
      if ((sp.playtracks & 1 << i) && i == rtrack)
      {                             /* De-activate from PLAY status" */
        sp.playtracks ^= (1 << i);         /* Turn off offending bit */
        deactv_trk = 1 << i;               /* Remember deactv'd trk  */
        break;
      }
    }
  }
  ClkAccum = 0;                           /* Reset clk-to-host accum */
  perror_flag = OFF;                      /* Reset event error flag  */
  actv_trks = sp.playtracks;              /* Set active track mask   */
  dmgreset(1,0);                          /* Reset all tracks        */

/*********************************************************************/
/*           Position tracks to requested measure                    */
/*********************************************************************/
  if (skip_bar_ctr > 1 && (start_bar_swt || safequit)) // Always if edit 9/26/93
  {
    start_bar_swt = OFF;                 /* Reset switch             */
    Locate_SPP(((skip_bar_ctr-1) * sp.tsign)*4); /* 16th notes to skip*/
    sysflag1 |= POSACTV;                 /* Indicate positioning on  */
  }
  if (!actv_trks)                        /* All trks overshot the end*/
    return(0);                           /* Abort                    */

  for (i=0; i < 16; ++i)                 /* Select 1st active track  */
    if (actv_trks & 1 << i)              /* to incr measure counter  */
    {
      ztrack = i;                        /* PMIDI will check this    */
      break;
    }
  RunSTrk = ztrack;                      /* Init. running status chk */

  if (sysflag1 & POSACTV)                /* If positioning is active */
  {
    measurectr = skip_bar_ctr;           /* Display positioned bar   */
    skiptrks = actv_trks;              /* Tell PMIDI to resend status*/
    for (i=0; i < 16; ++i)
    {
      trk[i].index = starting_index[i];        /* Set starting index */
      trk[i].trkxferctr = starting_xferctr[i]; /* Set trk xfer ctr   */
      trk[i].restctr = starting_restctr[i];    /* Set rest ctr       */
      trk[i].SPTime  = starting_sppadj[i];     /* Set timing adjust  */
    }
    if (sp.proflags & EXTCNTRL)          /* Xctl on, send songpos ptr*/
    {
      spp=((measurectr-1)*sp.tsign)*4;   /* # 16th notes to skip     */
      mpucmd(0xdf,0);                    /* Want to send system msg  */
      mpuputd(0xf2,0);                   /* Song Position ptr code   */
      mpuputd((BYTE)(spp % 128),0);      /* Send spp least sig. byte */
      mpuputd((BYTE)(spp / 128),LAST);   /* Send spp most sig. byte  */
    }
    TmgChkDefer = actv_trks;             // Skip timing chk for next measure
    WinSetDlgItemText(hWndClient, MN_B7, " Play");
  }

  for (ptrack = ztrack; ptrack < 16; ++ptrack) // Prime the EVP's
  {
    if (actv_trks & 1 << ptrack)         // If track is selected
    {
      TRK.flags1 = TRACK;                // Tell GTEV0 to read track data
      rc = get_event(ptrack);            // Get the 1st event from each trk
      if (rc)
        event_error(rc);                 // Stop process
      timing_total[ptrack] += EVP.deltime;// Incr time total/measure
    }
  }

  WinSendMsg(WinWindowFromID(hWndClient, MN_B5), (ULONG)GBM_SETSTATE,
              MPFROMSHORT(GB_UP), (MPARAM)TRUE); /* Set Pause button 'up'    */
  stopplay = fsustain = OFF;             /* Reset kill-play switches */
  q_flg1 = OFF;                          /* Reset quantize cntrl flgs*/
  return(actv_trks);                     /* Back to caller           */
}

/*********************************************************************/
/*           Cancel (Esc) Record/Overdub/Sys Ex                      */
/*********************************************************************/
void Esc_cancel( void )
{
  WinSendMsg(WinWindowFromID(hWndClient, actvfunc == OVERDUB ? MN_B3 : MN_B4),
              GBM_ANIMATE, MPFROMSHORT(FALSE), NULL);  // Stop Record animation
  WinSendMsg(WinWindowFromID(hWndClient, MN_B7),
              GBM_ANIMATE, MPFROMSHORT(FALSE), NULL);  // Stop Play animation
  if (actvfunc == SYSX)
  {
    mpucmd(0x96,0);                      // MPU Sysx-to-host: off
    if (RTI)                             // Something read
    {
      *(RT + RTI++) = 0;                 // Set timing byte to zero
      *(RT + RTI++) = END_OF_TRK;        // Set End of Trk mark
      RTL = RTI;                         // Set track data length
    }
    if (trackfull)
      umsg('E', hWndClient,  "Track Overflow");
    else
    {
      sprintf(msgbfr,"%u bytes of SysEx recv'd", RTI);
      WinSetWindowText(hWndSYSEX, msgbfr);
      DosBeep(75,35);                    // Comforting assurance
    }
    actvfunc = standby = trackfull = OFF;
    DosPostEventSem(SemSysx);
  }
  else if (standby || leadin_ctr)
  {
    RTI = RTL - 2;                      /* Overlay previous EOT    */
    standby=leadin_ctr=OFF;             /* Clear switches/ctrs     */
    actvfunc = CANCEL;                  /* Tell MAIN0 to cancel    */
    mpucmd(0x10,0);                     /* Issue stop-record       */
    metro(OFF);                         /* Turn metronome off      */
    mpucmd(0x94,0);                     /* Clk-to-Host OFF         */
  }
  else if (actvfunc == RECORD || actvfunc == OVERDUB)
    rec_control(OFF);                   /* Recording in progress   */
}

/*********************************************************************/
/*                            DMGRESET                               */
/*     Path=1 - Set all track index's to 'rvalue'                    */
/*              Reset track-xfer and measure counters                */
/*     Path=2 - Do a general reset/initialization                    */
/*     Path=4 - Reset MPU-401                                        */
/*     Path=8 - Set profile-related MPU functions  9/7/91            */
/*********************************************************************/
void dmgreset(int path, int rvalue)
{
  BYTE i;

  if (path & 1)                           /* Reset track data        */
  {
    measurectr = 1;                       /* Reset measure counter   */
    skiptrks = OFF;                       /* Reset Status resend     */
    for (i = 0; i < 16; ++i)
    {
      trk[i].index = rvalue;              /* Set index               */
      trk[i].trkxferctr = 255;            /* Reset track xfer ctr    */
      trk[i].restctr = 255;               /* Reset rest counter      */
      evp[i].dbyte1 = evp[i].dbyte2 = 0;  /* Reset key fields 3/30/93*/
      evp[i].evcode = evp[i].evtype = 0;
      trk[i].SPTime = 0;                  /* Clear SPP timing adj    */
      timing_total[i] = 0;                /* Reset measure timing    */
    }
  }

  if (path & 2)                           /* Re-initialize system    */
  {
    for (i = 0; i < 16; ++i)
    {
      *(trk[i].trkaddr) = 0;              /* Set timing byte to zero */
      *(trk[i].trkaddr+1) = END_OF_TRK;   /* Set End of Track marker */
      sp.trkdatalnth[i] = 2;              /* Reset track contents    */
      sp.tcc[i] = 0;                      /* Set Track/Channel corr. */
      memset(sp.trkname[i], ' ', 16);     /* Clear name field        */
    }
    sp.tbase = 120;                       /* Default clocks per beat */
    tbase_save = 0;                       /* Clear time base save    */
    time_base_cmd = 0xc5;                 /* Default time base cmd   */
    sp.tempo = 100;                       /* Tempo                   */
    sp.tsign = 4;                         /* Beats per bar           */
    sp.transadj = sp.velocadj = 0;        /* Transposition & Velocity*/
    sp.trkmask = 0;                       /* Track adjust mask       */
    trksolo = 0;                          /* Track solo function     */
    sp.qadjctr = 0;                       /* Quantize control cntr   */
    quantize_amt = 0;                     /* Turn off quantization   */
    sp.playtracks = 0xffff;               /* Enable all tracks       */
    skip_bar_ctr = 1;                     /* Start offset from bar 1 */
    start_bar_swt = OFF;                  /* Measure start offset    */
    ending_measure = 9999;                /* Ending play measure     */
    actv_trks = OFF;                      /* Reset active trk flags  */
    rec_measures = 8;                     /* Reset bars to record    */
    rec_loop = 0;                         /* Reset record-repeats    */
    rtrack = 0;                           /* Default to track 1      */
    sysflag1 = OFF;                       /* Reset system-wide flags */
    sp.proflags &= 255-MTERROFF-VELLEVEL; /* Reset profile flags     */
    sp.ccswt = OFF;                       /* Indicate no cont cntrls */
    TmgChkDefer = OFF;                    /* Reset Timing Chk switch */

    ExtCtlSwt = (sp.proflags & EXTCNTRL) ? LEVEL1 : OFF;
    mpucmd(0x80, 0);                      /* Set internal sync       */

    set_attr(hWndClient, IDM_O_INDEXDISPLAY, MIA_CHECKED, ini.indexdispl);
    WinSendMsg(WinWindowFromID(hWndClient, MN_B5), (ULONG)GBM_SETSTATE,
                         MPFROMSHORT(GB_UP), (MPARAM)TRUE); // Set Pause button 'up'
    if (WinIsWindow(hAB, hWndCTLPANEL))
      WinPostMsg(hWndCTLPANEL, UM_DLGINIT, 0L, 0L); // Signal init routine
    if (WinIsWindow(hAB, hWndSPECFUNC))
      WinPostMsg(hWndSPECFUNC, UM_DLGINIT, 0L, 0L); // Signal init routine
    if (WinIsWindow(hAB, hWndSYSEX))
      WinPostMsg(hWndSYSEX   , UM_DLGINIT, 0L, 0L); // Signal init routine
    if (WinIsWindow(hAB, hWndSTDBY))
      WinPostMsg(hWndSTDBY, WM_COMMAND, MPFROMSHORT(264), 0L);    // Simulate Stop to Count-off Box

    for (i = 0; i < 16; ++i)                // Close Edit window(s)
      if (WinIsWindow(hAB, hWndETrk[i]))
        WinSendMsg(hWndETrk[i], WM_CLOSE, MPFROMSHORT(i), 0L);
  }

/***********************************************************************/
/*              Initialize MPU-401 driver functions                    */
/***********************************************************************/
  if (path & 4)
  {
    settbase(255);                          /* Set timebase controls   */
    mnswt = OFF;                            /* Reset Metronome switch  */
    C2Hswt = OFF;                           /* Reset clk-to host swt   */
    mpucmd(0x84, 0);                        /* Metronome: OFF          */
    mpucmd(0x94, 0);                        /* Clock-to-host: OFF      */
    stopplay = OFF;                         /* No longer valid         */

    if (sp.proflags & EXTCNTRL)             /* External control request*/
      mpucmd(0x91,0);                       /* Real-time affection: ON */
    else
      mpucmd(0x90,0);                       /* Real-time affection: OFF*/

    if (ini.iniflag1 & MTHRU)               /* MIDI THRU requested     */
      mpucmd(0x89,0);                       /* MIDI-THRU: ON           */
    else mpucmd(0x88,0);                    /* MIDI-THRU: OFF          */

    if (ini.RmtCtlS1 & RMT_CTL_DISABLD)     /* No remote control       */
      mpucmd(0x8a,0);                       /* Data-in-stop: OFF       */
    else mpucmd(0x8b,0);                    /* Data-in-stop: ON        */
  }
}

/***********************************************************************/
/*                   Delay / Block for n milliseconds                  */
/***********************************************************************/
VOID Delay(ULONG msecs)
{
  APIRET rc;
  ULONG  ulSize2 = sizeof(msecs), datalen = sizeof(IOCPARMS);

  if (msecs > 0)                          // Ignore if zero
  {
    if (iuo == LEVEL2)                    // Use HRT ring 3 call
      rc = DosDevIOCtl(HRT_hnd, HRT_IOCTL_CATEGORY, HRT_BLOCKUNTIL,
                         &msecs, ulSize2, &ulSize2, NULL, 0, NULL);
    else
    {                                     // Use driver routine
      IOCParms.ulmsTime = msecs;          // Set delay time
      rc = DosDevIOCtl(mpu_hnd, MlabCat, BLK_DELAY,
                     &IOCParms, sizeof(IOCPARMS), &datalen, NULL, 0, NULL);
    }
    if (rc)
      umsg('E', hWndClient,  "Delay or Block error");
  }
}
