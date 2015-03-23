.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*     MLABPLAY.IPF - *** MidiLab/2 Playback Help ***                */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
:p.Normal playback is initiated by pressing the :hp2.Play:ehp2. pushbutton.
 Any combination of the sixteen tracks can be played, whether or not they contain
any data.  Playback completes normally when all selected tracks reach their
end-of-track marker.
:p.When MidiLab is first started, all sixteen tracks are automatically selected
for playback.  All or any combination of the sixteen tracks may be subsequently
selected from the main control panel. Your selection will be remembered and
indicated, by highlighting, on the main display until you change it again.
:p.The tempo is initially set according to the song profile when a song is
loaded from disk, and can be subsequently changed during playback from the
main control panel. The transposition and velocity adjustments may also be set
or reset in the same way.
:dl break=all.
:dt.:hp2.Interrupting and resuming playback:ehp2.
:dd.Whenever the :hp2.|| Pause:ehp2. pushbutton (or right mouse button) is
pressed during playback, a pause function takes place and playback is suspended.
If :hp2.Pause:ehp2. is pressed again, playback will resume from the point of
interruption.
:p.The duration of playback, in minutes and seconds, including any time spent
in 'pause', may be displayed at normal completion in the Trace/Monitor window.
:dt.:hp2.Starting playback at a selected point in the song:ehp2.
:dd.You may choose to start the playback process at some point other than the
beginning of the song.  This is done by specifying the number of the measure
at which you wish to begin.  The tracks are then positioned to this measure
and playback proceeds normally.  Specify this number on the main control
panel.
:edl.
:note.The :hp2.Play:ehp2. and :hp2.Pause:ehp2. buttons are duplicated on the
MidiLab/2 main control panel.
