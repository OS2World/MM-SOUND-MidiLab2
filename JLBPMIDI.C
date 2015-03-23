/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBPMIDI -  ** OUTBOUND MIDI DATA PROCESSOR **                  */
/*                          Volume I                                 */
/*                                                                   */
/*        Copyright J. L. Bell        March 1987                     */
/*                                                                   */
/* 11/7/88 Add velocity leveling function                            */
/* 11/7/89 Add MIDI 1.0 File Spec changes                            */
/* 07/7/93 Add 16 track support                                      */
/* 03/26/94 Use only trk1 request to handle all tracks               */
/* 06/23/94 Convert to UART mode                                     */
/* 08/08/95 Implement High Resolution Timer                          */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

/*********************************************************************/
/*             UART Play processor                                   */
/*********************************************************************/
SHORT playmdata(BYTE trkreq)
{
  ULONG ulDelay;                          // # of milliseconds to wait
  BYTE i, note, vel, lstf = LAST;
  static LONG DelTm = 0;

  ptrack = trkreq;                        /* Establish current track */

/*********************************************************************/
/*             Verify correct timing and quantize if requested       */
/*********************************************************************/
  if (quantize_amt && actvfunc != OVERDUB && sp.trkmask & 1 << ptrack)
  {
    if (EVP.dbyte1 == MEASURE_END)        /* Make final correction   */
      EVP.deltime = clocks_per_bar - timing_total[ptrack];
    else EVP.deltime = quantize(EVP.deltime);/*Go quantize event time*/
  }
//lstf = (EVP.deltime) ? 0 : LAST;        // Pack events with 0 timing

/*********************************************************************/
/*   Convert delta time to millseconds and block for required time   */
/*********************************************************************/
  DelTm += EVP.deltime;                   // Accumulate delta time
  if (EVP.deltime == 240)                 // Just add in null time
    goto pmidi_exit;

  if (DelTm)
  {
    ulDelay = ms_per_tick * DelTm;        // Set msecs for HRT block
    Delay(ulDelay);
  }
  DelTm = 0;                              // Reset for next event

/*********************************************************************/
/*             MIDI 1.0 File Meta Event Processing                   */
/*********************************************************************/
  if (EVP.flags1 & METAEVT)               /* MIDI 1.0 Meta Event     */
  {
//  mpucmd(0x33, 0);                      /* Tell driver to block    */
    proc_meta(ptrack, ptrack);            /* Process Meta Event      */
    goto pmidi_exit;                      /* Go finish up            */
  }

  switch (EVP.dbyte1)
      {
      case MEASURE_END:
/*********************************************************************/
/*                 MEASURE END Processing                            */
/*    'ztrack' is the track selected for re-syncing the metronome    */
/*********************************************************************/
        if (ptrack == ztrack && actvfunc != OVERDUB)
          mpuputd(EVP.dbyte1, lstf);      /* Send it on to MPU       */
        else mpuputd(NOP, lstf);          /* Change it to a NO-OP    */

/*********************************************************************/
/*  The total clock timings for one measure should be equal to the   */
/*  number of beats per bar multiplied by the timebase (e.g. 120)    */
/*  At measure end time we will verify this to be true; otherwise    */
/*  we will force an error condition and stop playback.              */
/*                                                                   */
/*  1/23/94 - Disable the timing test for the measure immediately    */
/*  following advance positioning.                                   */
/*********************************************************************/
        if (timing_total[ptrack] != clocks_per_bar)
        {
          if (!(sp.proflags & MTERROFF) &&/* Unless chk is disabled  */
            !(TmgChkDefer & 1 << ptrack)) /* Ignore just after pos.  */
              event_error(256);           /* go to stop play         */
        }
        TmgChkDefer &= (0xFFFF - (1 << ptrack)); // Reset swt
        timing_total[ptrack] = 0;         /* Reset total counter     */
        goto pmidi_exit;                  /* Go finish up            */

/*********************************************************************/
/*             No-Operation (Null) Processing                        */
/*********************************************************************/
      case NOP:                           /* MPU-401 no-op           */
        mpuputd(NOP, lstf);
        goto pmidi_exit;                  /* Go finish up            */

/*********************************************************************/
/*                END OF TRACK Processing                            */
/*********************************************************************/
      case END_OF_TRK:
        mpuputd(END_OF_TRK, lstf);
        actv_trks ^= (1 << ptrack);      /* Mark this track inactive */
        ndxwrt(ptrack, CLR_BLACK);       /* Go set final index       */

        if (ptrack == ztrack)            /* If this was the track    */
          for (i=0; i < NUMTRKS; ++i)    /* metronome, we must pass  */
            if (actv_trks & 1 << i)      /* the baton to the next    */
            {                            /* active track, if any.    */
              ztrack = i;                /* PMIDI will test 'ztrack' */
              break;                     /* during end_meas. process */
            }
        mpuputd(0, RESET);               /* Purge output buffer      */
        return(0);                       /* Get right out (no ndxwrt)*/
      }

/*********************************************************************/
/*                Real MIDI data from here down                      */
/*********************************************************************/
  if (!(EVP.flags1 & RUNSTAT))            /* New Channel Status      */
  {
    if (sp.tcc[ptrack])          /* Re-map MIDI channel if necessary */
      EVP.evtype = EVP.evcode | sp.tcc[ptrack]-1; /* Force assigned chn */

    if (EVP.evcode == PGM_CHANGE && sp.ccswt == LEVEL3)
      return(0);                          /* If filtering pgm changes*/
    else mpuputd(EVP.evtype, 0);          /* Send status event type  */
  }

  switch (EVP.evcode)                     /* Begin analysis          */
  {
/*********************************************************************/
/*             Event type: NOTE ON/NOTE OFF                          */
/*********************************************************************/
    case NOTE_ON:
    case NOTE_OFF:
      note = EVP.dbyte1;                  /* Set current note        */
      if (sp.trkmask & 1 << ptrack)       /* If enabled for adj      */
        note = (note+sp.transadj > 127) ? note : note+sp.transadj;
      mpuputd(note, 0);                   /* Send note number        */

      vel = EVP.dbyte2;                   /* Set current velocity    */
      if ((EVP.evcode == NOTE_ON) && vel) /* If note-on              */
      {
        if (trksolo && (!(trksolo & 1 << ptrack)))
          vel = 0;                        /* Mute trk                */
        else
        {
          if (sp.trkmask & 1 << ptrack)   /* If track is enabled     */
          {
            if (sp.proflags & VELLEVEL)   /* Velocity leveling on,   */
              vel = 64;                   /* force uniform velocity  */
            vel = (vel+sp.velocadj > 127) ? 127 : vel+sp.velocadj;
          }
        }
        mpuputd(vel, lstf);               /* Send veloc (NOTE_ON) &  */
        break;                            /* update index display    */
      }
      mpuputd(vel, lstf);                 /* Send veloc (NOTE_ON) &  */
      return(0);                          /* >don't< update ndx disp */

/*********************************************************************/
/*             Event type: CONTROLLER CHANGE                         */
/*********************************************************************/
    case CONTROLLER:
      mpuputd(EVP.dbyte1,0);                 /* Send controller number  */
      if (EVP.dbyte1 == 64)                  /* Sustain pedal           */
        fsustain |= (1 << EVP.in_chan);      /* Remember it's down      */

      mpuputd(EVP.dbyte2, lstf);             /* Send control value      */
      if (!EVP.dbyte2 && (fsustain & 1 << EVP.in_chan))/* Sustain release*/
        fsustain &= 65535 - (1 << EVP.in_chan); /* Reset the flag       */
      break;

/*********************************************************************/
/*             Event type: PITCH-BEND WHEEL                          */
/*********************************************************************/
    case PITCH_BEND:
      mpuputd(EVP.dbyte1,0);                 /* Send bender LS byte  */
      mpuputd(EVP.dbyte2, lstf);             /* Send bender MS byte  */
      break;

/*********************************************************************/
/*             Event type: PROGRAM CHANGE                            */
/*********************************************************************/
    case PGM_CHANGE:
     if (sp.ccswt == LEVEL3)              /* Filtering pgm changes   */
     {
       if (EVP.flags1 & RUNSTAT)          /* Send noop if in running */
         return(0);                       /* status.                 */
     }
     else mpuputd(EVP.dbyte1, lstf);      /* Send program number     */
     break;

/*********************************************************************/
/*             Event type: CHANNEL AFTER-TOUCH                       */
/*********************************************************************/
    case AFTER_TOUCH:
     mpuputd(EVP.dbyte1, lstf);           /* Send after-touch value */
     break;

/*********************************************************************/
/*             Event type: POLYPHONIC AFTER-TOUCH                    */
/*********************************************************************/
    case POLY_AFTER:
     mpuputd(EVP.dbyte1,0);               /* Send note number        */
     mpuputd(EVP.dbyte2, lstf);           /* Send pressure           */
     break;

    default:
     return(8);
  }

/*********************************************************************/
/*                           Module Exit                             */
/*********************************************************************/
 pmidi_exit:                   /* Come here when event cases "break" */
  if (ini.indexdispl)
    ndxwrt(ptrack, CLR_BLUE);               /* Go to display index's */
  return(0);
}
