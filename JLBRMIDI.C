/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBRMIDI -   ** INCOMING MIDI DATA PROCESSOR **                 */
/*                                                                  */
/*       Copyright  J. L. Bell        March 1987                     */
/*                                                                   */
/* This module processes incoming packets of data from the MPU       */
/* during recording.                                                 */
/*                                                                   */
/* Quantization code was added on 2/26/87                            */
/* Note_array for note-off added 9/10/87                             */
/* Quantize routine revised 1/1/88                                   */
/* 11/7/88 Add velocity leveling function                            */
/* 5/23/89 Add forced measure_end fixup for note-on's                */
/* 11/24/89 Removed forced measure_end fixup (too much trouble)      */
/* 09/09/92 Changed to Get_Event scheme for MPU data                 */
/* 03/20/93 Added Remote Control Facility                            */
/*********************************************************************/
#define EXT extern
#include "MLABPM.H"
#include "JLBCSA00.C"

//LONG RmtCtlTime = 0;

int recmdata(void)
{
  APIRET rc;
  LONG deltime_save, elapsed_msec;
  BYTE nsave, vsave, true_note_on = OFF;

/*********************************************************************/
/*                Process Remote-control events   3/20/93            */
/*********************************************************************/
  rc = Remote_ctl(&EVMN);          // Go check if remote control event
  if (rc == 8)                     // Discard this event
  {
//  RmtCtlTime = EVMN.deltime;     // Need to save the event timing
    return(0);                     // Quick exit back to MAIN0
  }

/*********************************************************************/
/*    Convert elapsed time from previous event into delta time       */
/*********************************************************************/
  if (!leadin_ctr)                          // We've finished count-off
  {
    deltime_save = *pulTimer;               // Get msec since previous event
    elapsed_msec = deltime_save - rec_deltime;
    EVMN.deltime = elapsed_msec / ms_per_tick;
    rec_deltime = deltime_save;             // Save current time for next event

//  EVMN.deltime += RmtCtlTime; // Add time for discarded control events, if any
//  RmtCtlTime = 0;             // Reset for next time

    while (EVMN.deltime >= 240)             // Write Ovflo's if necessary
    {
      *(trk[rtrack].trkaddr + trk[rtrack].index++) = TIMING_OVFLO;
      EVMN.deltime -= 240;                  // Reduce by overflow amt
    }
  }
  else EVMN.deltime = 0;                    // Anticipatory notes

/*********************************************************************/
/*                Check for special MPU marks                        */
/*********************************************************************/
  switch (EVMN.dbyte1)
  {
    case END_OF_TRK:                      /* All-end mark            */
     if (rec_loop && !trackfull)          /* Repeat this recording   */
     {
       uindex = trkxfer(trk[rtrack].trkaddr+trk[rtrack].index,
                        trk[rtrack].index, (BYTE)rec_loop, TRKXFER);
       trk[rtrack].index += uindex;       /* Incr over meta-event    */
     }
     *(trk[rtrack].trkaddr + trk[rtrack].index++) = EVMN.deltime; /* Store timing byte       */
     *(trk[rtrack].trkaddr + trk[rtrack].index++) = END_OF_TRK; /* Store eot */
     ndxwrt(rtrack, CLR_RED);             /* Display index           */
     return(4);                           /* Signal recording done   */

#ifdef OLDCODE
    case MEASURE_END:                     /* End-measure mark        */
/*********************************************************************/
/* When metro is on, we defer STOP until measure end - OR - if       */
/* a fixed number of measures was to be recorded, we'll stop now.    */
/*********************************************************************/
     if (stoprec || rec_measures == measurectr-1)     // 4/12/93
     {
       --measurectr;                      /* Correct measure counter */
       rec_control(OFF);                  /* Issue STOP recording    */
     }
//   *(trk[rtrack].trkaddr + trk[rtrack].index++) = EVMN.deltime; /* Store timing byte       */
     *(trk[rtrack].trkaddr + trk[rtrack].index++) = 0;           /* Store timing byte       */
     *(trk[rtrack].trkaddr + trk[rtrack].index++) = MEASURE_END; /* Store meas. end */
     ndxwrt(rtrack, CLR_RED);             /* Display index           */
     return(0);
#endif
  }

/*********************************************************************/
/*             Straight MIDI data from this point on                 */
/*********************************************************************/
  if (trackfull) return(0);               /* Do nothing, trk is full */
  if (standby)                            /* Antcipatory notes       */
    EVMN.flags1 &= 255 - RUNSTAT;         /* Reset running-status    */
  switch (EVMN.evcode)                    /* Process incoming events */
  {
/*********************************************************************/
/*             Event type: NOTE ON/NOTE OFF                          */
/*  For note on/off events, we will get all the MPU data at one time.*/
/*********************************************************************/
    case  NOTE_ON:                        /* Status = NOTE ON        */
    case  NOTE_OFF:                       /*    -or-  NOTE OFF       */
      nsave = EVMN.dbyte1;                /* Save note number        */
      vsave = EVMN.dbyte2;                /* Save velocity           */
      if (EVMN.evcode == NOTE_ON && vsave)
        true_note_on = ON;                /* Indicate a real note-on */

     *(trk[rtrack].trkaddr + trk[rtrack].index++) = EVMN.deltime;
      if (!(EVMN.flags1 & RUNSTAT))       /* If not running status   */
        *(RT + RTI++) = EVMN.evtype;      /* Store channel command   */
      *(RT + RTI++) = nsave;              /* Store note number       */
/*********************************************************************/
/* During note-on/off, if the transpose value is non-zero, we will   */
/* create a shadow note, displaced from the first by the transpose   */
/* value. This is for playing automatic octaves or other harmonies.  */
/*                                                                   */
/* If the velocity leveling function is active, we will force a      */
/* uniform velocity of 64 plus/minus the adjustment amount.          */
/*                                                                   */
/* Also set the active_note array for clean-up when a fixed number   */
/* of measures are being recorded.                                   */
/*********************************************************************/
      if (true_note_on)                   /* If note-on, adjust      */
      {
        note_array[nsave] = ON;           /* Indicate note turned on */
        if (sp.proflags & VELLEVEL &&     /* Velocity leveling on,   */
          sp.trkmask & 1 << rtrack)
          vsave = 64 + sp.velocadj;       /* force uniform velocity  */
      }
      else note_array[nsave] = OFF;       /* Note has been turned off*/
      *(RT + RTI++) = vsave;              /* Store velocity          */

      if (sp.transadj && (sp.trkmask & 1 << rtrack))
      {                                   /* If shadow is active     */
        *(RT + RTI++) = 0;                /* Shadow timing always 0  */
        *(RT + RTI++) = nsave+sp.transadj;/* Get note number & adjust*/
        if (true_note_on)                 /* Set shadow status       */
          note_array[nsave+sp.transadj] = ON;
        else note_array[nsave+sp.transadj] = OFF;
        *(RT + RTI++) = vsave;            /* Set shadow velocity     */
      }
      break;

/*********************************************************************/
/*             Event type: CONTROLLER CHANGE                         */
/*********************************************************************/
    case  CONTROLLER:                     /* Status = CONTROLLER CHNG*/
      *(RT + RTI++) = EVMN.deltime;       /* Store timing byte       */
      if (!(EVMN.flags1 & RUNSTAT))       /* If not running status   */
        *(RT + RTI++) = EVMN.evtype;      /* Store channel command   */
      *(RT + RTI++) = EVMN.dbyte1;        /* Store 1st data byte     */
      *(RT + RTI++) = EVMN.dbyte2;        /* Store 2nd data byte     */
      break;

/*********************************************************************/
/*             Event type: PROGRAM CHANGE                            */
/*********************************************************************/
    case  PGM_CHANGE:                     /* Status = PROGRAM CHANGE */
      *(RT + RTI++) = EVMN.deltime;       /* Store timing byte       */
      if (!(EVMN.flags1 & RUNSTAT))       /* If not running status   */
        *(RT + RTI++) = EVMN.evtype;      /* Store channel command   */
      *(RT + RTI++) = EVMN.dbyte1;        /* Store program number    */
      break;

/*********************************************************************/
/*             Event type: CHANNEL AFTER-TOUCH                       */
/*********************************************************************/
    case  AFTER_TOUCH:                    /* Status=CH AFTER TOUCH   */
      *(RT + RTI++) = EVMN.deltime;       /* Store timing byte       */
      if (!(EVMN.flags1 & RUNSTAT))       /* If not running status   */
        *(RT + RTI++) = EVMN.evtype;      /* Store channel command   */
      *(RT + RTI++) = EVMN.dbyte1;        /* Store value             */
      break;

/*********************************************************************/
/*             Event type: PITCH-BEND WHEEL                          */
/*********************************************************************/
    case  PITCH_BEND:                     /* Status=PITCH BEND WHEEL */
      *(RT + RTI++) = EVMN.deltime;       /* Store timing byte       */
      if (!(EVMN.flags1 & RUNSTAT))       /* If not running status   */
        *(RT + RTI++) = EVMN.evtype;      /* Store channel command   */
      *(RT + RTI++) = EVMN.dbyte1;        /* Store 1st data byte     */
      *(RT + RTI++) = EVMN.dbyte2;        /* Store 2nd data byte     */
      break;

/*********************************************************************/
/*             Event type: POLYPHONIC AFTER-TOUCH                    */
/*********************************************************************/
    case  POLY_AFTER:                     /* Status=POLYPHONIC AFT TC*/
      *(RT + RTI++) = EVMN.deltime;       /* Store timing byte       */
      if (!(EVMN.flags1 & RUNSTAT))       /* If not running status   */
        *(RT + RTI++) = EVMN.evtype;      /* Store channel command   */
      *(RT + RTI++) = EVMN.dbyte1;        /* Store 1st data byte     */
      *(RT + RTI++) = EVMN.dbyte2;        /* Store 2nd data byte     */
      break;

    default:
      printf("Incorrect data received (X'%X') at T%d:%u\n",
      EVMN.evcode, rtrack+1, TRKMN.index);
      rec_control(OFF);                   /* Issue STOP recording    */
  }
  ndxwrt(rtrack, CLR_RED);                /* Display index           */

  if (RTI >= (ini.trksize[rtrack]-20) && !trackfull) // Track overrun
  {
//  DosBeep(700,300); DosBeep(400,300);   // This causes problems in stopping
    trackfull = ON;                       // Set full switch
    rec_control(OFF);                     // Stop recording
    return(3);                            // For future MAIN0 processing
  }
  return(0);                              /* Go back for more data   */
}

/*********************************************************************/
/*        Quantization Subroutine - Called from JLBPMIDI             */
/*********************************************************************/
BYTE quantize(BYTE qval)
{
  SHORT i, j;

  if (q_flg1 & (1 << ptrack))             /* Need to force timing=0  */
  {
    qval = 0;                             /* Event time = 0          */
    q_flg1 ^= (1 << ptrack);              /* Reset the flag          */
  }
  else
  {
    j = qval % quantize_amt;              /* Set remainder, if any   */
    if (j)                                /* Exit if comes out even  */
    {
      if (!(i = qval/quantize_amt))       /* evtime < quantize amt   */
      {
        if (qval >= q_limit)
          qval = quantize_amt;            /* Correct to quantize amt */
        else qval = 0;                    /* Drop insignificant time */
      }
      else                                /* qval > quantize amt     */
      {
        if (j > q_limit) ++i;             /* Move to next multiple   */
        qval = i * quantize_amt;          /* Set to multiple of quan.*/
      }
    }
  }
  return(qval);                           /* Return processed timing */
}
