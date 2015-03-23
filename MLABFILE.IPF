.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*     MLABFILE.IPF - *** File Manager Panel Help ***                */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
:p.All song files are loaded and saved using the controls on this panel.
Select a directory and then a file from within that directory.  All files
in the selected directory will be displayed, whether or not they contain
MIDI data.  Double-click on the file to load it.
For a detailed description of each control, select from the following list:
:ul compact.
:li.:link refid=LOAD    reftype=hd.Load:elink.
:li.:link refid=SAVE    reftype=hd.Save:elink.
:li.:link refid=DELETE  reftype=hd.Delete:elink.
:li.:link refid=MIDIFMT reftype=hd.MIDI File Format:elink.
:li.:link refid=MTEXT   reftype=hd.Track Text Display:elink.
:eul.
.*
:h1 id=LOAD nosearch.Load song file
:p.Load a file by first selecting one and then pushing this button, or
double-click on a file in the file window.
:h1 id=SAVE nosearch.Save song file
:p.Select a file in the file window, then press this button to save it.
If the file exists, you will be prompted to overlay it or abandon the task.
:h1 id=DELETE nosearch.Delete song file
:p.Select a file in the file window, then press this button to delete it.
You will be prompted to confirm the delete or to abandon the task.
:h1 id=MIDIFMT nosearch.Save as MIDI format 1
:p.By default, files will be saved in the MidiLab/2 native file format.
However, they can also be saved in standard MIDI format by checking this box.
:p.MidiLab/2 conforms to the Standard MIDI Files 1.0 Format 1 specification
dated July 1988.  It will read 1.0 files produced on different systems, but
may not necessarily support all defined features which may be present.
:p.A file can be loaded from either format, and stored in the opposite format
without any loss of function (see File Compatibility below).  The native format
is more efficient and should be used during development of a song, prior to
exporting it as a standard file.
:p.The following features of the 1.0 Specification are supported and will be
handled as described.
:ul.
:li.Track format 1
:p.Format 1 supports a maximum of sixteen tracks, plus one containing tempo and
time signature data.  Additional ones are ignored.
:li.Supported Meta-events
:ul.
:li.X'01' - General text
:li.X'03' - Sequence/Track name
:li.X'04' - Instrument name
:li.X'05' - Lyric text
:li.X'06' - Marker text
:li.X'07' - Cue Point
:li.X'20' - MIDI Channel prefix
:li.X'2F' - End-of-Track
:p.This event only exists on disk files.  It is stripped off when the file
is loaded and replaced with the MPU-401 End-of-Track message ($FC); it is
added to the end of each of the sixteen tracks when the file is saved.
:li.X'51' - Set Tempo
:li.X'58' - Time Signature  (only the numerator portion)
:li.X'7F' - MidiLab/2-Specific
:p.The MidiLab/2 track orders are implemented as sequencer-specific
meta-events.
:p.The first 16 characters of Sequence/Track name and Instrument name will be
placed in the associated MidiLab/2 track name window.  When a file is loaded,
track 1 is pre-scanned for any meta events so as to set tempos, etc.  on
initial load.  Meta-events occurring in other tracks are processed as they are
played.
:eul.
:eul.
:p.MidiLab/2 uses two types of file extensions to indicate how the the file was
stored:
:ul.
:li.filename.:hp2.ML:ehp2.  - Native MidiLab/2 format
:li.filename.:hp2.MID:ehp2. - MIDI 1.0 format
:eul.
:h1 id=MTEXT nosearch.Display track text during Load
:p.When a song file is loaded, and this box is checked, any text meta-events
embedded in the tracks will be displayed as they are encountered, as an aid to
understanding the nature of the file.
:p.These include such defined Meta-Events as Cue points, Markers, Lryics, etc.
These are normally ignored to speed up the loading and playing process.
