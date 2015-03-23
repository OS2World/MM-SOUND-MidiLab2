/*********************************************************************/
/* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
/*                                                                   */
/*   JLBCSA00 -    ** Common variable pool **                        */
/* This file is included in all other members as an 'extern' file    */
/*                                                                   */
/*       Copyright  J. L. Bell        January 1992                   */
/*********************************************************************/
#ifndef   EXT         // If not imbedded as 'extern' in another module
  #define EXT         // Re-define as NULL
  #define REALCSA     // Indicate this is for real
  #define REALDPm     // Define real DLL parameter block
  #include "mlabpm.h" // Needed for included types such as SHORT, USHORT, etc
#endif

/*********************************************************************/
/*               MidiLab Common Executable variables                 */
/*********************************************************************/
EXT BYTE
     indata,                              /* Incoming MPU data byte  */
     outdata,                             /* Outbound MPU data byte  */
     note_array[128],                     /* Active note-on table    */
     starting_xferctr[NUMTRKS],           /* For measure positioning */
     starting_restctr[NUMTRKS],           /* For measure positioning */
     leadin_ctr,                          /* Record lead-in countdwn */
     beatctr,                             /* Beat display counter    */
     beatswt,                             /* Beat display switch     */
     standby,                             /* Record/Overdub standby  */

     stopplay,                            /* Playback interrupted    */
     stoprec,                             /* Time to stop recording  */
     trackfull,                           /* Array overrun switch    */
     reccmd,                              /* Record/Overdub MPU cmd  */
     omniswt,                             /* OMNI mode on/off        */
     mnswt,                               /* Metronome switch        */
     mmode,                               /* Metronome mode cmd      */
     C2Hswt,                              /* Clock-to-Host switch    */
     ExtCtlSwt,                           /* External Ctl swt (3way) */

     perror_flag,                         /* Event error flag        */
     clikmult,                            /* Click multiplier 1/4:1/8*/
     time_base_cmd,                       /* MPU time base command   */
     start_bar_swt,                       /* Process measure offset  */
     chngdini,                            /* MLABPM.INI changed      */
     tracefull,                           /* Trace List Box is full  */

     sysflag1,                            /* Reset-able flag byte    */
      #define POSACTV   0x80              /* Song positioning active */
      #define SHUTDOWN  0x40              /* Shut MidiLab down       */

     actvfunc;                            /* Active function code:   */
                                          /*    1 - Record           */
                                          /*    2 - Playback         */
                                          /*    3 - Overdub          */
                                          /*    4 - Edit             */
                                          /*    5 - Capture          */
EXT SHORT
     rtrack,                              /* Active record track     */
     ptrack,                              /* Active playback track   */
     xtrack,                              /* Edit utility byte       */
     ztrack,                              /* Trk# for measurectr updt*/
     RunSTrk,                             /* Trk# to remember run st */
     actv_trks,                           /* Trks not yet completed  */
     deactv_trk,                          /* Trk deactivated for Odub*/
     trksolo,                             /* Track solo function     */

     measurectr,                          /* Measure counter         */
     max_measures,                        /* No. of measures in song */
     total_bars[NUMTRKS],                 /* Total bars in each track*/
     skip_bar_ctr,                        /* Measure positioning ctr */
     ending_measure,                      /* Last measure to play    */
     clocks_per_bar,                      /* Used in PMIDI measureend*/
     rec_measures,                        /* No. of bars to record   */
     rec_loop,                            /* No. of times to repeat  */
     quantize_amt,                        /* Quantize mode (1/4,1/8) */
     q_limit,                             /* Quantize tolerance      */
     q_flg1,                              /* Quantize control flags  */
     tbase_save,                          /* Save MIDI file time base*/
     cxClient, cyClient,                  /* Misc.  window parms     */
     grid_left, grid_bottom,
     grid_size, grid_content,
     grid_chn, grid_index, grid_trkname,
     TempoPos, TrnsPos, VelPos;           /* Scroll Bar positioning  */

EXT USHORT
     TmgChkDefer,                         /* Defer meas-end check    */
     InitTrks,                            /* Track Initialize Map    */
     starting_sppadj[NUMTRKS];            /* For measure positioning */

