/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBVIEW1 -    ** MidiLab Song Editor Subroutines                */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*  Split off from JLBVIEW0 5/30/94                                  */
/*********************************************************************/
#define   EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

VOID Define_Rgn(VOID);
VOID Set_Bar(HPS bHPS);

extern BYTE RgnPresent;
extern SHORT sHscrlMax, sHscrlPos, sVscrlMax, sVscrlPos, one_bar, every_4_bars,
       yHdrText, yHdrLine, yTrkRow, x1stMeas, yTopBar, yBotBar, xBarHome,
       yBotLine, yRow, cxEdit, cyEdit;

extern RECTL MarkRcl, HscrlRcl, VscrlRcl, InvalRcl, rclBound, ClipRgn[2],
       MarkRgn[1], MarkRgnPrev[1];
extern HWND  hWndHscrl, hWndVscrl;
extern HRGN  hRgnClip, hRgnMark;
extern POINTL Edptl, ptlOffset, BarPtl;

/*********************************************************************/
/*           WM_PAINT routine - called from JLBVIEW0                 */
/*********************************************************************/
VOID wm_paint(HWND hWnd)
{
  APIRET rc;
  HRGN OldRgn;
  BYTE any_note_on = OFF;
  HPS paint_hps;
  SHORT i, j, k, M;

/****************************************************************/
/* WM_PAINT- Set horiz. scroll bar in case a new file was loaded*/
/****************************************************************/
  sHscrlMax = max(0, (max_measures * one_bar) - cxEdit + x1stMeas + cxChar);
  sHscrlPos = min(sHscrlPos, sHscrlMax);
  WinSendMsg(hWndHscrl, SBM_SETSCROLLBAR, MPFROM2SHORT(sHscrlPos,0),
             MPFROM2SHORT(0, sHscrlMax));
  WinEnableWindow(hWndHscrl, sHscrlMax ? TRUE:FALSE);

  paint_hps = WinBeginPaint(hWnd, 0, &InvalRcl);
  WinFillRect(paint_hps, &Edrcl, ini.EwndClr);   // Set window color
  GpiSetColor(paint_hps, ini.EfgndClr);     // Set foreground color

/****************************************************************/
/* WM_PAINT- Draw header line and 'Trk/Measure' identifiers     */
/****************************************************************/
  Edptl.x = cxEdit;                      // Go to right side
  Edptl.y = yHdrLine;                    // Position to header line
  GpiMove(paint_hps, &Edptl);            // Mark it
  Edptl.x = 0;                           // Reset to left side
  GpiLine(paint_hps, &Edptl);            // Draw header line

  Edptl.y = yHdrText;                    // Set text line position
  GpiCharStringAt(paint_hps, &Edptl, 15, "Trk  Measure ");

/****************************************************************/
/* WM_PAINT- Draw hard coded stuff for measure 1                */
/****************************************************************/
  if (sHscrlPos != 0)
    GpiExcludeClipRectangle(paint_hps, &ClipRgn[1]);
  Edptl.x = x1stMeas-cxChar-sHscrlPos;   // Position to 1st measure
  GpiCharStringAt(paint_hps, &Edptl, 1,"1");// Wrt meas #1 indicator
  Edptl.y = yHdrLine;                    // Drop down to tick position
  Edptl.x += cxChar - sHscrlPos;         // Set tick position
  GpiSetMarker(paint_hps, MARKSYM_PLUS); // Select '+' as tick mark
  GpiMarker(paint_hps, &Edptl);          // Wrt tick for 1st measure

/****************************************************************/
/* WM_PAINT- Draw measure numbers over header line              */
/****************************************************************/
  if (max_measures > 1)                  // If something is loaded
  {
    j = 4;                               // Start at measure 4
    Edptl.x = (x1stMeas - sHscrlPos) + every_4_bars - one_bar;
    for (i = 0; i < max_measures; ++i)
    {
      Edptl.y = yHdrLine;                // Drop down a tad for tick
      GpiMarker(paint_hps, &Edptl);         // Wrt tick
      Edptl.y = yHdrText;                // Come back up to text pos.
      Edptl.x -= cxChar;                 // Move nums left 1 char pos
      GpiCharStringAt(paint_hps, &Edptl, (j < 10) ? 1 : 3,
                   _itoa(j, ndxbfr,10)); // Wrt meas. nums
      j += 4;                            // Incr measure number by 4
      Edptl.x += (every_4_bars+cxChar);  // Increment measure# position
      if (Edptl.x > cxEdit+one_bar) break; // No need to go any further
    }
  }

/****************************************************************/
/* WM_PAINT- Draw bottom line                                   */
/****************************************************************/
  Edptl.y = yBotLine + sVscrlPos;        // Position to bottom line
  Edptl.x = 0;                           // Reset to left side
  GpiMove(paint_hps, &Edptl);            // Mark it
  Edptl.x = cxEdit;                      // Go to right side
  GpiLine(paint_hps, &Edptl);            // Draw bottom line

/****************************************************************/
/* WM_PAINT- Draw Measure boundary line                         */
/****************************************************************/
  Edptl.x = xBarHome - 1;                // Set boundary
  GpiMove(paint_hps, &Edptl);            // Mark it
  Edptl.y = cyEdit;                      // Position to top of client
  GpiLine(paint_hps, &Edptl);            // Draw boundry line

/****************************************************************/
/* WM_PAINT- Write track numbers and names, and grid lines      */
/****************************************************************/
  Edptl.y = yTrkRow + sVscrlPos;         // Initialize 1st row
  for (i = 0; i < 16; ++i)
  {
    if (ini.iniflag1 & EDITGRID)         // Draw grid lines if requested
    {
      Edptl.x = 0;                       // Reset to left side
      Edptl.y -= cyDesc;                 // To get line below markers
      GpiMove(paint_hps, &Edptl);        // Mark it
      Edptl.x = cxEdit;                  // Go to right side
      GpiLine(paint_hps, &Edptl);        // Draw grid line
      Edptl.y += cyDesc;                 // Back to normal position
    }
    if (sVscrlPos != 0)
      GpiExcludeClipRectangle(paint_hps, &ClipRgn[1]);
    Edptl.x = 0;                         // Reset to left side
    GpiCharStringAt(paint_hps, &Edptl, 2, _itoa(i+1, ndxbfr,10)); // Wrt track number
    Edptl.x += cxChar * 3;               // Position to trk name
    GpiCharStringAt(paint_hps, &Edptl, 14, sp.trkname[i]); // Wrt track name

/****************************************************************/
/* WM_PAINT- Write measure markers, analyzing types of data the measures contain */
/****************************************************************/
    GpiSetClipRegion(paint_hps, hRgnClip, &OldRgn); // Set clip region
    Edptl.x  = x1stMeas - sHscrlPos;     // Go to first measure
    Edptl.y += cyDesc;
    if (trk[i].num_measures > 0)         // If not an empty track
    {
      TRKLD.trkaddr = trk[i].trkaddr;    // Use Load EVP so we don't conflict with active function
      TRKLD.flags1 = TRACK;              // Tell GTEV0 to read track
      TRKLD.index = 0;                   // Reset index
      EVLD.dbyte1 = EVLD.dbyte2 = 0;     // Reset key fields
      EVLD.flags1 = EVLD.evcode = EVLD.evtype = 0;
      memset(note_array, OFF, sizeof(note_array)); // Clear note-on array
      M = 0;                             // Clear for Measure-End check
      while (!(EVLD.flags1 & EOT))       // Scan to end of track
      {
        rc = get_event(LOAD_EV);         // Get an event
        M += EVLD.deltime;               // Accumulate delta time
        if (M > clocks_per_bar * 5)      // We're probably missing a Meas-End
        {
          umsg('E', hWnd, "Missing Measure Markers; Song Editor cannot be used");
          i = 17;
          break;
        }

        if (EVLD.flags1 & MPUMARK)
        {
          if (EVLD.dbyte1 == MEASURE_END)// Time to write a measure mark
          {
            M = 0;                       // Re-prime Meas End check
// Next line noped because of uneven painting with slidertrack horiz scroll
//          if (Edptl.x >= InvalRcl.xLeft-one_bar) // Paint only invalid area
            {
              GpiSetMarker(paint_hps, MARKSYM_SQUARE); // Assume no note-on's
              if (any_note_on)           // We turned on a note
              {
                GpiSetMarker(paint_hps, MARKSYM_SOLIDSQUARE);
                any_note_on = OFF;       // Reset indicator
              }
              else  // See if any notes are still on from a previous measure
                for (k = 0; k < 128; ++k)
                  if (note_array[k])     // At least one is
                  {
                    GpiSetMarker(paint_hps, MARKSYM_SOLIDSQUARE);
                    break;               // No need to look further
                  }
              GpiMarker(paint_hps, &Edptl); // Write measure symbol
            }
            Edptl.x += one_bar;          // Go to next meas position
            if (Edptl.x > cxEdit+one_bar) break; // No need to go any further
          }
          continue;                      // Ignore other marks
        }

        else
          if ((EVLD.flags1 & MLABMETA) && (EVLD.evtype == TRKREST))
          {
            GpiSetMarker(paint_hps, MARKSYM_DIAMOND); // Set rest marker
            for (j = 0; j < EVLD.dbyte1; ++j)
            {
              GpiMarker(paint_hps, &Edptl); // Write measure symbol
              Edptl.x += one_bar;        // Go to next meas position
            }
          }

        else
          if ((EVLD.flags1 & MLABMETA) && (EVLD.evtype == TRKXFER))
          {
            GpiSetMarker(paint_hps, MARKSYM_SOLIDDIAMOND); // Set xfer marker
            for (j = 0; j < EVLD.dbyte1; ++j)
            {
              GpiMarker(paint_hps, &Edptl); // Write measure symbol
              Edptl.x += one_bar;        // Go to next meas position
            }
          }

        else
        if (EVLD.flags1 & METAEVT)
          continue;                      // Ignore other meta evts
        else
        {
          if (EVLD.evcode == NOTE_ON && EVLD.dbyte2) // If note-on
          {
            any_note_on = ON;                // Indicate a note is on
            note_array[EVLD.dbyte1] = ON;    // Indicate which note
          }
          else if ((EVLD.evcode == NOTE_ON && !EVLD.dbyte2) ||
                    EVLD.evcode == NOTE_OFF) // If note-off
          {
            any_note_on = ON;                // Indicate a note was on
            note_array[EVLD.dbyte1] = OFF;   // Indicate note turned off
          }

          else
          {
            GpiSetMarker(paint_hps, MARKSYM_SMALLCIRCLE);
            GpiMarker(paint_hps, &Edptl);    // Indicate other MIDI data
          }
        }
      }
    }
    GpiSetMarker(paint_hps, MARKSYM_EIGHTPOINTSTAR); // End-of-trk symbol
    GpiMarker(paint_hps, &Edptl);
    Edptl.y = (yTrkRow + sVscrlPos) - ((i+1) * yRow);  // Set next row
    GpiSetClipRegion(paint_hps, (HRGN)NULL, &OldRgn);  // Release clip
  }

/****************************************************************/
/* WM_PAINT- Set moving bar position and mark region            */
/****************************************************************/
  if (!actvfunc)  // Added this test 9/18/94 to prevent paint hang during play
    Set_Bar(paint_hps);                        // Go paint measure bar
  if (RgnPresent)                              // We have a marked area
  {
    GpiSetMix(paint_hps, FM_INVERT);
    rc = GpiPaintRegion(paint_hps, hRgnMark);  // Paint marked area
    if (rc == FALSE)
    {
      sprintf(msgbfr, "GpiPaintRegion2: %lX", WinGetLastError(hAB));
      umsg('E', hWnd, msgbfr);
    }
    GpiSetMix(ed_hps, FM_DEFAULT);
  }

  WinEndPaint(paint_hps);        /* End of WM_PAINT */
}

