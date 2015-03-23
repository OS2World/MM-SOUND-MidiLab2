/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*  JLBMPU00 -    ** MPU-401 DATA/COMMAND ROUTINES **                */
/*                                                                   */
/*       Copyright  J. L. Bell        March 1987                     */
/*                                                                   */
/*     Callable Functions:                                           */
/*                          MPUCMD - Issue MPU command               */
/*                         MPUGETD - Get data byte from MPU          */
/*                         MPUPUTD - Send data byte to MPU           */
/*                                                                   */
/* 05/18/90 - Incorporated MPU simulation for all functions. This    */
/* change obviates the module JLBTEST0. (finally!)                   */
/* 06/20/91 - Incorporated MPU device driver for all MPU I/O         */
/* 07/28/92 - Moved to DLL side of system                            */
/* 06/11/94 - Implemented UART mode                                  */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
#define GBUFSZ 250
#define PBUFSZ 100
extern HMQ hMQ;

ULONG gbufrndx = 0, pbufrndx = 0, bytes_read = 0;
HMTX  SemPutd;      // MPUPUTD semaphore
BYTE  gbufr[GBUFSZ], pbufr[PBUFSZ], hi_water_mark=0;

/*********************************************************************/
/*              MPU-401 Command processor                            */
/*********************************************************************/
APIRET mpucmd(BYTE cmd, BYTE dbyte)
{
  ULONG datalen;
  APIRET rc;

  if (trace & TRC_CMDS)                 // We're tracing something
  {
    if (cmd >= 0xe0 && cmd <= 0xef)     // Uses a data byte
      sprintf(msgbfr,"Cmd %2X Data:%3d  ",cmd, dbyte);
    else sprintf(msgbfr,"Cmd %2X  ",cmd);
    Wrt_Trace(msgbfr);
  }

  if (sysflag2 & NOMPU)                 // MPU simulation
  {
    UART_Cmd(cmd);                      // Go handle
    return(0);
  }

 again:
  datalen = sizeof(IOCPARMS);
  IOCParms.ucMPUCmd = cmd;
  IOCParms.ucCmdData = dbyte;

  if (cmd == MPU_TEMPO)
  { // Take some key values along for the ride
    IOCParms.usCParm1 = sp.tbase;       // Time base
    IOCParms.ucCParm1 = sp.tsign;       // Time signature
  }

  rc = DosDevIOCtl(mpu_hnd, MlabCat, MPU_CMD,
                     &IOCParms, sizeof(IOCPARMS), &datalen, NULL, 0, NULL);
  if (!rc)                              // Completed OK
    cmd_in_progress = cmd;              // Save current command

  else switch (rc)
  {
   case 0xfffb:                         // FFFB - MPU is not respoding
    break;

   case 0xfff5:                         // FFF5 - Command discarded by driver
    UART_Cmd(cmd);                      // Go handle
    break;

   case 0xfff4:                         // FFF4 - UART mode ended
    cmd_in_progress = OFF;              // No ACK in this case
    if (!(sysflag1 & SHUTDOWN))         // If not in shutdown
      ini.iniflag1 &= 255-UARTMODE;     // Reset flag
    break;

   case 0xfff3:                         // FFF3 - Issued during UART mode
    UART_Cmd(cmd);                      // Go handle
    break;

   case 0xfff2:                         // FFF2 - Busy with previous command
    goto again;                         // Re-issue command

   case 0xfff1:                         // FFF1 - Invalid cmd
    sprintf(msgbfr, "Invalid Command: $%X", cmd);
    umsg('E', hWndClient,  msgbfr);
    break;

   default:
    sprintf(msgbfr, "In deep yogurt - RC: $%X", rc);
    umsg('E', hWndClient,  msgbfr);
  }
  return(rc);
}