EXT LONG
     delta_time,                          /* Accumulated delta-time  */
     rec_deltime,                         /* Record delta time       */
     lnthdelta,                           /* Bytes added during save */
     ltime,                               /* Utility long integer1   */
     uindex,                              /* Utility index counter   */
     timing_total[NUMTRKS],               /* Delta-time accumulator  */
     starting_index[NUMTRKS],             /* For measure positioning */
     ClkIncr,                             /* Clock-to-Host increment */
     ClkAccum,                            /* Clock-to-Host accumulatr*/
     fsustain;                            /* Sustain pedal down flags*/

EXT ULONG
     start_time,                          /* Play/Record start-time  */
     end_time,                            /* Play/Record end-time    */
     ulFileAttribute,                     /* Song File Attributes    */
     MIDIoflgs,                           /* Song File Open Flags    */
     MIDImode,                            /* Song File Mode switches */
     BytesXfrd,                           /* Song File bytes xfered  */
     uart_time,                           /* Deltime in millisecs    */
     MlabCat,                             /* DevIOCtl catehory code  */
     *pulTimer,                           /* Pointer to hi-res timer */
     action;                              /* OPEN Action             */

EXT char
     ndxbfr[8],                           /* For dynamic index disply*/
     *filename,                           /* Ptr to song file name   */
     songpath[60],                        /* Song file search path   */
     songname[14],                        /* Song name storage       */
     msgbfr[90],                          /* Text message buffer     */
     szChars[20];                         /* Utility chars           */

EXT float
     ms_per_beat,                         /* Milliseconds per beat   */
     ms_per_tick;                         /* Milliseconds per pulse  */

EXT IOCPARMS
    IOCParms;                             /* DevIOCtl parameters     */

EXT HFILE
     INIfile,                             /* Initialization file     */
     HRT_hnd,                             /* Handle for hi-res timer */
     infile, outfile;                     /* MIDI song file blocks   */

EXT TID                                    /* Thread id's            */
     tid, tid1, tid2, tidU1, tidU2,tidFF, tidMet, tidClk;

EXT MRESULT
     mresReply;                           /* General result/reply    */

EXT HPS
     ml_hps;                              /* General HPS             */

EXT RECTL
     ndxrcl[NUMTRKS],                     /* For writing indices     */
     tccrcl[NUMTRKS],                     /* For writing trk/chn     */
     namrcl[NUMTRKS],                     /* For writing trk names   */
     mctrrcl,                             /* Measure counter         */
     beatrcl,                             /* Beat counter            */
     metrorcl,                            /* Metronome               */
     temporcl;                            /* Tempo                   */

EXT HEV
     SemClk,                              /* Metro Click semaphore   */
     SemSysx;                             /* Recv-Sysx semaphore     */

EXT HBITMAP
     hbm_metro1,                          /* Metronome handle (tick) */
     hbm_metro2;                          /* Metronome handle (tock) */

/*********************************************************************/
/*      MidiLab DLL Common Executable variables - Exported           */
/*********************************************************************/
EXT PCHAR
     P_metadata;                          /* Pointer to meta data    */

EXT BYTE
     safequit,                            /* Make sure nothing's lost*/
     trace,                               /* Debug trace switch      */
     iuo,                                 /* "Internal Use Only" swt */
     cmd_in_progress,                     /* MPU command switch      */
     vlqlnth,                             /* #Bytes in MIDI 1.0 vlq  */
     sysflag2;                            /* Permanent flag byte     */

EXT HAB
     hAB;                                 /* MidiLab PM anchor block */

EXT SHORT
     emode,                               /* EDIT mode switch        */
     skiptrks,                            /* Measure-skip flags      */
     cxChar, cyChar, cxCaps, cyDesc;      /* Font metrics            */

EXT HWND
     hWndClient,                          /* Client Window handle    */
     hWndETrk[NUMTRKS];                   /* Handles to edit windows */

EXT HFILE
     mpu_hnd;                             /* Dev drvr handle         */

EXT HEV
     SemEdit,                             /* Play seq semaphore      */
     SemStep;                             /* Step play semaphore     */

/*********************************************************************/
/*      MidiLab DLL Common Executable variables - Non-Exported       */
/*********************************************************************/
EXT ULONG
     ClipSize,                            /* Size of clipboard data  */
     Ndx_of_Mark;                         /* Index of 1st marked item*/

