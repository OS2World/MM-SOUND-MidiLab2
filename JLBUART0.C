/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*  JLBUART0 -    ** MPU-401 UART command processing **              */
/*                                                                   */
/*       Copyright  J. L. Bell        June 1994                      */
/*                                                                   */
/* 06/16/94 - Genesis of this function                               */
/* 06/25/94 - Moved MAIN0 play-request to here                       */
/* 08/08/95 - Implement High Resolution Timer                        */
/*********************************************************************/
#define   EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

VOID UART_Play(VOID);

/*********************************************************************/
/*     This routine traps and simulates MPU commands in UART mode    */
/*********************************************************************/
VOID UART_Cmd(BYTE cmd)
{
  APIRET rc;
  BYTE i;

  switch (cmd)
  {
   case 0x05:                                   // Stop Play
     break;

   case 0x0a:                                   // Start Play
   case 0x0b:                                   // Continue Play
     DosCreateThread((PTID)&tidU1, (PFNTHREAD)UART_Play, (ULONG)0,
          (ULONG)0, (ULONG)STKSZ);
     rc = DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, +7, tidU1);
     if (rc) DosBeep(800,1000);
     break;

   case 0x15:                                   // Stop Overdub
     space_bar();
     stopplay = ON;
   case 0x10:                                   // Stop Standby
   case 0x11:                                   // Stop Record
     Build_EVP(&EVMN, 0, END_OF_TRK, MPUMARK, 0);
     Analyze_Input(END_OF_TRK, 2);              // Invoke RMIDI
     break;

   case 0x20:                                   // Record Standby
     break;

   case 0x2a:                                   // Start Overdub
     DosCreateThread((PTID)&tidU1, (PFNTHREAD)UART_Play, (ULONG)0,
          (ULONG)0, (ULONG)STKSZ);
     DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, +7, tidU1);
   case 0x22:                                   // Start Record
     rec_deltime = *pulTimer;                   // Initialize delta timer
     break;

   case 0x33:                                   // Emergency reset
     break;

   case 0x83:                                   // Start metro (normal)
   case 0x85:                                   // Start metro (accented)
     break;

   case 0x84:                                   // Stop metro
     DosPostEventSem(SemClk);                   // Insure Click Thread ends
     break;

   case 0x88:                                   // MIDI THRU: OFF
   case 0x89:                                   // MIDI THRU: ON
   case 0x8a:                                   // Data-in-stop: OFF
   case 0x8b:                                   // Data-in-stop: ON
   case 0x90:                                   // Realtime affection: OFF
   case 0x91:                                   // Realtime affection: ON
     break;

   case 0x94:                                   // Clock-to-Host: OFF
     C2Hswt = OFF;                              // Indicate inactive
     break;

   case 0x95:                                   // Clock-to-Host: ON
     C2Hswt = ON;                               // Indicate active
     break;

   case 0xb9:                                   // Clear play map/All Notes Off
     for (i = 0; i < NUMTRKS; ++i)
     {
       mpucmd(0xd7, 0);                         // Issue want-to-send-data
       mpuputd(176 | i, 0);                     // Send all-notes-off
       mpuputd(123, 0);                         // (X'B0 7B 00')
       mpuputd(0, LAST);
     }
     break;

   default:
//   sprintf(msgbfr, "Cmd $%X issued in UART mode", cmd);
//   umsg('W', hWndClient,  msgbfr);
     break;
  }
}

