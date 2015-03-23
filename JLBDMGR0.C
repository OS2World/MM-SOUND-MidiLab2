/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBDMGR0 -     ** MIDI FILE STORAGE MANAGER **                  */
/*                                                                   */
/*       Copyright  J. L. Bell   -    March 1987                     */
/*                                                                   */
/*     Callable Functions:                                           */
/*                         DMGRLOAD - Load song data from disk       */
/*                         DMGOPEN  - Open a file                    */
/* Removed DMGRPUT function 10/19/92                                 */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

USHORT t, spsize;
SHORT tt;

/*********************************************************************/
/*            Open a disk file for input operations                  */
/*                                                                   */
/*  -- New  Window facility added 2/23/89. (pre-PM)                  */
/*  -- MIDI 1.0 File Support added 9/16/89                           */
/*********************************************************************/
char dmgopen(void)
{
  APIRET rc;
  LONG *idptr = &ltime;

  ulFileAttribute = FILE_ARCHIVED + FILE_READONLY;
  MIDIoflgs = OPEN_ACTION_OPEN_IF_EXISTS;  // Open if there, fail otherwise
  MIDImode = OPEN_FLAGS_FAIL_ON_ERROR + OPEN_FLAGS_NO_CACHE + OPEN_FLAGS_SEQUENTIAL +
             OPEN_FLAGS_NOINHERIT + OPEN_SHARE_DENYNONE + OPEN_ACCESS_READONLY;
  rc = DosOpen(filename, &infile, &action, 0,
                ulFileAttribute, MIDIoflgs, MIDImode, 0L);
  if (rc)
  {
    sprintf(msgbfr,"%.12s cannot be opened, Try again  RC:%d",filename, rc);
    umsg('E', hWndClient, msgbfr);
    return(8);                             // Try again
  }

  DosRead(infile, idptr, sizeof(sp.mlab_id), &BytesXfrd);
  if (ltime != MLAB_FILE_VERIFIER &&        /* Not a MidiLab file    */
      ltime != MLAB_FILE_VERNEW)
  {
    rc = strncmp(MThd.hdrid,(char *)idptr,4); /*Validate MIDI 1.0    */
    if (rc)
    {
      sprintf(msgbfr,"%.12s cannot be played by MidiLab/2",filename);
      umsg('E', hWndClient, msgbfr);
      DosClose(infile);
      return(8);                            /* Bag it                */
    }
    else sysflag2 |= MFILE10;               /* Indicate MIDI 1.0 file*/
  }
  else
  {
    sysflag2 &= 255-MFILE10;                /* Indicate native format*/
    spsize = (ltime == MLAB_FILE_VERNEW) ? sizeof(sp) : sizeof(sp) - 32;
  }
  rc = DosSetFilePtr(infile, 0L, FILE_BEGIN, &action); // Return to beginning
  return(0);                                /* Return to caller      */
}

