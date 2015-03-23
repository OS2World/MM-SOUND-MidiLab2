/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBMAIN0 -    ** CENTRAL CONTROL AND DISPATCHER **              */
/*                                                                   */
/*       Copyright  J. L. Bell        March 1987                     */
/*********************************************************************/
#define  EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

SHORT BeepValue, ClickVel;

/*********************************************************************/
/* Begin main pgm loop. If no MPU input data, the thread will block. */
/*********************************************************************/
VOID mainloop(VOID)
{
  APIRET rc;
  HMQMain = WinCreateMsgQueue(hAB, 0);  /* Set up msg queue          */

  while (!(sysflag1 & SHUTDOWN))          /* Loop until shutdown     */
  {
    TRKMN.flags1 = MPU;                   /* Tell GTEV0 to read MPU  */
    rc = get_event(MAIN_EV);              /* Go for MIDI input data  */
    if (rc)
    {
      mpucmd((actvfunc == RECORD) ? 0x11 : 0x15, 0); // Issue Stop
      mpucmd(0x33, 0);                    /* Issue driver reset      */
      if (rc == 8 || rc == 20)
        sprintf(msgbfr, "Incoming MIDI data error  RC:%d, \nDeltaTime:%d, EvCode:%d, DByte1:%d, DByte2:%d",
                rc, EVMN.deltime, EVMN.evcode, EVMN.dbyte1, EVMN.dbyte2);
      else sprintf(msgbfr, "Incoming MIDI data error  RC:%d", rc);
      umsg('E', hWndClient, msgbfr);
      continue;
    }
    else Analyze_Input(EVMN.dbyte1, EVMN.length); // Process input data
  }
  WinDestroyMsgQueue(HMQMain);            /* Destroy MAIN's msg queue*/
}

/*********************************************************************/
/*                 Analyze input from the MPU                        */
/*********************************************************************/
VOID Analyze_Input(BYTE DByte, BYTE DLnth)
{
  APIRET rc;
  LONG spp, bfgclr, bbgclr;
  BYTE  sppL, sppH;
  static BYTE  altbeats=0;

/*********************************************************************/
/*            Chk for input data from an active function             */
/*********************************************************************/
  if (DLnth > 1)
  {
    rc = recmdata();                      /* Go to input processor   */
    if (rc == 4)                          /* ALL-END received        */
    {
    rec_end:
      if (actvfunc != CANCEL)             /* If record not cancelled */
      {
        RTL = trk[rtrack].index;          /* Save track data length  */
        end_time = *pulTimer;             /* Get elapsed time        */
        safequit = ON;                    /* To guard against loss   */
      }
      else end_time = start_time;         /* Nothing recorded        */

      sprintf(msgbfr,"T%d Record stopped: %02ld.%-02.02ld secs",
        rtrack+1, (end_time-start_time)/1000, (end_time-start_time)%1000);
      Wrt_Trace(msgbfr);
      metro(OFF);                         /* Turn metronome off      */
      mpucmd(0x94,0);                     /* Clk-to-Host OFF         */
      if (ini.RmtCtlS1 & RMT_CTL_DISABLD) /* Rem Control disabled    */
        mpucmd(0x8a,0);                   /* Data-in-stop: OFF       */

      trackfull = OFF;                    /* Reset switch            */
      WinDismissDlg(hWndSTDBY, FALSE);    /* Count-off Box           */
      if (deactv_trk)                     /* Deactivated for Overdub */
      {
        sp.playtracks |= deactv_trk;      /* Restore deactivated trk */
        deactv_trk = OFF;
      }
      WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)0x54, NULL); //Updt display
      max_measures = measurectr;     /* To show actual bars recorded */
      WinSendMsg(WinWindowFromID(hWndClient, actvfunc == OVERDUB ? MN_B3 : MN_B4),
                  GBM_ANIMATE, MPFROMSHORT(FALSE), NULL);  // Stop animation
      trk[rtrack].num_measures = measurectr;/* Set number of measures*/
      WinPostMsg(hWndSongEditor, WM_COMMAND, MPFROM2SHORT(IDM_E_CLEAR,0), 0L);
      actvfunc = OFF;
      DosBeep(75,40);                     /* Comforting assurance    */
    }
    return;
  }
  else indata = DByte;

