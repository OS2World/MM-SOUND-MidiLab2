/***************************************************************************/
/*               C and OS/2 Defines and Includes                           */
/***************************************************************************/
#define INCL_WIN       // Try to keep this to just what I need in order
#define INCL_GPI       // to reduce compile time.
#define INCL_HELP
#define INCL_DOS
#define INCL_DOSPROCESS
#define INCL_SUB
#define INCL_BASE
#define INCL_OS2MM                 /* Required for OS/2 MM stuff    */
#define INCL_SW                    /* Required for OS/2 MM stuff    */

#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "global.h"

/***************************************************************************/
/*               MidiLab definitions                                       */
/***************************************************************************/
#define MLAB_FILE_VERIFIER  102734      /* MidiLab native format id        */
#define MLAB_FILE_VERNEW    042345      /* New format id (longer trk name) */

#define STKSZ        16384           /* Thread stack size            */

#define TRKCMDHDR    255             /* Track order identifier       */
#define TRKXFER      241             /* Trk Order - Track Transfer   */
#define TRKINIT      242             /* Trk Order - Track Iniitate   */
#define TRKCMDX      243             /* Trk Order - MPU Command      */
#define TRKREST      244             /* Trk Order - Rest Transfer    */

#define NUMTRKS      16              // Number of tracks
#define MAIN         1
#define MAIN_EV      16              // Main event package
#define LOAD_EV      17              // Load event package
#define LAST         3

#define RESET        4

#define ON           1
#define OFF          0               /* Switch constants             */
#define LEVEL1       1
#define LEVEL2       2
#define LEVEL3       3

#define ENTER        13
#define CANCEL       27                     /* Esc          */
#define ESC          27
#define BIGPLUS      43

#define TRC_CMDS     0x01
#define TRC_OUTDATA  0x02
#define TRC_INDATA   0x04            /* Trace items         */
#define TRC_CLK2HOST 0x08
#define TRC_ACK      0x10
#define TRC_READBUFR 0x20

#define TNML         16              /* Lnth of track name field    */

#define PT           trk[ptrack].trkaddr            /* Space savers */
#define RT           trk[rtrack].trkaddr
#define PTI          trk[ptrack].index
#define RTI          trk[rtrack].index
#define PTL          sp.trkdatalnth[ptrack]
#define RTL          sp.trkdatalnth[rtrack]
#define EVP          evp[ptrack]
#define EVR          evp[rtrack]
#define TRK          trk[ptrack]
#define EVMN         evp[MAIN_EV]
#define TRKMN        trk[MAIN_EV]
#define EVLD         evp[LOAD_EV]
#define TRKLD        trk[LOAD_EV]
#define nop          rc=rc                          /* No-operation        */

/***************************************************************************/
/*               MidiLab Window Procedure User Messages                    */
/***************************************************************************/
#define WM_FAILEDVALIDATE (WM_USER+0)
#define UM_DLGINIT        (WM_USER+1)
#define UM_PAINT          (WM_USER+2)
#define UM_SET_HEAD       (WM_USER+3)
#define UM_SET_TEMPO      (WM_USER+5)
#define UM_MOVE_BAR       (WM_USER+6)

/***************************************************************************/
/*               Definitions of sysflag2 flag byte                         */
/***************************************************************************/
      #define MFILE10   0x80              /* Standard files (default)*/
      #define BIGBARS   0x40              /* Big Measure Display     */
      #define DISKLOAD  0x20              /* Disk load in progress   */
      #define QSAVE     0x10              /* Save in Mlab format     */
      #define INITCMPL  0x08              /* Initialization complete */
      #define NOMPU     0x04              /* Run in MPU simulation   */
      #define EDITCMPL  0x01              /* Edit window initialized */

