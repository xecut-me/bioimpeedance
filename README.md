# bioimpeedance
this thing lives under the sink

If you want to add something, there are a few rules!

SD card folder structure:
Folders are named 01-99
Files are named 001.mp3-255.mp3

NORMALIZE EVERYTHING please, so that the sound levels would be more or less equal
mp3 tag everything please, since there would be no more file names

Folder 01 is for perfect loops only!
That is: mp3s that can be played repeatedly without providing a feeling of start/end of the song
The hardware should be able to handle it
If we ever go beyond 255 perfect loops, create folder 02 and add it as perfectLoops to sound_mappings.cpp

Folder 10 is for short-ish and funny stuff, preferably not too annoying
These should be added as nonLoops to sound_mappings.cpp
If we ever get more than 255.mp3 of those create folder 09 and carry on

Folders 11-99 is for songs you like and think fit in these can be as long as possible, preferably not too annoying
(use scripts/nameconverter.sh to suffer less)


Enumerating files on SD card doesn't work, DFplayer has it's chip name erased, we have to hardcode available names, sorry.
Also randomness might suck.