/*********************************************************************/
/*    Process miscellaneous inbound MPU marks, messages, etc.        */
/*********************************************************************/
  switch (indata)
  {
/*********************************************************************/
/*               Timing Overflow  (F8$)                              */
/*********************************************************************/
   case TIMING_OVFLO:
    *(trk[rtrack].trkaddr + trk[rtrack].index++) = TIMING_OVFLO;
    ndxwrt(rtrack, CLR_RED);
    break;

/*********************************************************************/
/*               All Tracks finished playing (FC$)                   */
/*********************************************************************/
   case END_OF_TRK:
    if (actvfunc == PLAY)
    {
      end_time = *pulTimer;               // Get elapsed time
      stopplay = OFF;
      mpucmd(0x05,0);                     // Send stop_play cmd
      mpucmd(0x94,0);                     // Clock-to-Host: off
      WinPostMsg(hWndSongEditor, WM_COMMAND, MPFROM2SHORT(IDM_E_CLEAR,0), 0L);
      if (measurectr > 1)
        --measurectr;                     // Adj to show actual bars
      WinSetDlgItemShort(hWndBIGBARS, 202, measurectr, TRUE);  // Set max bars
      WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)0x14, NULL); //Updt indexes & measure ctr

      if (ini.iniflag1 & PLAYLOOP)        // If repeating entire song
      {
        DosSleep(900);                    // Allow time for manual intervention
        WinPostMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B7,0), 0L); // Play
      }
      else DosBeep(75,35);                // Comforting assurance
      sprintf(msgbfr,"Play end: %02ld.%-02.02ld secs",
             (end_time-start_time)/1000, ((end_time-start_time)%1000));
      Wrt_Trace(msgbfr);
    }
    break;                                // Keep rolling if overdub

