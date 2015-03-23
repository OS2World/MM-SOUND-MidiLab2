.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*     MLABSYSX.IPF - *** MidiLab/2 Sys Excl Help ***                */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
MidiLab/2 provides for saving and restoring system exclusive data from attached
MIDI devices.  This information is saved as track data; i.e., up to sixteen
tracks worth of SysEx information can be named, saved, and restored much the
same as normal song data.
:note.An individual track must contain either SysEx data or MIDI music data;
however, SysEx tracks can co-exist with music tracks in the same file.
:p.Normally, if data is to be transmitted from the MIDI device to MidiLab/2, a
manual operation must be initiated.  Usually this takes the form of a button, on
the control panel of the device, which causes a SysEx transmission to take
place.  In other words, MidiLab/2 does not automatically solicit SysEx
information; it simply is placed in 'listening' mode, and waits for data to
arrive.
:p.However, it's possible to solicit SysEx data if the device matches one
of those in the Dump Request window.  When :hp2.Receive:ehp2. is pressed, a
pre-defined request is sent to the MIDI device causing a dump to be
automatically sent to MidiLab/2.  The Dump Request window should be set to
:hp2.None:ehp2. if no matches exist for your personal equipment.
:p.A Send, or Restore, function causes the contents of a selected track to
be transmitted to MIDI OUT, and will be received by the device from which it was
originally sent.  It cannot be re-channeled to a different destination.
:p.The MidiLab/2 SysEx support is intended to provide a convenient means of
'backing up' the information stored in your keyboards, drum machines, etc., and
as such no editing facilities are provided.
