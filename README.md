# bioimpeedance
this thing lives under the sink

It's an esp32-c3 mini connected to a DY-SV5W MP3 Player Module which powers a speaker
Using this library that hasn't been updated for 4 years, but it works https://github.com/SnijderC/dyplayer/


If you want to add something to the playlist you can!

SD card folder structure:
There are 3 folders, PERFECT, MEME and MUSIC
Files are named 00001.mp3-65535.mp3

If you want to add something, please normalize it first, so that the sound levels of different tracks would be more or less equal
Also, since the file naming is a mess tag everything please, it would be easier to find and delete stuff later

Folder PERFECT is for perfect loops only!
That is: mp3s that can be played repeatedly without a feeling of start/end of the song
The hardware handles it quite nicely by looping these files

Folder MEME is for short-ish and funny stuff, preferably not too annoying
These should be played before going into a nice loop, so you can be creative

Folder MUSIC is for normal songs, they can be as long as you want (I think), use your best judgement content-wise

~~Enumerating files on SD card doesn't work, DFplayer has it's chip name erased, we have to hardcode available names, sorry.~~
DY can tell us the number of files in a folder, for random play that's good enough. 
You can add mp3s to the appropriate folders and they should be in the random draw pool after the esp restarts and requeries the number of files. 

Also randomness might suck.
