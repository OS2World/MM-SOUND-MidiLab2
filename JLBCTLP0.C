/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBCTLP0 -    ** Main MIDI Control dialog procedure             */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
#define EXT extern
#include "MLABPM.h"
#include "MLABPM.rch"
#include "JLBCSA00.C"

MRESULT EXPENTRY CTLPANELMsgProc(HWND hWndDlg,
                                 USHORT message, MPARAM mp1, MPARAM mp2)
{
 SHORT i;
 BYTE result;
 static BYTE act_allswt = 1, enb_allswt = 0, solo_allswt = 0;
 static HWND  hWndParent;

 switch(message)
   {
    case WM_INITDLG:
      hWndParent = (HWND)mp2;
      WinPostMsg(hWndDlg, UM_DLGINIT, 0L, 0L);    // Signal init routine
      WinAssociateHelpInstance(hWndMLABHelp, hWndDlg); // Link to HELP 12/31/92
      cwCenter(hWndDlg, (HWND)hWndParent);
      break; /* End of WM_INITDLG */

/**********************************************************************/
/*   System Reset - Initialize all controls within the Dialog Box     */
/**********************************************************************/
    case UM_DLGINIT:
      WinSendDlgItemMsg(hWndCTLPANEL, 297+sp.ccswt  , BM_SETCHECK, MPFROMSHORT(1), 0L); //Set Filter all

      WinSendDlgItemMsg(hWndDlg, 310, EM_SETTEXTLIMIT, MPFROMSHORT(4), 0L);
      WinSetDlgItemShort(hWndDlg, 310, skip_bar_ctr, TRUE);
      WinSetDlgItemShort(hWndDlg, 127,
          ending_measure == 9999 ? 9999 : ending_measure-skip_bar_ctr, TRUE);
      WinSendDlgItemMsg(hWndDlg, 127, EM_SETTEXTLIMIT, MPFROMSHORT(4), 0L);
      WinSendDlgItemMsg(hWndDlg, 127, EM_SETSEL, MPFROM2SHORT(0,4), 0L);

      WinSendDlgItemMsg(hWndCTLPANEL, 301, BM_SETCHECK, MPFROMSHORT(ExtCtlSwt) ,0L); //Set extctl

      result = (sp.proflags & VELLEVEL) ? ON : OFF;
      WinSendDlgItemMsg(hWndCTLPANEL, 302, BM_SETCHECK, MPFROMSHORT(result), 0L); //Set Vel Lvl

      result = (ini.iniflag1 & PLAYLOOP) ? ON : OFF;
      WinSendDlgItemMsg(hWndCTLPANEL, 128, BM_SETCHECK, MPFROMSHORT(result), 0L); //Set Rpt Seq

      WinSendDlgItemMsg(hWndCTLPANEL, 305, BM_SETCHECK, MPFROMSHORT(mnswt) , 0L); //Set Metro

      for (i = 0; i < 16; ++i)  // Set solo'd trks
      {
        result = (trksolo & 1 << i) ? ON : OFF;
        WinSendDlgItemMsg(hWndCTLPANEL, 401+i, BM_SETCHECK, MPFROMSHORT(result), 0L);
      }
      for (i = 0; i < 16; ++i)  // Set activated tracks
      {
        result = (sp.playtracks & 1 << i) ? ON : OFF;
        WinSendDlgItemMsg(hWndCTLPANEL, 501+i, BM_SETCHECK, MPFROMSHORT(result), 0L);
      }
      for (i = 0; i < 16; ++i)  // Set enabled tracks
      {
        result = (sp.trkmask & 1 << i) ? ON : OFF;
        WinSendDlgItemMsg(hWndCTLPANEL, 601+i, BM_SETCHECK, MPFROMSHORT(result), 0L);
      }
      WinEnableControl(hWndDlg, 291, ExtCtlSwt == 2 ? FALSE : TRUE);  // Enable/Disable Tempo Bar
      WinSetDlgItemShort(hWndDlg, 258, sp.tbase, TRUE); // Set time base

/*********************************************************************/
/*   Load Quantize list boxes                                        */
/*********************************************************************/
      mresReply = WinSendDlgItemMsg(hWndCTLPANEL, 259,
                         LM_QUERYITEMCOUNT, NULL, NULL);
      if (mresReply == NULL)
      {
        for ( i = 0; i < 8; i++ )
        {
          WinLoadString( hAB, 0, IDS_QNT_OFF + i,
                         5,                 // Length of box entry
                         (PSZ)szChars );

          WinSendDlgItemMsg( hWndCTLPANEL, 259, LM_INSERTITEM,
                             MPFROM2SHORT( LIT_END, 0 ),
                             MPFROMP(szChars) );
        }
/**********************************************************************/
/*  Load Time Signature list box                                      */
/**********************************************************************/
        for ( i = 0; i < 8; i++ )
        {
          WinLoadString( hAB, 0, IDS_TSG2 + i,
                         4,                  //Length of box entry
                         (PSZ)szChars );

          WinSendDlgItemMsg( hWndCTLPANEL, 287, LM_INSERTITEM,
                             MPFROM2SHORT( LIT_END, 0 ),
                             MPFROMP(szChars) );
        }
      }  // end mresReply == NULL
      WinSendDlgItemMsg(hWndCTLPANEL, 259, LM_SELECTITEM,    // Set quantize
                        MPFROMSHORT(sp.qadjctr),  MPFROMSHORT(TRUE));

      WinSendDlgItemMsg(hWndCTLPANEL, 287, LM_SELECTITEM, MPFROMSHORT(sp.tsign-2),
                        MPFROMSHORT(TRUE)); // Set time signature

/**********************************************************************/
/*        Set Velocity, Tempo and Transpose scroll bars               */
/**********************************************************************/
      VelPos = sp.velocadj + 15;             // Set offset
      WinSendDlgItemMsg(hWndCTLPANEL, 131, SBM_SETSCROLLBAR,
                        MPFROM2SHORT(VelPos, 0), MPFROM2SHORT(0, 65));
      WinSetDlgItemShort(hWndCTLPANEL,262, sp.velocadj, TRUE);

      TrnsPos = sp.transadj + 36;
      WinSendDlgItemMsg(hWndCTLPANEL, 307, SBM_SETSCROLLBAR,
                MPFROM2SHORT(TrnsPos, 0), MPFROM2SHORT(0, 72));
      WinSetDlgItemShort(hWndCTLPANEL,286, sp.transadj, TRUE);

    case UM_SET_TEMPO:
      TempoPos = sp.tempo;
      WinSendDlgItemMsg(hWndCTLPANEL, 291, SBM_SETSCROLLBAR,
                MPFROM2SHORT(TempoPos, 0), MPFROM2SHORT(40, 250));
      WinSetDlgItemShort(hWndCTLPANEL,288, TempoPos, TRUE);

      break; /* End of WM_UINIT */

    case WM_CONTROL:
      i = SHORT1FROMMP(mp1);
/**********************************************************************/
/*             Set track check boxes                                  */
/**********************************************************************/
      if (i >= 401 && i <= 416)       //Solo
      {
        i -= 401;
        trksolo ^= (1 << i);          /* Toggle indivdual ones   */
        break;
      }
      else if (i >= 501 && i <= 516)  //Activate
      {
        i -= 501;
        sp.playtracks ^= (1 << i);
        start_bar_swt = ON;         // Force re-positioning in SUBS0
        break;
      }
      else if (i >= 601 && i <= 616)  //Enable
      {
        i -= 601;
        sp.trkmask ^= (1 << i);
        break;
      }

      switch (i)
      {
       case 258: /* TimeBase Entry field                                       */
         switch(SHORT2FROMMP(mp1)) /* switch on Notification Code  */
         {
          case EN_SETFOCUS:/* Entry field is receiving the focus   */
               break;

          case EN_KILLFOCUS: /* Entry field is losing the focus    */
               WinQueryWindowText(WinWindowFromID(hWndDlg, 258),
                                  4, szChars);
               i = atoi(szChars);
               if (chk_range(i, "Time Base", 48, 480))
               {
                 sp.tbase = i;       // Set time base
                 settbase(4);        // Go to re-calc temporal values
               }
               else break;

          default: /* Default other messages                       */
               return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
         }

       case 259: /* Quantize List box                                          */
         switch(SHORT2FROMMP(mp1)) /* switch on Notification Code  */
         {
          case LN_SELECT:    /* Field is being selected    */
            sp.qadjctr = SHORT1FROMMR(WinSendDlgItemMsg(hWndCTLPANEL,
                         259, LM_QUERYSELECTION, 0L, 0L ) );
            switch (sp.qadjctr)
            {
              case 0: quantize_amt = 0; break;               // Off
              case 1: quantize_amt = sp.tbase;     break;
              case 2: quantize_amt = (sp.tbase<<1)/3; break;
              case 3: quantize_amt = sp.tbase / 2; break;
              case 4: quantize_amt = sp.tbase / 3; break;
              case 5: quantize_amt = sp.tbase / 4; break;
              case 6: quantize_amt = sp.tbase / 6; break;
              case 7: quantize_amt = sp.tbase / 8; break;    // 1/32
            }
            q_limit = quantize_amt / 2;
            break;
          }

       case 297: /* Radiobutton text: "All"                           */
           sp.ccswt = OFF;
           mpucmd(0x86,0);                /* Disable continuous ctls */
           break;

       case 298: /* Radiobutton text: "None"                          */
           sp.ccswt = LEVEL1;     /* Enable all cont ctrls   */
           mpucmd(0x87,0);       /* Enable pitch-bend, etc. */
           break;

       case 299: /* Radiobutton text: "Aft Tch"                       */
           sp.ccswt = LEVEL2;     /* Disable chan after-touch*/
           break;

       case 300: /* Radiobutton text: "Pgm Chg"                       */
           sp.ccswt = LEVEL3;     /* Filter pgm changes      */
           mpucmd(0x86,0);       /* Disable continuous ctls */
           break;

       case 301: /* Checkbox text: "External Cntl"                    */
           {
            if (chkact()) break;      /* Insure ok to do this now*/
            if (ExtCtlSwt == OFF)
            {
              sp.proflags |= EXTCNTRL; /* Activate ext. control  */
              ExtCtlSwt = LEVEL1;
            }
            else if (ExtCtlSwt == LEVEL1)
            {
              mpucmd(0x82, 0);
              ExtCtlSwt = LEVEL2;
            }
            else if (ExtCtlSwt == LEVEL2)
            {
              sp.proflags &= 255-EXTCNTRL; /* Deactivate ext. ctl*/
              mpucmd(0x80, 0);
              ExtCtlSwt = OFF;
            }
            dmgreset(12,0);            /* Reset MPU              */
            WinCheckButton(hWndCTLPANEL, 301, ExtCtlSwt);
           }
           break;

       case 302: /* Checkbox text: "Velocity Level"                   */
           sp.proflags ^= VELLEVEL;   /* Flip switch in profile */
           break;

       case 305: /* Checkbox text: "Metronome"                        */
           metro(mnswt ? OFF : ON);    // Toggle metronome
           break;

       case 306: /* Checkbox text: "8th Note Click"                   */
           if (clikmult == 1)
             clikmult = 2;           /* Set for 1/8 notes       */
           else clikmult = 1;        /* Set for 1/4 notes       */
           settbase(10);             /* Set timebase controls   */
           break;

       case 128: /* Checkbox text: "Loop Sequence"                    */
           ini.iniflag1 ^= PLAYLOOP;  /* Flip switch                 */
           chngdini = ON;
           break;

       case 287: /* Time Signature List box                                          */
           switch(SHORT2FROMMP(mp1)) /* switch on Notification Code  */
           {
            case LN_SELECT:          /* Field is being selected      */
                 sp.tsign = SHORT1FROMMR(WinSendDlgItemMsg(hWndCTLPANEL,
                            287, LM_QUERYSELECTION, 0L, 0L ) );
                 sp.tsign += 2;      // map to index
                 settbase(2);        /* Re-set time signature        */
                 break;

            default: /* Default other messages                       */
                 return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
           }
           break;

       case 310:     // Starting measure
       case 127:     // Measures to play
           switch(SHORT2FROMMP(mp1)) /* switch on Notification Code  */
           {
            case EN_SETFOCUS:/* Entry field is receiving the focus   */
     // WinSendDlgItemMsg(hWndDlg, 127, EM_SETSEL, MPFROM2SHORT(0,4), 0L);
                 start_bar_swt = ON;  // Signal start-bar search
                 sysflag1 &= 255-POSACTV; /* Positioning now invalid */
                 break;

            case EN_KILLFOCUS: /* Entry field is losing the focus    */
                 WinQueryWindowText(WinWindowFromID(hWndDlg, 310),
                                    5, szChars);
                 i = atoi(szChars);
                 if (chk_range(i, "Starting Measure", 1, 9999))
                   skip_bar_ctr = i;       // Set starting measure
                 else break;

                 WinQueryWindowText(WinWindowFromID(hWndDlg, 127),
                                    5, szChars);
                 i = atoi(szChars);        // Get number of measures
                 if (chk_range(i, "Measures", 1, 9999))
                   ending_measure = skip_bar_ctr + i;
                 break;

            default: /* Default other messages                       */
                 return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
           }
           break;
     }
     WinEnableControl(hWndDlg, 291, ExtCtlSwt == 2 ? FALSE : TRUE);  // Enable/Disable Tempo Bar
     break; /* End of WM_CONTROL */

    case WM_HSCROLL:        /* Tempo Scroll bar event handler */
         switch(SHORT1FROMMP(mp1))
           {
            case 291:        // Tempo
                 switch (SHORT2FROMMP (mp2))
                      {
                      case SB_LINEDOWN :
                           if (TempoPos < 250)
                             ++TempoPos;
                           break ;

                      case SB_LINEUP :
                           if (TempoPos > 40)
                             --TempoPos;
                           break ;

                      case SB_PAGEDOWN :
                           TempoPos += 10;
                           break ;

                      case SB_PAGEUP :
                           TempoPos -= 10;
                           break ;

                      case SB_SLIDERTRACK :
                           TempoPos = SHORT1FROMMP(mp2);
                           break ;

                      default :
                           return 0 ;
                      }
                      WinSendDlgItemMsg(hWndCTLPANEL, 291, SBM_SETPOS,
                               MPFROMSHORT(TempoPos), NULL);
                      WinSetDlgItemShort(hWndCTLPANEL, 288, TempoPos, TRUE);
                      sp.tempo = TempoPos;
                      WinSendMsg(hWndClient, UM_SET_HEAD,
                               (MPARAM)0x04, NULL); // Set main window (8/19/93)
                      settbase(2);                  // Reset timing parms
                 break;

            case 307:          // Transpose
                 switch (SHORT2FROMMP (mp2))
                      {
                      case SB_LINEDOWN :
                           if (TrnsPos < 72)
                             ++TrnsPos;
                           break ;

                      case SB_LINEUP :
                           if (TrnsPos > 0)
                             --TrnsPos;
                           break ;

                      case SB_PAGEDOWN :
                           TrnsPos += 12;
                           break ;

                      case SB_PAGEUP :
                           TrnsPos -= 12;
                           break ;

                      case SB_SLIDERTRACK :
                           TrnsPos = SHORT1FROMMP(mp2);
                           break ;

                      default :
                           return 0 ;
                      }
                      WinSendDlgItemMsg(hWndCTLPANEL, 307, SBM_SETPOS,
                                        MPFROMSHORT(TrnsPos), NULL);
                      sp.transadj = TrnsPos - 36;
                      WinSetDlgItemShort(hWndCTLPANEL, 286, sp.transadj, TRUE);
                 break;

          case 131:    // Velocity scroll bar
               switch (SHORT2FROMMP (mp2))
                    {
                    case SB_LINEDOWN :
                         if (VelPos < 65)
                           ++VelPos;
                         break ;

                    case SB_LINEUP :
                         if (VelPos > 0)
                           --VelPos;
                         break ;

                    case SB_PAGEDOWN :
                         VelPos += 10;
                         if (VelPos > 65)
                           VelPos = 65;
                         break ;

                    case SB_PAGEUP :
                         VelPos -= 10;
                         if (VelPos < 0)
                           VelPos = 0;
                         break ;

                    case SB_SLIDERTRACK :
                         VelPos = SHORT1FROMMP(mp2);
                         break ;

                    default :
                         return 0 ;
                    }
                    WinSendDlgItemMsg(hWndCTLPANEL, 131, SBM_SETPOS,
                                      MPFROMSHORT(VelPos), NULL);
                    sp.velocadj = VelPos - 15;
                    WinSetDlgItemShort(hWndCTLPANEL,262, sp.velocadj, TRUE);
               break;
           }
         break;

    case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
           {
            case 113: /* Auxiliary Play */
                 WinSendMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B7,0), 0L);
                 break;

            case 114: /* Auxiliary Pause*/
            case WM_BUTTON2DOWN:
                 WinSendMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B5,0), 0L);
                 break;

            case 122: /* Auxiliary Repeat Record */
                 WinSendMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B2,0), 0L);
                 break;

            case 123: /* Auxiliary Reset */
                 WinSendMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B0,0), 0L);
                 break;

            case 124: /* Auxiliary All-Notes_Off */
                 WinSendMsg(hWndClient, WM_COMMAND, MPFROM2SHORT(MN_B1,0), 0L);
                 break;

            case 130: /* Reset starting-measures */
                 skip_bar_ctr = 1;
                 ending_measure = 9999;
                 sysflag1 &= 255-POSACTV;       /* Positioning now invalid */
                 WinSetDlgItemShort(hWndDlg, 310, skip_bar_ctr, TRUE);
                 WinSetDlgItemShort(hWndDlg, 127, ending_measure, TRUE);
                 break;

            case 400: /* Button text: " Solo All"                          */
                 solo_allswt ^= 1;
                 trksolo = (solo_allswt == 1) ? 0xffff : 0;
                 for (i = 0; i < 16; ++i)
                   WinSendDlgItemMsg(hWndCTLPANEL, 401+i, BM_SETCHECK,
                                     MPFROMSHORT(solo_allswt), 0L);
                 break;

            case 500: /* Button text: " Actv All"                          */
                 act_allswt ^= 1;
                 sp.playtracks = (act_allswt == 1) ? 0xffff : 0;
                 for (i = 0; i < 16; ++i)
                   WinSendDlgItemMsg(hWndCTLPANEL, 501+i, BM_SETCHECK,
                                     MPFROMSHORT(act_allswt), 0L);
                 break;

            case 600: /* Button text: "Enable All"                         */
                 enb_allswt ^= 1;
                 sp.trkmask = (enb_allswt == 1) ? 0xffff : 0;
                 for (i = 0; i < 16; ++i)
                   WinSendDlgItemMsg(hWndCTLPANEL, 601+i, BM_SETCHECK,
                                     MPFROMSHORT(enb_allswt), 0L);
                 break;
           }
         break; /* End of WM_COMMAND */

    case WM_CLOSE:
         WinDestroyWindow(hWndDlg);
         hWndCTLPANEL = 0;
         break;

    case WM_FAILEDVALIDATE:
         WinAlarm(HWND_DESKTOP, WA_ERROR);
         WinSetFocus(HWND_DESKTOP, WinWindowFromID(hWndDlg, SHORT1FROMMP(mp1)));
         return((MRESULT)TRUE);      /* End of WM_FAILEDVALIDATE */

    default:
         return(WinDefDlgProc(hWndDlg, message, mp1, mp2));
   }
 return FALSE;
} /* End of CTLPANELMsgProc */
