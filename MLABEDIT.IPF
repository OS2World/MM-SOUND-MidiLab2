.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*     MLABEDIT.IPF - *** MidiLab/2 Edit Facility Help ***           */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
:p.The MidiLab/2 Track Edit facility is invoked by double-clicking on
the :hp2.Content:ehp2. field in the main window. The resultant panel
provides all Edit functions for the selected track. One or
more tracks may be edited simultaneously.
:p.MidiLab/2 provides for extensive manipulation of the recorded track data. These
facilities can be used for a variety of reasons, including error correction,
rephrasing song passages, changing notes and/or their duration, looping
short sequences, and inserting repeat bars.
:p.The MidiLab/2 editor allows you to examine the details of a recorded track of
MIDI data in real time.  Each event is presented on the screen showing its
timing data and associated parameters, while, at the same time, it can be
heard through an audio playback system.  For example, a note-on event shows
the timing, note, and the velocity with which it was originally played.  Each
event is presented, step-by-step.
:warning.
During edit, it is your responsibility to insure that the MIDI data stream
remains intact.  It is possible to modify data at any point; however, close
attention must be paid to event timing bytes to insure against losing any
quantization, particularly when inserting or appending data.  For this reason,
it is recommended to use the 'measure-end' events as markers for these
functions.
:ewarning.
:hp2.Error conditions:ehp2.
:p.Incorrect editing of the track data may result in an error message during
Playback or Edit.  When this occurs, information will be presented to you
indicating the track in error, and the index at which the error was detected.
If normal Playback was in effect, it will be immediately terminated.  The
following types of errors are possible:
:ol.
:li.Event Data Error
:p.This means an incorrect sequence of MIDI status and/or data has been
detected.  MidiLab/2 will attempt to analyze the error and send a correct data
byte in place of the faulty one in order to complete the event.  You should save
any song data you had been recording, because, in some rare instances, the error
may not be recoverable; that is, the system may "hang up", necessitating a
power off/on at the computer to clear the condition.
:li.Measure Timing Error
:p.Each beat of a measure contains a number of clock pulses equal to the current
timebase.  Therefore, a song with a time signature of 4/4 and a timebase
of 120, for example, must contain a total of 480 clock pulses for each measure.
This total is verified at the end of each measure during playback for all active
tracks.  If an error is detected, a message will appear indicating the erroneous
timing total.
:p.This error check may be disabled by a checkbox in the Preferences
dialog box.
:eol.
:p.For a description of the controls select from the following list:
:ul compact.
:li.:link refid=EVENT_LIST    reftype=hd.Event List:elink.
:li.:link refid=EVENT_SPINS   reftype=hd.Event Spin Buttons:elink.
:li.:link refid=EDIT_BUTTONS  reftype=hd.Edit Push Buttons:elink.
:li.:link refid=EDIT_CLIPBRD  reftype=hd.Edit Clipboard:elink.
:li.:link refid=PLAY_SEQ      reftype=hd.Play Sequence:elink.
:eul.
.*
:h1 id=EVENT_LIST nosearch.Event List
:p.The event list box shows each MIDI event in the track, formatted in such a
way as to represent each component of the event. The index of the event, or its
displacement into the track, appears as the first column on the left side of the
event list. The second column (T') represents the delta time associated with
each event. The remaining information is particular to the type of event.
:p.One or more events can be selected for processing by the Edit controls.  When
an event is selected, and the :hp2.Send Event:ehp2. checkbox is checked, the
event is transmitted, via MIDI, to the channel assigned to the track being
edited. Refer to :link refid=PLAY_SEQ reftype=hd.Play Sequence:elink. for
additional information.
.*
:h1 id=EVENT_SPINS nosearch.Event Spin Buttons
:p.The three spin buttons are used to modify specific components of an
event.  Their function takes on different meanings depending on what type of
event is selected.  Headings appropriate for the selected event appear for
each button. The values and ranges are set as required.
.*
:h1 id=EDIT_BUTTONS nosearch.Event Push Buttons
:p.The pushbuttons are used to perform the edit functions. Most of them are
disabled until one or more events have been selected.  The function of each
button is described below.
:dl break=all.
:dt.:hp2.Copy:ehp2.
:dd.Copys marked item(s) to MidiLab/2 clipboard.
.*
:dt.:hp2.Cut:ehp2.
:dd.Copys marked item(s) to MidiLab/2 clipboard, and then deletes them from
the list.
.*
:dt.:hp2.Delete:ehp2.
:dd.Deletes all marked items.
.*
:dt.:hp2.Paste:ehp2.
:dd.Pastes the contents of the MidiLab/2 clipboard into the track at the point
following the current marked item.
.*
:dt.:hp2.Start Block:ehp2.
:dd.Used to define the start of a block.  After a mark has been selected,
the next location marked will define a displacement, either prior to, or after,
the original mark.  Useful for marking large sections of track data, when it is
too far to drag the mouse.
:p.Mouse button 2 can also be used for this function.
.*
:dt.:hp2.Mark to End:ehp2.
:dd.This button provides a fast way to mark a section of track data from the
current marked item to the end of the track.
.*
:dt.:hp2.Insert:ehp2.
:dd.Inserts new track data as selected from the :hp2.Insert:ehp2. dialog box.
See the help information provided there.
.*
:dt.:hp2.Modify:ehp2.
:dd.Modifies the marked block according to the settings of the transpose,
velocity, and quantize adjustments.
.*
:dt.:hp2.Save:ehp2.
:dd.Saves the current edit data into the track.
.*
:dt.:hp2.Done:ehp2.
:dd.Exits the editor.
.*
:dt.:hp2.All Notes Off:ehp2.
:dd.Sends the MIDI message ALL_NOTES_OFF and sends a Release-Sustain to all 16
channels.
.*
:dt.:hp2.Clear Track:ehp2.
:dd.Clears the entire track to an empty state.
:edl.
.*
:h1 id=EDIT_CLIPBRD nosearch.Edit Clipboard
:p.The MidiLab/2 clipboard is used to hold selected events for subsequent use
in a Paste operation. It can be displayed by marking the :hp2.Show Clipbrd:ehp2.
checkbox on the Edit panel. Once displayed, it can be removed from view by
either selecting the :hp2.Hide Clipboard:ehp2. pushbutton
on the clipboard panel itself or by re-marking the checkbox on the Edit panel
:p.The clipboard can be scrolled if the data exceeds the window size.

:note.The MidiLab/2 clipboard is unrelated to the OS/2 clipboard.
.*
:h1 id=PLAY_SEQ   nosearch.Play Sequence
:p.The track being edited can be played by marking a block and then
pushing the :hp2.:ehp2. button.  The block will be played until it ends, or
can be stopped by pushing :hp2.||:ehp2..
:p.The :hp2.Move Banner:ehp2. checkbox can be used to display the selection
banner as the events in the track progress.  The trade-off is one of performance
versus the convenience of being able to locate the events as the track is playing.
:p.While the sequence is playing, if the :hp2.Step-play:ehp2. checkbox is
selected, the automatic advance will stop, and you must then press the '+'
key to advance to the next event. This facilitates listening closely to
the resultant audio, and picking up errors in the recorded MIDI data.
.*
:h1 res=2100.Edit Insert Functions
:p.This panel provides several means of inserting new data into the track.
Data are inserted at the point following the current mark. For a description of
the available types select from the following list:
:ul compact.
:li.:link refid=TRACK_EVENTS  reftype=hd.Track Events:elink.
:li.:link refid=TRACK_ORDERS  reftype=hd.Track Orders:elink.
:eul.
.*
:h1 id=TRACK_EVENTS nosearch.Insert Track Event
:p.Track events are a subset of standard MIDI events which conform
to MIDI specifications.
:dl break=all.
:dt.:hp2.Program change:ehp2.
:dd.Inserts a MIDI Program Change channel event into the track.
.*
:dt.:hp2.Meta Text:ehp2.
:dd.Inserts text for a MIDI 1.0 Text or Cue Point meta event.
:dt.:hp2.Generic Event:ehp2.
:dd.This dialog box may be used to insert any MIDI channel event or MPU
Mark, so long as the basic rules of parameter range are not violated.  It
affords the lowest possible level of control; you are responsible for the
exact configuration of each byte in the event.
.*
:dt.:hp2.TBA:ehp2.
:dd.To Be announced
:edl.
.*
:h1 id=TRACK_ORDERS nosearch.Insert Track Order
:p.Track Orders are MidiLab/2 proprietary events which serve to perform
special functions in the track.
:dl break=all.
:dt.:hp2.Transfer:ehp2.
:dd.Provides a means of looping a section of track data for a desired
number of times. The index of the target event is specified, and must precede
the location of the Transfer.
.*
:dt.:hp2.Rest:ehp2.
:dd.Inserts a specified number of measures of rest.
.*
:dt.:hp2.Initiate:ehp2.
:dd.This function allows you to start one or more inactive tracks from a
point in the current track during play.
.*
:dt.:hp2.Command:ehp2.
:dd.An MPU command, Transpose adjustment, or Velocity adjustment may be inserted
into the track for dynamic control during play.
:edl.