/*********************************************************************/
/*             Thread to play MIDI files                             */
/*********************************************************************/
VOID UART_Play(VOID)
{
  APIRET rc;
  LONG   LoTime, Amt_Over;
  ULONG  ulDelay;                          // # of milliseconds to wait
  SHORT  CurTrk;

  HMQUART = WinCreateMsgQueue(hAB, 0);  /* Set up msg queue          */
  if (actvfunc == PLAY)
    Analyze_Input(0xfd , 1);              // Send initial Clock-to-Host

/*********************************************************************/
/* What we're going to do here is find which active track has the    */
/* smallest delta time associated with it. This will be the track we */
/* process on this call.  We'll then reduce the timing of all the    */
/* other active tracks by this current amount.                       */
/*********************************************************************/
  while (actv_trks && !stopplay)
  {
    LoTime = evp[ztrack].deltime;         // Prime LoTime with 1st actv trk
    CurTrk = ztrack;                      // Prime CurTrk the same way

    for (ptrack = ztrack; ptrack < NUMTRKS; ++ptrack)
      if (actv_trks & 1 << ptrack)        // If track is active playing
        if (EVP.deltime < LoTime)
        {
          LoTime = EVP.deltime;           // Save lowest delta time
          CurTrk = ptrack;                // and its track number
        }

    for (ptrack = ztrack; ptrack < NUMTRKS; ++ptrack)
      if (actv_trks & 1 << ptrack)        // If track is active playing
      {
        if (ptrack == CurTrk)             // Skip over trk with lowest timing
          continue;
        else EVP.deltime -= LoTime;       // Decrement all other trks accordingly
      }

/*********************************************************************/
/*       If playing, send Clock-to-Host at half-beat intervals       */
/*********************************************************************/
    if (!C2Hswt)
    {
      if ((ClkAccum + LoTime) >= ClkIncr) // Time to send Clock-to-host
      {
        Amt_Over = (ClkAccum + LoTime) - ClkIncr; // Save overage if any
        while (Amt_Over > ClkIncr)        // If overage > clock time
        {
          ulDelay = (ClkIncr-ClkAccum) * ms_per_tick; // Set msecs for HRT delay
          Delay(ulDelay);
          Analyze_Input(0xfd , 1);        // Send Clock-to-Host
          ClkAccum = 0;                   // Reset accumulator
          Amt_Over -= ClkIncr;            // Decrement overage
        }
        ulDelay = (ClkIncr-ClkAccum) * ms_per_tick; // Set msecs for HRT delay
        Delay(ulDelay);
        Analyze_Input(0xfd , 1);          // Send Clock-to-Host
        ClkAccum = LoTime = Amt_Over;     // Refresh accumulated time
      }
      else ClkAccum += LoTime;            // Accumulate clk time
    }

    ptrack = CurTrk;                      // Set the track we're processing
    EVP.deltime = LoTime;                 // Set adjusted delta time
    if (ptrack != RunSTrk)                // Same track as last one processed?
      EVP.flags1 &= 255-RUNSTAT;          // No, reset running status

    playmdata(ptrack);                    // Go to Play processor

    if (!(EVP.flags1 & MPUMARK))          // If not an MPU mark,
      RunSTrk = ptrack;                   // save for running status check

    if (actv_trks & (1 << ptrack))        // If track is still active
    {
      EVP.flags1 = TRACK;                 // Tell GTEV0 to read Track 8/19/95
      rc = get_event(ptrack);             // Get next track event
      if (rc) event_error(rc);            // Stop process

      if (EVP.deltime < 240)              // If not Overflow  4/4/94
      {
        EVP.deltime += TRK.SPTime;        // Add in chase timing adjmnt, if any
        if (EVP.deltime > 239)            // If over the mark,
        {
          TRK.SPTime = EVP.deltime - 239; // save excess
          EVP.deltime = 239;              // Set to maximum
        }
        else TRK.SPTime = 0;              // Clear chase timing adjustment
      }
      timing_total[ptrack] += EVP.deltime;// Incr time total/measure
    }
  }
  mpucmd(0xb9, 0);                      // All notes off
  mpuputd(0, RESET);                    // Purge output buffer

  if (!stopplay)                        /* If not Pause              */
    Analyze_Input(END_OF_TRK, 1);       /* Send End of track         */
  WinSendMsg(WinWindowFromID(hWndClient, MN_B7), GBM_ANIMATE,
                MPFROMSHORT(FALSE), NULL);  // Stop animation
  if (actvfunc == PLAY)                 /* Keep rolling if OVERDUB   */
  {
    cmd_in_progress = 0x94;             /* Fake clock to host:off    */
    Analyze_Input(CMD_ACK , 1);         /* Send Ack to above cmd     */
  }
  WinDestroyMsgQueue(HMQUART);          /* Kill msg q   8/20/95      */
}
