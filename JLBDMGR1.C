/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*           JLBDMGR1 -  ** MIDI 1.0 File Subroutines **             */
/*    proc_meta - Process MIDI 1.0 Meta Events within track data     */
/*    dmgrsave  - Save MIDI song files to disk                       */
/*    write_varlen - write variable length numbers to MIDI files     */
/*    read_varlen - read var. lngth numbers from MIDI files or tracks*/
/*                                                                   */
/*       Copyright  J. L. Bell         Sept 1989                     */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

char metabfr[250], errbfr[36];          /* Meta text message buffers */
/*********************************************************************/
/*    proc_meta - Process MIDI 1.0 Meta Events within track data     */
/*    ptrack is id of evp; mtrk is track represented by MAIN evp     */
/*********************************************************************/
int proc_meta(SHORT ptrack, SHORT mtrk)
{
  long tmult;
  BYTE c, rcode = 0, txtswt = OFF;
  char *p_mtext, *txtptr;

  if (EVP.flags1 & MLABMETA)            /* New-style track order     */
    rcode = trkcmd();                   /* Process Mlab track order  */
  else
  {
    p_mtext = PT+PTI-EVP.MetaLn;        /* Point to 1st data byte    */

    switch (EVP.evcode)
    {
      case 0x01:
           txtptr = "General Text";
           txtswt = ON;
           break;

      case 0x02:
           txtptr = "Copyright";
           txtswt = ON;
           break;

      case 0x03:                           /* Sequence/Track name      */
      case 0x04:                           /* Instrument name          */
           c = (EVP.MetaLn > 16) ? 16 : EVP.MetaLn; // 16 characters max
           memset(sp.trkname[mtrk], ' ', 16);       // Clear name field
           memcpy(sp.trkname[mtrk], p_mtext, c);    // Copy text in
           WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)8, NULL);
           rcode = 12;                     /* Discard during load      */
           break;

      case 0x05:
           txtptr = "Lyric text";
           txtswt = ON;
           break;

      case 0x06:
           txtptr = "Marker Text";
           txtswt = ON;
           break;

      case 0x07:
           txtptr = "Cue Point";
           txtswt = ON;
           break;

      case 0x20:                           /* MIDI Channel prefix      */
           sp.tcc[mtrk] = *(p_mtext);      /* Set Track/Channel corr.  */
           WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)0x20, NULL);
           rcode = 12;                     /* Discard during load      */
           break;

      case 0x51:                             /* Set tempo              */
           m_tempo = (M_TEMPO *)p_mtext;     /* Map the area           */
           tmult = m_tempo->m1tempo << 8;    /* Get the tempo          */
           rev_int((char *)&tmult, 4);       /* Put in PC format       */
           if (tmult <= 0)
           {
             tmult = 600000;
             umsg('E', hWndClient, "Invalid tempo; forced to 100");
           }
           else if (tmult < 240000)          /* Tempo will be too high */
             tmult = 240000;                 /* Force it to 250        */
           sp.tempo = 60000000 / tmult;      /* Set actual tempo       */
           settbase(1);                      /* Set timebase controls  */
           WinPostMsg(hWndClient, UM_SET_HEAD, (MPARAM)0x04, NULL); // Upt tempo display
           break;

      case 0x54:                             /* SMPTE Offset           */
           break;                            /* Do nothing for now     */

      case 0x58:                             /* Set time signature     */
           m_tsign = (M_TSIGN *)(p_mtext);   /* Map the area           */
           sp.tsign = m_tsign->m2numer;
           settbase(2);                      /* Set timebase controls  */
           if (WinIsWindow(hAB, hWndCTLPANEL))
             WinPostMsg(hWndCTLPANEL, UM_DLGINIT, 0L, 0L);
           break;

      case 0x7f:                             /* Sequencer specific     */
             break;

        default:
           if (sysflag2 & DISKLOAD && OFF) /* Ignore all others 6/21/93*/
           {
             sprintf(errbfr,"Unsupported meta-event X'%02X' on T%d; ignored",
                             EVP.evcode, mtrk+1);
             umsg('W', hWndClient,  errbfr);
           }
           break;
    }
    if (txtswt && (sysflag2 & DISKLOAD) &&
        (ini.iniflag1 & MTEXT))             /* Display meta text      */
    {
      sprintf(metabfr, "%s on track %d:\n\n%.*s",
              txtptr, mtrk+1, EVP.MetaLn, p_mtext);
      umsg('I', hWndClient, metabfr);
    }
  }
  return(rcode);
}

