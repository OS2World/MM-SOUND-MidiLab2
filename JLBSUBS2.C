/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for the IBM Personal Computer  */
/*                                                                   */
/*   JLBSUBS2 -      ** GENERAL SUBROUTINES   Volume III **          */
/*                                                                   */
/*   - umsg                     - chk_range                          */
/*   - set_attr                 - rev_int                            */
/*   - cntrl_type               - DLLReset                           */
/*   - cwCenter                 - cwCreateWindow                     */
/*   - PtrBusy                  - TrkBld                             */
/*                                                                   */
/*       Copyright  J. L. Bell         July 1992                     */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
extern SHORT cyView;
extern HWND hWndFrame;

/*********************************************************************/
/*            Issue Informational and Error Messages                 */
/*********************************************************************/
USHORT umsg(char lvl, HWND hWndMsg, PSZ text)
{
  USHORT usResponse;

  if (lvl == 'I')
  {
    usResponse = WinMessageBox(HWND_DESKTOP, hWndMsg, text,
                  "MidiLab/2", 0, MB_OK | MB_INFORMATION);
  }
  else if (lvl == 'W')                        /* Warning message     */
  {
    usResponse = WinMessageBox(HWND_DESKTOP, hWndMsg, text,
                  "MidiLab/2", 0, MB_OK | MB_WARNING);
  }
  else if (lvl == 'C')                        /* Confirmation msg    */
  {
    DosBeep(1100,80);
    usResponse = WinMessageBox(HWND_DESKTOP, hWndMsg, text,
                  "MidiLab/2", 0, MB_YESNOCANCEL | MB_ICONQUESTION);
  }
  else if (lvl == 'E')                        /* Error message       */
  {
    usResponse = WinMessageBox(HWND_DESKTOP, hWndMsg, text,
                  "MidiLab/2", 0, MB_OK | MB_ERROR);
  }
  return(usResponse);
}

/*********************************************************************/
/* chk_range routine - insures an integer lies in a specified range  */
/*********************************************************************/
int chk_range(LONG inputint, char *text, LONG min, LONG max)
{
  if ((inputint < min) || (inputint > max))
  {
    sprintf(msgbfr,"%s must be in the range of %d to %d, please...",
                    text, min, max);
    umsg('E', HWND_DESKTOP, msgbfr);
    return(FALSE);
  }
  else return(TRUE);
}

/*********************************************************************/
/* set_attr routine - Sets/Resets an attribute in a pulldown item    */
/*********************************************************************/
void set_attr(HWND hWnd, SHORT item, USHORT attr, BYTE state)
{
  WinSendMsg(WinWindowFromID(WinQueryWindow(hWnd, QW_PARENT), FID_MENU),
             MM_SETITEMATTR, MPFROM2SHORT(item, TRUE),
             MPFROM2SHORT(attr, state ? attr : 0));
}

/*********************************************************************/
/*      Routine to reverse order of PC integer representation        */
/*      for compatibility with 1.0 integer representation            */
/*********************************************************************/
void rev_int(BYTE *loc, BYTE length)
{
  BYTE i;
  for (i=0; i < length; ++i)
    msgbfr[i] = *(loc + (length-1) - i);  /* Make mirror image of loc*/
  for (i=0; i < length; ++i)
          *(loc + i) = msgbfr[i];         /* and put it back         */
}

/*********************************************************************/
/*         - Resolve controller type for Edit functions -            */
/*********************************************************************/
char *cntrl_type(BYTE ctlnum)
{
char *utilpntr;

        switch (ctlnum)                        /*What kind of control*/
        {
          case 1:  utilpntr = "Mod Wheel";
                   break;
          case 2:  utilpntr = "Breath Cont";
                   break;
          case 4:  utilpntr = "Foot Cont";
                   break;
          case 5:  utilpntr = "Porta time";
                   break;
          case 6:  utilpntr = "Data Entry";
                   break;
          case 7:  utilpntr = "Main Vol";
                   break;
          case 10: utilpntr = "PanPot";
                   break;
          case 64: utilpntr = "Sustain";
                   break;
          case 65: utilpntr = "Portamento";
                   break;
          case 66: utilpntr = "Sostenuto";
                   break;
          case 67: utilpntr = "Soft Pedal";
                   break;
          case 96: utilpntr = "+Data";
                   break;
          case 97: utilpntr = "-Data";
                   break;
         case 123: utilpntr = "All Notes Off";
                   break;
         case 124: utilpntr = "OMNI Off";
                   break;
         case 125: utilpntr = "OMNI On";
                   break;
          default:
                   utilpntr = "Gen'l control";
        }
        return(utilpntr);
}

