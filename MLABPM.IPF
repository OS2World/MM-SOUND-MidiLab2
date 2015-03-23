.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*   MLABPM.IPF -  ** MidiLab Help Text **                           */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
:userdoc.
:prolog.
:title.MidiLab/2 Help
:eprolog.
:body.
:h1 res=1.MidiLab/2
:p.:hp2.Important-:ehp2. See :link refid=SWREG reftype=hd.Registration
Information:elink. for shareware information and disclaimers.
:p.MidiLab/2 is a sixteen-track MIDI data manager with built-in edit and file
management capabilities for use in recording, editing, and playing a musical
performance with MIDI compatible equipment.

:p.MidiLab/2 features four primary functions: Record, Overdub, Playback, and
Track Edit. These, and the other supporting functions, are selected from the
main window controls.
:p.MidiLab/2 supports the basic functions of the MIDI 1.0 File Specification,
with a few exceptions noted in the
:link refid=MIDIFMT reftype=hd.MIDI File Format:elink. section.  There
are no special hardware nor software requirements other than a Roland
MPU-401/IMC/IPF MIDI Processing Unit (or compatible equivalent), and the
appropriate outboard MIDI sound equipment.  MidiLab/2 runs under OS/2 Version
3.0 or later.
:p.While a musician unfamiliar with computers should be able to quite easily
use MidiLab/2 as a multi-track recorder, its full capability cannot be
realized without a reasonable knowledge of MIDI concepts and terminology, as
well as a basic knowledge of general computer operation.  The reader is
assumed to have some familiarity with MIDI-compatible equipment, and to have
made at least a perfunctory scan of the MPU-401 reference information to be
aware of its role in a MidiLab/2-based music system.
:p.For details of the MidiLab/2 main window, select from the list below&colon.
:ul compact.
:li.:link refid=SWREG reftype=hd.Registration Information:elink.
:li.:link refid=TRACK_DISPLAY reftype=hd.Track List:elink.
:li.:link refid=PLAY_BUTTON   reftype=hd.Play:elink.
:li.:link refid=VCR_BUTTON    reftype=hd.FastFwd:elink.
:li.:link refid=PAUSE_BUTTON  reftype=hd.Pause:elink.
:li.:link res=1500            reftype=hd.Record:elink.
:li.:link res=1500            reftype=hd.Overdub:elink.
:li.:link refid=RETAKE_BUTTON  reftype=hd.Re-Take:elink.
:li.:link refid=NOTES_OFF_BUTTON  reftype=hd.Notes-Off:elink.
:li.:link refid=SYSRESET_BUTTON  reftype=hd.Sys Reset:elink.
:li.:link refid=BTM reftype=hd.Beat, Tempo, and Measure windows:elink.
:li.:link res=2200  reftype=hd.Track Editor:elink.
:p.For basic MIDI concepts select&colon.
:ul compact.
:li.:link refid=MIDI_TUTORIAL reftype=hd.MIDI Tutorial:elink. :eul.
:eul.
.*
:h1 id=TRACK_DISPLAY nosearch.Track List
:p.This main part of the display shows information related to the status of the
sixteen tracks.  The data represents track size, current content, channel
assignment, index value, and name.  The File Name is presented in the
window title. The file name will remain anonymous (untitled) until you assign one
during a Save operation.
:dl break=all.
:dt.:hp2.Size:ehp2.
:dd.This column shows the size (capacity) of each track.  This size is stored in
MLABPM.INI and can be changed by double-clicking on the size field you wish to
modify.  Note that once it is modified, MidiLab/2 must be restarted in order for
the change to take effect.
.*
:dt.:hp2.Content:ehp2.
:dd.This column shows the current contents of each track, in bytes.  At a minimum,
each track will contain the MPU end-of-track mark (252) preceded by a timing
byte, so its content will always be at least 2.
:p.Double-clicking this field will invoke the Track Edit facility.
:dt.:hp2.Ch (Track-to-Channel Assignment):ehp2.
:dd.The MIDI channel to which each track is assigned, is displayed in this
column.  At start-up time, each track is assigned a default channel number
of '0'.
:p.Double-clicking this field will provide the means to assign a channel.