/*********************************************************************/
/*                       Save file to disk                           */
/* First find if it's a SYSX track (1st byte = $F0). If so, find out */
/* how long the varible length sequence will be so that it can be in-*/
/* cluded in the chunk length field.  Then write the track chunk     */
/* prefix. If it's not a sysx track, just write it out as is.  In all*/
/* cases the meta end-of-trk sequence is appended. If QSAVE flag is  */
/* on, write the file out in old MidiLab format.                     */
/*********************************************************************/
dmgrsave(void)
{
  APIRET rc;
  ULONG pos1, pos2, temp;
  LONG  ltempo;
  BYTE sysxbyte = SYSX;
  char *fptr;

  if (chkact()) return(8);              /* 4/4/93                    */
  spptr = &sp;                          /* Set pointer to profile    */

  ulFileAttribute = FILE_ARCHIVED + FILE_NORMAL;
  MIDIoflgs = OPEN_ACTION_CREATE_IF_NEW + OPEN_ACTION_OPEN_IF_EXISTS;
  MIDImode = OPEN_FLAGS_FAIL_ON_ERROR + OPEN_FLAGS_NO_CACHE + OPEN_FLAGS_SEQUENTIAL +
             OPEN_FLAGS_NOINHERIT + OPEN_SHARE_DENYREADWRITE + OPEN_ACCESS_READWRITE;

  fptr = strrchr(filename, '.');        // Find ext., if present
  if (fptr != NULL)                     // Is there one?
    *(fptr) = '\0';                     // Yes, chop it off
  if (sysflag2 & QSAVE)
    strcat(filename, ".ML");            // Append ext for native fmt
  else strcat(filename, ".MID");        // Ext for normal 1.0 files

  rc = DosOpen(filename, &outfile, &action, 0,
                ulFileAttribute, MIDIoflgs, MIDImode, 0L);
  if (rc == 0)
  {
    if (action == 1)                    /* File exists               */
    {
      sprintf(msgbfr, "Replace %.12s?",filename);
      if (umsg('C', hWndClient,  msgbfr) != MBID_YES)
      {
        DosClose(outfile);
        return(4);
      }
    }
    else if (action == 2)              /* File did not exist         */
    {
      nop;                             /* Ok, file was created       */
    }
  }
  else
  {
    sprintf(msgbfr, "DosOpen error - Return Code:%d", rc);
    umsg('E', hWndClient, msgbfr);
    return(8);
  }
  PtrBusy(ON);                          /* Set Mouse pointer         */

/*********************************************************************/
/* If we're saving in Mlab format, we can save either in the new     */
/* 12-char name format or the old 8-char name format.     1/25/93    */
/*********************************************************************/
 if (sysflag2 & QSAVE)                          /* Save in Mlab fmt  */
 {
   sp.mlab_id = MLAB_FILE_VERNEW;
   DosWrite(outfile, (char*)spptr, sizeof(sp), &BytesXfrd); // Write profile
   for (ptrack = 0; ptrack < NUMTRKS; ++ptrack)      /* Write out the trks*/
     DosWrite(outfile, PT, PTL, &BytesXfrd);   // Write the tracks
 }

/*********************************************************************/
/*   We're going to do a 1.0 MIDI File Format; write Header chunk    */
/*********************************************************************/
 else                                   /* Save in MIDI 1.0 format   */
 {
  /* If tbase was originally non-std, use original value             */
  MThd.hdrtdiv  = (tbase_save) ? tbase_save : sp.tbase;
  rev_int((char *)&MThd.hdrtdiv, 2);    /* Reverse format            */

  MThd.hdrntrks = 1;                    /* Set default # of tracks   */
  for (ptrack = 0; ptrack < NUMTRKS; ++ptrack)/* Check for empty tracks   */
    if (PTL > 2) ++MThd.hdrntrks;       /* Increment track count     */
  rev_int((char *)&MThd.hdrntrks, 2);   /* Reverse format            */
  DosWrite(outfile, &MThd, sizeof(MThd), &BytesXfrd); // Write Hdr chunk
  dmgreset(1,0);                        /* Clear index's             */

/*********************************************************************/
/* Write the first track with tempo and other basic information      */
/*********************************************************************/
  ltempo = 60000000 / sp.tempo;           /* Develop usecs/qtr-note  */
  ltempo <<= 8;                           /* Move to left side       */
  rev_int((char *)&ltempo, 4);            /* Put in external format  */
  memcpy(&t1.tempo, &ltempo, 3);          /* Set in meta event       */
  t1.tsig_num = sp.tsign;                 /* Set time signature      */
  DosWrite(outfile, &t1, sizeof(t1), &BytesXfrd); // Write track 1

/*********************************************************************/
/*      Write the song tracks, skipping any empty ones.              */
/*********************************************************************/
  for (ptrack = 0; ptrack < NUMTRKS; ++ptrack) /* Set up to wrt trks */
  {
    if (PTL == 2) continue;             /* Omit empty tracks         */
    /* Basic length is trkdatalnth without $00FC plus meta_eot */
    MTrk.trklnth = (PTL - 2) + sizeof(meta_eot);
    delta_time = 0;                     /* Clear timing counter      */
    lnthdelta = 0;                      /* Clear VLQ adjustment ctr  */

/*********************************************************************/
/* If Sysex track, find #bytes in vlq and add it to total length     */
/*********************************************************************/
    if (*PT == SYSX)           /* 1st byte in track         */
    {
      write_varlen((long)PTL-3, LEVEL2);/* Remove  $F0               */
      MTrk.trklnth += vlqlnth;
    }

    DosSetFilePtr(outfile, 0L, FILE_CURRENT, &pos1); // Keep loc of trk hdr
    DosSetFilePtr(outfile, sizeof(MTrk), FILE_CURRENT, &temp);   // Leave space

    if (*(PT) == SYSX)           /* SysEx track            */
    {
      DosWrite(outfile, &sysxbyte, 1, &BytesXfrd);         // Write $F0
      write_varlen((long)PTL-3, LEVEL1);
      DosWrite(outfile, PT+1, PTL-3, &BytesXfrd); // Write sysx
    }

/*********************************************************************/
/* Normal song track; precede with meta events for name and channel  */
/*********************************************************************/
    else                                  /* Normal song track       */
    {
      memcpy(&tpx.tnm_name, &sp.trkname[ptrack], 12);   // Set track name
      tpx.tch_chan = sp.tcc[ptrack];                    // Set channel
      DosWrite(outfile, &tpx, sizeof(tpx), &BytesXfrd); // Write meta events
      MTrk.trklnth += sizeof(tpx);                      // Include in length

      TRK.flags1 |= TRACK;
      while (PTI < PTL-2)
      {
        if (get_event(ptrack))            // Scan and process track (unless err)
        {
          umsg('E', hWndClient, "Severe error scanning file");
          PtrBusy(OFF);                   // Reset Mouse pointer
          return(8);
        }

        if ((EVP.flags1 & MLABMETA) &&    // MLAB Rest or Transfer
           ((EVP.evtype == TRKREST) ||
            (EVP.evtype == TRKXFER)))
        {
          if (EVP.evtype == TRKREST)      // Bump delta time by #bars rest
            delta_time += ((sp.tbase * sp.tsign) * (EVP.dbyte1));
          else delta_time += EVP.deltime; // Throw XFERs away; just save timing
          MTrk.trklnth -= EVP.length;     // Decrement length accordingly
        }

        else if (EVP.flags1 & METAEVT)    // Write all other metas
        {
          write_varlen(EVP.deltime + delta_time, LEVEL1); // Write timing
          lnthdelta += --vlqlnth;         // Incr # of added bytes
          DosWrite(outfile, PT + PTI - EVP.length + 1,
              2, &BytesXfrd);             // Write $FF and meta code
          write_varlen(EVP.MetaLn, LEVEL1); // Write meta length
          lnthdelta += --vlqlnth;         // Incr # of added bytes
          DosWrite(outfile, PT + PTI - EVP.length + 4,
               EVP.length-4, &BytesXfrd); // Write remainder of event
        }

        else if (EVP.flags1 & MPUMARK)
        {
          delta_time+=EVP.deltime;        // Accumlate time for mark
          MTrk.trklnth -= EVP.length;     // and ignore it
        }

        else    // Plain old channel event
        {
          write_varlen(EVP.deltime + delta_time, LEVEL1); /* Write timing            */
          lnthdelta += --vlqlnth;         // Incr # of added bytes
          --EVP.length;                   // Remove timing byte from length
          if (!(EVP.flags1 & RUNSTAT))    // If not running status
          {
            DosWrite(outfile, &EVP.evtype, 1, &BytesXfrd); // Write ch status
            --EVP.length;                 // Remove ch. status from length
          }
          DosWrite(outfile, &EVP.dbyte1, EVP.length, &BytesXfrd); // Write data
        }
      }
    }
    write_varlen(delta_time, LEVEL1);     /* Write eot delta time    */
    lnthdelta += vlqlnth;                 // Incr # of added bytes
    DosWrite(outfile, meta_eot, sizeof(meta_eot), &BytesXfrd); // Meta trk-end
    DosSetFilePtr(outfile, 0L, FILE_CURRENT, &pos2); // Hold this spot

    MTrk.trklnth += lnthdelta;            /* Add in lnth caused by vlq*/
    rev_int((char *)&MTrk.trklnth, 4);    /* Reverse format          */
    DosSetFilePtr(outfile, pos1, FILE_BEGIN, &pos1); // Position to trk hdr
    DosWrite(outfile, &MTrk, sizeof(MTrk), &BytesXfrd); // Track chunk prefix
    DosSetFilePtr(outfile, pos2, FILE_BEGIN, &pos2); // Back to current pos
  }
 }
  DosClose(outfile);
  safequit = OFF;                         /* Everything's safely out */
  PtrBusy(OFF);                           /* Reset Mouse pointer     */
  return((action == 1) ? 2 : 0);          /* 2=replaced; 0=new file  */
} /* end dmgrsave */

