.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*     RECDOVDB.IPF - *** Record and Overdub Help ***                */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
:p.Recording consists of selecting the :hp2.Record:ehp2. or :hp2.Overdub:ehp2.
buttons (or by pressing the 'R' or 'O' keys, respectively) and selecting one
of the 16 tracks.  The functions are similar except that during Overdub, all
other active tracks will play during recording.
:p.Enter the number of measures you want to record, which can be any number
from 1 to 9999.
:p.The tempo window is updated at each measure to provide a dynamic display of
tempo variations, if any.
:p.Enter the number of times you want the recorded passage to be repeated.
Any number from 0 to 254 may be used as a repeat count, and a MidiLab/2
track-transfer order will be automatically appended to the recorded material.
After the track has been selected, a "standby" will be presented to give you
time to make final preparations for your take.
When ready, select :hp2.Start Recording:ehp2. to begin recording.
:p.During recording, Measure-End marks are inserted into the track along
with the other musical information, so that subsequent editing can take place
on a measure-by-measure basis.  The metronome makes a different sound while
recording is in progress, as compared with the sound it makes at all other
times.  The accent pattern of the beats is varied to allow for easier tempo
correlation by the performer.
:p.Recording will continue until either the :hp2.Stop Recording:ehp2. function
is invoked or the pre-requested number of measures has been recorded.  The
recording will end, and, if Trace/Monitor is active, the duration in minutes and
seconds will be noted.  If the metronome was running, recording will stop at
the end of the current measure, and the metronome will be silenced.

:dl break=all.
:dt.:hp2.Lead-in/Countoff:ehp2.
:dd.If the metronome is running at the time recording starts, a countoff takes
place before recording actually begins.  The number of lead-in measures can be
adjusted from 0 to 8, from the secondary control panel.  It is set at 1
normally.  The metronome will begin a different pattern of accented sounds, to
help "feel" the lead-in.  Recording begins exactly on the downbeat of the bar
following the lead-in.
:p."Pick-up" notes played during the countoff are not recorded.  This includes
any anticipatory notes which may tie into the first measure.  If these notes
are desired, it is necessary to use an additional measure, after the countoff,
to accommodate them.
:p.If the metronome is :hp1.NOT:ehp1. running at the time the :hp2.Start
Record:ehp2. button is pressed, recording begins immediately.

:dt.:hp2.Re-using previous track and option selections:ehp2.
:dd.Pressing the :hp2.Re-Take:ehp2. button causes a short-cut to take place
and whatever setup you used previously, including the track selection, number
of measures to record, and times to repeat will be automatically placed in
effect.  This will provide you with a quick restart capability when
re-recording a track.

:dt.:hp2.Creating 'shadow notes':ehp2.
:dd.The Transposition adjustment, on the main control panel, has a special
function when used during recording.  Its use will result in automatically
producing a second note for each note that is played.  This new note
will be displaced from the original by the transposition factor, and the
original note will be retained.
:p.One useful effect of this feature is being
able to record octaves while playing only single notes.  In other words, setting
the transpose value to +12 will create an extra note one octave higher in
addition to the one actually being played.  The velocity value of the shadow
note will be the same as the original, including any leveling adjustment.  When
the transpose factor is zero, this function is disabled.
:note.The track must be enabled by checking the appropriate box on the Main
Control Panel.

:dt.:hp2.Input Channel Selection:ehp2.
:dd.Any or all of the 16 channels may be activated to supply input during
recording.  Press the desired checkboxes to select or de-select channels.
The :hp2.ALL:ehp2. button may be used to force all channels on or off.
:edl.
