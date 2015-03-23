/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal System/2  */
/*                                                                   */
/*        JLBSCRN0 -  ** Draw Main Client Window **                  */
/*                                                                   */
/*              Copyright  J. L. Bell        March 1987              */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

void paint(HPS hps, BYTE path)
{
  int i;
  RECTL urcl;
  POINTL ptl;
  LONG fClr, bClr, vattr;

  if (path == 255)
  {
/*********************************************************************/
/*         Draw a box around the grid                                */
/*********************************************************************/
    GpiSetLineWidth(hps, LINEWIDTH_THICK);  // Go for the bold look
    ptl.x = grid_left;
    ptl.y = cyClient;
    GpiMove(hps, &ptl);
    ptl.x = cxClient;
    ptl.y = grid_bottom;
    GpiBox(hps, DRO_OUTLINE, &ptl, 15, 15);

/*********************************************************************/
/*         Draw horizontal lines of grid                             */
/*********************************************************************/
    for (i = 1; i < 17; ++i)
    {
      ptl.x = grid_left;
      ptl.y = cyClient - ((cyChar+cyDesc) * i);
      GpiMove(hps, &ptl);
      ptl.x = cxClient;
      GpiLine(hps, &ptl);
    }

    ptl.x = grid_size;
    ptl.y = cyClient;
    GpiMove(hps, &ptl);
    ptl.y = grid_bottom;
    GpiLine(hps, &ptl);    // Draw "size" divider

    ptl.x = grid_content;
    GpiMove(hps, &ptl);
    ptl.y = cyClient;
    GpiLine(hps, &ptl);    // Draw "content" divider

    ptl.x = grid_chn;
    GpiMove(hps, &ptl);
    ptl.y = grid_bottom;
    GpiLine(hps, &ptl);    // Draw "chn" divider

    ptl.x = grid_index;
    GpiMove(hps, &ptl);
    ptl.y = cyClient;
    GpiLine(hps, &ptl);    // Draw "index" divider
    ptl.x = grid_trkname;
    GpiMove(hps, &ptl);
    ptl.y = grid_bottom;
    GpiLine(hps, &ptl);    // Draw "track name" divider
    GpiSetLineWidth(hps, LINEWIDTH_NORMAL);  // Back to normal

    GpiSetColor(hps, CLR_RED);     // Set title foreground color
    ptl.y = cyClient - cyChar;
    ptl.x = grid_left+2;
    GpiCharStringAt(hps, &ptl, 1, "T");
    ptl.x = grid_size;
    GpiCharStringAt(hps, &ptl, 5, " Size");
    ptl.x = grid_content;
    GpiCharStringAt(hps, &ptl, 8, " Content");
    ptl.x = grid_chn;
    GpiCharStringAt(hps, &ptl, 3, " Ch");
    ptl.x = grid_index;
    GpiCharStringAt(hps, &ptl, 6, " Index");
    ptl.x = grid_trkname;
    GpiCharStringAt(hps, &ptl, 11, " Track Name");
    GpiSetColor(hps, ini.MfgndClr);  // Back to normal

    WinInflateRect(hAB, &beatrcl,  cyDesc, cyDesc);
    WinInflateRect(hAB, &temporcl, cyDesc, cyDesc);
    WinInflateRect(hAB, &mctrrcl,  cyDesc, cyDesc);

    WinDrawBorder(hps, &beatrcl,  cyDesc, cyDesc, CLR_BLACK, CLR_YELLOW, DB_STANDARD);
    WinDrawBorder(hps, &temporcl, cyDesc, cyDesc, CLR_BLACK, CLR_BACKGROUND, DB_STANDARD);
    WinDrawBorder(hps, &mctrrcl,  cyDesc, cyDesc, CLR_BLACK, CLR_RED, DB_STANDARD);

    WinSetRect(hAB, &metrorcl,  beatrcl.xRight, beatrcl.yBottom,
      beatrcl.xRight + cxClient/12, cyClient / 7);   // Set Metronome position

    WinInflateRect(hAB, &beatrcl,  -cyDesc, -cyDesc);
    WinInflateRect(hAB, &temporcl, -cyDesc, -cyDesc);
    WinInflateRect(hAB, &mctrrcl,  -cyDesc, -cyDesc);

    i = cyClient / 16;        // This controls space between headings and boxes
    beatrcl.yBottom  += i;    // Go up a line to draw headings
    beatrcl.yTop     += i;
    temporcl.yBottom += i;
    temporcl.yTop    += i;
    mctrrcl.yBottom  += i;
    mctrrcl.yTop     += i;
    mctrrcl.xLeft    -= cxChar;
    mctrrcl.xRight   += cxChar;

    WinDrawText(hps, -1, "Beat",  &beatrcl,
                ini.MfgndClr, ini.MwndClr, DT_CENTER|DT_VCENTER|DT_ERASERECT);
    WinDrawText(hps, -1, "Tempo", &temporcl,
                ini.MfgndClr, ini.MwndClr, DT_CENTER|DT_VCENTER|DT_ERASERECT);
    WinDrawText(hps, -1, "Measure", &mctrrcl,
                ini.MfgndClr, ini.MwndClr, DT_CENTER|DT_VCENTER|DT_ERASERECT);

    beatrcl.yBottom  -= i;                  // Go back down a line
    beatrcl.yTop     -= i;
    temporcl.yBottom -= i;
    temporcl.yTop    -= i;
    mctrrcl.yBottom  -= i;
    mctrrcl.yTop     -= i;
    mctrrcl.xLeft    += cxChar;
    mctrrcl.xRight   -= cxChar;

    WinDrawText(hps, -1, "1", &beatrcl,
                CLR_RED, CLR_WHITE, DT_CENTER|DT_VCENTER|DT_ERASERECT);

    sprintf(msgbfr, "%d", sp.tempo);
    WinDrawText(hps, -1, msgbfr, &temporcl,
                CLR_WHITE, CLR_BLUE, DT_CENTER|DT_VCENTER|DT_ERASERECT);
    WinDrawText(hps, -1, "1", &mctrrcl,
                CLR_WHITE, CLR_RED, DT_CENTER|DT_VCENTER|DT_ERASERECT);
    WinDrawBitmap(hps, hbm_metro2, NULL, (PPOINTL)&metrorcl, 0L, 0L, DBM_STRETCH);
    WinSetDlgItemShort(hWndBIGBARS, 202, max_measures, TRUE);
  }

/*********************************************************************/
/*         Draw track number and track sizes                         */
/*********************************************************************/
  if (path & 0x80)         // Write track number and sizes
  {
    for (i=0; i < 16; ++i)
    {
      if (emode & (1 << i))    // If Trk is in Edit
      {
        fClr = CLR_WHITE;
        bClr = CLR_RED;
      }
      else
      {
        fClr = ini.MfgndClr;
        bClr = ini.MwndClr;
      }
      urcl = tccrcl[i];
      urcl.xLeft = grid_left+2;
      urcl.xRight = grid_size-2;
      WinDrawText(hps, -1, _itoa(i+1, ndxbfr, 10), &urcl, fClr, bClr,
                  DT_LEFT | DT_VCENTER | DT_ERASERECT);

      urcl = tccrcl[i];
      urcl.xLeft = grid_size+2;
      urcl.xRight = grid_content-2;
      WinDrawText(hps, -1, _itoa(ini.trksize[i], ndxbfr, 10), &urcl, ini.MfgndClr,
                  ini.MwndClr, DT_RIGHT | DT_VCENTER | DT_ERASERECT);
    }
  }

/*********************************************************************/
/*         Draw track contents                                       */
/*********************************************************************/
  if (path & 0x40)         // Write track contents
  {
    for (i=0; i < 16; ++i)
    {
      urcl = tccrcl[i];
      urcl.xLeft = grid_content+4;
      urcl.xRight = grid_chn-2;
      ltime = sp.trkdatalnth[i];
      WinDrawText(hps, -1, _ltoa(ltime, ndxbfr, 10), &urcl, ini.MfgndClr,
                  ini.MwndClr, DT_RIGHT | DT_VCENTER | DT_ERASERECT);
    }
  }

/*********************************************************************/
/*         Draw track-to-channel assignments                         */
/*********************************************************************/
  if (path & 0x20)         // Write track/channel assignments
  {
    for (i=0; i < 16; ++i)
    {
      _itoa(sp.tcc[i],ndxbfr,10);
      WinDrawText(hps, -1, &ndxbfr[0], &tccrcl[i], ini.MfgndClr, ini.MwndClr,
                DT_RIGHT | DT_VCENTER | DT_ERASERECT);
    }
  }

/*********************************************************************/
/*         Draw track index counters                                 */
/*********************************************************************/
  if (path & 0x10)         // Write track index counters
  {
    for (i=0; i < 16; ++i)
    {
      vattr = ini.MwndClr;
      if (sp.playtracks & 1 << i)
        vattr = CLR_BLUE;                   /* Highlite play track(s) */

      if (((actvfunc == RECORD || actvfunc == OVERDUB || actvfunc == SYSX) &&
            i == rtrack) || (emode & 1 << i))
        vattr = CLR_RED;                    /* Highlite record track */

      ndxwrt(i, vattr);                     /* Go to write index     */
    }
  }

/*********************************************************************/
/*         Draw track names                                          */
/*********************************************************************/
  if (path & 0x08)         // Write track names
  {
    for (i=0; i < 16; ++i)
    {
      WinDrawText(hps, 16, &sp.trkname[i][0], &namrcl[i], ini.MfgndClr, ini.MwndClr,
                DT_LEFT | DT_VCENTER | DT_ERASERECT);
    }
  }

/*********************************************************************/
/*         Update measure counter and tempo                          */
/*********************************************************************/
  if (path & 0x04)         // Write measure counter
  {
    WinDrawText(hps, -1, _itoa(measurectr,ndxbfr,10), &mctrrcl,
                CLR_WHITE, CLR_RED, DT_CENTER|DT_VCENTER|DT_ERASERECT);
    if (sysflag2 & BIGBARS)  // In-your-face measures
      WinSetDlgItemShort(hWndBIGBARS, 201, measurectr, TRUE);
    WinDrawText(hps, -1, _itoa(sp.tempo  ,ndxbfr,10), &temporcl,
                CLR_WHITE, CLR_BLUE, DT_CENTER|DT_VCENTER|DT_ERASERECT);
  }
}
