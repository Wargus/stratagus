//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name sound_server.cpp - The sound server (hardware layer and so on) */
//
//      (c) Copyright 1998-2006 by Lutz Sammer, Fabrice Rossi, and
//                                 Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//


//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "sound_server.h"

#ifdef USE_FLUIDSYNTH
#include "fluidsynth.h"
#endif

#include "iocompat.h"
#include "iolib.h"
#include "unit.h"

#include "SDL.h"
#include "SDL_mixer.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static bool SoundInitialized;    /// is sound initialized
static bool MusicEnabled = true;
static bool EffectsEnabled = true;
static double VolumeScale = 1.0;
static int MusicVolume = 0;

static void (*MusicFinishedCallback)();

#ifdef USE_WIN32
static volatile bool threadWaiting = false;
static std::string externalFile;
static HANDLE hWaitingThread;
static PROCESS_INFORMATION pi;

DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	if (threadWaiting) {
		MusicFinishedCallback();
		threadWaiting = false;
	}
	return 0;
}

static void killPlayingProcess() {
	if (threadWaiting) {
		threadWaiting = false;
		TerminateProcess(pi.hProcess, 0);
		WaitForSingleObject(hWaitingThread, INFINITE);
		threadWaiting = false;
	} else {
		TerminateProcess(pi.hProcess, 0);
	}
}