:dt.:hp2.Index:ehp2.
:dd.This column displays the sixteen track index counters.  These counters are
used internally to control the positioning within each track, and externally
to provide you a real-time view of track activity, to monitor the playback and
record activity.  This can be especially useful in determining how much track
space remains during recording, or, while play is in progress, to assist in
locating an area of a track for editing.  Keep in mind that this column
represents the value of the index counters, and not the contents of the
tracks.
:p.Double-clicking this field will allow you to completely erase the track.
.*
:dt.:hp2.Name:ehp2.
:dd.This column is provides space for assigning your own titles or
names to each track.  You may enter up to 16 characters per each name.
These fields are also set by MIDI Meta-events 3 and 4 if they are
encountered during playback or file load.
:p.Double-clicking a field will provide the means to name a track.
:edl.
.*
:h1 id=BTM nosearch.Beat, Tempo, and Measure windows
:p.During record and play, the beat count is displayed, with the first beat
of a measure highlighted.  Additionally, a visual metronome is displayed to
help you keep track of the tempo.
:p.The measure counter tells you which measure you are in as you record or
playback.
.*
:h1 id=PLAY_BUTTON   nosearch.Play Pushbutton
:i1.Play
.im mlabplay.ipf
.*
:h1 id=VCR_BUTTON    nosearch.FastFwd Push Button
:p.This button is used to rapidly advance through the song file, measure by
measure, as long as the button is held down.  When the button is released,
playback will start (or resume) at the currently positioned measure.
:p.The measure indicator is updated as fast-forwarding is in progress.
.*
:h1 id=PAUSE_BUTTON  nosearch.Pause Pushbutton
:p.Used to interrupt playback. If pressed again, playback will resume at
the point of interruption.
:note.This button function is duplicated on the MidiLab/2 main control
panel.
.*
:h1 id=RETAKE_BUTTON  nosearch.Re-Take Pushbutton
:p.This button facilitates previous track and option selections.  When it is
pushed, a short-cut takes place and whatever Record/Overdub setup you used
previously, including the track selection, number of measures to record, and
times to repeat will be automatically placed in effect.  This will provide
you with a quick restart capability when re-recording a track.
:note.This button function is duplicated on the MidiLab/2 main control
panel.
.*
:h1 id=NOTES_OFF_BUTTON  nosearch.Notes-Off Pushbutton
:p.This button is available if one or more notes or other MIDI events become
"stuck" during play or edit operation.  The most common example of this is
when a Note-On event is issued without a corresponding Note-Off. In addition
to issuing the MIDI "ALL NOTES OFF" command, it also issues a
sustain-release to all channels in case a sustain-down is inadvertently
left active.
:note.This button function is duplicated on the MidiLab/2 main control
panel.
.*
:h1 id=SYSRESET_BUTTON  nosearch.Sys Reset Pushbutton
This button will cause a general reset to the entire MidiLab/2 system, and
return it to the same state as when it was initially loaded.  The only
settings that are unaffected are the MIDI Format, MIDI-Thru, External Control,
and 8th-Note Click.
:note.This button function is duplicated on the MidiLab/2 main control
panel.
.*
:h1 id=SWREG nosearch.Registration and disclaimers
:hp1.MidiLab/2:ehp1.
:p.(C) Copyright 1995-96, Far Pavilions Studio, All rights reserved.

:p.Unregistered copies of MidiLab/2 are freely distributable and copying is
encouraged.  If you like it, become a registered user by registering via
Compuserve (GO SWREG) The program id is 9510; registration fee is $25 (US).
Non-Compuserve users may contact the author directly at the address listed
under the :hp2.Product Information:ehp2. pulldown.

:p.MidiLab/2 is supplied "as is" without any warranties either expressed or
implied. Certain functions appearing in the user interface controls are
present only as 'place-holders' and may never be implemented.

