                              MidiLab/2
                            _____________

 (c) Copyright 1995-96, Far Pavilions Studio, All rights reserved.

Unregistered copies of MidiLab/2 are freely distributable and copying is
encouraged.  If you like it, become a registered user by registering via
Compuserve (GO SWREG) The program id is 9510; registration fee is $25 (US).
Non-Compuserve users may contact the author directly at the address listed
at the end of this file.

MidiLab/2 is supplied "as is" without any warranties either expressed or
implied. Certain functions appearing in the user interface controls are
present only as 'place-holders' and may never be implemented.

IN NO EVENT IS THE AUTHOR LIABLE TO YOU FOR ANY INCIDENTAL, SPECIAL,
PUNITIVE, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ARISING FROM OR CONNECTED WITH
THIS AGREEMENT OR USE OF THIS PRODUCT, REGARDLESS WHETHER WE KNOW OR HAVE
REASON TO KNOW OF THE POSSIBILITY OF SUCH DAMAGES.  SOME STATES MAY NOT ALLOW
EXCLUSION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES AND SO THE FOREGOING
EXCLUSION MAY NOT APPLY TO YOU.


********************************************************************************
                 ** MidiLab/2 Installation and Notes **
********************************************************************************
IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT

The High Resolution Timer package is included (TIMER0.ZIP) and must be
installed before attempting to run MidiLab/2.  See the README.TMR file for
installation instructions.

OS/2 Base Multimedia Software Support must be present.  This can be done
via OS/2 Selective Install.
********************************************************************************

MidiLab/2 must be installed within an OS/2 Warp environment, in its own
directory.

Installation instructions:

1. Create a directory for MidiLab/2 on a drive of your choice.  The following
   examples and instructions assume the directory is named MLABPM, and it is
   set as the current directory.

2. After copying MLABVxxx.EXE into the MLABPM directory, simply execute it.
   It's a self-extracting ZIP file, and will install all required files.

3. Create a MidiLab/2 icon by dragging a program template to an existing folder
   (or to the desktop) specifying the following settings:

   Program
     - Path and file name: [d:]\MLABPM\MLABPM.EXE
     - Parameters: /P=[d:]\songlib
     - Working directory: [d:]\MLABPM
   General
     - Title: MidiLab/2

   The optional /P parameter is used to specify a default directory for loading
   and saving MIDI data files.  Extensions of .MID (standard MIDI format 1) and
   .ML (MidiLab/2 native format) are supported.
   See 'Invoking MidiLab/2' under the Help Index for other optional parameters.

4. Add a DEVICE statement to CONFIG.SYS for the MidiLab/2 MPU device driver:
      DEVICE=[d:]\MLABPM\MIDILAB.SYS ii aaa
   where:
    ii  is the interrupt for which the MPU hardware is wired. Default is 5.
    aaa is the port address for your MIDI hardware device. Default is hex 330.
    These values must be entered as hex numbers. An example of an installation
    with IRQ 10 and Address of hex 330 would be:
                     DEVICE=C:\MLABPM\MIDILAB.SYS A 330

    NOTE:  MidiLab/2 will run in a 'demo' mode if you don't have the MIDI device
    hardware.  The play and record functions will be disabled, but the file
    manager, editor, and most of the dialog boxes and windows will be available.
    The DEVICE statement is not required in this case, but a parameter of /Z
    should be used to avoid warning messages.

    For a user without the required hardware, this mode can still provide some
    useful facilities, such as examining, editing, or even creating MIDI files.

5. After re-booting your system, MidiLab/2 will be ready to run by mousing the
   MidiLab/2 icon created in step 3.

   A sample songfile created by MidiLab/2 is included, to be placed
   in a directory of your choice. This songfile is NOT a standard general MIDI
   file.  It is highly unlikely that it will play with the correct voicing on
   your synthesizer; however, it provides you the opportunity to familiarize
   yourself with some of MidiLab/2's functions to voice the tracks properly.

------------------------------------------------------------------------------
The online HELP contains the documentation and description of the product,
as well as an introductory MIDI tutorial. Everything (I hope) you need to know
is contained in the help support.  Most, if not all, dialogs and pull-downs
have at least some basic verbage on their functions.

IMPORTANT: MidiLab/2 is designed for a high-resolution display (1024 X 768).
           You may experience some problems achieving display device
           independence.  If the MidiLab/2 main window is incorrectly
           positioned when first started, reposition it with the mouse (or
           Alt-F7) and then Save Preferences.  This will save the current
           window position, and it will be subsequently positioned correctly.

           A related problem is that some of the dialog box text may be
           truncated.  Dragging a Helv 8 font from the Font Palette to the
           affected dialog box may correct the problem.

------------------------------------------------------------------------------
Release 3.03 notes:   February 1996
- Allow loading MIDI files from CD ROM.
- Corrected errors occuring during loading files with invalid tempos
- Improved performance of Fast-Forward operation
- Broader device driver support of non-MPU hardware.

Release 3.02 notes:   December 1995
- Support of MPU-401 compatible devices in UART mode.
- Misc. bug fixes and code restructure

------------------------------------------------------------------------------
Miscellaneous:

  - If errors or system hangs occur immediately following installation, the
    most likely cause is that your hardware does not support/emulate the
    MPU-401, or another application (OS/2, DOS, or WINDOWS) has control of
    the MIDI device. Also, insure the correct IRQ setting and address for the
    MPU hardware and/or the DEVICE statement in CONFIG.SYS.

  - Do not attempt to run two applications which access the same MIDI
    hardware at a time. One must be deactivated in order to use the other.
    It may be necessary to take WIN/OS2 down in some cases. In rare cases it
    may help to REM out any CONFIG.SYS statements (other than MidiLab's) which
    reference the MIDI device.

  - If you get the message 'The system cannot find the file SW', it's
    because OS/2 Base Multimedia Support is not present. See 'IMPORTANT' above.

  - MidiLab/2 supports a maximum of sixteen virtual tracks, and will load only
    the first seventeen tracks of a Format 1 MIDI file containing more than 17.
    The very first track is assumed to contain setup information such as
    tempo, etc., and does not appear in the track list.

  - MidiLab/2 will NOT operate with an on-board DSP synthesizer. It is designed
    to work with outboard MIDI devices, such as keyboards, drum machines, etc.

------------------------------------------------------------------------------

                                 James L. Bell
                              Far Pavilions Studio
                                  PO Box 2314
                            Saratoga, Ca. 95070-0314

                       E-mail: 71034.3001@compuserve.com
                              Tel: (408) 378-9649

                    Take a few moments to visit my Web Page:
                http://ourworld.compuserve.com/homepages/JLBell

