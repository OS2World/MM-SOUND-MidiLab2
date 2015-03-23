/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBGTEV0 -  ** Get Midi Event from various sources **           */
/*                TRK.flags1 = TRACK: get event from track           */
/*                TRK.flags1 = MPU  : get event from MPU             */
/*                TRK.flags1 = DISK : get event from MIDI disk file  */
/*                                                                   */
/*        Copyright J. L. Bell        March 1987                     */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
#define PKT evp[tRK]                 // Define packet to save space

void get_byte(BYTE tRK);
BYTE indata;

int get_event(SHORT tRK)
{
  BYTE    c, rcode = 0;
  USHORT  TLnth;
  SHORT   ptrack = tRK;

  PKT.flags1 &= 255-RUNSTAT-METAEVT-MPUMARK-DBYTES2-EOT-MLABMETA; //Reset some flags
  vlqlnth = 1;                             // Set default vlq

  if (TRK.flags1 & DISK)
    PKT.deltime = read_varlen(tRK+1);      // Read delta time from disk buffer
  else
  {
    get_byte(tRK);                         // Get indata from MPU or track
    if (indata >= 240)                     // MPU Single-byte message
    {
      if (indata == SYSX && (TRK.flags1 & TRACK))
      {
        rcode = 16;                        // Trying to play SYSX track
        goto pre_exit;
      }

      if (indata == TRKCMDHDR && !(TRK.flags1 & MPU)) // Old style track order
      {
        Load_Trk_Order(tRK, PT + PTI);     // Go to load evp
        PKT.length = 6;                    // Set length
        PTI += 5;                          // Point beyond order
      }
      else
      {
        PKT.dbyte1 = indata;
        PKT.deltime = (indata == TIMING_OVFLO) ? 240 : 0;
        PKT.length = 1;                   // Set length
        PKT.flags1 |= MPUMARK;            // Indicate Mark (even tho it isn't)
      }
      goto pre_exit;                      // Go to validity check
    }
    else PKT.deltime = indata;
  }

/*********************************************************************/
/*      Get next byte in stream if disk or track                     */
/*********************************************************************/
  if (!(TRK.flags1 & MPU))             // 8/13/95
    get_byte(tRK);

  switch (indata)
  {
/*********************************************************************/
/*       Measure End, End-of-Track, and NOP                          */
/*********************************************************************/
    case MEASURE_END:
    case END_OF_TRK:
    case NOP:
      PKT.flags1 |= MPUMARK;                // Indicate Mark
      PKT.dbyte1 = indata;
      PKT.length = 2;                       // Set length
      if (indata == END_OF_TRK)
        PKT.flags1 |= EOT;                  // Set end of track
      goto pre_exit;                        // Go to validity check

/*********************************************************************/
/*                   1.0 Meta Event or SYSX data                     */
/*********************************************************************/
    case 255:                               // MIDI 1.0 Meta Event
      get_byte(tRK);                        // Get meta-event code
    case SYSX:                              // (8/02/93)
      PKT.dbyte1 = indata;                  // To keep old code honest
      PKT.evcode = indata;                  // Set evcode

      get_byte(tRK);                        // Get meta-event length
      if (TRK.flags1 & DISK)                // If reading a file buffer, process (7/17/93)
      {                                     // as Variable Length data
        if ((PKT.MetaLn = indata) & 0x80)   // Develop lnth of event data
        {
          PKT.MetaLn &= 0x7f;
          do
          {
            get_byte(tRK);                  // Get next length byte
            PKT.MetaLn = (PKT.MetaLn << 7) + ((c = indata)  & 0x7f);
            ++vlqlnth;                      // Incr #bytes in var lnth q.
          } while (c & 0x80);
        }
      }
      else PKT.MetaLn = indata;             // Just one length byte otherwise

/*********************************************************************/
/*   If this is a MidiLab meta event, load the event packet.         */
/*********************************************************************/
      if (PKT.evcode == 0x7f)     // Sequencer specific meta event
      {
        m_mlab = (M_MLAB *)(PT + PTI);           // Map the area
        if (!memcmp(Mlabid, m_mlab->mlabid, sizeof(Mlabid)))  // MLab Meta?
        {
          switch (m_mlab->mlabfunc)
          {
            case TRKCMDHDR:
              Load_Trk_Order(tRK, &m_mlab->mlabtrkc); // Go to load packet
              break;

            default:
              sprintf(msgbfr, "T%d - Invalid MidiLab/2 Meta event at I:%d", tRK+1, PTI);
              umsg('E', hWndClient,  msgbfr);
              rcode = 12;                 // Set error return code
          }  // End of switch(mlabfunc)
        }
      }
      if (PKT.evcode != SYSX)
        PKT.flags1 |= METAEVT;            // Indicate Meta event
      else rcode = 16;                    // unless it's a SysEx event
      TLnth = PKT.MetaLn + vlqlnth + 3;   // Set total length of event
      P_metadata = PT+PTI;                // Save pointer to meta data
      PTI += PKT.MetaLn;                  // Point beyond event
      if (PKT.MetaLn > 250)               // **Temporary limitation**
      {
        sprintf(msgbfr, "T%d - Meta event at I:%d truncated to 250 bytes", tRK+1, PTI);
        umsg('W', hWndClient,  msgbfr);
        PKT.MetaLn = 250;                 // Set truncated data length
        PKT.length = 254;                 // Set truncated total length
      }
      else PKT.length = TLnth;            // Set actual length
      skiptrks |= (1 << ptrack);          // Running status broken,fix
      goto pre_exit;                      // Go to validity check
  }

/*********************************************************************/
/*             Channel Status processing                             */
/*********************************************************************/
  if (indata >= 128)                    // New Channel Status
  {
    PKT.flags1 &= 255 - RUNSTAT;        // Reset running-status
    PKT.evtype = indata;                // Save channel status
    PKT.evcode = indata & 0xf0;         // Save raw event code
    PKT.in_chan =indata & 0x0f;         // Save incoming channel #
    PKT.prev_sts = indata;              // Save in case of forced resend
  }
  else                                  // Running Status active
  {
    if (skiptrks & 1 << ptrack)         // Forced resend of status if
    {                                   // skipping measures or meta event
      PKT.evtype = EVP.prev_sts;        // Get prior channel status
      PKT.evcode = EVP.prev_sts & 0xf0; // Re-build event code
      PKT.in_chan =EVP.prev_sts & 0x0f; // Re-build incoming channel #
    }
    PKT.dbyte1 = indata;
    PKT.flags1 |= RUNSTAT;              // Set running-status
  }

  switch (PKT.evcode)                   // Get rest of packet
  {
/*********************************************************************/
/*         Get data bytes for two-byte event types                   */
/*********************************************************************/
    case NOTE_ON:
    case NOTE_OFF:
    case CONTROLLER:
    case PITCH_BEND:
    case POLY_AFTER:

      PKT.flags1 |= DBYTES2;               // Indicate 2 data bytes
      get_byte(tRK);
      if (PKT.flags1 & RUNSTAT)            // Already got 1st one
      {
        PKT.dbyte2 = indata;
        PKT.length = 3;                    // Set length
        break;
      }
      else
        PKT.dbyte1 = indata;

      get_byte(tRK);
      PKT.dbyte2 = indata;
      PKT.length = 4;                      // Set length
      break;
/*********************************************************************/
/*         Get data bytes for one-byte event types                   */
/*********************************************************************/
    case PGM_CHANGE:
    case AFTER_TOUCH:

      if (PKT.flags1 & RUNSTAT)            // Already got 1st one
      {
        PKT.length = 2;                    // Set length
        break;
      }
      get_byte(tRK);
      PKT.dbyte1 = indata;
      PKT.length = 3;                      // Set length
      break;

    default:
      rcode = 8;                          // Set error return code
      PKT.length = 0;                     // Set null length
      break;
  }

  if (skiptrks & 1 << ptrack)           // Force resend of status if
  {                                     // skipping measures or meta event
    PKT.flags1 &= 255 - RUNSTAT;        // Reset running-status
    skiptrks ^= (1 << ptrack);          // Reset swt for this trk
  }

/*********************************************************************/
/*     Validity check the Event Packet and return to caller          */
/*********************************************************************/
  pre_exit:
  if (rcode == 0 && !(PKT.flags1 & MPUMARK + METAEVT) && // If ok so far and
      !(PKT.flags1 & MLABMETA))           // not a mark or meta event
  {
    if (!(TRK.flags1 & DISK) && PKT.deltime > 239 ||  // Validate timing
        PKT.evcode  < 128 ||              // Validate raw event code lo limit
        PKT.evcode  > 224 ||              // Validate raw event code hi limit
        PKT.dbyte1  > 127 ||              // Validate data byte 1
        ((PKT.flags1 & DBYTES2) &&
          PKT.dbyte2 > 127))              // Validate data byte 2
      rcode = 20;                         // Set error code if any of above
  }
  return(rcode);
}

