/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBVIEW0 -    ** MidiLab Song Editor Window **                  */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*  Work begun 5/3/94, in pretty good shape by 5/24                  */
/*  10/11/96 - Move setting of sp.playtracks to avoid positioning err*/
/*********************************************************************/
#define   EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

VOID Define_Rgn(VOID);
VOID Set_Bar(HPS bHPS);

APIRET rc;
BYTE  MarkSwt, RgnPresent, NotFirstTime = OFF;
SHORT sHscrlMax, sHscrlPos, sVscrlMax, sVscrlPos, one_bar, every_4_bars,
      yHdrText, yHdrLine, yTrkRow, x1stMeas, yTopBar, yBotBar, xBarHome,
      yBotLine, yRow, cxEdit, cyEdit, sHscrlInc, sVscrlInc, i,
      v_skip_bar_ctr, v_ending_measure, v_trksolo, v_actv_trks, v_playtracks;
SHORT TrkSave;

RECTL MarkRcl, HscrlRcl, VscrlRcl, InvalRcl, rclBound, ClipRgn[2],
      MarkRgn[1], MarkRgnPrev[1];
HWND  hWndHscrl, hWndVscrl;
HRGN  hRgnClip, hRgnMark;
GRIDHIT ghArea, gh1stMark, ghMem;
POINTL  Edptl, ptlOffset = {0,0}, BarPtl;