/*********************************************************************/
/*                     - DLL general reset -                         */
/*********************************************************************/
VOID DLLReset( VOID )
{
  cmd_in_progress = OFF;                   /* Reset MPU command swt  */
  emode = OFF;                             /* Reset Edit mode switch */
  XeqSwt = AutoOff = OFF;                  /* Reset Edit flags       */
  safequit = editchng = OFF;               /* Reset safe quit swts   */
  OverRun  = OFF;                          /* Reset ListBox overrun  */
  SeqPlay  = OFF;                          /* Reset Sequence play swt*/
//ClipPtr  = 0;                            /* Set Clipboard ptr NULL */
}

/***************************************************************************/
/*  cwCenter Function                                                      */
/*                                                                         */
/*  Centers a dialog box on the client area of the caller                  */
/*                                                                         */
/*  hWnd - handle of the window to be centered                             */
/*  hWndParent - handle of the window on which to center                   */
/***************************************************************************/
INT cwCenter(HWND hWnd, HWND hWndParent)
{
  ULONG  SrcX, SrcY;                /* Center of parent                    */
  INT    ix, iy;                    /* Destination points                  */
  SWP    DlgSwp, ParentSwp;         /* Set window position structures      */
  ULONG  ScreenWidth, ScreenHeight; /* Dimensions of the DESKTOP           */
  POINTL pt;                        /* Point                               */

  /* Determine the width and height of the DESKTOP so the dialog box       */
  /* will not be positioned to a point off of the screen.                  */
  ScreenWidth  = (ULONG)WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
  ScreenHeight = (ULONG)WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);

  /* Query width and depth of dialog box                                   */
  WinQueryWindowPos(hWnd, (PSWP)&DlgSwp);

  /* Query width and depth of caller                                       */
  WinQueryWindowPos(hWndParent, (PSWP)&ParentSwp);

  /* Map the point parent points to the Desktop                            */
  pt.x = ParentSwp.x;
  pt.y = ParentSwp.y;

  /* Convert the point from having our window as the origin to having      */
  /* the DESKTOP as the origin                                             */
  WinMapWindowPoints(hWndParent, HWND_DESKTOP, &pt, 1);

  SrcX = pt.x + (ParentSwp.cx / 2);
  SrcY = pt.y + (ParentSwp.cy / 2);

  /* Determine the point to move the dialog box to                         */
  ix = (INT)(SrcX - (DlgSwp.cx / 2));
  iy = (INT)(SrcY - (DlgSwp.cy / 2));

  /* if either point is less than zero, then set that point to zero        */
  /* so the dialog box will not be positoned off of the window             */
  ix = (ix < 0) ? 0 : ix;
  iy = (iy < 0) ? 0 : iy;

  /* if either point plus the height or width of the dialog box is         */
  /* greater than the height or width of the screen adjust point           */
  if(((ULONG)ix + DlgSwp.cx) > ScreenWidth)
    ix = (INT)(ScreenWidth - DlgSwp.cx);
  if(((ULONG)iy + DlgSwp.cy) > ScreenHeight)
    iy = (INT)(ScreenHeight - DlgSwp.cy);

  /* move the dialog box                                                   */
  return(WinSetWindowPos(hWnd, HWND_TOP, ix, iy, 0, 0, SWP_MOVE));
}