EXT SHORT
     bar_ctr,                             /* Measure counter         */
     tmg_ctr,                             /* Measure timing counter  */
     xindex[NUMTRKS],                     /* Top or only marked item */
     zindex[NUMTRKS],                     /* End of dragged block    */
     lastndx[NUMTRKS],                    /* Index of last list item */
     itemct[NUMTRKS],                     /* Number of marked items  */
     totlength[NUMTRKS],                  /* Length of marked items  */
     startblk[NUMTRKS],                   /* Start-of-block pointer  */
       #define Xndx xindex[ptrack]
       #define Zndx zindex[ptrack]
       #define Lndx lastndx[ptrack]
       #define Ict  itemct[ptrack]
       #define Tlnth totlength[ptrack]
       #define SBlk  startblk[ptrack]

               /* 16-bit Switches */
     editchng,                            /* EDIT changed something  */
     OverRun,                             /* ListBox overrun switches*/
     BadStart,                            /* First mark no good      */
     XeqSwt,                              /* Xeq selected event      */
     AutoOff;                             /* Turn selected notes off */

EXT USHORT
     Clpln,                               /* Accumulative Clpbrd lnth*/
     EdRow;                               /* Current row in View win */

EXT BYTE
     xferswt,                             /* Xfer insert switch      */
     hdrbfr[40],                          /* List Box header buffer  */
     PlayBanner,                          /* Swt to move list box bar*/
     SeqPlay,                             /* Play selected sequence  */
     Stepmode,                            /* Step Play sequence      */
     ClipShow;                            /* Clipboard display swt   */

EXT MRESULT
     mresReply;                           /* General result/reply    */

EXT PCHAR
     ClipPtr;                             /* Pointer to clipboard    */

EXT PVOID
     evaptr[NUMTRKS];                     /* Allocated memory ptrs   */

EXT HPS
     ed_hps;                              /* HPS for View window     */

EXT RECTL
     Edrcl;                               /* For View window         */

EXT TID
     PlayTid;                             /* TID for play-event thrd */

EXT HMQ
     hMQPSt;                              /* Handle to PSt msg queue */

EXT HWND
     hWndCLIP;                            /* Handle to Edit Clipboard*/

/*********************************************************************/
/*               Parameter block for Play-Sequence thread            */
/*********************************************************************/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
struct PST_ParmBlk
{
  BYTE  ptrack;                            // Play track
  HWND  hWnd;                              // Handle of edit dialog
#ifdef REALCSA
};
struct PST_ParmBlk PSt;
#else
} PST;
PST *PStptr;                              // Pointer to parm block map
extern struct PST_ParmBlk PSt;
#endif
/********************* End of PST Parameter Block ********************/

/***************************************************************************/
/* These structures are used to add the program name to the task list      */
/***************************************************************************/
EXT HSWITCH hSwitch;
EXT SWCNTRL Swctl;
EXT HELPINIT hiMLABHelp;     /* Help initialization structure           */
EXT HWND     hWndMLABHelp;   /* Handle to Help window                   */
EXT char     helpMLABWindowTitle;
EXT char     HelpMLABLibraryName;

EXT CHAR szAppName[20]; /* class name of application                    */
EXT CHAR szMLEDITAppName[30]; /* class name of child                    */

EXT HMQ  hMQ, HMQMain, HMQUART;  /* Handles to Message Queues           */
EXT HWND hWndFrame, hWndFILE, hWndEDITFrame, hWndCTLPANEL, hWndSYSEX, hWndTRACE,
         hWndSPECFUNC, hWndSTDBY, hWndButt[8], hWndFontDlg, hWndBIGBARS,
         hWndSongEditor;

/*********************************************************************/
/*         External Track #1 for exported file parameters            */
/*  This structure is written to disk as track #1 during a 1.0 save  */
/*********************************************************************/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
_Packed struct track_1
{
  CHAR t1_id[4];                          // "MTrk"
  LONG t1_lnth;                           // Length of this track

  BYTE tempo_prfx[4];                     // Tempo meta event prefix
  BYTE tempo[3];                          // Tempo value tttttt

  BYTE tsign_prfx[4];                     // Time Signature meta prefix
  BYTE tsig_num;                          // T. sig numerator
  BYTE tsig_end[3];                       // Remainder of event (fixed)

  BYTE end_of_trk[4];                     // Meta end of track 00FF2F00

#ifdef  REALCSA      // If not imbedded as a map in another module
};
_Packed struct track_1 t1 =                   // Structure initialization
          {
            "MTrk", 19,                       // Track chunk
            {0, 0xff, 0x51, 3}, {0,0,0},      // Tempo event
            {0, 0xff, 0x58, 4}, 4, {2,24,8},  // T.Sign event
            {0, 0xff, 0x2f, 0}                // End of Track event
          };