/**************************************************************************/
/*             Define the boundaries of a marked area                     */
/* MarkRcl contains track & measure nums; MarkRgn is set to window coords */
/**************************************************************************/
VOID Define_Rgn(VOID)
{
  MarkRgn[0].yBottom = (yTrkRow - cyDesc + sVscrlPos) - MarkRcl.yBottom * yRow;
  MarkRgn[0].yTop = (yTrkRow + cyChar-cyDesc + sVscrlPos) - MarkRcl.yTop * yRow;
  MarkRgn[0].xLeft  = xBarHome - sHscrlPos + (one_bar * MarkRcl.xLeft);
  MarkRgn[0].xRight = xBarHome - sHscrlPos + one_bar + (one_bar * MarkRcl.xRight);
}

/**************************************************************************/
/*            Locate area selected by mouse                               */
/*  Return codes: 0 = ok, coordinates are valid                           */
/*                8 = out of bounds                                       */
/*               12 = beyond end of track                                 */
/*               16 = Track is inactive                                   */
/**************************************************************************/
GRIDHIT Loc_Area(PUSHORT p_mMsg)
{
  SHORT xM, yM, yUpper, yLower, xLeft;
  GRIDHIT ghRtn = {0,0,0};

  xM = MOUSEMSG(p_mMsg)->x;                         // Get mouse coordinates
  yM = MOUSEMSG(p_mMsg)->y;

  if (yM < 3 || xM < xBarHome || yM > yHdrLine)     // Out of bounds (3 means near the bottom)
  {
    ghRtn.rc = 8;                                   // Set error return code
    return(ghRtn);                                  // Return to caller
  }

  for (ghRtn.Trk = 0; ghRtn.Trk < 16; ++ghRtn.Trk)  // Find track
  {
    yUpper = yHdrLine + sVscrlPos - (yRow * ghRtn.Trk); // Set top of current row
    yLower = yUpper - yRow;                         // Set bottom
    if ((yM <= yUpper) && (yM >= yLower))           // Check for match
    {
      if (sp.playtracks & 1 << ghRtn.Trk)           // If selected track is active
        break;                                      // Break 'for'
      else
      {
        ghRtn.rc = 16;                              // Set 'inactive trks' code
        return(ghRtn);                              // Take error return
      }
    }
  }

  for (ghRtn.Meas = 0; ghRtn.Meas < cxEdit; ghRtn.Meas += one_bar) // Find measure
  {
    xLeft  = xBarHome + ghRtn.Meas;
    if (xM >= xLeft && xM <= (xLeft + one_bar))
    {
      ghRtn.Meas = (xLeft - xBarHome + sHscrlPos) / one_bar;    // Develop measure number
      if (ghRtn.Meas > trk[ghRtn.Trk].num_measures) // Beyond end of trk
      {
        ghRtn.rc = 12;                              // Set appropriate rtn code
        return(ghRtn);                              // ghRtn.Meas = measure number
      }
      else break;                                   // Break 'for'
    }
  }
  return(ghRtn);                                    // Return Track and Measure
}

