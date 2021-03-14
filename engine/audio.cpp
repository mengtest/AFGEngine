#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include <fishsound/fishsound.h>
#include <oggz/oggz.h>
#include <RtAudio.h>
#include "audio.h"

int FishCb_Decode(FishSound *fsound, float **pcm, long frames, AudioFile *file)
{
	if (!file->begun)
	{
		fish_sound_command(fsound, FISH_SOUND_GET_INFO, &file->info, sizeof (FishSoundInfo));
		file->begun = true;
	}

	memcpy(file->oggFile+file->byteOffset, pcm, 2*frames*sizeof(float));
	file->byteOffset += 2*frames*sizeof(float);

	return 0;
}

int OggCb_PacketRead(OGGZ *oggz, oggz_packet *packet, long serialno, FishSound* decoder)
{
	fish_sound_prepare_truncation(decoder, packet->op.granulepos, packet->op.e_o_s);
	int check = fish_sound_decode(decoder, packet->op.packet, packet->op.bytes);
    //std::cout << check << "\t";
    return 0;
}

AudioFile::AudioFile(const char* filename) : byteOffset(0), begun(false)
{
    oggFile = new unsigned char[0x2000000];

    musicVorbis = fish_sound_new(FISH_SOUND_DECODE, nullptr);
    if(fish_sound_set_decoded_float_ilv(
		musicVorbis,
		reinterpret_cast<FishSoundDecoded_FloatIlv>(FishCb_Decode),
		this)
	)
	{
		std::cerr << "fishsound callback error\n";
	}

    OGGZ* musicOGG = oggz_open(filename, OGGZ_READ);

	int oggcheck = oggz_set_read_callback(
		musicOGG,
		-1,
		reinterpret_cast<OggzReadPacket>(OggCb_PacketRead),
		musicVorbis);
	
	if(oggcheck)
		std::cerr << "OGGZ callback error: ";
	switch(oggcheck)
	{
	case OGGZ_ERR_BAD_SERIALNO:
		std::cerr << "bad serialno.\n";
		break;
	case OGGZ_ERR_BAD_OGGZ:
		std::cerr << "bad oggz\n";
		break;
	case OGGZ_ERR_INVALID:
		std::cerr << "invalid\n";
		break;
	case OGGZ_ERR_OUT_OF_MEMORY:
		std::cerr << "Out of memory\n";
		break;
	case 0:
		std::cout << "Ok.";
		break;
	default:
		std::cerr << oggcheck << ": unknown error\n";
	}
	oggz_run(musicOGG);

	fish_sound_delete(musicVorbis);
	oggz_close(musicOGG);

	std::cout << byteOffset << " bytes processed\n";
}


int AudioCb_Music(void *outputBuffer, void */*inputBuffer*/, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, Song *song)
{
	int endByte = song->songFile.byteOffset;
	int toBeRead = song->readBytes + 2*nFrames*sizeof(float);
	if(toBeRead > endByte)
	{
		memcpy(outputBuffer, song->songFile.oggFile + song->readBytes, toBeRead - endByte);
		song->readBytes = (2*372094*sizeof(float));
		memcpy(outputBuffer+(toBeRead - endByte), song->songFile.oggFile + song->readBytes, 2*nFrames*sizeof(float) - (toBeRead - endByte) );
	}
	else
	{
		memcpy(outputBuffer, song->songFile.oggFile + song->readBytes, 2*nFrames*sizeof(float));
	}

	song->readBytes += 2*nFrames*sizeof(float);
	if(status)
		std::cout << "\nMusic audio underflow\n";

	return 0;
}

Song::Song(const char* filename) : readBytes(0), songFile(filename)
{
    if( playback.getDeviceCount() < 1 )
		std::cerr << playback.getDeviceCount() << ": no audio devices!!\n";
	else
	{
		RtAudio::StreamParameters parameters;
		parameters.deviceId = playback.getDefaultOutputDevice();
		parameters.nChannels = 2;
		unsigned int sampleRate = 44100;
		unsigned int bufferFrames = 512; // 256 sample frames
		RtAudio::StreamOptions options;
		playback.openStream(
			&parameters,
			nullptr,
			RTAUDIO_FLOAT32,
			sampleRate,
			&bufferFrames,
			reinterpret_cast<RtAudioCallback>(&AudioCb_Music),
			this,
			&options);
	}
}