#else
} T1;
extern _Packed struct track_1 t1;
#endif
/********************* End of Track 1 data ***************************/

/*********************************************************************/
/*         External Track prefix for exported file parameters        */
/*  This structure is written to disk as the track prefix for all 8  */
/*********************************************************************/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
_Packed struct trk_prfx
{
  BYTE tnm_prfx[4];                           // Track name meta prefix
  CHAR tnm_name[TNML];                        // Track name text

  BYTE tch_prfx[4];                           // Channel meta prefix
  BYTE tch_chan;                              // Channel assignment

#ifdef  REALCSA      // If not imbedded as a map in another module
};
_Packed struct trk_prfx tpx =                 // Structure initialization
          {
            {0, 0xff, 0x03, TNML}, " ",       // Track name
            {0, 0xff, 0x20, 1}, 0             // Channel assignment
          };
#else
} TPX;
TPX *tpxptr;                              /* Pointer to mapped area  */

extern _Packed struct trk_prfx tpx;
#endif
/********************* End of Track prefix ***************************/

/*********************************************************************/
/*              MidiLab Song Profile    Lnth = 420  (8/31/95)        */
/*********************************************************************/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
_Packed struct song_profile               /* Song Profile structure  */
{
  LONG
    mlab_id,                              /* MidiLab file verifier   */
    trkdatalnth[NUMTRKS];                 /* Track content length    */
  SHORT
    tbase,                                /* System Time Base        */
    trkmask,                              /* Track adjustment mask   */
    playtracks,                           /* Selected play tracks    */
    spare_int1,
    spare_int2;
  BYTE
    tcc[NUMTRKS],                         /* Trk/channel correlation */
    ccswt,                                /* Continuous control swt  */
    qadjctr,                              /* Quantize selector cntr  */
    spare_array1[NUMTRKS];
  signed char
    transadj,                             /* Transposition factor    */
    velocadj;                             /* Velocity adjustment     */
  BYTE
    tempo,                                /* Tempo                   */
    tsign,                                /* Time signature          */
    proflags,                             /* Flag Byte               */
      #define EXTCNTRL     0x80           /* External Control on     */
      #define MTERROFF     0x40           /* Ignore meas timing errs */
      #define VELLEVEL     0x20           /* Velocity leveling on    */
    trkname[TNML][TNML+1],                /* Track names array       */
    lspare[31];                           /* Future expansion        */
#ifdef  REALCSA      // If not imbedded as a map in another module
};
_Packed struct song_profile sp = {MLAB_FILE_VERNEW,    // Revised 10/26/92
                         {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},   // Trk datalnth
                         0,0,0,0,0,  // spare1, tbase, trkmask, playtracks, spare2
                         {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},   // Trk/Chn correlation
                         0,0,0,                        // Filter, Sync, & Quantize
                         0,                            // Spare_array
                         0,0,100,4,0,255};             // tempo. etc.
#else
} SP;
SP *spptr;                                /* Pointer to mapped area  */
extern _Packed struct song_profile sp;
#endif
/********************* End of Song Profile ***************************/

/*********************************************************************/
/*               Event data packet structures  (24 bytes each)       */
/*                                                                   */
/*  For MidiLab track orders, the following redefinitions apply:     */
/*     evtype  -  Track Order code                                   */
/*     dbyte1  -  First adjustable byte                              */
/*     dbyte2  -  Second adjustable byte                             */
/*    in_chan  -  Flags for Trk Cmd                                  */
/*  trkcmddata -  For USHORT values where used                       */
/*********************************************************************/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
struct event_packet
{
  PCHAR
    Meta_ptr;                          // Pointer to meta event data

  LONG
    deltime;                           // Full delta-time

  USHORT
    MetaLn,                            // Meta event length
    trkcmddata;                        // Used by MidiLab track commands

  BYTE
    dbyte1,                            // Event data byte 2 (status)
    dbyte2,                            // Event data byte 3 (data 1)
    evtype,                            // Event channel status
    prev_sts,                          // Previous status
    evcode,                            // Raw event code
    length,                            // Length of this event
    in_chan,                           // Incoming channel number