/***************************************************************************/
/* cwCreateWindow Function                                                 */
/*                                                                         */
/* The following function is used to create a window (the main window,     */
/* a child window, an icon window, etc.) and set it's initial size and     */
/* position. It returns the handle to the frame window.                    */
/***************************************************************************/
HWND cwCreateWindow(
   HWND   hWndParent,  /* Handle to the parent of the window to be created */
   ULONG  ctldata,     /* Frame control flags for the window               */
   PCH    appname,     /* Class name of the window                         */
   PCH    title,       /* Title of the window                              */
   USHORT ResID,       /* Resource id value                                */
   INT    x,           /* Initial horizontal and vertical location         */
   INT    y,
   INT    cx,          /* Initial width and height of the window           */
   INT    cy,
   PHWND  p_hWndClient,/* Ptr to handle to the client area of the window   */
   ULONG  lfStyle,     /* Frame window style                               */
   USHORT uSizeStyle)  /* User defined size and location flags             */
{
   APIRET rc;
   USHORT SizeStyle;     /* local window positioning options               */
   CHAR   MsgBuffer[80]; /* buffer for error messages                      */
   HPS    hPS;           /* handle to a presentation space                 */
   HWND   hWnd;

   /* Create the frame window                                              */
   hWnd = WinCreateStdWindow(hWndParent,       /* parent of window         */
                        lfStyle,               /* frame window style       */
                        &ctldata,              /* frame flags              */
                        appname,               /* class name               */
                        title,                 /* window title             */
                        0L,                    /* client window style      */
                        0,                     /* module for resources     */
                        ResID,                 /* resource id              */
                        (HWND FAR *)p_hWndClient); /* client handle        */

   /* if hWnd is NULL, an error occured when opening the window,      */
   if(hWnd == 0)
   {
     WinLoadString(hAB, 0, IDS_ERR_WINDOW_CREATE, 80, MsgBuffer);
     WinMessageBox(HWND_DESKTOP, hWndParent, MsgBuffer,
                   0, 0, MB_OK|MB_ICONEXCLAMATION);
     return((HWND)0);
   }

   if (ini.fontDlg.fAttrs.usRecordLength) // ini file present and set
     hPS = WinGetPS(hWnd);                // Get handle to window
   else hPS = WinGetPS(HWND_DESKTOP);

   GpiQueryFontMetrics(hPS, (LONG)sizeof(FONTMETRICS), &ini.fm);
   cxChar = (INT)ini.fm.lAveCharWidth;
   cyChar = (INT)ini.fm.lMaxBaselineExt;
   cxCaps = (INT)ini.fm.lEmInc;
   cyDesc = (INT)ini.fm.lMaxDescender;
   WinReleasePS(hPS);

   /* Set up size options                                                  */
   SizeStyle = SWP_ACTIVATE | SWP_ZORDER | uSizeStyle;
   SizeStyle |= SWP_SIZE + SWP_MOVE;

   /* Set the size and position of the window and activate it              */
  if (p_hWndClient == &hWndClient)  // We're creating Main window
  {
    if (!ini.MainPos.cx)            // First time, set SWP
    {
      rc = WinSetWindowPos(hWnd, HWND_TOP, x * cxChar, y * cyChar,
                             cx * cxChar, cy * cyChar, SizeStyle);

      if (!rc)                      // WinSetPosition failed
      {
        WinLoadString(hAB, 0, IDS_ERR_WINDOW_POS, 80, MsgBuffer);
        WinMessageBox(HWND_DESKTOP, hWndParent, MsgBuffer, 0, 0, MB_OK|MB_ICONEXCLAMATION);
        return((HWND)0);
      }
      else WinQueryWindowPos(hWnd, &ini.MainPos); // Set first-time position
    }
    else WinSetWindowPos(hWnd, HWND_TOP, ini.MainPos.x, ini.MainPos.y,
                           ini.MainPos.cx, ini.MainPos.cy, SizeStyle);
  }
  return(hWnd);                     // Return the handle to the frame window

}  /* End of cwCreateWindow */

/*********************************************************************/
/*       Routine to build track data from an event packet            */
/* Passed an input event from which to create track data.            */
/*********************************************************************/
LONG adjustment;

LONG TrkBld(PVOID P_InptEv, SHORT ptrack)
{
  EVPK *InptEv;
  USHORT i, j;
  adjustment = 0;                         // Reset length adjustment
  InptEv = P_InptEv;                      // Set up event pointers

  if (PTI > ini.trksize[ptrack] - 4)      // Make sure it's gonna fit
  {
    umsg('E', HWND_DESKTOP, "Track overrun");
    return(-1);
  }
  if (!(InptEv->flags1 & (EVASPACE+EVAHEADR))) // Not a dummy/header slot
  {

/*********************************************************************/
/*       Convert delta time into one or more timing overflows        */
/*********************************************************************/
    if (sysflag2 & DISKLOAD)                // As opposed to Edit
      Set_Deltime(InptEv->deltime, ptrack); // Go to insert meas.ends
    else
    {
       /* If stand-alone timing overflow, not much to do */
      if ((InptEv->flags1 & MPUMARK) && (InptEv->deltime == 240))
      {
        *(PT+PTI++) = TIMING_OVFLO;         // Set timing overflow
        return(0);
      }
      j = InptEv->deltime / 240;            // Find # of tmg ovflo's
      for (i = 0; i < j; ++i)
        *(PT+PTI++) = TIMING_OVFLO;         // Set timing overflow(s)
      *(PT+PTI++) = InptEv->deltime % 240;  // Set remainder
      adjustment += j;                      // Reflect difference in length
    }

    if (InptEv->flags1 & MPUMARK)           // MPU mark
      *(PT+PTI++) = InptEv->dbyte1;         // Set mark

    else if ((InptEv->flags1 & MLABMETA) && !(InptEv->flags1 & METAEVT))
      goto New_style;
    else if (InptEv->flags1 & METAEVT)      // Some kind of Meta event
    {
     New_style:
      if (InptEv->flags1 & MLABMETA)        // New-style trk order
      {
        memcpy(PT+PTI, trkcmd_meta, sizeof(trkcmd_meta));
        PTI += sizeof(trkcmd_meta);
        *(PT+PTI++) = TRKCMDHDR;            // Set Header
        *(PT+PTI++) = InptEv->evtype;       // Set order type
        if (InptEv->evtype == TRKCMDX)
        {
          *(PT+PTI++) = InptEv->in_chan;    // Set flag byte
          *(PT+PTI++) = InptEv->dbyte1;     // Set cmd byte
          *(PT+PTI++) = InptEv->dbyte2;     // Set data byte
        }
        else
        {
          memcpy(PT+PTI, &InptEv->trkcmddata, 2);
          PTI += 2;
          *(PT+PTI++) = InptEv->dbyte1;     // Set data byte
        }
        if (InptEv->evtype == TRKINIT)
        *(PT+PTI++) = InptEv->dbyte2;       // If Trk Init, set trks 9-16 (7/14/93)
        else *(PT+PTI++) = EOX;             // Set normal trailer
      }
      else                                  // 1.0 Meta event
      {
        *(PT+PTI++) = 255;                  // Set Meta event code
        *(PT+PTI++) = InptEv->evcode;       // Set type code
        *(PT+PTI++) = InptEv->MetaLn;       // Set meta length
        memcpy(PT+PTI, InptEv->Meta_ptr, InptEv->MetaLn);
        PTI += InptEv->MetaLn;              // Jump over meta data
      }
    }
    else                                    // Real MIDI event
    {
      if (!(InptEv->flags1 & RUNSTAT))      // Need channel command?
        *(PT+PTI++) = InptEv->evtype;       // Yes, set it
      *(PT+PTI++) = InptEv->dbyte1;         // Set first/only data byte
      if (InptEv->flags1 & DBYTES2)         // If 2-byte event,
        *(PT+PTI++) = InptEv->dbyte2;       // Set second data byte
    }
  }
  return(adjustment);
}