:p.IN NO EVENT IS THE AUTHOR LIABLE TO YOU FOR ANY INCIDENTAL, SPECIAL,
PUNITIVE, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ARISING FROM OR CONNECTED WITH
THIS AGREEMENT OR USE OF THIS PRODUCT, REGARDLESS WHETHER WE KNOW OR HAVE
REASON TO KNOW OF THE POSSIBILITY OF SUCH DAMAGES.  SOME STATES MAY NOT ALLOW
EXCLUSION OF INCIDENTAL OR CONSEQUENTIAL DAMAGES AND SO THE FOREGOING
EXCLUSION MAY NOT APPLY TO YOU.
.*
:h1 res=1000.Product Information
:p.This panel shows general information about MidiLab/2.
.*
:h1 res=1100.File Manager
:i1.File Manager
.im mlabfile.ipf
.*
:h1 res=1200.Main Control Panel
:i1.Main Control Panel
.im mainctlp.ipf
.*
:h1 res=1300.Utilities - Bulk Dump/Restore
:i1.System exclusive
:i1.Bulk Dump/Restore
.im mlabsysx.ipf
.*
:h1 res=1400.User Preferences
:i1.User preferences
.im scndctlp.ipf
.*
:h1 res=1500.Set up Record/Overdub
:i1.Record
:i1.Overdub
.im recdovdb.ipf
.*
:h1 res=1600.Utilities - Send Program Change
:i1.Program Change
:p.Enter the program or voice number (1 thru 128), select the MIDI channel
to which it is intended, and then press :hp2.Send:ehp2..
.*
:h1 res=1800.Channel Assign
:i1.Channel assignments
:p.The MidiLab/2 track to channel assignment facility, invoked by mousing the
:hp2.Ch:ehp2. column on the main display, can be applied to a variety of
recording situations.  When MidiLab/2 is started, all channel assignments are
set to zero.  This means that no 're-mapping' is done during recording and/or
playback.
:p.As an example, if track 4 is assigned to channel 12, any information
recorded on track 4 will be forced (permanently) to channel 12.  During
Playback, however, regardless on what channel the track was recorded, it
can be temporarily re-mapped to any other channel you wish, again using the
:hp2.ASSIGN:ehp2. control.  Each of the sixteen tracks can be assigned to any
of the 16 MIDI channels that your equipment can be set up to receive.  Also,
more than one track can be assigned to the same channel for additional merging
capability.
:note.Meta-event $20 will assign channels automatically if encountered
during playback.
:p.The assignments are stored in the song profile, and recalled and
activated whenever a song file is loaded.
.*
:h1 res=1900.Name Track
:i1.Track names
:p.This control, invoked by mousing the track name field on the main display,
allows you to apply a name of your choice, of up to 16 characters, to each of
the sixteen tracks.  It performs no internal function, but serves as a helpful aid
in identifying the track contents as you are composing or recording.  The names
are saved along with the song and will remain there until the track is erased.
:p.The track name fields are also set by MIDI Meta-events 3 and 4 if
encountered during playback.
.*
:h1 res=2000.Set MPU Trace options
:i1.Trace, Diagnostic
:p.Data and commands to and from the MIDI interface may be traced during
operation.  Select one or more of the options to provide a trace of the
selected activity.  Certain lines of the trace are preceded by a special
symbol to aid in identifying the type of trace.
:p.The options and their symbols, where applicable, are&colon.
:dl break=all.
:dt.:hp2.Commands ():ehp2.
:dd.MPU commands issued by MidiLab/2 are displayed as they are executed.
:dt.:hp2.Command ACKs ():ehp2.
:dd.The acknowledgement of a previously issued command is displayed.
:dt.:hp2.Outgoing data ():ehp2.
:dd.MIDI data going to the MPU is displayed as a series of MIDI events,
consisting of the timing byte and channel information.
:dt.:hp2.Incoming data:ehp2.
:dd.This checkbox has three states:
:ol.
:li.:hp2.Off - :ehp2.No input trace.
:li.:hp2.Checked () - :ehp2.MIDI data arriving from the MPU is displayed as a
series of MIDI events, consisting of the timing byte and channel information.
:li.:hp2.Filled - :ehp2.The interface to the device driver is monitored.
This shows the the number of MIDI bytes delivered, the return code from the
DosRead, and the buffer high water mark (HWM) indicating the largest number of
data bytes yet delivered per single call.
:eol.
:dt.:hp2.Clock pulses ():ehp2.
:dd.The timing clock pulses used to synchronize Play/Record functions are
displayed during operation.  This trace is used primarily for test and debug
functions.
:edl.
:p.A checkmark appears next to your selection when the selection is active.
.*
:h1 res=2200.MidiLab/2 Editor
:i1 id=mlabedit.Editor
.im mlabedit.ipf
.*
:h1 res=3100.Utilities - Remote Control
:i1.Remote Control
:p.Selected MidiLab/2 functions can be controlled from a remote MIDI device.
The functions of Play, Pause, Re-take, Start-record, and Stop-record can be
invoked by any standard MIDI function such as a key, sustain pedal, program-change
button, etc.
:p.All of the Remote Control functions are located on this panel.
Select from the following list for a detailed description of each item:
:ul compact.
:li.:link refid=CTLEV    reftype=hd.Functions and Events:elink.
:li.:link refid=CAPTURE  reftype=hd.Capture:elink.
:li.:link refid=DISABLE  reftype=hd.Disable Remote Control:elink.
:li.:link refid=DONE     reftype=hd.Done/Cancel:elink.
:eul.
.*
:h1 res=3300.Set Track Size
:i1.Track Size
:p.The track size may be changed to accomodate larger then normal files, or
to provide a finer control on storage usage. Enter the number of bytes you
want the track to hold.  The range of the number must be from 1000 to 999999.
:p.After the change is made, MidiLab/2 must be restarted to allow the new
value to be placed in effect.
.*
:h1 id=CTLEV   nosearch.Functions and Events
:p.This group of buttons is used to set up a control event for capturing.
Select the one you wish to set up and then select :hp2.Capture:ehp2..
A description of the associated MIDI event is displayed adjacent to the button.
:p.:hp2.Play:ehp2. and :hp2.Re-take:ehp2. are identical in function to the
push buttons in the
MidiLab/2 main window; however, :hp2.Pause/Start/Stop:ehp2. serves as a
multifunction control. If a song is playing, the normal pause function
is invoked. If Record/Overdub standby is in effect, the control will cause
the record count-down to begin.  Finally, during recording, the control will
cancel the record function.
:h1 id=CAPTURE nosearch.Capture
:p.To capture an event to be used as a control event, select the radio button
corresponding to the desired function and then Press :hp2.Capture:ehp2..
Then press the key or control button on the MIDI device you wish to use. A
description of the captured event will appear adjacent to the selected button.
:p.From this point on, whenever the captured MIDI event is received by
MidiLab/2, the corresponding MidiLab/2 function will be invoked. (assuming the
remote control function is not disabled)  Pressing the button again will cancel
the capture operation.
:note.The :hp1.velocity:ehp1. data byte for a :hp2.NOTE-ON:ehp2. event is not
examined, unless it is a velocity of zero, indicating a note_off.
In all other cases, the incoming event must identically match the
captured event.
:h1 id=DISABLE nosearch.Disable Remote Control
:p.The Remote Control function is disabled when this box is checked. If
MidiLab/2 is idle, no incoming data will be accepted.
:h1 id=DONE    nosearch.Done/Cancel
:p.Select :hp2.Done/Cancel:ehp2. to exit Remote Control setup. If a capture is
pending, it will be cancelled.
.*
:h1 res=999.Keys Help
:p.The following keys can be used while the MidiLab/2 main window has the focus:
:ul.
:li.F2 - Toggle MIDI-THRU
:li.F3 - Exit MidiLab/2
:li.F4 - Invoke Song Editor
:li.F5 - Clear and Reset MidiLab/2
:li.F6 - Toggle metronome
:li.F7 - Main Control Panel
:li.F8 - User Preference settings
:li.F9 - File Control Panel
:li.G - Send Program Change
:li.J - Display jumbo measure indicator
:li.O - Overdub
:li.P - Play
:li.B - SysEx bulk data utility
:li.R - Record
:li.S - Save current song
:li.Numeric 1 thru 8 - Invoke Track Edit (trks 1-8 only)
:eul.
.*
:h1 res=1001.Disk File Access
MidiLab/2 provides a service to allow access to song files as they are
stored on disk.  A scrollable window is presented with a list of all files
within a directory which you specify.  A file may be selected for loading, or
it may also be erased or renamed.  The window is also called by pressing
Function key 9.
.*
:h1 res=1050.Save new file
.*
:p.This resets the system in preparation for a new song.  All controls and
heading are set to their initial state.  It performs the same funtion as the
'Reset' push button on the main window.
.*
:h1 res=1101.Load or Save song file
:p.Select a file/directory combination first, then double-click on the file
you wish to load or save.  Or, type in the file name at the top of the box and
select the required button.  Once a file is selected, it may also be saved or
erased by selecting the appropriate push buttons.
:h1 res=1102.Save song on disk
Saves the track data and song profile in a file on disk.  The path preceding the
search pattern currently in effect is used to determine where a song file will
stored. Any valid file name may be used, up to eight characters.  The extension
will be set to ".MID" for standard MIDI files, and ".ML" for native MidiLab/2
format.
:p.If a file by the supplied name already exists, you will receive a
confirmation prompt.  If you reply affirmative, it is replaced.
.*
:h1 res=1350.Terminate and Exit
:p.This will exit MidiLab/2, first asking you to confirm the request if there
is a possibility of losing any unsaved data.
.*
:h1 res=2050.Song Edit Window
:i1.Song Edit
:p.The Song Editor window is used to display the tracks and measures associated
with the current.  Individual measures can be selected for detailed editing of
track data.
.*
:h1 res=3000.MidiLab/2 Control Panel
:p.This is the control panel for all general switches and MIDI controls.
.*
:h1 res=3050.Main Controls
:p.All of the main MidiLab/2 control functions are located on this panel.
Select the 'Help' button for more information.
.*
:h1 res=4000.Utilities
:p.This pull-down contains functions which, generally speaking, are not part
of the mainstram MIDI processing, but which have utility value in helping
you navigate your way thru some of the more basic functions.
.*
:h1 res=4050.Bulk Dump/Restore
:p.MidiLab System Exclusive data dump and restore functions.  Use the HELP
pushbutton for more information.
.*
:h1 res=4100.Utilities - Toggle Index Display
:i1.Index Display
:p.This switch allows the automatic display of the track indexes during
playback operation.  The main reason for having them switchable is because the
index display represents a significant processing overhead, and in the case of
some note-intensive songs, this could be a detriment to performance.
.*
:h1 res=4150.Program Change
:p.Used to send a MIDI program change. Use the dialog HELP
pushbutton for more information.
.*
:h1 res=4300.Remote Control
:p.This function permits MidiLab to be controlled from a remote MIDI device.
.*
:h1 res=4352.Utilities - Jumbo Measures
:i1.Jumbo Measures
:p.This switch causes a large measure indicator to be displayed for ease of
viewing when working away from the computer. The current measure being played
is shown, along with the total number of measures in the file.
:note.The total number of measures is derived from the longest of all tracks.
.*
:h1 res=4354.Utilities - Trace
:i1.Trace
:p.Allow tracing MIDI data entering or leaving the system.
.*
:h1 res=4356.Utilities - User preference settings
:i1.User settings
:p.Allow user to set personal options
.*
:h1 res=7777.Command Line parameters and switches
:i1.Invoking Midilab/2
:p.The following command line parameters can be used when starting MidiLab/2:
:dl break=all.
:dt.:hp2./M:ehp2.
:dd.This switch will cause all files to be saved in MIDI Format 1.
:dt.:hp2./P:ehp2.
:dd.A default file directory can be specified with this parameter, in case the
MIDI files are located in a directory other than the default one. Specify
:hp2./P=(directory name):ehp2.
:dt.:hp2./X:ehp2.
:dd.This switch will enable the External Control facility at start-up time.
:dt.:hp2./Z:ehp2.
:dd.This switch will force MidiLab/2 to operate in an MPU simulation mode.
:edl.
.*
:h1 id=MIDI_TUTORIAL nosearch.MIDI Tutorial
.im tutorial.ipf
.*
:euserdoc.