    flags1,                            // Flag byte 1
     #define RUNSTAT   0x80            // Running Status
     #define METAEVT   0x40            // Meta Event
     #define MLABMETA  0x20            // MidiLab Meta Event
     #define DBYTES2   0x10            // Two-data-byte event
     #define MPUMARK   0x08            // MPU-401 Mark
     #define EOT       0x04            // End-of-track indicator
     #define EVAHEADR  0x02            // Event Header slot (NOTE ON, etc)
     #define EVASPACE  0x01            // Event array space slot

    flags2,                            // Flag byte 2
     #define LASTEVENT 0x80            // Last event called up for Edit

    spare[3];                          // Reserved

#ifdef REALCSA
};
struct event_packet evp[18];   // 1 packet per trk; plus 1 for MPU & 1 for Load
#else
} EVPK;
EVPK *EVPptr;
extern struct event_packet evp[18];
#endif
/********************* End of Event Packet ***************************/

/*********************************************************************/
/*                Track Block structures  (16 bytes each)            */
/*    These are the anchors for each of the tracks in the system     */
/*********************************************************************/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
struct track_block
{
  PCHAR
    trkaddr;                           // Pointer to track

  LONG
    index;                             // Track index

  USHORT
    SPTime;                            // SPP timing adjustment

  SHORT
    num_measures;                      // Number of measures in this track

  BYTE
    trkxferctr,                        // Track-Transfer counter
    restctr,                           // Measure rest counter
    flags1,                            // Track flags
      #define MPU       0x80           // MPU event
      #define TRACK     0x40           // Track event
      #define DISK      0x20           // 1.0 Disk File event
    spare;                             // Reserved

#ifdef REALCSA
};
struct track_block trk[NUMTRKS+2];     // Tracks; plus 1 for MPU & 1 for Load
#else
} TRKK;
extern struct track_block trk[NUMTRKS+2];
#endif
/********************* End of Track Block ****************************/

/*********************************************************************/
/*                MidiLab Initialization structure                   */
/*********************************************************************/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
struct Initialization
{
  LONG
    trksize[NUMTRKS];               // Track data sizes

  CHAR
    play_ctlev[3],                  // Remote Control event- PLAY
    retake_ctlev[3],                // Remote Control event- Re-take
    multi_ctlev[3],                 // Remote Control event- Pause/Start/Stop

    play_disp[18],                  // Text for play event
    retake_disp[18],                // Text for re-take event
    multi_disp[18],                 // Text for multi event
    filler1;                        // For CHAR alignment

  BYTE
    RmtCtlS1,                       // Remote Control flag byte 1
      #define RMT_CTL_DISABLD 0x40  // Remote control disabled
      #define CAPTURE_ARMED   0x20  // Waiting to capture event

    RmtCtlS2,                       // Remote Control Switch byte
      #define CAPTURE_PLAY    0     // Capture Play selected
      #define CAPTURE_RETAKE  1     // Capture Re-take selected
      #define CAPTURE_MULTI   2     // Capture multi selected

    acc_chn_lo,                     // Acceptable chns 1 - 8
    acc_chn_hi,                     // Acceptable chns 9 - 16
    indexdispl,                     // Display Track index

    iniflag1,                       // General flag byte 1
      #define MTEXT           0x80  // Display meta text during load
      #define MTHRU           0x40  // MIDI-THRU and OMNI mode
      #define PLAYLOOP        0x20  // Repeat Playback sequence
      #define EDITGRID        0x10  // Song Editor Window grid lines
      #define UARTMODE        0x08  // Run MPU-401 in UART mode
      #define METROMID        0x04  // Send Metronome clicks to MIDI OUT

    iniflag2,                       // General flag byte 2
    leadin_bars;                    // No. lead-in bars times 2

  LONG
    EfgndClr,                       // Song Editor foreground color
    EwndClr,                        // Song Editor window color
    EbarClr,                        // Song Editor moving bar color
    MfgndClr,                       // Main window foreground
    MwndClr;                        // Main window color

  ULONG
    MetNote,                        // Metronome MIDI note
    MetChn,                         // Metronome MIDI channel
    MetDur,                         // Metronome click duration
    ulSpare[30];                    // Future expansion