/*********************************************************************/
/*                Clock-to-host (FD$)                                */
/*                                                                   */
/* For displaying the beats during record and playback.              */
/* If the metronome is running, we will count a user-specified       */
/* lead-in based on Clock-to-host pulses from the MPU. (2 per beat)  */
/*********************************************************************/
   case  0xfd:
    if (trace & TRC_CLK2HOST)             /* Tracing clock to host   */
    {
      sprintf(msgbfr, "%cClock", beatswt ? '' : '');
      Wrt_Trace(msgbfr);
    }

    if (mnswt)
    {
      if (standby == LEVEL1)              /* Just standing by        */
      {
        BeepValue = 350;                  /* Set standby click level */
        ClickVel  = 60;
      }
      else if (standby == LEVEL2)         /* Counting off            */
      {
        BeepValue = 450;                  /* Set normal click level  */
        ClickVel  = 85;
      }
      else                                /* Rolling...              */
      {
        BeepValue = (!beatctr && !beatswt) ? 600 : 450; // Accent 1st beat
        ClickVel  = (!beatctr && !beatswt) ? 127 : 85;  // Accent 1st beat
      }
    }

    if (!(beatswt ^= 1))                  /* Ignore alternate clks   */
    {
      if (clikmult == 2 && mnswt)         /* Click on 8th notes      */
        DosPostEventSem(SemClk);          /* Post Click thread       */
      return;
    }

    if (leadin_ctr && standby == LEVEL2)  /* Counting                */
    {
      --leadin_ctr;                       /* Decrement lead-in ctr   */

      if (leadin_ctr == 1)                /* Getting close           */
        mpucmd(0x8b,0);                   /* Data-in-stop: ON  $$    */

      else if (leadin_ctr == 0)           /* Time to get serious     */
      {
        rec_control(ON);                  /* Go set up START command */
        standby = OFF;
        BeepValue = 600;                  /* Prime first accent      */
        ClickVel  = 127;
        mpucmd(reccmd,0);                 /* Issue START             */
      }
    }

    if (mnswt)
      DosPostEventSem(SemClk);            /* Sound metro click       */

    ml_hps = WinGetPS(hWndClient);        /* Get a PS handle         */
    WinDrawBitmap(ml_hps, (altbeats ^= 1) ? hbm_metro1 : hbm_metro2,
                    NULL, (PPOINTL)&metrorcl, 0L, 0L, DBM_STRETCH);
    if (actvfunc && !standby)
    {
      ++beatctr;                          /* Increment the beat ctr  */
      if (beatctr == 1)                   /* Beginning of a measure  */
      {
        if (measurectr == ending_measure) /* Chk for premature stop  */
        {
          WinPostMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B5,0), 0L); // Pause
          if (ini.iniflag1 & PLAYLOOP)    /* Repeating sequence      */
            WinPostMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B7,0), 0L); // Play
        }
        WinPostMsg(hWndClient, UM_SET_HEAD,
                  (MPARAM)0x04, NULL); // Set main tempo & meas box (8/19/93)
        if (WinIsWindow(hAB, hWndCTLPANEL))
          WinPostMsg(hWndCTLPANEL, UM_SET_TEMPO, NULL, NULL);
        if (WinIsWindow(hAB, hWndSongEditor))
          WinPostMsg(hWndSongEditor, UM_MOVE_BAR, NULL, NULL);
        bfgclr = CLR_RED;                 /* Hi lite first beat      */
        bbgclr = CLR_WHITE;
      }
      else
      {
        bbgclr = CLR_RED;                 /* Normal beat coloring    */
        bfgclr = CLR_WHITE;
      }
      WinDrawText(ml_hps, -1, _itoa(beatctr,ndxbfr,10), &beatrcl,
                      bfgclr, bbgclr, DT_CENTER|DT_VCENTER|DT_ERASERECT);

      if (actvfunc != PLAY && rec_measures == measurectr-1)
      {
        --measurectr;                     /* Correct measure counter */
        rec_control(OFF);                 /* Issue STOP recording    */
      }

      if (beatctr >= sp.tsign)            /* End of a measure        */
      {
        beatctr = 0;                      /* Reset to 1st beat of bar*/
        ++measurectr;                     /* Increment measure ctr   */
      }
    }
    WinReleasePS(ml_hps);
    break;

/*********************************************************************/
/*        MPU Command acknowedgement and post-processing (FE$)       */
/*********************************************************************/
   case  CMD_ACK:
    if (trace & TRC_CMDS)
    {
      sprintf(msgbfr, "ACK Cmd:%X", cmd_in_progress);
      Wrt_Trace(msgbfr);
    }

    if (cmd_in_progress >= 0xa0 && cmd_in_progress <= 0xaf)
      indata = mpugetd();               /* Get associated data byte  */

    switch (cmd_in_progress)            /* Special post-processing   */
    {
        case 0x94:                      /* Active function ending    */
          cmd_in_progress = OFF;        /* So D7 cmd will execute    */
          Rel_sustain(FALSE);           /* Rlease any active sustains*/
          actvfunc = OFF;               /* Reset function indicator  */
          break;

        case 0x3f:                      /* Entering UART mode        */
          ini.iniflag1 |= UARTMODE;     // Set indicator
          break;

        default:
          break;
    }
    cmd_in_progress = OFF;
    break;

    case 0xf0:
      if (actvfunc == SYSX)
        proc_sysx(LEVEL1);              // Go process sysx bulk dump
      else proc_sysx(LEVEL2);           // Discard incoming sysx data
      break;

