/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBEDIT3 -    ** MidiLab Track Editor INSERT procedures         */
/*                                                                   */
/*       Copyright  J. L. Bell        September 1992                 */
/*********************************************************************/
#define INCL_WINHOOKS
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"
extern HWND hWndMLABHelp;   /* Handle to Help window                 */
extern USHORT InitTrks;

SHORT ptrack;
/**********************************************************************/
/*         Insert/Append Edit message processing routine - Level1     */
/**********************************************************************/
MRESULT EXPENTRY E1MsgProc(HWND hWndDlg,
                              USHORT message, MPARAM mp1, MPARAM mp2)
{
  SHORT cmd, cntl, DlgId;
  PCHAR trkptr;

  switch(message)
  {
    case WM_INITDLG:
      trkptr = PVOIDFROMMP(mp2);
      ptrack = *trkptr;
      break;

    case WM_COMMAND:
      cmd = SHORT1FROMMP(mp1);
      switch(cmd)
      {
        case 114:                       // Help(!)
         break;

        case 115:                       // Cancel
//       WinDismissDlg(hWndDlg, TRUE);  // Get rid of 'Type' dlg box
         break;
      }
      break;

    case WM_CONTROL:
      WinDismissDlg(hWndDlg, TRUE);  // Get rid of 'Type' dlg box
      cntl = SHORT1FROMMP(mp1);
      switch(cntl)
      {
        case 118:                   // Pgm Change
         DlgId = IDLG_E1PGM;
         WinDlgBox(HWND_DESKTOP,hWndDlg,(PFNWP)E3MsgProc,0,IDLG_E1PGM, &DlgId);
         break;

        case 119:                   // Meta Text
         DlgId = IDLG_E1TEXT;
         WinDlgBox(HWND_DESKTOP,hWndDlg,(PFNWP)E3MsgProc,0,IDLG_E1TEXT, &DlgId);
         break;

        case 120:                   // Generic Event
         DlgId = IDLG_E1GENERIC;
         WinDlgBox(HWND_DESKTOP,hWndDlg,(PFNWP)E3MsgProc,0,IDLG_E1GENERIC, &DlgId);
         break;

        case 121:                   // Watch this space
         break;

        case 122:                   // Transfer
        case 123:                   // Rest
         DlgId = (cntl == 122) ? IDLG_E1XFER : IDLG_E1REST;
         WinDlgBox(HWND_DESKTOP, hWndDlg, (PFNWP)E3MsgProc, 0, DlgId, &DlgId);
         break;

        case 124:                   // Initiate
         DlgId = IDLG_E1INIT;
         WinDlgBox(HWND_DESKTOP,hWndDlg,(PFNWP)E3MsgProc,0,IDLG_E1INIT, &DlgId);
         break;

        case 125:                   // Command
         DlgId = IDLG_E1CMD;
         WinDlgBox(HWND_DESKTOP,hWndDlg,(PFNWP)E3MsgProc,0,IDLG_E1CMD, &DlgId);
         break;

        default:
         break;
      }
  }
  return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
}