/*********************************************************************/
/*           Set_Bar - Position moving measure indicator             */
/*********************************************************************/
VOID Set_Bar(HPS bHPS)
{
  GpiSetColor(bHPS, ini.EwndClr);   // Set foreground color to erase
  BarPtl.y = yTopBar;               // Set top of bar

 /* UM_MOVE_BAR - Erase current position of bar in case we're starting Play */
  GpiMove(bHPS, &BarPtl);           // Set top of prior position
  BarPtl.y = yBotBar;               // Go to bottom
  if (BarPtl.x >= xBarHome)
    GpiLine(bHPS, &BarPtl);         // Erase prior position

 /* UM_MOVE_BAR - Erase bar from measure behind the current measure    */
  BarPtl.x = (xBarHome + (one_bar * (measurectr-1))) - sHscrlPos; // Set cur meas-1
  if (BarPtl.x >= xBarHome)         // If left scrl didn't move it out of range
  {
    BarPtl.x -= one_bar;            // Back up to previous measure
    GpiMove(bHPS, &BarPtl);         // Set bottom of previous measure
    BarPtl.y = yTopBar;             // Go to top
    GpiLine(bHPS, &BarPtl);         // Erase previous measure
    BarPtl.x += one_bar;            // Reset to current measure

 /* UM_MOVE_BAR - Draw bar at current measure position                 */
    GpiSetColor(bHPS, ini.EbarClr); // Set foregrnd to Bar color
    GpiMove(bHPS, &BarPtl);         // Mark top
    BarPtl.y = yBotBar + sVscrlPos; // Set bottom
    GpiLine(bHPS, &BarPtl);         // Draw new bar line
  }
}