static bool External_Play(const std::string &file) {
	if (threadWaiting && file == externalFile) {
		return true;
	}

	static std::string midi = ".mid";
	auto it = midi.begin();
	if (file.size() > midi.size() &&
			std::all_of(std::next(file.begin(), file.size() - midi.size()), file.end(), [&it](const char & c) { return c == ::tolower(*(it++)); })) {
		// midi file, use external player, since windows vista+ does not allow midi volume control independent of process volume
		std::string full_filename = LibraryFileName(file.c_str());

		static const char* midiplayerExe = "stratagus-midiplayer.exe";
		static const int midiplayerExeSz = strlen(midiplayerExe);
		
		// set up a job so our children die with us
		static bool firstRun = true;
		static HANDLE hJob;
		if (firstRun) {
			hJob = CreateJobObject(NULL, NULL);
			JOBOBJECT_BASIC_LIMIT_INFORMATION limitInfo;
			ZeroMemory(&limitInfo, sizeof(limitInfo));
			limitInfo.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			SetInformationJobObject(hJob, JobObjectBasicLimitInformation, &limitInfo, sizeof(limitInfo));
			AssignProcessToJobObject(hJob, GetCurrentProcess());
			firstRun = false;
		}

		int sz = midiplayerExeSz + 2 + 3 + 2 + full_filename.size() + 1; // exe + 2 spaces + 3 volume + 2 quotes + filename + nullbyte
		char *cmdline = new char[sz];
		snprintf(cmdline, sz, "%s %3d \"%s\"", midiplayerExe, std::min(MusicVolume, 127), full_filename.c_str());
		DebugPrint("Using external command to play midi on windows: %s\n" _C_ cmdline);
		killPlayingProcess();
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		bool result = true;
		if (CreateProcess(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
			AssignProcessToJobObject(hJob, pi.hProcess);
			externalFile = file;
			hWaitingThread = CreateThread(NULL, 0, MyThreadFunction, NULL, 0, NULL);
			threadWaiting = true;
		} else {
			result = false;
			DebugPrint("CreateProcess failed (%d).\n" _C_ GetLastError());
		}
		delete[] cmdline;
		return result;
	}
	killPlayingProcess();
	return false;
}

static bool External_IsPlaying() {
	return threadWaiting;
}

static bool External_Stop() {
	if (External_IsPlaying()) {
		killPlayingProcess();
		return true;
	}
	return false;
}

static bool External_Volume(int volume, int oldVolume) {
	if (External_IsPlaying() && externalFile.size() > 0) {
		if (oldVolume != volume) {
			External_Stop();
			External_Play(externalFile);
		}
		return true;
	}
	return false;
}
#else
#define External_Play(file) false
#define External_IsPlaying() false
#define External_Stop() false
#define External_Volume(volume, oldVolume) false
#endif

extern volatile bool MusicFinished;

/// Channels for sound effects and unit speech
struct SoundChannel {
	Origin *Unit;          /// pointer to unit, who plays the sound, if any
	void (*FinishedCallback)(int channel); /// Callback for when a sample finishes playing
};

#define MaxChannels 64     /// How many channels are supported

static SoundChannel Channels[MaxChannels];

static void ChannelFinished(int channel);

/**
**  Check if this sound is already playing
*/
bool SampleIsPlaying(Mix_Chunk *sample)
{
	for (int i = 0; i < MaxChannels; ++i) {
		if (Mix_GetChunk(i) == sample && Mix_Playing(i)) {
			return true;
		}
	}
	return false;
}

bool UnitSoundIsPlaying(Origin *origin)
{
	for (int i = 0; i < MaxChannels; ++i) {
		if (origin && Channels[i].Unit && origin->Id && Channels[i].Unit->Id
			&& origin->Id == Channels[i].Unit->Id && Mix_Playing(i)) {
			return true;
		}
	}
	return false;
}

/**
**  A channel is finished playing
*/
static void ChannelFinished(int channel)
{
	if (Channels[channel].FinishedCallback) {
		Channels[channel].FinishedCallback(channel);
	}
	delete Channels[channel].Unit;
	Channels[channel].Unit = NULL;
}

/**
**  Set the channel volume
**
**  @param channel  Channel to set
**  @param volume   New volume 0-255, <0 will not set the volume
**
**  @return         Current volume of the channel, -1 for error
*/
int SetChannelVolume(int channel, int volume)
{
	return Mix_Volume(channel, volume * VolumeScale);
}

/**
**  Set the channel stereo
**
**  @param channel  Channel to set
**  @param stereo   -128 to 127, out of range will not set the stereo
**
**  @return         Current stereo of the channel, -1 for error
*/
void SetChannelStereo(int channel, int stereo)
{
	if (Preference.StereoSound == false) {
		Mix_SetPanning(channel, 255, 255);
	} else {
		int left, right;
		if (stereo > 0) {
			left = 255 - stereo;
			right = 255;
		} else {
			left = 255;
			right = 255 + stereo;
		}
		Mix_SetPanning(channel, left, right);
	}
}

/**
**  Set the channel's callback for when a sound finishes playing
**
**  @param channel   Channel to set
**  @param callback  Callback to call when the sound finishes
*/
void SetChannelFinishedCallback(int channel, void (*callback)(int channel))
{
	if (channel < 0 || channel >= MaxChannels) {
		return;
	}
	Channels[channel].FinishedCallback = callback;
}

/**
**  Get the sample playing on a channel
*/
Mix_Chunk *GetChannelSample(int channel)
{
	if (Mix_Playing(channel)) {
		return Mix_GetChunk(channel);
	}
	return NULL;
}

/**
**  Stop a channel
**
**  @param channel  Channel to stop
*/
void StopChannel(int channel)
{
	Mix_HaltChannel(channel);
}

/**
**  Stop all channels
*/
void StopAllChannels()
{
	Mix_HaltChannel(-1);
}

static Mix_Music *currentMusic = NULL;

static Mix_Music *LoadMusic(const char *name)
{
	if (currentMusic) {
		Mix_HaltMusic();
		Mix_FreeMusic(currentMusic);
	}
	currentMusic = Mix_LoadMUS(name);
	if (currentMusic) {
		return currentMusic;
	}

	CFile *f = new CFile;
	if (f->open(name, CL_OPEN_READ) == -1) {
		printf("Can't open file '%s'\n", name);
		delete f;
		return NULL;
	}
	currentMusic = Mix_LoadMUS_RW(f->as_SDL_RWops(), 1);
	return currentMusic;
}

static Mix_Chunk *LoadSample(const char *name)
{
#ifdef DYNAMIC_LOAD
	Mix_Chunk *r = (Mix_Chunk *)calloc(sizeof(Mix_Chunk), 1);
	r->allocated = 0xcafebeef;
	r->abuf = (Uint8 *)(strdup(name));
	return r;
#else
	return ForceLoadSample(name);
#endif
}

static Mix_Chunk *ForceLoadSample(const char *name)
{
	Mix_Chunk *r = Mix_LoadWAV(name);
	if (r) {
		return r;
	}
	CFile *f = new CFile;
	if (f->open(name, CL_OPEN_READ) == -1) {
		printf("Can't open file '%s'\n", name);
		delete f;
		return NULL;
	}
	return Mix_LoadWAV_RW(f->as_SDL_RWops(), 1);
}

/**
**  Load a music file
**
**  @param name  File name
**
**  @return      Mix_Music pointer
*/
Mix_Music *LoadMusic(const std::string &name)
{
	const std::string filename = LibraryFileName(name.c_str());
	Mix_Music *music = LoadMusic(filename.c_str());

	if (music == NULL) {
		fprintf(stderr, "Can't load the music '%s'\n", name.c_str());
	}
	return music;
}

/**
**  Load a sample
**
**  @param name  File name of sample (short version).
**
**  @return      General sample loaded from file into memory.
**
**  @todo  Add streaming, caching support.
*/
Mix_Chunk *LoadSample(const std::string &name)
{
	const std::string filename = LibraryFileName(name.c_str());
	Mix_Chunk *sample = LoadSample(filename.c_str());

	if (sample == NULL) {
		fprintf(stderr, "Can't load the sound '%s': %s\n", name.c_str(), Mix_GetError());
	}
	return sample;
}

/**
**  Play a sound sample
**
**  @param sample  Sample to play
**
**  @return        Channel number, -1 for error
*/
int PlaySample(Mix_Chunk *sample, Origin *origin)
{
#ifdef DYNAMIC_LOAD
	if (sample->allocated == 0xcafebeef) {
		char *name = (char*)(sample->abuf);
		Mix_Chunk *loadedSample = ForceLoadSample(name);
		memcpy(sample, loadedSample, sizeof(Mix_Chunk));
		free(name);
	}
#endif
	int channel = -1;
	DebugPrint("play sample %d\n" _C_ sample->volume);
	if (SoundEnabled() && EffectsEnabled && sample) {
		channel = Mix_PlayChannel(-1, sample, 0);
		Channels[channel].FinishedCallback = NULL;
		if (origin && origin->Base) {
			Origin *source = new Origin;
			source->Base = origin->Base;
			source->Id = origin->Id;
			Channels[channel].Unit = source;
		}
	}
	return channel;
}

/**
**  Set the global sound volume.
**
**  @param volume  the sound volume 0-255
*/
void SetEffectsVolume(int volume)
{
	VolumeScale = (volume * 1.0) / 255.0;
}

/**
**  Get effects volume
*/
int GetEffectsVolume()
{
	return VolumeScale * 255;
}

/**
**  Set effects enabled
*/
void SetEffectsEnabled(bool enabled)
{
	EffectsEnabled = enabled;
}

/**
**  Check if effects are enabled
*/
bool IsEffectsEnabled()
{
	return EffectsEnabled;
}

/*----------------------------------------------------------------------------
--  Music
----------------------------------------------------------------------------*/

/**
**  Set the music finished callback
*/
void SetMusicFinishedCallback(void (*callback)())
{
	MusicFinishedCallback = callback;
	Mix_HookMusicFinished(callback);
}

/**
**  Play a music file.
**
**  @param file  Name of music file, format is automatically detected.
**
**  @return      0 if music is playing, -1 if not.
*/
int PlayMusic(const std::string &file)
{
	if (!SoundEnabled() || !IsMusicEnabled()) {
		return -1;
	}
	DebugPrint("play music %s\n" _C_ file.c_str());

	if (External_Play(file)) {
		MusicFinished = false;
		return 0;
	}

	Mix_Music *music = LoadMusic(file);
	if (music) {
		MusicFinished = false;
		Mix_PlayMusic(music, 0);
		return 0;
	} else {
		DebugPrint("Could not play %s\n" _C_ file.c_str());
		return -1;
	}
}

/**
**  Stop the current playing music.
*/
void StopMusic()
{
	if (External_Stop()) {
		return;
	}
	Mix_FadeOutMusic(200);
}

/**
**  Set the music volume.
**
**  @param volume  the music volume 0-255
*/
void SetMusicVolume(int volume)
{
	// due to left-right separation, sound effect volume is effectively halfed,
	// so we adjust the music
	int oldVolume = MusicVolume;
	MusicVolume = volume;
	Mix_VolumeMusic(MusicVolume);
	External_Volume(MusicVolume, oldVolume);
}

/**
**  Get music volume
*/
int GetMusicVolume()
{
	return MusicVolume;
}

/**
**  Set music enabled
*/
void SetMusicEnabled(bool enabled)
{
	if (enabled) {
		MusicEnabled = true;
	} else {
		MusicEnabled = false;
		StopMusic();
	}
}

/**
**  Check if music is enabled
*/
bool IsMusicEnabled()
{
	return MusicEnabled;
}

/**
**  Check if music is playing
*/
bool IsMusicPlaying()
{
	return Mix_PlayingMusic();
}

/*----------------------------------------------------------------------------
--  Init
----------------------------------------------------------------------------*/

/**
**  Check if sound is enabled
*/
bool SoundEnabled()
{
	return SoundInitialized;
}

/**
**  Initialize sound card hardware part with SDL.
**
**  @param freq  Sample frequency (44100,22050,11025 hz).
**  @param size  Sample size (8bit, 16bit)
**
**  @return      True if failure, false if everything ok.
*/
static int InitSdlSound()
{
	// just activate everything we can by setting all bits
	Mix_Init(std::numeric_limits<unsigned int>::max());
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024)) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return -1;
	} else {
		printf("Supported sound decoders:");
		for (int i = 0; i < Mix_GetNumChunkDecoders(); i++) {
			printf(" %s", Mix_GetChunkDecoder(i));
		}
		printf("\nSupported music decoders:");
		for (int i = 0; i < Mix_GetNumMusicDecoders(); i++) {
			printf(" %s", Mix_GetMusicDecoder(i));
		}
		printf("\n");
	}
	return 0;
}

/**
**  Initialize sound card.
**
**  @return  True if failure, false if everything ok.
*/
int InitSound()
{
	//
	// Open sound device, 8bit samples, stereo.
	//
	if (InitSdlSound()) {
		SoundInitialized = false;
		return 1;
	}
	SoundInitialized = true;
	Mix_AllocateChannels(MaxChannels);
	Mix_ChannelFinished(ChannelFinished);

	// Now we're ready for the callback to run
	Mix_ResumeMusic();
	Mix_Resume(-1);
	return 0;
}

/**
**  Cleanup sound server.
*/
void QuitSound()
{
	Mix_CloseAudio();
	Mix_Quit();
	SoundInitialized = false;
}

//@}