/*********************************************************************/
/*              MPU-401 Put Data function                            */
/*********************************************************************/
void mpuputd(BYTE pdata, BYTE cflg)
{
  APIRET rc;
  ULONG ParmLnth, DataLnth;

  DosRequestMutexSem(SemPutd, SEM_INDEFINITE_WAIT); // Avoid conflict w/metronm
  if (cflg == RESET)                    // Purge output buffer
    goto purgebfr;

  if (trace & TRC_OUTDATA)              // Tracing output
  {
    sprintf(msgbfr, "Put:%3d Ndx:%d",pdata, pbufrndx);
    Wrt_Trace(msgbfr);
  }
  pbufr[pbufrndx] = pdata;              // Save data byte on stack
  ++pbufrndx;                           // Increment stack index

  if (cflg==LAST)                       // Time to send event(s)
  {
//  if (pbufrndx < PBUFSZ-5 && iuo == LEVEL2) return; // Wait until buffer's full 12/30/94

 purgebfr:
    if (pbufrndx)                       // Something in the buffer
    {
      IOCParms.ulBufrLn = pbufrndx;     // Store data length
      ParmLnth = sizeof(IOCPARMS);
      DataLnth = pbufrndx;
      if (!(sysflag2 & NOMPU))          // Not in MPU simulation
      {
        rc = DosDevIOCtl(mpu_hnd, MlabCat, SEND_DATA,
                            &IOCParms, sizeof(IOCPARMS), &ParmLnth,
                            pbufr,  pbufrndx, &DataLnth);
        if (rc)
        {
          sprintf(msgbfr, "SEND_DATA Error  RC: $%X", rc);
          umsg('E', hWndClient,  msgbfr);
        }
      }
      if (trace & TRC_OUTDATA)          // Tracing output
      {
        sprintf(msgbfr, "Write %d bytes; rc:%X",pbufrndx, rc);
        Wrt_Trace(msgbfr);
      }
      pbufrndx = 0;                     // Reset buffer index
    }
  }
  DosReleaseMutexSem(SemPutd);
}

/*********************************************************************/
/*              MPU-401 Get Data function                            */
/*********************************************************************/
BYTE mpugetd(void)
{
  APIRET rc;
  static ULONG Biggest_Chunk= 0;
  BYTE input_byte;

read:
  if (gbufrndx < bytes_read)              // Something in input buffer
  {
    input_byte = gbufr[gbufrndx];         // Set input data byte
    if (trace & TRC_INDATA)               // Tracing input data
    {
      sprintf(msgbfr, "Get:%3d Ndx:%d",input_byte, gbufrndx);
      Wrt_Trace(msgbfr);
    }
    gbufrndx++;                           // Incr input buffer index
    return(input_byte);                   // Return with input data
  }
  else                                    // Input buffer is empty
  {
    gbufrndx = 0;                         // Reset buffer index
    rc = DosRead(mpu_hnd                  // Read some bytes from driver
         , &gbufr[0]                      // into the input buffer
         , GBUFSZ                         // #Bytes to read
         , &bytes_read);                  // #Bytes actually read
    if (rc)                               // 6/9/96 fix
    {
      mpucmd((actvfunc == RECORD) ? 0x11 : 0x15, 0); // Issue Stop
      mpucmd(0x33, 0);                    /* Issue driver reset      */
      umsg('E', hWndClient,  "Input buffer overrun; restart");
      DosExit(EXIT_THREAD, 8);            // End the thread
    }

    if (trace & TRC_READBUFR)             // Tracing DosRead
    {
      if (bytes_read > Biggest_Chunk)
        Biggest_Chunk = bytes_read;       // Set new high water mark
      sprintf(msgbfr, "Read %lu, rc:%X. HWM:%lu", bytes_read, rc, Biggest_Chunk);
      Wrt_Trace(msgbfr);
    }

    if (rc == 0x1f)
      DosExit(EXIT_THREAD, 0);            // End the thread
    else goto read;
  }
  return(0);
}