/*********************************************************************/
/*       Insert Measure_End marks during load of MIDI 1.0 file       */
/* (I spent more time here than I should have during September '93)  */
/*********************************************************************/
VOID Set_Deltime(LONG deltime, SHORT ptrack)
{
  extern SHORT clocks_per_bar;
  SHORT measure_end_tmg, ovfls_per_bar, time_to_go, i;

  timing_total[ptrack] += deltime;           // Accumulate delta time

  /* Find out if this delta time can be contained in the current measure */
  if (timing_total[ptrack] < clocks_per_bar) // Yes, it can
  {
    while (deltime > 239)
    {
      *(PT+PTI++) = TIMING_OVFLO;           // Set measure-end
      deltime -= 240;                       // Decr time accordingly
      ++adjustment;                         // Indicate increase in trk lnth
    }
    *(PT+PTI++) = deltime;                  // Set timing byte for current event
    return;                                 // All done
  }

  /* We went over; First, we must fill out the time remaining in current measure */
  time_to_go = clocks_per_bar - (timing_total[ptrack] - deltime);

  while (time_to_go > 239)
  {
    *(PT+PTI++) = TIMING_OVFLO;             // Set measure-end
    time_to_go -= 240;                      // Decr time accordingly
    ++adjustment;                           // Indicate increase in trk lnth
  }
  *(PT+PTI++) = time_to_go;                 // Set timing byte remaining time
  *(PT+PTI++) = MEASURE_END;                // Set measure-end
  adjustment += 2;                          // Indicate increase in trk lnth

  /* Now we'll take care of the overage */
  deltime = timing_total[ptrack] - clocks_per_bar; // Get overage
  ovfls_per_bar = clocks_per_bar / 240;     // No. of overflows per bar
  measure_end_tmg = clocks_per_bar % 240;   // Leftover timing for meas. end

  while (deltime >= clocks_per_bar)         // Insert null measures
  {
    for (i = 0; i < ovfls_per_bar; ++i)
    {
      *(PT+PTI++) = TIMING_OVFLO;           // Insert an overflow
      deltime -= 240;                       // Decr time accordingly
      ++adjustment;                         // Indicate increase in trk lnth
    }
    *(PT+PTI++) = measure_end_tmg;
    *(PT+PTI++) = MEASURE_END;              // Set measure-end
    deltime  -= measure_end_tmg;            // Reduce time accordingly
    adjustment += 2;                        // Indicate increase in trk lnth
  }

  timing_total[ptrack] = deltime;           // Seed timing ctr for next event
  while (deltime > 239)
  {
    *(PT+PTI++) = TIMING_OVFLO;             // Set measure-end
    deltime -= 240;                         // Decr time accordingly
    ++adjustment;                           // Indicate increase in trk lnth
  }
  *(PT+PTI++) = deltime;                    // Set timing byte for next event
  return;
}

/*********************************************************************/
/*   Routine to set the mouse pointer to show busy/free status       */
/*   mode = ON : busy; mode = OFF : free                             */
/*********************************************************************/
VOID PtrBusy(BYTE mode)
{
  WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP,
                  mode == ON ? SPTR_WAIT : SPTR_ARROW, FALSE));
}
