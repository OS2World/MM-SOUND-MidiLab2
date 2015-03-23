/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBPMID1 -  ** Event Error and Track Command processor **       */
/*                                                                   */
/*        Copyright J. L. Bell        March 1987                     */
/*                                                                   */
/* 9/6/92 - Change trkcmd routine to process event packets           */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

/*********************************************************************/
/*             Event Error handling routine                          */
/*                                                                   */
/* If an out-of-range value is detected during processing any event  */
/* in PMIDI or PMID1, we will pass a valid value to this routine to  */
/* keep the MPU from hanging up, and finish the event. Then we       */
/* will stop the active function and return to the main menu.        */
/*                                                                   */
/* We also use this routine to process a clock/timing error wherein  */
/* the timing bytes for a measure do not total the beats times 120.  */
/*********************************************************************/
void event_error(int dvalue)
{
  if (!perror_flag)                /* 1st occurrence                 */
  {
    WinPostMsg(WinWindowFromID(hWndClient, MN_B7),
                GBM_ANIMATE, MPFROMSHORT(FALSE), NULL);  // Stop Play animation
    if (dvalue == 257)             /* Output buffer overrun          */
      sprintf(msgbfr, "Output buffer overrun");
    else if (dvalue == 256)        /* Measure timing error           */
      sprintf(msgbfr,"Timing Error: T%d measure %d totals %d, should be %d",
              ptrack+1, measurectr-1, timing_total[ptrack], clocks_per_bar);
    else sprintf(msgbfr,"Event Data Error (T%d) at Index %u \nError code=%d",
                  ptrack+1,PTI-1, dvalue);
    umsg('E', hWndClient,  msgbfr);
    perror_flag = ON;              /* Indicate first occurrence      */
    stopplay = ON;                 /* Set stop indicator             */
    mpucmd(0x05,0);                /* Stop play                      */
    mpucmd(0x94,0);                /* Clock-to-host: off             */
  }
}
/*********************************************************************/
/*    TRKCMD  - Process MidiLab track order (old and new styles)     */
/*********************************************************************/
int trkcmd(VOID)
{
  BYTE rptctr, i, rc = 0;
  unsigned int ndxchk;

  if (sysflag2 & DISKLOAD)              /* Disk Load                 */
    return(rc);                         /* Take quick exit           */

  switch (EVP.evtype)                   /* Switch based on type      */
  {
  case TRKXFER:                         /* Track transfer            */
  case TRKREST:                         /* Track rest measures       */
    rptctr = (EVP.evtype == TRKXFER)    /* Establish working counter */
      ? TRK.trkxferctr : TRK.restctr;

    if (rptctr == 255)                  /* Time to initialize        */
      rptctr = EVP.dbyte1;              /* Load repeat counter       */

    if (rptctr > 0)                     /* Process the transfer      */
    {
      ndxchk = PTI;                     /* Save current index        */
      PTI -= 6;                         /* Remove Meta prefix data   */
      PTI -= EVP.trkcmddata;            /* Set target index          */
      if (PTI > PTL)                    /* Verify target (7/12/90)   */
      {
        PTI = ndxchk;                   /* Restore current index     */
        goto targerr;                   /* Go to event error         */
      }
      rc = 12;                          /* Tell PMIDI xfer took place*/
      --rptctr;                         /* Decrement repeat counter  */
    }
    else rptctr = 255;                  /* All done; signal re-load  */

    if (EVP.evtype == TRKXFER) TRK.trkxferctr = rptctr;
    else TRK.restctr = rptctr;          /* Update appropriate ctr    */
    break;

/*********************************************************************/
/*   Track Initiate function added 7/3/87.                           */
/*   Allows starting up one or more idle tracks with a trk cmd from  */
/*   a currently active one.  (For PLAY only)                        */
/*********************************************************************/
  case TRKINIT:                         /* Track Initiate            */
    if (actvfunc == PLAY)               /* Exclude Overdub           */
    {
      InitTrks = EVP.dbyte2;            /* Set tracks 9 - 16         */
      InitTrks <<= 8;                   /* Shift to hi order         */
      InitTrks |= EVP.dbyte1;           /* Set tracks 1 - 8          */
      for (i=0; i < 16; ++i)            /* Set starting index for    */
        if (InitTrks & 1 << i)          /* all selected tracks       */
        {
          if (sp.trkdatalnth[i] > EVP.trkcmddata)
          {
            trk[i].index = EVP.trkcmddata; /* Set starting index     */
            trk[i].trkxferctr = 255;    /* Reset trk xfer ctr(5/8/88)*/
            trk[i].restctr = 255;       /* Reset rest ctr    (5/8/88)*/
            ndxwrt(i, CLR_BLUE);        /* Go to display index's     */
            trk[i].flags1 = TRACK;      // Tell GTEV0 to read track data
            rc = get_event(i);          /* Prime this EVP            */
            timing_total[i] = 0;        /* Reset measure clock ctr   */
          }
          else
          {
            sprintf(msgbfr,"Initiated index for T%d too high",i+1);
            umsg('E', hWndClient, msgbfr);
            goto cancel_init;
          }
        }
      TmgChkDefer |= InitTrks;          /* Skip timing chk for 1st measure */
      actv_trks |= InitTrks;            /* OR into current tracks   */
      for (i=0; i < 16; ++i)            /* Select 1st active track  */
        if (actv_trks & 1 << i)         /*                          */
        {
          ztrack = i;                   /* PMIDI & MAIN0 chks this  */
          break;
        }
      if (WinIsWindow(hAB, hWndSongEditor))  // If Song Editor window is active
      {
        Locate_SPP(-1);                 /* Go Reset max measures    */
        WinInvalidateRect(hWndSongEditor, &Edrcl, FALSE); // Repaint it
      }
    }
  cancel_init:
    break;

/*********************************************************************/
/*   Track Command function added 9/29/87.                           */
/*   Allows executing an MPU-401 command during playback             */
/*   10/11/88 - Expand to include transposition and velocity mods    */
/*********************************************************************/
  case TRKCMDX:                         /* Track Command             */
    if (EVP.in_chan & TCMTRN)
    {
      sp.transadj += EVP.dbyte2;
      if (WinIsWindow(hAB, hWndCTLPANEL))
        WinPostMsg(hWndCTLPANEL, UM_DLGINIT, 0L, 0L); // Update ctl panel
    }
    else if (EVP.in_chan & TCMVEL)
    {
      sp.velocadj += EVP.dbyte2;
      if (WinIsWindow(hAB, hWndSPECFUNC))
        WinPostMsg(hWndSPECFUNC, UM_DLGINIT, 0L, 0L); // Update ctl panel
    }
    else if (ExtCtlSwt != LEVEL2)       /* Only if int sync          */
      mpucmd(EVP.dbyte1, EVP.dbyte2);   /* Execute command           */
    break;

    default:
    targerr:
      event_error(TRKCMDHDR);           /* Invalid track order       */
  }
  return(rc);
}
