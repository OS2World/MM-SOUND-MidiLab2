.*********************************************************************/
.* PC MidiLab - MIDI Sequencer/Editor for OS/2 Presentation Manager  */
.*                                                                   */
.*     TUTORIAL.IPF - *** Introduction to MIDI ***                   */
.*                                                                   */
.*       Copyright  J. L. Bell         Jan 1993                      */
.*********************************************************************/
:p.This section is provided to give you an understanding of the data stream and
MIDI event types you will encounter when working with recorded material.  It
is presented in a general fashion without particular reference to MidiLab/2.
The term 'command', as used below, can be related to the MidiLab/2 'event'.
:p.MIDI (Musical Instrument Digital Interface) data is transmitted by the
sending and receiving of eight bits of information.  These eight bits of data
are sent together at one time and make up what is called a byte.  Each bit can
show one of two possible states, on or off (called 1 or 0).  Eight bits (two
raised to the eighth power) yield 256 possible combinations in the same way
that normal numerical digits can form 1000 combinations by using the numbers
000 to 999 (ten raised to the third power).
:p.MIDI commands are usually composed of one, two, or three bytes of data
arranged and transmitted one after another.  The first byte sent is called a
"status byte" (although the MIDI Spec refers to it as a "status command", in
this documentation we'll call it status byte to avoid confusion).  The next
one or two bytes, if used, represent the "which" and "how much" parts of the
command.  A note-on command, for example, is comprised of three bytes, the
first of which is the status byte.  This byte tells the synthesizer to play a
note and also supplies the channel number.  The second byte in the command
tells the receiver which note to play, and the third byte carries the velocity
value for the note.
:note.In most cases the MPU-401 prefixes each command with its own timing
byte.  This specifies the time quantity associated with the command.  This is
not, however, a MIDI specified function.
:p.Part of the MIDI structure is a protocol that was put in place to
facilitate the writing of software designed to handle MIDI data.  This
protocol specifies that the status command byte's "significant" (leftmost) bit
is always a 1. In all following data bytes the most significant bit is 0. By
following this protocol, the software always knows, if the most significant
bit is a 1, that this is a new command for it to begin processing on.
Additionally, by decoding the status byte, the software always knows how many
data bytes should follow.
:p.Being able to discern the difference between status and data bytes makes it
possible to utilize a technique called "running status".  Simply stated, once
a particular status byte has been received (like "note-on"), the receiving
instrument can continue to operate under that command until a different status
byte is encountered.  As an example, a burst of notes can be played by sending
the first note-on with its status byte, and then followed by pairs of data
bytes containing note number and note velocity, without the need to send a new
status byte until it's necessary to send a new command, like pitch-bend, etc.
When there are many notes being sent, utilizing "running status" can improve
the overall speed by about a third of a millisecond per note which makes a
significant difference.
:p.The note-off command has its own status byte as well.  But needing to send
a note-off command while sending many notes would cause running status to be
interrupted.  To avoid this, a more efficient way of doing the same thing is
to send a special version of the note-off command, a note-on command with a
velocity of zero.  This zero-velocity note-on command can be used this way
only if the sending synthesizer doesn't also send a release velocity, which
uses a separate note-off status byte with its own associated velocity data.
Today, however, most synthesizers do not send a release velocity.
:p.Using the most significant bit of the MIDI byte as a "status or data" flag,
seven bits are left to make up 128 different combinations.  This provides the
ability to send note-on commands specifying any of 128 notes with 127
different velocity values (remember, that the value of 0 is used to simulate a
note-off.)  Having only seven bits remaining to handle pitch-bend (if you were
to bend up and down 1 octave, having 128 divisions would allow for 1/5
semitone) would not be sufficient to do a good job.  Thus two data words are
used to provide bending control, one for coarse and one for fine control.
:p.Just as with data words, each status byte is left with seven "variable"
bits, or 128 combinations.  This may seem like more commands than we could
ever use, but this isn't really true.  Most MIDI commands belong to a group
called channel messages.  These commands are able to direct themselves to any
of 16 possible device destinations by using four bits to define a channel
number.  Using four bits to specify the channel and one bit to define the
status of a command, we are left with only three bits to spare, or eight
combinations.
:p.Listed in numerical order, the possible types are:  (1) note-off (128), (2)
note-on (144), (3) polyphonic key after-touch (160), (4) controller (176), (5)
program change (192), (6) channel after-touch (208), (7) pitch-bend (224), and
(8) system.
:p.System commands have no channel number attached to them, so there are
actually sixteen different non-channel-oriented command combinations to work
with.  It is beyond the scope of this tutorial to cover these, but the other
seven commands will be covered in some detail.
:dl break=all.
:dt.:hp2.NOTE-OFF COMMAND:ehp2.
:dd.The Note-off command is made up of two data bytes.  The first specifies which
MIDI note (numbered from 0 to 127) is to be affected.  Remember that this
number is not a frequency, it just refers to a specific key on the keyboard.
For example, the number 60 refers to Middle C on most five-octave boards.  The
second byte is the release velocity indicating how fast you removed your
fingers from the key.  This might be used, for instance, to define the decay
rate of each note played.  Currently, the implementation for this feature is
not common, although it may become less rare in the future.
:dt.:hp2.NOTE-ON COMMAND:ehp2.
:dd.The note-on command contains one data byte defining the note number.  The
second byte, when used, carries the velocity with which each note is struck.
As mentioned before, a note-on command with a velocity value of 0 is
interpreted as a note-off command.
:dt.:hp2.POLYPHONIC KEY AFTER-TOUCH:ehp2.
:dd.This command is used in keyboards where each key has its own pressure
transducer so that you can hold down a chord and adjust the pressure applied
to each note, which will generate a different response for each.  Two data
bytes are necessary for this command.  The first defines which key is being
pressed.  The second defines the current pressure value.  In addition to the
more sophisticated keyboards having this feature, some of the new
tone-generators also support polyphonic key after-touch.
:dt.:hp2.CONTROLLER:ehp2.
:dd.There are two data bytes required for any controller command.  The first
defines which controller is being used, and the second usually gives a value
for the position of that controller.  Sixty-four of the possible 128
controller numbers handle the standard control devices like volume pedals,
breath controllers, mod wheel, etc.  This excludes pitch-bend and polyphonic
after-touch as they have their own set of controller codes.  Only 32 different
controllers are allowed, but each can make use of two commands, a "most
significant" data byte (MSB) and a "least significant" data byte (LSB),
doubling the quantity of control numbers allotted for the 32 controllers.  In
most of these commands, the LSB is ignored because the resolution provided by
two bytes isn't necessary.  The common "continuous controllers" pass their
generated value in the MSB.
:p.32 more of the controller numbers are used to indicate on/off functions,
like foot-switches or data entry switches.  A low number (0) indicates open
switch, and a high value (usually 127) shows a closed switch.  As of this
writing, 26 numbers are not defined.
:p.This leaves us with six numbers to send channel mode messages.  These are
used to change the reception mode of the receiving device, for instance, from
mono to omni mode.  The defined controller numbers are listed below.
:ul.
:li.(1)   modulation wheel
:li.(2)   breath controller
:li.(4)   foot controller
:li.(5)   portamento time
:li.(6)   data entry
:li.(7)   main volume
:li.(64)  damper pedal (sustain)
:li.(65)  portamento on/off
:li.(66)  sostenuto
:li.(67)  soft pedal
:li.(96)  data increment
:li.(97)  data increment
:eul.
:dt.:hp2.PROGRAM CHANGE:ehp2.
:dd.Program Change uses a single data byte to tell the slave device to which
program number to change.  There is some confusion associated with this
command regarding what the numbers really mean.  There are many MIDI devices
that use octal numbers for programming.  (The digits that are used range from
1 to 8.)  Most of these devices use program 11 to indicate program change
number 1. Octal 11 is commonly associated at the data byte level with numeric
value 0. An octal 88 is shown as program change 64 but the numeric value sent
is really 63.  This provides a glimpse into some of the confusions.  With this
exception, the command itself is straightforward.
:p.There is nothing in the specification covering what to do if the slave
device has only 32 patches and receives a program change 50 command.
Possibilities include wrapping and starting over, ignoring the command as
invalid, or switching to the external cartridge.  Some machines use the lower
locations for presets and leave the higher program numbers for the
user-programmable areas.  This makes the user-programmable areas inaccessible
to master devices with a limited range of numbers.  There are, in addition,
programmable outboard signal processing devices with very few MIDI program
positions.  Determining an efficient way to coordinate all of these
inconsistencies is the difficulty.
:dt.:hp2.CHANNEL AFTER-TOUCH:ehp2.
:dd.This is the common type of after-touch command.  Using a single data byte,
this command specifies one overall value that defines the pressure applied to
the keyboard.  It is sent out on the channel to which the keyboard is
currently assigned and is capable of defining this value at any given time.
Precise results to this command are dependent on the synthesizer receiving the
command and its capabilities, as well as the current patch.  Over-all volume,
brightness, modulation level, LFO speed or other parameters may all be
controlled by channel after-touch.
:dt.:hp2.PITCH-BEND:ehp2.
:dd.Pitch-bend has its own command because of the frequency with which its
used.  It is distinct from the controller groups.  This command uses two data
byes, the first is the LSB and the second is the MSB.  This provides 14 bits
of resolution for pitch changes.  A normal controller command would require
six bytes to handle this level of resolution (a most significant byte
controller command and a least significant byte controller command, each three
bytes long).  Using a great deal of pitch wheel motion would take up much of
the MIDI cable's transmission ability (the amount of data it is capable of
sending each second).  Most synthesizers do not take advantage of this level
of resolution but instead send the number 0 for the LSB.  This then provides
for 128 steps of bend.  Remember that these increments cover about 1/5
semitone if you bend up or down a full octave.  This is not acceptable for
very slow pitch transitions, but for most bending is satisfactory.  The
increments are much smaller if the range is up or down a third.  This,
however, makes it hard to hear the discrete steps.
:edl.