/*********************************************************************/
/*                  MIDI System Messages (FF$)                       */
/*********************************************************************/
   case  0xff:
    WinSetDlgItemText(hWndClient, MN_B7, " Play"); // Insure normal setting
    indata = mpugetd();                   /* Get message type        */
    switch(indata)                        /* See what it is          */
    {
      case 0xfa:                          /* MIDI START              */
        UART_Cmd(0x0a);   // 11/21/95
        WinPostMsg(WinWindowFromID(hWndClient, MN_B7), GBM_ANIMATE,
                     MPFROMSHORT(TRUE), NULL); // Strt Play animation
        break;

      case 0xfb:                          /* MIDI CONTINUE           */
        mpucmd(0x95,0);                   // Clock-to-host: on
        if (actv_trks)                    // Make sure there's something to play
        {
          for (ptrack = ztrack; ptrack < NUMTRKS; ++ptrack) // Prime the EVP's
          {
            if (actv_trks & 1 << ptrack)         // If track is selected
            {
              TRK.flags1 = TRACK;                // Tell GTEV0 to read track data
              rc = get_event(ptrack);            // Get the 1st event from each trk
              if (rc)
                event_error(rc);                 // Stop process
            }
          }
          WinPostMsg(WinWindowFromID(hWndClient, MN_B7), GBM_ANIMATE,
                       MPFROMSHORT(TRUE), NULL); // Strt Play animation
          stopplay = OFF;
          actvfunc = PLAY;
        }
        break;

      case 0xfc:                          // MIDI STOP
        mpucmd(0x94,0);                   // Clock-to-host: off
        WinPostMsg(WinWindowFromID(hWndClient, MN_B7), GBM_ANIMATE,
                     MPFROMSHORT(FALSE), NULL); // Stop Play animation
        stopplay = ON;
        break;

      case 0xf2:                          // Song Position Pointer
        sppL=mpugetd(); sppH=mpugetd();   // Read spp data bytes
        spp = sppH;                       // Get most significant byte
        spp <<= 7;                        // Multiply by 128
        spp |= sppL;                      // OR in least significant byte
        Locate_SPP(spp);                  // Go do it
        break;                            // Wait for external START

      default:
        sprintf(msgbfr,"Unsupported System Message: $%X",indata);
        umsg('E', hWndClient, msgbfr);
    }
    break;

    default:
      sprintf(msgbfr, "MAIN- Unexpected input: $%X\n", indata);
      umsg('E', hWndClient,  msgbfr);
  }
  return;
}
/*********************************************************************/
/*                Locate intermediate song starting point            */
/*               (e.g. Song Position Ptr or starting measure)        */
/*              Strt_Pt = number of 16th notes into the song         */
/*                                                                   */
/* 3/11/94 - If Strt_Pt = -1, we'll scan each track to determine the */
/* number of measures contained therein, for the BigBars display.    */
/*********************************************************************/
VOID Locate_SPP(LONG Strt_Pt)
{
  APIRET rc;
  LONG skiptime, sppdelta, xfercount;
  LONG i;
  BYTE rptctr, typesave;

  if (Strt_Pt == -1)                       // Calling from DMGR0
    sppdelta = 0x7FFFFFFF;                 // Use maximum positive integer
  else
  {
    sppdelta = Strt_Pt * (sp.tbase / 4);   // Convert start point to delta time
    WinSetDlgItemText(hWndClient, MN_B7, "locating");
  }

  dmgreset(1,0);                           // Reset tracks
  for (ptrack = 0; ptrack < NUMTRKS; ++ptrack)
  {
    total_bars[ptrack] = 0;                // Reset measures per trk
    if (sp.playtracks & (1 << ptrack))     // Is this trk selected? 4/25/93
    {
      skiptime = 0;
      TRK.flags1 = TRACK;                  // Get input from track data
      while (skiptime < sppdelta)
      {
        rc = get_event(ptrack);            // Speed thru the track
        if (rc) break;                     // Error in track; bag it
        skiptime += EVP.deltime;           // Accumulate delta time

        if (EVP.flags1 & MLABMETA)         // Mlab track order
        {
          if (EVP.evtype == TRKREST || EVP.evtype == TRKXFER)
          {
            typesave = EVP.evtype;         // Remember xfer or rest
            rptctr = (typesave == TRKXFER) ?  // Establish working counter
               TRK.trkxferctr : TRK.restctr;

            i = PTI;                       // Save current index
            PTI -= 6;                      // Remove Meta prefix data
            PTI -= EVP.trkcmddata;         // Set target index
            xfercount = 0;                 // Reset transfer time counter
            while (PTI < i)                // Determine time within xfer scope
            {
              get_event(ptrack);
              xfercount += EVP.deltime;
            }
            PTI = i;                       // Restore current index

            rptctr = EVP.dbyte1;           // Load working counter
            while (rptctr > 0)
            {
              skiptime += xfercount;       // Add in time for xfer's scope
              if (skiptime >= sppdelta)    // We're at or over the mark
              {
                skiptime -= xfercount;     // Back up one repeat's worth
                PTI = i - 6 - EVP.trkcmddata; // Set first event in scope
                while (skiptime < sppdelta) // Accumulate time up to the
                {                           // point we're searching for
                  get_event(ptrack);
                  skiptime += EVP.deltime;
                }
                break;                      // Get out of repeat ctr loop
              }
              else --rptctr;                // Remove one repeat
            }
            if (!rptctr)                    // If counted down to zero, we used
              rptctr = 255;                 // up the entire rest, signal reload
            else --rptctr;                  // Fudge factor #n

            if (typesave == TRKXFER) TRK.trkxferctr = rptctr;
            else TRK.restctr = rptctr;      // Update appropriate ctr
          }
        }
        else if ((EVP.flags1 & MPUMARK) && EVP.dbyte1 == END_OF_TRK)
        {
          total_bars[ptrack] = skiptime / clocks_per_bar; // Save total bars
          actv_trks &= 0xffff - (1 << ptrack); // Shut off this track
          PTI -= 2;                         // Back up to end-of-trk
          break;                            // Break WHILE
        }
      }

      if (Strt_Pt > 0)                        // Normal positioning call
      {
        if (skiptime > sppdelta)              // Remove excess timing, if any
          TRK.SPTime = skiptime - sppdelta;   // Save for next trk request
        else TRK.SPTime = 0;                  // Clear it out
        starting_index[ptrack]  = PTI;        // Save starting point
        starting_sppadj[ptrack] = TRK.SPTime; // Save adjustment timing
        starting_xferctr[ptrack] = TRK.trkxferctr;  // Save xfer ctr
        starting_restctr[ptrack] = TRK.restctr;     // Save rest ctr
        skiptrks |= 1 << ptrack;              // Force resend of status
      }
    }
  }
  if (Strt_Pt > 0)                          // Normal positioning call
  {
    measurectr = sppdelta / clocks_per_bar + 1; // Set starting measure
    beatctr = (sppdelta % clocks_per_bar) / sp.tbase; // Set beat ctr
    if (!actv_trks)                         // No hits on any tracks
      umsg('W', hWndClient, "Song Positioning beyond end of track(s)");
    else
    {
      WinSetDlgItemText(hWndClient, MN_B7, "SP ready");
      TmgChkDefer |= actv_trks;             // Skip timing chk for next measure
    }
  }
  else    // Set maximum number of measures in song for Jumbo Display (3/11/93)
  {
    max_measures = total_bars[0];           // Total bars for trk 1
    trk[0].num_measures = total_bars[0];    // Save count in track block
    for (i = 1; i < NUMTRKS; ++i)           // If another track has
    {
      if (max_measures < total_bars[i])     // more bars, use it.
        max_measures = total_bars[i];
      trk[i].num_measures = total_bars[i];  // Save count in track block
    }
    if (!max_measures) max_measures = 1;    // Insure it's at least 1
    WinSetDlgItemShort(hWndBIGBARS, 202, max_measures, TRUE);
  }
}