/**********************************************************************/
/*         Insert/Append Edit message processing routine - Level2     */
/* The following controls are for the selected type of Insert/Append  */
/**********************************************************************/
MRESULT EXPENTRY E3MsgProc(HWND hWndDlg, USHORT message, MPARAM mp1, MPARAM mp2)
{
 APIRET rc;
  static LONG Spin_1, Spin_2, Spin_3, Spin_4, ulUtility;
  static BYTE Flags1, TCtype, TxtCode;
  static char szText[250];
  SHORT cmd, cntl, usUtility;
  PSHORT pDlgId;
  USHORT i, j;
  BYTE t18, t916;
  EVPK *EVptr, *EVptr2;

  switch(message)
  {
/**********************************************************************/
/*     Initialize controls according to type of Insert functions      */
/**********************************************************************/
   case WM_INITDLG:
    pDlgId = PVOIDFROMMP(mp2);     // Get pointer to Id of selected dialog box
    switch (*pDlgId)
    {
     case IDLG_E1XFER:
     case IDLG_E1REST:
      WinSendDlgItemMsg(hWndDlg, 153, SPBM_SETLIMITS,  // Repeat/Rest Spin Btn
                         MPFROMLONG(254), MPFROMLONG(1));
     break;

     case IDLG_E1PGM:
      WinSendDlgItemMsg(hWndDlg, 153, SPBM_SETLIMITS,  // Program number
                         MPFROMLONG(127), MPFROMLONG(0));
      WinSendDlgItemMsg(hWndDlg, 154, SPBM_SETLIMITS,  // Channel number
                         MPFROMLONG(16), MPFROMLONG(1));
      break;

     case IDLG_E1GENERIC:
      WinSendDlgItemMsg(hWndDlg, 153, SPBM_SETLIMITS,  // Timing
                         MPFROMLONG(240), MPFROMLONG(0));
      WinSendDlgItemMsg(hWndDlg, 154, SPBM_SETLIMITS,  // Status
                         MPFROMLONG(239), MPFROMLONG(128));
      WinSendDlgItemMsg(hWndDlg, 155, SPBM_SETLIMITS,  // Data 1
                         MPFROMLONG(127), MPFROMLONG(0));
      WinSendDlgItemMsg(hWndDlg, 156, SPBM_SETLIMITS,  // Data 2
                         MPFROMLONG(127), MPFROMLONG(0));
      Flags1 = DBYTES2;                                // Default to 2 bytes
      WinSendDlgItemMsg(hWndDlg, 125, BM_SETCHECK, MPFROMSHORT(ON) , 0L); //Set 2 dbytes
      break;

     case IDLG_E1CMD:
      TCtype = OFF;
      WinSendDlgItemMsg(hWndDlg, 153, SPBM_SETLIMITS,  // MPU command
                         MPFROMLONG(255), MPFROMLONG(0));
      WinSendDlgItemMsg(hWndDlg, 154, SPBM_SETLIMITS,  // Command data byte
                         MPFROMLONG(255), MPFROMLONG(0));
      WinSendDlgItemMsg(hWndDlg, 155, SPBM_SETLIMITS,  // Transpose
                         MPFROMLONG(36), MPFROMLONG(-36));
      WinSendDlgItemMsg(hWndDlg, 156, SPBM_SETLIMITS,  // Velocity
                         MPFROMLONG(50), MPFROMLONG(-15));
      break;

     case IDLG_E1TEXT:
      TxtCode = 7;                       // Default to Cue Point
      WinSendDlgItemMsg(hWndDlg, 129, BM_SETCHECK, MPFROMSHORT(ON) , 0L); //Set Cue Pt.
      WinSendDlgItemMsg(hWndDlg, 121, EM_SETTEXTLIMIT, // Meta text length
                         MPFROMSHORT(250), 0);
      break;

     case IDLG_E1INIT:
      InitTrks = OFF;
      break;
    }
    for (i = 153; i < 157; ++i)
      WinSendDlgItemMsg(hWndDlg, i, SPBM_SETCURRENTVALUE, // Initial value
        MPFROMLONG(0), 0);
    break;

/**********************************************************************/
/*           Process control options for Insert functions             */
/**********************************************************************/
   case WM_CONTROL:
    cntl = SHORT1FROMMP(mp1);
    if (cntl >= 201 && cntl <= 216)  // Track Initiate check boxes
    {
     cntl -= 201;
     InitTrks |= 1 << cntl;          // Set tracks to start
     break;
    }
    else switch(cntl)
    {
/**********************************************************************/
/*                      Generic event controls                        */
/**********************************************************************/
     case 123:                       // MPU Mark checkbox (Generic event)
      rc = WinQueryButtonCheckstate(hWndDlg, 123);
      if (rc)
      {
        Flags1 |= MPUMARK+RUNSTAT;
        WinSendDlgItemMsg(hWndDlg, 155, SPBM_SETLIMITS,  // Data 1
                         MPFROMLONG(254), MPFROMLONG(240));
        WinEnableControl(hWndDlg, 124, FALSE);   // Disable Running status
        WinEnableControl(hWndDlg, 125, FALSE);   // Disable DBYTES2
        WinEnableControl(hWndDlg, 154, FALSE);   // Disable Spin_2
        WinEnableControl(hWndDlg, 156, FALSE);   // Disable Spin_4
      }
      else
      {
        Flags1 &= 255-MPUMARK-RUNSTAT;
        WinSendDlgItemMsg(hWndDlg, 155, SPBM_SETLIMITS,  // Data 1
                         MPFROMLONG(127), MPFROMLONG(0));
        WinEnableControl(hWndDlg, 124, TRUE);    // Enable Running status
        WinEnableControl(hWndDlg, 125, TRUE);    // Enable DBYTES2
        WinEnableControl(hWndDlg, 154, TRUE);    // Enable Spin_2
        WinEnableControl(hWndDlg, 156, TRUE);    // Enable Spin_4
      }
      break;

     case 124:                       // Running Status checkbox
      rc = WinQueryButtonCheckstate(hWndDlg, 124);
      if (rc)
      {
        Flags1 |= RUNSTAT;
        WinEnableControl(hWndDlg, 154, FALSE);   // Disable Spin button 2
      }
      else
      {
        Flags1 &= 255-RUNSTAT;
        WinEnableControl(hWndDlg, 154, TRUE);    // Enable Spin button 2
      }
      break;

     case 125:                   // Use two data bytes checkbox
      rc = WinQueryButtonCheckstate(hWndDlg, 125);
      if (rc)
      {
        Flags1 |= DBYTES2;
        WinEnableControl(hWndDlg, 156, TRUE);    // Enable Spin button 4
      }
      else
      {
        Flags1 &= 255-DBYTES2;
        WinEnableControl(hWndDlg, 156, FALSE);   // Disable Spin button 4
      }
      break;

/**********************************************************************/
/*                      Track Command controls                        */
/**********************************************************************/
     case 126:                   // MPU Cmd radio button
     case 127:                   // Transpose radio button
     case 128:                   // Velocity radio button
      WinEnableControl(hWndDlg, 164, TRUE);     // Enable 'Done'
      for (i = 153; i < 157; ++i)
        WinEnableControl(hWndDlg, i, FALSE);    // Disable all Spin buttons
      if (cntl == 126)           // MPU Cmd
      {
        TCtype = OFF;
        WinEnableControl(hWndDlg, 153, TRUE);   // Enable Spin button 1
        WinEnableControl(hWndDlg, 154, TRUE);   // Enable Spin button 2
      }
      else if (cntl == 127)      // Transpose
      {
        WinEnableControl(hWndDlg, 155, TRUE);   // Enable Spin button 3
        TCtype = TCMTRN;         // Indicate transpose
      }
      else if (cntl == 128)      // Velocity
      {
        WinEnableControl(hWndDlg, 156, TRUE);   // Enable Spin button 4
        TCtype = TCMVEL;         // Indicate velocity
      }
      break;

/**********************************************************************/
/*                       Meta Text controls                           */
/**********************************************************************/
     case 129:                   // Cue Point
     case 130:                   // Plain Text
      TxtCode = (cntl == 129) ? 7 : 1;   // Set appropriate meta text code
      break;

/**********************************************************************/
/*            General code to get Spin Button values                  */
/**********************************************************************/
     case 153:                   // Spin button 1
     case 154:                   // Spin button 2
     case 155:                   // Spin button 3
     case 156:                   // Spin button 4
      switch(SHORT2FROMMP(mp1))  // Switch on Notification Code
      {
       case SPBN_ENDSPIN:
       case SPBN_CHANGE:
        WinSendDlgItemMsg(hWndDlg, cntl, SPBM_QUERYVALUE, // Get value
                           &ulUtility, 0);
        if (cntl == 153) Spin_1 = ulUtility;
        else if (cntl == 154) Spin_2 = ulUtility;
        else if (cntl == 155) Spin_3 = ulUtility;
        else if (cntl == 156) Spin_4 = ulUtility;
        break;
      }
      break;
    }
    break;

/**********************************************************************/
/*      Process 'Done/Cancel' buttons for all Insert functions        */
/**********************************************************************/
   case WM_COMMAND:
    cmd = SHORT1FROMMP(mp1);
    switch(cmd)
    {
     case 151:                   // Cancel (All types)
      WinDismissDlg(hWndDlg, TRUE);
      break;

/**********************************************************************/
/*                  Process Track Transfer                            */
/**********************************************************************/
     case 160:                       // Done - Transfer
      WinQueryWindowText(WinWindowFromID(hWndDlg, 152), 6, szText);
      i = atoi(szText);                       // Convert target index
      if (!chk_range(i, "Target index", 0, Ndx_of_Mark))
        return((MRESULT)8);

      Build_EVP(&EVP, 0, Spin_1, METAEVT+MLABMETA, TRKXFER);
      EVP.trkcmddata = Ndx_of_Mark + 9 - i;  // Set target
      EVP.length = 13;
      Tlnth = 13;                    // Set length for adj_xfer routine
      xferswt = ON;                  // Set transfer insert swt
      EditPaste(hWndETrk[ptrack], &EVP, sizeof(EVP), ptrack);
      editchng |= 1 << ptrack;       // Indicate something has changed
      break;

/**********************************************************************/
/*                  Process Track Rest                                */
/**********************************************************************/
     case 161:                       // Done - Rest
       /* Build an event for a Timing_Overflow */
      Build_EVP(&EVP, 240, TIMING_OVFLO, MPUMARK, 0);
      EVP.length = PTI = 1;

      usUtility = spptr->tsign * spptr->tbase; // Develop timing total/bar
      j = usUtility / 240;           // Find # of timing ovflos to insert
      for (i = 0; i < j; ++i)
      {
        Tlnth = 1;                   // Set length for adj_xfer routine
        EditPaste(hWndETrk[ptrack], &EVP, sizeof(EVP), ptrack);
        ++PTI;                       // Increment track index
        ++Xndx;                      // Increment Event Index to next event
      }

       /* Build an event for Measure_End */
      Build_EVP(&EVP, usUtility - (j * 240), MEASURE_END, MPUMARK, 0);
      EVP.length = PTI = 2;
      Tlnth = 2;                     // Set length for adj_xfer routine
      EditPaste(hWndETrk[ptrack], &EVP, sizeof(EVP), ptrack);
      PTI += 2;                      // Increment track index
      ++Xndx;                        // Increment Index to next event

         /* Build an event for a Track Rest Order if > 1 measure */
      if (--Spin_1)                  // More than 1 measure
      {
        Build_EVP(&EVP, 0, Spin_1, METAEVT+MLABMETA, 0);
        EVP.evtype = TRKREST;        // Set Track Order type
        EVP.trkcmddata = 9 + j;      // Set target for Rest
        EVP.length = PTI = 13;
        Tlnth = 13;                  // Set length for adj_xfer routine
        EditPaste(hWndETrk[ptrack], &EVP, sizeof(EVP), ptrack);
      }
      editchng |= 1 << ptrack;       // Indicate something has changed
      break;

/**********************************************************************/
/*                  Process Program Change                            */
/**********************************************************************/
     case 162:                   // Done - Program Change
      EVptr = (EVPK *)malloc(sizeof(EVP) << 1); // Get stg for 2 events
      Build_EVP(EVptr, 0, Spin_1, EVAHEADR, PGM_CHANGE | Spin_2);
      Build_EVP(EVptr+1, 0, Spin_1, 0, PGM_CHANGE | --Spin_2);
      (EVptr+1)->length = 3;
      Tlnth = 3;                 // Set length for adj_xfer routine
      EditPaste(hWndETrk[ptrack], EVptr, sizeof(EVP) << 1, ptrack);
      free(EVptr);
      editchng |= 1 << ptrack;   // Indicate something has changed
      break;

/**********************************************************************/
/*                  Process Generic MIDI event                        */
/**********************************************************************/
     case 163:                   // Done - Generic Event
      usUtility = sizeof(EVP);   // Storage for 1 event
      if (!(Flags1 & RUNSTAT))
        usUtility <<= 1;         // Need another event
      EVptr = (EVPK *)malloc(usUtility); // Get stg for event(s)

      if (!(Flags1 & RUNSTAT))   // Nead a header
      {
        Build_EVP(EVptr, Spin_1, Spin_3, EVAHEADR, Spin_2);
        EVptr2 = EVptr + 1;      // Set alternate pointer to 2nd event
      }
      else EVptr2 = EVptr;       // Set alternate pointer to 1st event
      Build_EVP(EVptr2, Spin_1, Spin_3, Flags1, Spin_2);
      EVptr2->dbyte2 = Spin_4;
      if (EVptr2->flags1 & MPUMARK) EVptr2->length = // Set 1 of 4 possible lnths
            (EVptr->deltime == 240 && EVptr->dbyte1 == TIMING_OVFLO) ? 1 : 2;
      else EVptr2->length = (EVptr->flags1 & RUNSTAT) ? 3 : 4;
      Tlnth = EVptr2->length;     // Set length for adj_xfer routine

      if (EVptr->deltime == 240 && EVptr->dbyte1 != TIMING_OVFLO)
        umsg('E', hWndETrk[ptrack], "Timing of 240 permitted only for Overflow MPU Mark");
      else
      {
        EditPaste(hWndETrk[ptrack], EVptr, usUtility, ptrack);
        editchng |= 1 << ptrack;   // Indicate something has changed
      }
      free(EVptr);
      break;

/**********************************************************************/
/*                  Process Track Command or Adjustment               */
/**********************************************************************/
     case 164:                   // Done - Track command
      Build_EVP(&EVP, 0, Spin_1, METAEVT+MLABMETA, TRKCMDX);
      EVP.length = PTI = 13;
      Tlnth = 13;                // Set length for adj_xfer routine
      EVP.in_chan = TCtype;      // Set flag byte
      EVP.dbyte2 = Spin_2;       // Assume command byte
      if (TCtype)                // If transpose or velocity
        EVP.dbyte2 = (TCtype == TCMTRN) ? Spin_3 : Spin_4; // Overlay dbyte2
      else if (EVP.dbyte1 >= 0xe0 && EVP.dbyte1 <= 0xef)   // Needs 1 byte
        EVP.in_chan = TCDATA;    // Indicate data byte required
      EditPaste(hWndETrk[ptrack], &EVP, sizeof(EVP), ptrack);
      editchng |= 1 << ptrack;   // Indicate something has changed
      break;

/**********************************************************************/
/*                  Process Meta Text event                           */
/**********************************************************************/
     case 165:                             // Done - Meta Text
      EVptr = (EVPK *)malloc(sizeof(EVP)); // Get stg for 1 event
      WinQueryWindowText(WinWindowFromID(hWndDlg, 121), 250, szText);
      Build_EVP(EVptr, 0, TxtCode, METAEVT, 0);
      EVptr->evcode = TxtCode;             // Set appropriate code
      EVptr->MetaLn = strlen(szText)+1;    // Set length of text
      EVptr->Meta_ptr = malloc(EVptr->MetaLn); // Get stg for text
      strcpy(EVptr->Meta_ptr, szText);     // Copy text
      EVptr->length = EVptr->MetaLn+4;     // Set total event length
      PTI = EVptr->length;                 // Point beyond event
      Tlnth = EVptr->length;               // Set length for adj_xfer routine
      EditPaste(hWndETrk[ptrack], EVptr, sizeof(EVP), ptrack);
      free(EVptr);
      editchng |= 1 << ptrack;             // Indicate something has changed
      break;

/**********************************************************************/
/*                  Process Track Initiate                            */
/**********************************************************************/

     case 166:                   // Done - Track Initiate
      t18 = InitTrks;            // Extract tracks 1 to 8
      t916 = InitTrks >> 8;      // Extract tracks 9 - 16
      Build_EVP(&EVP, 0, t18, METAEVT+MLABMETA, TRKINIT);
      EVP.dbyte2 = t916;
      EVP.trkcmddata = 0;        // ***TEMP always start at index zero
      EVP.length = PTI = 13;
      Tlnth = 13;                // Set length for adj_xfer routine
      EditPaste(hWndETrk[ptrack], &EVP, sizeof(EVP), ptrack);
      editchng |= 1 << ptrack;   // Indicate something has changed
      break;
    }
  }   // End switch(message)
  return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
}

/**********************************************************************/
/*                  Build prototype Event Packet                      */
/**********************************************************************/
VOID Build_EVP(PVOID Evpk, LONG deltime, BYTE dbyte1, BYTE flags1, BYTE evtype)
{
  EVPK *event;

  event = (EVPK *)Evpk;                  // Set up EVPK pointer
  event->deltime = deltime;              // Set delta time
  event->dbyte1 = dbyte1;                // Set data byte 1
  event->evtype = evtype;                // Set event code
  event->flags1 = flags1;                // Set flag byte 1

  event->length = 0;                     // Initialize length to 0
  event->evcode = event->evtype & 0xf0;  // Set raw event code
  event->in_chan= event->evtype & 0x0f;  // Extract channel number
}