/*********************************************************************/
/*      Routine to write variable length numbers to MIDI files       */
/*       Mode = LEVEL1: Write directly to output file                */
/*       Mode = LEVEL2: To determine number of bytes in Var lnth quan*/
/*********************************************************************/
void write_varlen(long value, char mode)
{
  LONG work;
  BYTE LoWork;

  vlqlnth = 0;
  work = value & 0x7f;
  while ((value >>= 7) > 0)
  {
    work <<= 8;
    work |= 0x80;
    work += (value & 0x7f);
  }
  while (TRUE)
  {
    if (mode == LEVEL1)
    {
      LoWork = (BYTE)work;              // Get lo order byte of work
      DosWrite(outfile, &LoWork, 1, &BytesXfrd); // Write to disk
    }
    ++vlqlnth;                          /* Incr number of bytes      */
    if (work & 0x80)
      work >>= 8;
    else break;
  }
  delta_time = 0;                       // Reset time accumulator
}

/*********************************************************************/
/* Routine to read variable length numbers from MIDI files or tracks */
/*********************************************************************/
long read_varlen(SHORT tRK)
{
  LONG value;
  BYTE c;

  vlqlnth = 1;                          /* Set default vlq           */
  if (tRK)
    value = *(trk[tRK-1].trkaddr + trk[tRK-1].index++);
  else DosRead(infile, &value, 1, &BytesXfrd);

  if (value & 0x80)
  {
    value &= 0x7f;
    do
    {
      if (tRK)
        c = *(trk[tRK-1].trkaddr + trk[tRK-1].index++);
      else DosRead(infile, &c, 1, &BytesXfrd);
      value = (value << 7) + (c & 0x7f);
      ++vlqlnth;                      /* Incr number of bytes in vlq */
    }
    while (c & 0x80);
  }
  return(value);
}
