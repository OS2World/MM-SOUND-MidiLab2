.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*     SCNDCTLP.IPF - *** Secondary Control Panel Help ***           */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
:p.This window contains user preference settings.
For details of each control, select from the list below:
:ul compact.
:li.:link refid=FONTS  reftype=hd.Fonts:elink.
:li.:link refid=COLORS reftype=hd.Colors:elink.
:li.:link refid=MIDITHRU reftype=hd.MIDI-Thru:elink.
:li.:link refid=DISABLE_MT reftype=hd.Disable Measure Timing:elink.
:li.:link refid=LEADIN reftype=hd.Lead-in Measures:elink.
:li.:link refid=METROCLK reftype=hd.Metronome Click settings:elink.
:eul.
.*
:h1 id=MIDITHRU nosearch.MIDI-THRU
:p.The MIDI-THRU function is enabled.
MIDI-THRU allows any data appearing at the system's MIDI-IN port to be
passed directly to its MIDI-OUT port.
.*
:h1 id=LEADIN nosearch.Lead-in Measures
:p.Type in the number of 'lead-in' or 'countoff' measures you wish before
recording commences.  It can be any number from 0 to 8.

:h1 id=DISABLE_MT nosearch.Disable Measure Timing Check
:p.Each beat of a measure contains a number of clock pulses equal to the
current timebase.  Therefore, a song with a time signature of 4/4 and
a timebase of 120, for example, must contain a total of 480 clock pulses for
each measure.  This total is verified at the end of each measure during
playback for each active track.  If an error is detected, a message will
appear indicating the erroneous timing total.
:p.This check may be disabled by checking this box.
.*
:h1 id=COLORS nosearch.Colors
:i1.Colors
:p.Color verbage.... (more to come)
.*
:h1 id=FONTS nosearch.Fonts
:i1.Fonts
:p.Font verbage.... (more to come)
.*
:h1 id=METROCLK nosearch.Metronome Click Settings
:p.The audible metronome clicks can be selected to come from the :hp2.Internal
speaker:ehp2., or be directed to a MIDI device such as a drum machine.
The :hp2.Duration:ehp2. of the click in milliseconds (1-250) can be selected.
:p.Selecting :hp2.MIDI:ehp2. will enable you to further select a specific
MIDI note and its associated channel.