/*********************************************************************/
/*         Get a single data byte from selected source               */
/*********************************************************************/
void get_byte(BYTE tRK)
{
  SHORT ptrack = tRK;
  if (TRK.flags1 & TRACK)              // Get data byte from track
    indata = *(PT + PTI++);

  else if (TRK.flags1 & MPU)           // Get data byte from MPU
  {
    indata = mpugetd();
    if (indata == CLK_TO_HOST)
    {
      Analyze_Input(CLK_TO_HOST, 1);   // Send Clock-to-Host
      indata = mpugetd();              // Get next byte
    }
  }

  else if (TRK.flags1 & DISK)          // Get data byte from disk buffer
    indata = *(PT + PTI++);
}

/*********************************************************************/
/*        Load MidiLab track order into the event packet             */
/*  The following EVP redefinitions apply:                           */
/*     evtype  -  Track Order code                                   */
/*     dbyte1  -  First adjustable byte                              */
/*     dbyte2  -  Second adjustable byte                             */
/*    in_chan  -  Flags for Trk Cmd                                  */
/*  trkcmddata -  For USHORT values where used                       */
/*********************************************************************/
VOID Load_Trk_Order(SHORT tRK, char *trkorder)
{
  switch (*trkorder)
  {
   case TRKXFER:                            // Track transfer
   case TRKREST:                            // Track rest measures
    ttptr = (TT *)(trkorder - 1);           // Map the area
    PKT.evtype = ttptr->trkcmdtype;
    PKT.dbyte1 = ttptr->ttrepeat;
    PKT.trkcmddata = ttptr->ttarget;
    break;

   case TRKINIT:                            // Track Initiate
    tiptr = (TI *)(trkorder - 1);           // Map the area
    PKT.evtype = tiptr->trkcmdtype;
    PKT.dbyte1 = tiptr->tiselect;           // Get lo order 1 - 8
    PKT.dbyte2 = tiptr->tiselect >> 8;      // Trk 9-16  (7/14/93)
    PKT.trkcmddata = tiptr->tistrtndx;
    break;

   case TRKCMDX:                            // Track Command
    tcptr = (TC *)(trkorder - 1);           // Map the area
    PKT.evtype = tcptr->trkcmdtype;
    PKT.dbyte1 = tcptr->tcmpucmd;
    PKT.dbyte2 = tcptr->tccmdata;
    PKT.in_chan = tcptr->tcflags;
    break;

   default:
    sprintf(msgbfr, "T:%d - Invalid Track Order at I:%d", trk+1, PTI);
    umsg('E', hWndClient,  msgbfr);
    break;
  }
  PKT.flags1 |= MLABMETA;   // Indicate MidiLab meta event
}