/*********************************************************************/
/*   DMGRLOAD - Load profile and tracks from the selected file       */
/*                                                                   */
/*   After determining the file format, starting with track 1        */
/*   I will read the number of bytes indicated in the heading for    */
/*   each track, then switch to the next track and reiterate the     */
/*   process until all tracks have been read in.                     */
/*********************************************************************/
dmgrload(void)
{
  APIRET rc;
  LONG datalnth, deltaadj = 0;
  ULONG position;
  SHORT maxtrks, tbasediv = 1;
  char eot_found = OFF;
  extern LONG adjustment;

  dmgreset(3,0);                       /* Reset everything except MPU*/
  spptr = &sp;                         /* Set pointer to profile     */
  sysflag2 |= DISKLOAD;                /* Tell everybody about Load  */

  if (sysflag2 & MFILE10)              /* MIDI 1.0 File format       */
  {
    p_mthd = (MTHD *)malloc(sizeof(MTHD));    /* For hdr chunk       */
    p_mtrk = (MTRK *)malloc(sizeof(MTrk));    /* For trk prefix(s)   */

    DosRead(infile, (char*)p_mthd, sizeof(MThd), &BytesXfrd); // Rd hdr chunk
    rev_int((char *)&p_mthd->hdrlnth,  4);    /* Reverse format      */
    rev_int((char *)&p_mthd->hdrfmt,   2);    /* Reverse format      */
    rev_int((char *)&p_mthd->hdrntrks, 2);    /* Reverse format      */
    rev_int((char *)&p_mthd->hdrtdiv,  2);    /* Reverse format      */

    if (p_mthd->hdrlnth != 6)
    {
      sprintf(msgbfr,"Unsupported MIDI file");
      goto errexit;
    }

/*********************************************************************/
/* Read in track data. If the 1st data char in the track is $F0,     */
/* it's a SysEx track;  otherwise it's a song track.                 */
/* Trk 1 is assumed to be control info and is overlaid by the next.  */
/*********************************************************************/
    sp.tbase = p_mthd->hdrtdiv;  // Set system time base from header

    if (p_mthd->hdrntrks > 17)            /* Too many for MidiLab    */
    {
      sprintf(msgbfr,"Note: Only the first 17 tracks loaded; next %d ignored",
                p_mthd->hdrntrks - 17);
      umsg('W', hWndClient, msgbfr);
      maxtrks = 17;
    }
    else maxtrks = p_mthd->hdrntrks;

    PtrBusy(ON);                           /* Set Mouse pointer busy */
    for (tt = 0; tt < maxtrks; ++tt)       /* Iterate per #trks      */
    {
      t = tt? tt - 1 : tt;                 // This will cause trk2 to overlay 1
      trk[t].index = 0;
      DosRead(infile, (char*)p_mtrk, sizeof(MTrk), &BytesXfrd);// Rd trk prfix
      if (memcmp(p_mtrk->trkid, "MTrk", 4))    /* and validate       */
      {
        sprintf(msgbfr,"Track Chunk not found for T%d", t+1);
        goto errexit;
      }
      rev_int((char *)&p_mtrk->trklnth, 4);    /* Reverse format     */
      if (p_mtrk->trklnth > ini.trksize[t])    /* Too big for track  */
      {
        sprintf(msgbfr,"Overrun on T%d; %d bytes required. Increase track size.",
                  t+1, p_mtrk->trklnth);
      errexit:
        umsg('E', hWndClient, msgbfr);                  /* Issue error msg        */
        DosClose(infile);                  /* Close the file         */
        sysflag2 &= 255-DISKLOAD;          /* Reset flag             */
        PtrBusy(OFF);
        return(8);
      }
      rc = DosSetFilePtr(infile, 0L, FILE_CURRENT, &position); // Mark where we are

/*********************************************************************/
/*      Process Sysx track, end-of-track code must follow it         */
/*********************************************************************/
      DosRead(infile, &xtrack, 1, &BytesXfrd); // Read first byte
      *trk[t].trkaddr = xtrack;
      if (xtrack == SYSX)
      {
        datalnth = read_varlen(0);          /* Get lnth of sysx data */
        p_mtrk->trklnth -= vlqlnth;         /* Remove vlq from total */
        DosRead(infile, trk[t].trkaddr+1, datalnth, &BytesXfrd); // Rest of sysx
        datalnth = read_varlen(0);          /* Read delta-time of eot*/
        DosRead(infile, msgbfr, sizeof(meta_eot), &BytesXfrd); //Shd be MetaEot
        if (memcmp(meta_eot, msgbfr, sizeof(meta_eot)))
        {
          sprintf(msgbfr,"T%d: End-of-Track must follow SysEx data", t+1);
          goto errexit;
        }
        else      /* Replace Meta eot with MPU-401 end-of-trk        */
        {
          sp.trkdatalnth[t] = p_mtrk->trklnth - vlqlnth - 3;
          *(trk[t].trkaddr+sp.trkdatalnth[t]++) =  datalnth;
          *(trk[t].trkaddr+sp.trkdatalnth[t]++) =  END_OF_TRK;
        }
      }
/*********************************************************************/
/* It's a normal song track. (Assume trk 1 contains only setup data) */
/*********************************************************************/
      else                                  /* Normal song track     */
      {
        rc = DosSetFilePtr(infile, position, FILE_BEGIN, &position); //Strt of trk data
        rtrack = t;
        TRKLD.trkaddr = malloc(p_mtrk->trklnth); // Get storage for disk buffer
        DosRead(infile, TRKLD.trkaddr, p_mtrk->trklnth,
                      &BytesXfrd);          //Read in rest of track
        TRKLD.index = 0;
        p_mtrk->trklnth -= 2;               /* Remove X'2F00' eot    */

        settbase(0);                       // Set clocks_per_bar  (8/31/93)
        timing_total[t] = 0;               // Reset timing counter
        while (RTI < p_mtrk->trklnth)
        {
/*********************************************************************/
/*               Read event packets from disk                        */
/*********************************************************************/
          TRKLD.flags1 = DISK;              // Tell GTEV0 to read disk buffer
          rc = get_event(LOAD_EV);         // Get an event

  /* If the time base is a non-supported, but even, multiple of one of the
     MPU's, we'll fudge it dividing the delta time by the multiple (2/24/94) */
          EVLD.deltime += deltaadj;        // Add in previous remainder if any
          deltaadj = EVLD.deltime % tbasediv; // Get new remainder
          EVLD.deltime /= tbasediv;        // Adjust timing if too big

          if (rc)
          {
            if (rc == 16)                  // SysEx data present
            {
              sprintf(msgbfr, "System Exclusive data on T:%d ignored", t+1);
              umsg('W', hWndClient, msgbfr);
              *(RT + RTI++) = 0;           // Set delta time = 0
              *(RT + RTI++) = END_OF_TRK;  // Cut the track short
              eot_found = ON;              // Pretend EOT was found
              break;                       // Hang it up
            }
            else                           // Some other error
            {
              ptrack = t;                  // Send trk# to err msg
              goto get_err;
            }
          }

          p_mtrk->trklnth -= --vlqlnth;    // Reduce trk lnth by vlq amount
          if ((EVLD.flags1 & METAEVT) && (EVLD.evcode == 0x2f)) // Meta eot found
          {
            eot_found = ON;                       // Indicate found
            adjustment = 0;                       // Reset for Set_Deltime
            Set_Deltime(EVLD.deltime, t);         // Go to insert meas.ends
            p_mtrk->trklnth += adjustment;        // Increment length by amt added
            *(RT + RTI++) = END_OF_TRK;           // Set end-of-track

            if (p_mtrk->trklnth != RTI)           // Should be equal
            {
              sprintf(msgbfr,"Warning:T%d mismatch- Actual length:%d  Adjusted length:%d",
                             t+1, RTI, p_mtrk->trklnth);
              goto errexit;
            }
          }
          else
          {
              /* If this is a 1.0 meta event, save the data portion.*/
              /* Process the event, discarding track name (03, 04)  */
              /* and MIDI channel (20) event types.                 */
            if ((EVLD.flags1 & METAEVT) && !(EVLD.flags1 & MLABMETA))
            {
              rc = proc_meta(LOAD_EV, t);         // Go to execute event
              if (rc != 12)                       // Not 03,04, or 20 event
              {
                EVLD.Meta_ptr = malloc(EVLD.MetaLn);// Get stg for meta data
                memcpy(EVLD.Meta_ptr, TRKLD.trkaddr+TRKLD.index-EVLD.MetaLn, EVLD.MetaLn);  /* Copy meta data*/
                p_mtrk->trklnth += TrkBld(&EVLD, rtrack); // Move packet data to track and adjust length
                free(EVLD.Meta_ptr);              // Release meta data stg
              }
              else p_mtrk->trklnth -= EVLD.length; // Reduce trklnth for discarded event
            }
            else p_mtrk->trklnth += TrkBld(&EVLD, rtrack);  // Move packet data to track and adjust length
          }
/*
         // Set track default channel to the first one it hits. (10/24/95)
          if (iuo == LEVEL2    &&            // Testing for now...
              sp.tcc[t] == 0   &&            // No forced chn in effect
              !(EVLD.flags1 & METAEVT) &&    // Not a meta event
              !(EVLD.flags1 & MPUMARK) )     // Not an MPU mark
            sp.tcc[t] = EVLD.in_chan + 1;    // Set tcc to default channel
*/
        }
        if (eot_found == OFF)
        {
          sprintf(msgbfr,"Warning - End-of-Track not found for T%d", t+1);
          umsg('W', hWndClient, msgbfr);                /* Issue warning          */
        }
        eot_found = OFF;                /* Reset flags               */
        sp.trkdatalnth[t] = p_mtrk->trklnth;  /* Set internal length */
        ndxwrt(rtrack, CLR_BLUE);             // Go display index
      }
      free(TRKLD.trkaddr);               /* Free disk buffer          */
    }
    free((char *)p_mthd);               /* Free file hdr storage     */
    free((char *)p_mtrk);               /* Free track hdr storage    */
  }

/*********************************************************************/
/*                  Read MidiLab native format                       */
/*********************************************************************/
  else
  {
    DosRead(infile, (char*)spptr, spsize, &BytesXfrd); // Read in profile
    for (t = 0; t < NUMTRKS; ++t)       /* Iterate for # of tracks   */
    {
      if (sp.trkdatalnth[t] > ini.trksize[t])
      {
        sprintf(msgbfr,"Overrun on T%d; %ld bytes required. Increase track size.",
                  t+1, sp.trkdatalnth[t]);
        goto errexit;
      }
      DosRead(infile, trk[t].trkaddr, sp.trkdatalnth[t], &BytesXfrd); //Rd trk
    }
    /* Set time base for Midilab format files */
    if (!sp.tbase) sp.tbase = 120;      /* For old files             */
    time_base_cmd = 0xc2;               /* Lowest MPU base cmd       */
    for (t = 48; t <= 192; t += 24)
    {
      if (sp.tbase == t) break;
      ++time_base_cmd;
    }
/*********************************************************************/
/*    Pre-scan tracks for any meta events in native format files     */
/*********************************************************************/
    dmgreset(1,0);                      /* Reset track parameters    */
    for (ptrack = 0; ptrack < NUMTRKS; ++ptrack)
    {
      if((BYTE)*(PT) != SYSX)
      {
        TRK.flags1 = TRACK;             /* Indicate get track data   */
        while (PTI < PTL)
        {
          rc = get_event(ptrack);       /* Scan and process track    */
          if (rc)                       /* Error in track; bag it    */
          {
           get_err:
            sprintf(msgbfr, "This file contains invalid MIDI data\n on track %d", ptrack+1);
            umsg('E', hWndClient, msgbfr);
            dmgreset(3,0);
            DosClose(infile);
            return(8);
          }
          if (EVP.flags1 & METAEVT)     /* Meta event                */
            proc_meta(ptrack, ptrack);  /* Go do it                  */
        }
      }
    }
  }
  dmgreset(12,0);                       /* Done, Set MPU from profile*/
  Locate_SPP(-1);                       /* Go set max measures       */
  dmgreset(1,0);                        /* Finish reset              */
  sysflag2 &= 255-DISKLOAD;             /* Load complete             */
  WinSendMsg(hWndClient, UM_SET_HEAD, (MPARAM)255, NULL);
  WinPostMsg(hWndSongEditor, WM_COMMAND, MPFROM2SHORT(IDM_E_CLEAR,0), 0L); // Init. Editor
  DosClose(infile);
  PtrBusy(OFF);
  return(0);                            /* Normal return             */
}