  SWP
    bigbars,                        // Positioning for Big Bars
    MainPos,                        // Positioning for Main window
    EditPos;                        // Positioning for Song Edit window

  FONTDLG
    fontDlg;                        // Font dialog info structure

  FONTMETRICS
    fm;                             // Structure for determing font sizes

  LONG
    mlab_id;                        // MidiLab identifier
#ifdef REALCSA
};
struct Initialization ini = {   // Set defaults:
            {40000, 40000, 40000, 40000, 40000, 40000, 40000, 40000,
             40000, 40000, 40000, 40000, 40000, 40000, 40000, 40000},
            {144,36,64}, {144,38,64}, {144,40,64}, // Events
            "C1/36 V:64", "D1/38 V:64", "E1/40 V:64",
            'F',                // Filler1
            0x40,               // RmtCtlS1
            0,                  // RmtCtlS2
            255,255,            // Acceptable channels
            ON,                 // Index Display
            OFF,                // INI Flag Byte 1
            OFF,                // INI Flag Byte 2
            2,                  // Lead-in measures (really one)
            CLR_BLACK,          // Edit foreground
            CLR_BACKGROUND,     // Edit window
            CLR_RED,            // Edit moving bar
            CLR_BLACK,          // Main foreground
            CLR_BACKGROUND,     // Main window
            45,                 // Metro MIDI note
            10,                 // Metro MIDI channel
            75,                 // Metro click duration

                            };  // End of struct initialization
#else
} p_INI;
p_INI *INIptr;
extern struct Initialization ini;
#endif
/********************* End of Event Packet ***************************/

/*********************************************************************/
/*               Parameter block for MidiLab DLL                     */
/*********************************************************************/
#ifndef REALDPm      // If imbedded as a map in another module
typedef
#endif
struct DLL_ParmBlk
{
  PVOID *EventPkt;                         // Pointer to track's event packet
  PVOID *SongPrfl;                         // Pointer to song profile
  PCHAR SongName;                          // Pointer to song name
  SHORT EStrtMeas;                         // Starting measure of edited track
  SHORT ENumMeas;                          // Number of measures to edit
  BYTE ETrack;                             // Number of track being edited
#ifdef REALDPm
};
struct DLL_ParmBlk DPm;
#else
} DPM;
DPM *DPmptr;                              // Pointer to parm block map
extern struct DLL_ParmBlk DPm;
#endif
/********************* End of DLL Parameter Block ********************/

/*********************************************************************/
/*       MidiLab MIDI 1.0 File Format and associated elements        */
/*********************************************************************/
/*--------------------------- Header Chunk --------------------------*/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
_Packed struct hdrchunk
{
  char hdrid[4];                       /* Header chunk ID            */
  long hdrlnth;                        /* Header length      ( 6 )   */
  SHORT
    hdrfmt,                            /* Header format      ( 1 )   */
    hdrntrks,                          /* Number of tracks   ( 17)   */
    hdrtdiv;                           /* Time division      (120)   */
#ifdef REALCSA
};
_Packed struct hdrchunk MThd = {"MThd", 6, 1, 17, 120};
#else
} MTHD;
MTHD *p_mthd;
extern _Packed struct hdrchunk MThd;
#endif

/*--------------------- Track Chunk prefix --------------------------*/
#ifndef REALCSA      // If imbedded as a map in another module
typedef
#endif
_Packed struct trkchunk
{
  char trkid[4];                       /* Track chunk ID             */
  long trklnth;                        /* Track chunk length         */
#ifdef REALCSA
};
_Packed struct trkchunk MTrk = {"MTrk", 0};
#else
} MTRK;
MTRK *p_mtrk;
extern _Packed struct trkchunk MTrk;
#endif

/*--------------------- Misc file variables -------------------------*/
#ifdef REALCSA
BYTE
  Mlabid[3] = {0,10,27},                    /* MidiLab "mfg's" id    */
  meta_eot[3] = {0xff,0x2f,0},              /* Meta end-of-track     */
  trkcmd_meta[6] = {0xff,0x7f,9,0,10,27},   /* Meta prfx for trk cmds*/
  tempo_meta[7] = {0,0xff,0x51,3,0,0,0};    /* Tempo meta event      */
#else
extern BYTE
  Mlabid[3], meta_eot[3], trkcmd_meta[6], tempo_meta[7];
#endif
/********************* End of MIDI 1.0 File Format *******************/
