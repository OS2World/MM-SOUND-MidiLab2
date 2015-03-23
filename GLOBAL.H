/*********************************************************************/
/*      MidiLab/2 - MIDI Sequencer/Editor for the OS/2 Platform      */
/*                                                                   */
/*               GLOBAL.H -    ** System-wide header **              */
/*                                                                   */
/*            Copyright  J. L. Bell        August 1995               */
/*                                                                   */
/* Return codes from driver calls:                                   */
/*              FFF1 - Invalid command                               */
/*              FFF2 - Command already in progress                   */
/*              FFF3 - Command simulated in UART mode     6/15/94    */
/*              FFF4 - UART mode is ending (in response to MPU reset)*/
/*              FFF5 - Command discarded by driver        12/17/94   */
/*              FFF6 - Invalid function code to driver     8/1/95    */
/*              FFF7 - Invalid category code to driver     8/2/95    */
/*              FFF8 - Verify Access failed in driver      8/2/95    */
/*              FFF9 - Attempt to re-enter UART mode       8/3/95    */
/*              FFFA - Cannot decode MIDI event            8/3/95    */
/*              FFFB - MPU_RESET timeout                  11/16/95    */
/*              FFFB - MPU_UART timeout                   11/17/95   */
/*    Read codes: 00--; IOCTL codes: FF--                            */
/*********************************************************************/
#define MLAB_CAT     0x90         // Category for MidiLab DevIOCtl
#define MPU_CMD      0x01         // DevIOCtl: Execute MPU command
#define SEND_DATA    0x02         // DevIOCtl: Send outbound MIDI
#define SEND_SYSX    0x03         // DevIOCtl: Send outbound SysEx
#define BLK_DELAY    0x24         // DevIOCtl: General Block/Delay request
#define TESTLOOP     0x25         // DevIOCtl: MWAVE Loopback test
#define DEL_TIME     0x26         // DevIOCtl: Delta Time block request

#define CMD_ACK      0xfe         // MPU Command Acknowledgement
#define MPU_UART     0x3f         // MPU UART Mode Command
#define MPU_TEMPO    0xe0         // MPU Set tempo Command
#define MPU_RESET    0xff         // MPU Reset Command
#define DSR          0x80         // Data Set Ready
#define DRR          0x40         // Data Receive Ready

/* MidiLab functions            */
#define RECORD       1
#define PLAY         2
#define OVERDUB      3
#define EDIT         4
#define CAPTURE      5

/* MIDI/MPU-401 definitions     */
#define NOTE_OFF     0x80
#define NOTE_ON      0x90
#define POLY_AFTER   0xa0
#define CONTROLLER   0xb0
#define PGM_CHANGE   0xc0
#define AFTER_TOUCH  0xd0
#define PITCH_BEND   0xe0
#define CLK_TO_HOST  0xfd
#define SYSX         240             /* $F0-1st byte of sysex data   */
#define TIMING_OVFLO 248
#define NOP          248
#define MEASURE_END  249
#define END_OF_TRK   252
#define EOX          247

/***************************************************************************/
/*            Parameter block for DevIOCtl functions                       */
/***************************************************************************/
typedef struct
{
  long  ulmsTime;         // Delta Time in milliseconds
  long  ulBufrLn;         // Data Buffer length
  short usCParm1;         // Reserved
  short usCParm2;         // Reserved
  UCHAR ucCParm1;         // Reserved
  UCHAR ucChnSt;          // Channel status
  UCHAR ucMPUCmd;         // MPU-401 Cmd byte or MIDI byte 1
  UCHAR ucCmdData;        // MPU-401 Cmd data or MIDI byte 2
} IOCPARMS;
typedef IOCPARMS far *PIOCPARMS;