/*********************************************************************/
/*               Track Order Map:  Track Transfer/Rest               */
/*********************************************************************/
typedef struct trkcmdtt
{
  BYTE
    trkcmdhdr,                         /* Track cmd header           */
    trkcmdtype;                        /* Track Transfer id          */
  USHORT
    ttarget;                           /* Target index displacement  */
  BYTE
    ttrepeat,                          /* Repeat counter     (1-254) */
    tteox;                             /* Track command EOX          */
} TT;                                  /* Type name                  */
TT *ttptr;                             /* Pointer to mapped area     */

/*********************************************************************/
/*               Track Order Map:  Track Initiate                    */
/*********************************************************************/
typedef struct trkcmdti
{
  BYTE
    trkcmdhdr,                         /* Track cmd header           */
    trkcmdtype;                        /* Track_Initiate id          */
  USHORT
    tistrtndx;                         /* Starting index             */
  SHORT
    tiselect;                          /* Track-select bit map (1-16)*/
} TI;                                  /* Type name                  */
TI *tiptr;                             /* Pointer to mapped area     */

/*********************************************************************/
/*           Track Order Map:  Execute MPU command or adjustment     */
/*********************************************************************/
typedef struct trkcmdtc
{
  BYTE
    trkcmdhdr,                         /* Track cmd header           */
    trkcmdtype,                        /* Track_Command id           */
    tcflags,                           /* Flag Byte                  */
      #define TCDATA  0x80             /* Data byte required         */
      #define TCMTRN  0x40             /* Modify transposition       */
      #define TCMVEL  0x20             /* Modify velocity            */
    tcmpucmd,                          /* MPU-401 command byte       */
    tccmdata,                          /* Command data byte          */
    tceox;                             /* Track command EOX          */
} TC;                                  /* Type name                  */
TC *tcptr;                             /* Pointer to mapped area     */

/*********************************************************************/
/*                   MIDI 1.0 Meta-Event Maps                        */
/*********************************************************************/
/*------------------------ Tempo Meta-Event -------------------------*/
typedef struct
{
  long
      m1tempo;                         /* Tempo (usec per 1/4 note)  */
} M_TEMPO;
M_TEMPO *m_tempo;

/*-------------------- Time Signature Meta-Event --------------------*/
typedef struct
{
  BYTE
      m2numer,                         /* Time Signature numerator   */
      m2denom,                         /* Time Signature denominator */
      m2clks_click,                    /* Time Sig. clocks per click */
      m232nd_quar;                     /* Time Sig. 32nd notes/24clks*/
} M_TSIGN;
M_TSIGN *m_tsign;

/*-------------------------- MidiLab Meta-Event ---------------------*/
typedef _Packed struct
{
  BYTE
      mlabid[3],                       /* MidiLab identifier         */
      mlabfunc,                        /* MidiLab Meta sub-function  */
      mlabtrkc;                        /* Track order code           */
} M_MLAB;
M_MLAB *m_mlab;
/********************* End of MIDI 1.0 File Format *******************/

typedef struct GridHit     // Structure to return Mouse coordinates (Song Editor)
{
  SHORT Meas;              // Selected Measure
  BYTE  Trk,               // Selected Track
        rc;                // Return code (0 = ok; <>0 = invalid coordinates)
} GRIDHIT;

/***************************************************************************/
/*                    PM-generated Function Prototypes                     */
/***************************************************************************/
INT  cwCenter(HWND, HWND);
INT  cwFillFileListBox(HWND, PSZ, INT);
//HWND cwCreateWindow(HWND, ULONG, PCH, PCH, USHORT, INT, INT, INT, INT,
//   PHWND, ULONG, USHORT);
INT  cwFillDirListBox(HWND, USHORT, USHORT);

MRESULT EXPENTRY WndProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY EDITWndProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY FILEOPENMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY CTLPANELMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY SYSEXMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY SPECFUNCMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY RECRDMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY STDBYMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY UTILMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY RCTLMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY EDITMsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY E1MsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY E2MsgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY E3MsgProc(HWND, USHORT, MPARAM, MPARAM);

