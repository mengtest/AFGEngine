#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED


#include <fishsound/fishsound.h>
#include <oggz/oggz.h>
#include <RtAudio.h>

class AudioFile
{
public:
	FishSoundInfo info;
	unsigned char* oggFile;
	int byteOffset;

private:
	bool begun;
	FishSound* musicVorbis;

public:
    AudioFile(const char* filename);

    friend int FishCb_Decode(FishSound *fsound, float **pcm, long frames, AudioFile *file);
    friend int OggCb_PacketRead(OGGZ *oggz, oggz_packet *packet, long serialno, FishSound *file);
};

class Song
{
public:
	RtAudio playback;

private:
	int readBytes;
	AudioFile songFile;

public:
	Song(const char* filename);

	friend int AudioCb_Music(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, Song *song);
};


#endif // AUDIO_H_INCLUDED