MRESULT EXPENTRY EDITWndProc(HWND hWnd, USHORT message, MPARAM mp1, MPARAM mp2)
{
  USHORT SizeStyle = SWP_ACTIVATE | SWP_ZORDER | SWP_SHOW | SWP_SIZE + SWP_MOVE;

  ed_hps = WinGetPS(hWnd);    // Get and hold general handle for all msg types
  if (GpiQueryRegionBox(ed_hps, hRgnMark, &rclBound) == RGN_RECT)
    RgnPresent = ON;          // Indicate a marked region is present
  else RgnPresent = OFF;

  switch(message)
  {
/*********************************************************************/
    case WM_CREATE:
/*********************************************************************/
      hWndHscrl = WinWindowFromID(WinQueryWindow(hWnd, QW_PARENT),
                                  FID_HORZSCROLL);
      hWndVscrl = WinWindowFromID(WinQueryWindow(hWnd, QW_PARENT),
                                  FID_VERTSCROLL);
      WinPostMsg(hWnd, UM_DLGINIT, 0L, 0L);    // Signal init routine
      break; /* End of WM_CREATE */

/*********************************************************************/
    case UM_DLGINIT:
/*********************************************************************/
      if (!ini.EditPos.cx)            // Virgin INI file, set SWP
      {
        WinSetWindowPos(hWndEDITFrame, HWND_TOP, 0, 0,  // x, y, cx, cy
                           40 * cxChar, 20 * cyChar, SizeStyle);
        WinQueryWindowPos(hWndEDITFrame, &ini.EditPos); // Set first-time position
      }
      else WinSetWindowPos(hWndEDITFrame, HWND_TOP, ini.EditPos.x, ini.EditPos.y,
                             ini.EditPos.cx, ini.EditPos.cy, SizeStyle);
      set_attr(hWnd, IDM_E_GRID, MIA_CHECKED, ini.iniflag1 & EDITGRID);
      WinSetPresParam(hWnd, PP_FONTNAMESIZE, 22, "10.System Proportional");
      sprintf(msgbfr, "MidiLab/2 Song Editor - %s", filename);
      WinSetWindowText(hWndEDITFrame, msgbfr); // Wrt song name as window title
      sysflag2 |= EDITCMPL;                  // Indicate window created
      break;

/*********************************************************************/
    case WM_COMMAND:
/*********************************************************************/
      i = 0;                     // Clear Client message parameter
      switch(SHORT1FROMMP(mp1))
      {
       case IDM_F_SAVE_C1:
            i = IDM_F_SAVE; break;

       case IDM_F_EXIT_C1:
            goto M_Exit;

       case IDM_E_CUT:
            sprintf(msgbfr, "Lo Trk: %d; Hi Trk: %d; 1st Bar: %d; Last bar: %d",
                      MarkRcl.yTop+1, MarkRcl.yBottom+1, MarkRcl.xLeft+1,
                      MarkRcl.xRight+1);
            umsg('I', hWnd, msgbfr);
            break;

       case IDM_E_COPY:
       case IDM_E_PASTE:
       case IDM_E_DELETE:
            break;

       case IDM_E_TRKED:
            if (!chkact() && RgnPresent)  // Idle and something marked
            {
              TrkSave = sp.playtracks;    // Save across Locate_SPP
              for (i = MarkRcl.yTop; i <= MarkRcl.yBottom; ++i)
                sp.playtracks = actv_trks |= (1 << i); // For Locate_SPP 10/11/96
              Locate_SPP((MarkRcl.xLeft * sp.tsign) * 4); // Find index of starting measure
              sp.playtracks = TrkSave;    // Restore
              for (i = MarkRcl.yTop; i <= MarkRcl.yBottom; ++i)
                Trk_Edit(i, MarkRcl.xLeft, MarkRcl.xRight+1, hWnd);
            }
            break;

       case IDM_E_GRID:
            ini.iniflag1 ^= EDITGRID;
            set_attr(hWnd, IDM_E_GRID, MIA_CHECKED, ini.iniflag1 & EDITGRID);
            chngdini = ON;                   // Indicate init file changed
            WinInvalidateRect(hWndSongEditor, &Edrcl, FALSE);
            break;

       case IDM_E_CLEAR:
            sHscrlPos = sVscrlPos = 0;       // Reset Scroll bars
            ptlOffset.x = ptlOffset.y = 0;   // Reset region offsets
            NotFirstTime = MarkSwt = OFF;    // Reset these switches

            WinSendMsg(hWndHscrl, SBM_SETSCROLLBAR, MPFROM2SHORT(sHscrlPos,0),
                         MPFROM2SHORT(0, sHscrlMax));
            WinSendMsg(hWndVscrl, SBM_SETSCROLLBAR, MPFROM2SHORT(sVscrlPos,0),
                         MPFROM2SHORT(0, sVscrlMax));
            WinSetRect(hAB, &MarkRgn[0],  0,0,0,0); // Nullify mark rectangle
            GpiSetRegion(ed_hps, hRgnMark, 1L, MarkRgn); // Define null mark region

            WinInvalidateRect(hWndSongEditor, &Edrcl, FALSE); // Force re-paint

            sprintf(msgbfr, "MidiLab/2 Song Editor - %s", filename);
            WinSetWindowText(hWndEDITFrame, msgbfr); // Wrt song name as window title

            MarkRcl.xLeft = MarkRcl.xRight = 0;      // Init. boundaries
            MarkRcl.yTop = MarkRcl.yBottom = 0;      // Init. boundaries

            WinSetFocus(HWND_DESKTOP, hWnd);
            break;

       case IDM_E_PLAY:                              // Start playback
            if (RgnPresent)                          // We have a marked area
            {
              trksolo = 0;                           // Clear solo map
              for (i = MarkRcl.yTop; i <= MarkRcl.yBottom; ++i)
                trksolo |= 1 << i;                   // Solo selected trks
              skip_bar_ctr = MarkRcl.xLeft+1;        // Set starting bar
              ending_measure = MarkRcl.xRight+2;     // Set ending bar
              start_bar_swt = ON;                    // Signal SUBS0
            }
            i = MN_B7; break;                        // Post Play rtn in CWIN0

       case IDM_E_PAUSE:
            i = MN_B5; break;

       case IDM_E_METRO:
            i = IDM_METRO; break;

       case IDM_E_RESET:
            if (!actvfunc)
              i = MN_B0;
            break;

       case IDM_H_HELPFORHELP:
            if(hWndMLABHelp)
              WinSendMsg(hWndMLABHelp, HM_DISPLAY_HELP, 0L, 0L);
            break;

       default: break;
      }
      if (i)                                 // Notify Client proc
        WinSendMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(i,0), 0L);
      break; /* End of WM_COMMAND */

    case HM_QUERY_KEYS_HELP:
           /* If the user requests Keys Help from the help pulldown,       */
           /* IPF sends the HM_QUERY_KEYS_HELP message to the application, */
           /* which should return the panel id of the Keys Help panel      */
      return((MRESULT)999);

/*********************************************************************/
    case WM_SIZE:
/*********************************************************************/
      cxEdit = SHORT1FROMMP(mp2);            // Get width of window
      cyEdit = SHORT2FROMMP(mp2);            // Get height of window
      WinQueryWindowRect(hWnd, &Edrcl);      // Get size of client area

/* WM_SIZE- Set up global positioning variables (keep in this order) */
      x1stMeas = cxChar * 18;                // Set pos. of 1st meas sym
      xBarHome = x1stMeas - cxChar;          // Set moving bar home position
      yRow     = cyChar + cyDesc;            // One vertical row
      yHdrText = cyEdit - cyChar;            // Initialize header text pos.
      yHdrLine = cyEdit - yRow;              // Initialize header line pos.
      yBotLine = yHdrLine - (16 * yRow);     // Initialize bottom line pos.
      yTrkRow  = yHdrLine - cyChar;          // Initialize track 1 row
      yTopBar  = yHdrLine - 1;               // Top of moving bar - 1 pel
      yBotBar  = yBotLine + 1;               // Bottom of bar
      one_bar  = cxChar * 2;                 // One measure position
      every_4_bars = one_bar * 4;            // For measure column numbers

/* WM_SIZE- Set up scrolling mechanisms                              */
      HscrlRcl = VscrlRcl = Edrcl;           // Initialize scroll rcl's
      VscrlRcl.yTop = yHdrLine - cyDesc;     // Define vertical scroll area
      HscrlRcl.xLeft = xBarHome;             // Define horizontal scroll area

/* WM_SIZE- This rectangle defines the scrollable measure area       */
      ClipRgn[0].yTop = yHdrLine - cyDesc;
      ClipRgn[0].yBottom = 0;
      ClipRgn[0].xLeft = xBarHome;
      ClipRgn[0].xRight = cxEdit;
      hRgnClip = GpiCreateRegion(ed_hps, 1L, ClipRgn); // Establish clip region

/* WM_SIZE- This rectangle excludes the upper left-hand corner box   */
      ClipRgn[1].yTop = cyEdit;
      ClipRgn[1].yBottom = yHdrLine - cyDesc;
      ClipRgn[1].xLeft = 0;
      ClipRgn[1].xRight = xBarHome - 2;     // To preserve boundary line

/* WM_SIZE- Re-establish mark region if one exists                   */
      if (RgnPresent)                       // We have a marked area
      {
        GpiDestroyRegion(ed_hps, hRgnMark); // Get rid of old region
        sHscrlPos = sVscrlPos = 0;          // Reset Scroll bars
        Define_Rgn();                       // Define new one based on size change
        hRgnMark = GpiCreateRegion(ed_hps, 1L, MarkRgn);
      }

/* WM_SIZE- Set vertical scroll bar and parameters                   */
      sVscrlMax = max(0, yRow * 17 - cyEdit);
      sVscrlPos = min(sVscrlPos, sVscrlMax);
      WinSendMsg(hWndVscrl, SBM_SETSCROLLBAR, MPFROM2SHORT(sVscrlPos,0),
                    MPFROM2SHORT(0, sVscrlMax));
      WinEnableWindow(hWndVscrl, sVscrlMax ? TRUE:FALSE);

      break;       /* End of WM_SIZE                            */

/*********************************************************************/
    case WM_PAINT:
/*********************************************************************/
      WinReleasePS(ed_hps);             // Release general purpose PS
      wm_paint(hWnd);                   // Go to paint routine in VIEW1
      break; /* End of WM_PAINT */

/*********************************************************************/
      case UM_MOVE_BAR: // Position moving bar; erase prev. position
/*********************************************************************/
      Set_Bar(ed_hps);                  // Go position measure bar
      if (BarPtl.x > cxEdit && actvfunc)// Do auto scroll if playing
        WinSendMsg(hWnd, WM_HSCROLL, 0, MPFROM2SHORT(0, SB_PAGERIGHT));
      break;

/*********************************************************************/
    case WM_BUTTON1DOWN:                // Start marking area
/*********************************************************************/
      if (actvfunc) break;              // No can do if something going
      for (i = IDM_E_CUT; i < IDM_E_TRKED; ++i)
        set_attr(hWnd, i, MIA_DISABLED, ON);  // Disable edit pull-downs
      if (NotFirstTime)                 // We've been thru here before,
      {                                 // we need to erase previous mark
        GpiQueryRegionBox(ed_hps, hRgnMark, &MarkRgn[0]);
        if (MarkRgn[0].xLeft < xBarHome)// We scrolled mark out of left bounds
          MarkRgn[0].xLeft = xBarHome;  // Set it to boundary
        if (MarkRgn[0].yTop  > yHdrLine)// We scrolled mark over top boundary
          MarkRgn[0].yTop = yHdrLine - cyDesc; // Set it to boundary

       /* If some part of mark is showing, redefine and erase it  */
        if (MarkRgn[0].xRight > xBarHome && MarkRgn[0].yBottom < yHdrLine)
        {
          GpiSetRegion(ed_hps, hRgnMark, 1L, MarkRgn); // Re-define mark region
          GpiSetMix(ed_hps, FM_INVERT);
          GpiPaintRegion(ed_hps, hRgnMark);   // Erase previous marked region
        }
        GpiDestroyRegion(ed_hps, hRgnMark);
        GpiSetMix(ed_hps, FM_DEFAULT);
      }
      else NotFirstTime = ON;           // Do this only once

      ghArea = Loc_Area(&message);      // Go find track and measure
      if (ghArea.rc == 0)               // Valid hit
      {
        ghMem = ghArea;        // ghMem avoids needless cycles during mousemove
        gh1stMark = ghArea;             // Remember home position
        MarkRcl.xLeft = MarkRcl.xRight = gh1stMark.Meas;  // Initialize Mark RCL
        MarkRcl.yTop = MarkRcl.yBottom = gh1stMark.Trk;   // Initialize Mark RCL
        MarkSwt = LEVEL1;               // Indicate this is the home position
      }
      else                              // Fall thru to MOUSEMOVE if ok
      {
        WinSetFocus(HWND_DESKTOP, hWnd);
        break;
      }

/*********************************************************************/
    case WM_MOUSEMOVE:                  // Mark area for edit
/*********************************************************************/
      if (MarkSwt)                      // While button 1 is down
      {
        ghArea = Loc_Area(&message);    // Go find track and measure
        if (ghArea.rc > 0)
          MarkSwt = OFF;                // Reset mark if out of bounds

        else if (MarkSwt == LEVEL1   || // We just pressed button #1
           (ghArea.rc == 0           && // We're on a valid marker and
           (ghArea.Meas != ghMem.Meas|| // in a different cell from the
            ghArea.Trk != ghMem.Trk)))  // previous mousemove
        {
          ghMem = ghArea;                        // Save current position

          if (ghArea.Meas < gh1stMark.Meas)      // We're to the left of home
            MarkRcl.xLeft = ghArea.Meas;         // Establish left extent
          else if (ghArea.Meas > gh1stMark.Meas) // We're to the right of home
            MarkRcl.xRight = ghArea.Meas;        // Establish right extent
          else MarkRcl.xLeft = MarkRcl.xRight = ghArea.Meas; // We're back at home, reset

          if (ghArea.Trk < gh1stMark.Trk)        // We're above home position
            MarkRcl.yTop = ghArea.Trk;           // Establish top extent
          else if (ghArea.Trk > gh1stMark.Trk)   // We're below home
            MarkRcl.yBottom = ghArea.Trk;        // Establish bottom extent
          else MarkRcl.yTop = MarkRcl.yBottom = ghArea.Trk; // We're back at home, reset
          Define_Rgn();                          // Set region boundaries

          GpiSetMix(ed_hps, FM_INVERT);
          if (MarkSwt == LEVEL2)                 // Mark present
          {
            hRgnMark = GpiCreateRegion(ed_hps, 1L, MarkRgnPrev);
            rc = GpiPaintRegion(ed_hps, hRgnMark); // Erase previous marked area
            if (rc == FALSE)
            {
              sprintf(msgbfr, "GpiPaintRegion3: %lX", WinGetLastError(hAB));
              umsg('E', hWnd, msgbfr);
            }
            GpiDestroyRegion(ed_hps, hRgnMark);
          }
          hRgnMark = GpiCreateRegion(ed_hps, 1L, MarkRgn); // Establish Mark region
          rc = GpiPaintRegion(ed_hps, hRgnMark); // Paint newly marked area
          if (rc == FALSE)
          {
            sprintf(msgbfr, "GpiPaintRegion1: %lX", WinGetLastError(hAB));
            umsg('E', hWnd, msgbfr);
          }
          MarkRgnPrev[0] = MarkRgn[0];           // Save current region
          MarkSwt = LEVEL2;                      // We're no longer at home pos.
          GpiSetMix(ed_hps, FM_DEFAULT);

          if (MarkRgn[0].xLeft   < xBarHome ||
              MarkRgn[0].xRight  >= cxEdit  ||
              MarkRgn[0].yTop    > yHdrLine ||
              MarkRgn[0].yBottom < yBotLine)
              MarkSwt = OFF;                     // Quit marking if out of bounds

//        if (MarkRgn[0].xRight >= cxEdit)       // Auto scroll if at right edge
//          WinSendMsg(hWnd, WM_HSCROLL, 0, MPFROM2SHORT(0, SB_LINERIGHT));
//        if (MarkRgn[0].yTop >= yTrkRow)        // Auto scroll if at top
//          WinSendMsg(hWnd, WM_VSCROLL, 0, MPFROM2SHORT(0, SB_LINEUP));
        }
        WinSetFocus(HWND_DESKTOP, hWnd);
      }
      break;

/*********************************************************************/
    case WM_BUTTON1UP:         // Stop marking area
/*********************************************************************/
      if (MarkSwt)             // We've marked something
      {
        MarkSwt = OFF;
        for (i = IDM_E_CUT; i < IDM_E_TRKED+1; ++i)
          set_attr(hWnd, i, MIA_DISABLED, OFF);  // Enable edit pull-downs
      }
      else
      {
        sysflag1 &= 255-POSACTV;              // Positioning now invalid
        MarkRcl.xLeft = MarkRcl.xRight = 0;   // Init. boundaries
        MarkRcl.yTop = MarkRcl.yBottom = 0;   // Init. boundaries
        trksolo = OFF;                        // Reset solo map
      }
      break;

/*********************************************************************/
    case WM_BUTTON2CLICK:      // Invoke Track Editor
/*********************************************************************/
      WinSendMsg(hWnd, WM_COMMAND, MPFROM2SHORT(IDM_E_TRKED,0), 0L);
      break;

/*********************************************************************/
    case WM_CHAR:              // Allow keys to be used for scrolling
/*********************************************************************/
      switch (CHARMSG(&message)->vkey)
      {
        case VK_LEFT:
        case VK_RIGHT:
          WinSendMsg(hWndHscrl, message, mp1, mp2);
          break;

        case VK_UP:
        case VK_DOWN:
        case VK_PAGEUP:
        case VK_PAGEDOWN:
          WinSendMsg(hWndVscrl, message, mp1, mp2);
          break;
      }

      i = CHARMSG(&message)->fs;
      if (i & KC_KEYUP && !(i & KC_PREVDOWN))
        WinSendMsg(hWndClient, WM_BUTTON1UP, 0L, 0L);

      switch (CHARMSG(&message)->chr)
      {
        case '+':
          if (!(i & KC_KEYUP) && !(i & KC_PREVDOWN))
            WinSendMsg(hWndClient, WM_CONTROL, MPFROM2SHORT(MN_B6,0), 0L);
          break;
      }
      break;

/*********************************************************************/
    case WM_HSCROLL:
/*********************************************************************/
      i = ((cxEdit - x1stMeas) / one_bar) * one_bar; // Set up page scroll
      switch (SHORT2FROMMP(mp2))
      {
        case SB_LINELEFT:
          sHscrlInc = -one_bar; break;

        case SB_LINERIGHT:
          sHscrlInc =  one_bar; break;

        case SB_PAGELEFT:
          sHscrlInc = -i; break;

        case SB_PAGERIGHT:
          sHscrlInc =  i; break;

        case SB_SLIDERTRACK:
          sHscrlInc = SHORT1FROMMP(mp2) - sHscrlPos;
          break;

        default:
          return(0L);
      }
      sHscrlInc = max(-sHscrlPos, min(sHscrlInc, sHscrlMax - sHscrlPos));
      if (sHscrlInc != 0)
      {
        sHscrlPos += sHscrlInc;
        WinScrollWindow(hWnd, -sHscrlInc, 0, &HscrlRcl, &HscrlRcl, 0, NULL,
                          SW_INVALIDATERGN);
        WinSendMsg(hWndHscrl, SBM_SETPOS, MPFROMSHORT(sHscrlPos), NULL);

        if (RgnPresent)                 // We have a marked area
        {
          ptlOffset.x = -sHscrlInc;
          ptlOffset.y = 0;
          rc = GpiOffsetRegion(ed_hps, hRgnMark, &ptlOffset);
          if (rc == FALSE) goto offerr;
        }
      }
      break;

/*********************************************************************/
    case WM_VSCROLL:
/*********************************************************************/
      switch (SHORT2FROMMP(mp2))
      {
        case SB_LINEUP:
          sVscrlInc = -yRow; break;

        case SB_LINEDOWN:
          sVscrlInc = yRow; break;

        case SB_PAGEUP:
          sVscrlInc = -yRow * 16; break;

        case SB_PAGEDOWN:
          sVscrlInc = yRow * 16; break;

        case SB_SLIDERTRACK:
          sVscrlInc = SHORT1FROMMP(mp2) - sVscrlPos; break;

        default:
          return(0L);
      }
      sVscrlInc = max(-sVscrlPos, min(sVscrlInc, sVscrlMax - sVscrlPos));
      if (sVscrlInc != 0)
      {
        sVscrlPos += sVscrlInc;
        WinScrollWindow(hWnd, 0, sVscrlInc, &VscrlRcl, &VscrlRcl, 0, NULL,
                          SW_INVALIDATERGN);
        WinSendMsg(hWndVscrl, SBM_SETPOS, MPFROMSHORT(sVscrlPos), NULL);

        if (RgnPresent)                 // We have a marked area
        {
          ptlOffset.x = 0;
          ptlOffset.y = sVscrlInc;
          rc = GpiOffsetRegion(ed_hps, hRgnMark, &ptlOffset);
          if (rc == FALSE)
          {
           offerr:
            sprintf(msgbfr, "GpiOffset err: %lX", WinGetLastError(hAB));
            umsg('E', hWnd, msgbfr);
          }
        }
      }
      break;

/*********************************************************************/
    case WM_MOVE:
/*********************************************************************/
      if (sysflag2 & EDITCMPL)         // Window has been moved after creation
      {
        WinQueryWindowPos(hWndEDITFrame, &ini.EditPos); // Get new position
        chngdini = ON;                 // Indicate init file has changed
      }
      break;

/*********************************************************************/
    case WM_SETFOCUS:                  // We're losing/gaining cntrl */
/*********************************************************************/
      break; // Ignore until I figure out why actv_trks gets zero'd out
      if (SHORT1FROMMP(mp2) == TRUE)   // Gaining focus; save positioning parms
      {
        v_playtracks = sp.playtracks;          // Save Play map
        v_actv_trks = actv_trks;               // Save active tracks
        v_skip_bar_ctr = skip_bar_ctr;         // Save starting measure
        v_ending_measure = ending_measure;     // Save ending measure
        v_trksolo = trksolo;                   // Save solo map
      }
      else                             // Losing focus; restore pos. parms
      {
        sp.playtracks = v_playtracks;          // Restore Play map
        actv_trks = v_actv_trks;               // Restore active tracks
        sysflag1 &= 255-POSACTV;               // Positioning now invalid
        skip_bar_ctr = v_skip_bar_ctr;         // Restore starting measure
        ending_measure = v_ending_measure;     // Restore ending measure
        start_bar_swt = ON;                    // Force refetch of parms
        trksolo = v_trksolo;                   // Restore solo map
        MarkSwt = OFF;                         // No use for this to be on
      }
      break;

/*********************************************************************/
    case WM_CLOSE:                     // Close the window and exit
/*********************************************************************/
    M_Exit:
      GpiDestroyRegion(ed_hps, hRgnClip);
      if (RgnPresent)                  // We have a marked area
      {
        GpiDestroyRegion(ed_hps, hRgnMark);
        RgnPresent = OFF;
      }
      WinSendMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(IDM_E_CLOSE,0), 0L);
      break;

    default:
      WinReleasePS(ed_hps);            // Release general pupose PS
      return(WinDefWindowProc(hWnd, message, mp1, mp2));
  }                         /* End of switch(message)    */
  WinReleasePS(ed_hps);                // Release general pupose PS
  return(0L);
}                           /* End of WndProc            */