/***************************************************************************/
/*                  MidiLab Function Prototypes                            */
/***************************************************************************/
  int main(int argc, char *argv[]);
  VOID dmgreset(int path, int rvalue);
  VOID mainloop(VOID);
  int playprep(BYTE function);
  int recmdata(VOID);
  BYTE quantize(BYTE qval);
  int dmgrsave(VOID);
  int dmgrload(VOID);
  char dmgopen(VOID);
  SHORT playmdata(BYTE trkreq);
  int trk_order(char mode);
  int chkact(VOID);
  VOID metro(char func);
  VOID rec_prep(VOID);
  VOID rec_control(char cntl);
  int trkxfer(char *disp, USHORT target, BYTE repeat, long cmdtype);
  USHORT umsg(char lvl, HWND hWndMsg, PSZ text);
  APIRET mpucmd(BYTE cmd, BYTE dbyte);
  VOID mpuputd(BYTE pdata, BYTE cflg);
  BYTE mpugetd( VOID );
  VOID event_error(int dvalue);
  int trkcmd(VOID);
  char *cntrl_type(BYTE ctlnum);
  VOID proc_sysx(BYTE function);
  VOID write_varlen(long value, char mode);
  long read_varlen(SHORT trk);
  VOID rev_int(BYTE *loc, BYTE length);
  int proc_meta(SHORT ptrack, SHORT mtrk);
  VOID settbase(BYTE path);
  int get_event(SHORT trk);
  VOID Rel_sustain(BOOL force);
  VOID stackmgr(BYTE function, BYTE databyte);
  VOID bagit(PSZ message);
  VOID paint(HPS hps, BYTE path);
  VOID Esc_cancel( VOID );
  VOID space_bar( VOID );
  VOID ndxwrt(SHORT track, LONG bColor);
  int chk_range(LONG inputint, char *text, LONG min, LONG max);
  VOID set_attr(HWND hWnd, SHORT item, USHORT(attr), BYTE state);
  VOID Load_Trk_Order(SHORT trk, char *trkorder);
  BYTE Set_Client(VOID);
  BYTE Remote_ctl(PVOID p_EVPrc);
  VOID Locate_SPP(LONG Strt_Pt);
  VOID BigBars(VOID);
  VOID FormatDialog(HWND hwnd);
  VOID Wrt_Trace(char *text);
  VOID Trk_Edit(BYTE track, SHORT measure, SHORT numMeas, HWND hWndC);
  VOID wm_paint(HWND hWnd);
  GRIDHIT Loc_Area(PUSHORT p_mMsg);
  VOID UART_Cmd(BYTE cmd);
  VOID Analyze_Input(BYTE DByte, BYTE DLnth);
  VOID Delay(ULONG msecs);
  VOID MetroClk(VOID);

/***************************************************************************/
/*               MidiLab Function Prototypes (DLL)                         */
/***************************************************************************/
  VOID DLLReset( VOID );
  VOID additem(HWND hwnd, SHORT ptrack, BOOL Space);
  BYTE EvListBox(HWND hWndDlg, SHORT cntl, SHORT ncode, SHORT ptrack);
  VOID XeqEvent(SHORT ptrack, PVOID Event);
  VOID PlayEvnt(VOID);
  PCHAR Resolve_Note(BYTE note, BYTE lvl);
  BYTE decode_event(PVOID pevent, SHORT ptrack);
  VOID EditDel(HWND hWndDlg, SHORT ptrack);
  VOID EditPaste(HWND hWndDlg, PVOID Pdata, ULONG Psize, SHORT ptrack);
  VOID ClipWrite(SHORT Clipndx, SHORT Clipct, SHORT ptrack);
  VOID Build_EVP(PVOID Evpk, LONG deltime, BYTE dbyte1, BYTE flags1, BYTE evtype);
  VOID Re_Index(HWND hWndDlg, SHORT ptrack);
  VOID PtrBusy(BYTE mode);
  VOID adj_xfer(BYTE mode, SHORT ptrack);
  LONG TrkBld(PVOID P_InptEv, SHORT ptrack);
  VOID Set_Deltime(LONG deltime, SHORT ptrack);
