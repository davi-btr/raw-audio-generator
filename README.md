# raw-audio-generator
Code to generate a rawdata file containing the audio samples of a note (or group of notes played simultaneously).

Notes are described by their "key" and "velocity" according to the MIDI protocol.
The program relies on the Fluidsynth API and uses soundfonts to synthesize the notes.

It can be used to generate a database of waveforms relative to different notes, chords, or hinstruments (by changing the soundfont)

To compile it on Linux:

> gcc -Wall -g -o rawdata rawdata.c `pkg-config fluidsynth --libs`

In case Fluidsynth is not already installed on the machine it can be fixed with:

> sudo apt-get install fluidsynth
