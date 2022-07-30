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

#include <numeric>

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

uint32_t SDL_SOUND_FINISHED;

static bool SoundInitialized;    /// is sound initialized
static bool MusicEnabled = true;
static bool EffectsEnabled = true;
static double VolumeScale = 1.0;
static int MusicVolume = 0;

static void (*MusicFinishedCallback)();

#ifdef USE_WIN32
static bool externalPlayerIsPlaying = false;

static HANDLE g_hStatusThread;
static HANDLE g_hDebugThread;
static HANDLE g_hChildStd_IN_Wr;
static PROCESS_INFORMATION pi;

static DWORD WINAPI StatusThreadFunction(LPVOID lpParam) {
	CHAR chStatus;
	DWORD dwRead = 1;
	while (1) {
		if (!ReadFile((HANDLE)lpParam, &chStatus, 1, &dwRead, NULL) || dwRead == 0) {
			CloseHandle((HANDLE)lpParam);
			break;
		}
		// any write means we finished
		if (externalPlayerIsPlaying) {
			externalPlayerIsPlaying = false;
			MusicFinishedCallback();
		}
	}
	return 0;
}

static DWORD WINAPI DebugThreadFunction(LPVOID lpParam) {
	DWORD dwRead = 1;
	while (1) {
		char *chStatus[1024] = {'\0'};
		if (!ReadFile((HANDLE)lpParam, &chStatus, 1024, &dwRead, NULL) || dwRead == 0) {
			CloseHandle((HANDLE)lpParam);
			break;
		}
		DebugPrint("%s" _C_ chStatus);
	}
	return 0;
}

static void KillPlayingProcess() {
	externalPlayerIsPlaying = false;
	if (g_hChildStd_IN_Wr) {
		TerminateProcess(pi.hProcess, 0);
		CloseHandle(g_hChildStd_IN_Wr);
		g_hChildStd_IN_Wr = NULL;
		WaitForSingleObject(StatusThreadFunction, 0);
		WaitForSingleObject(DebugThreadFunction, 0);
	}
}

static bool External_Play(const std::string &file) {
	static std::string midi = ".mid";
	auto it = midi.begin();
	if (file.size() > midi.size() &&
			std::all_of(std::next(file.begin(), file.size() - midi.size()), file.end(), [&it](const char & c) { return c == ::tolower(*(it++)); })) {
		// midi file, use external player, since windows vista+ does not allow midi volume control independent of process volume
		
		std::string full_filename = LibraryFileName(file.c_str());

		// try to communicate with the running midiplayer if we can
		if (g_hChildStd_IN_Wr != NULL) {
			// already playing, just send the new song
			// XXX: timfel: disabled, since the midiplayer behaves weirdly when it receives the next file, just kill and restart
			KillPlayingProcess();
			/*
			// negative value signals a new filename
			int fileSize = full_filename.size() & 0xffff;
			char loSize = fileSize & 0xff;
			char hiSize = (fileSize >> 8) & 0xff;
			char buf[2] = {loSize, hiSize};
			externalPlayerIsPlaying = true;
			if (!WriteFile(g_hChildStd_IN_Wr, buf, 2, NULL, NULL)) {
				KillPlayingProcess();
			} else {
				// then write the filename
				if (!WriteFile(g_hChildStd_IN_Wr, full_filename.c_str(), fileSize, NULL, NULL)) {
					KillPlayingProcess();
				} else {
					return true;
				}
			}
			*/
		}
		// need to start an external player first

		// setup pipes to player
		HANDLE hChildStd_IN_Rd = NULL;
		HANDLE hChildStd_OUT_Rd = NULL;
		HANDLE hChildStd_OUT_Wr = NULL;
		HANDLE hChildStd_ERR_Rd = NULL;
		HANDLE hChildStd_ERR_Wr = NULL;
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;
		CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0);
		CreatePipe(&hChildStd_ERR_Rd, &hChildStd_ERR_Wr, &saAttr, 0);
		CreatePipe(&hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0);
		SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);

		// start the process
		std::vector<std::string> args = QuoteArguments({ "stratagus-midiplayer.exe", std::to_string(std::min(MusicVolume, 127)), full_filename });
		std::string cmd = std::accumulate(std::next(args.begin()), args.end(), args[0], [](std::string a, std::string b) { return a + " " + b; });
		DebugPrint("Using external command to play midi on windows: %s\n" _C_ cmd.c_str());
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.hStdError = hChildStd_ERR_Wr;
   		si.hStdOutput = hChildStd_OUT_Wr;
   		si.hStdInput = hChildStd_IN_Rd;
   		si.dwFlags |= STARTF_USESTDHANDLES;
		ZeroMemory(&pi, sizeof(pi));
		bool result = true;
		char* cmdline = strdup(cmd.c_str());
		if (CreateProcess(NULL, cmdline, NULL, NULL, TRUE, /* Handles are inherited */ CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
			CloseHandle(hChildStd_OUT_Wr);
			CloseHandle(hChildStd_ERR_Wr);
      		CloseHandle(hChildStd_IN_Rd);
			externalPlayerIsPlaying = true;
			g_hStatusThread = CreateThread(NULL, 0, StatusThreadFunction, hChildStd_OUT_Rd, 0, NULL);
			g_hDebugThread = CreateThread(NULL, 0, DebugThreadFunction, hChildStd_ERR_Rd, 0, NULL);
		} else {
			result = false;
			DebugPrint("CreateProcess failed (%d).\n" _C_ GetLastError());
		}
		free(cmdline);
		return result;
	}
	KillPlayingProcess();
	return false;
}

static bool External_IsPlaying() {
	return externalPlayerIsPlaying;
}

static bool External_Stop() {
	if (External_IsPlaying()) {
		KillPlayingProcess();
		return true;
	}
	return false;
}

static bool External_Volume(int volume, int oldVolume) {
	if (External_IsPlaying()) {
		char buf[2] = {0, volume & 0xFF};
		if (!WriteFile(g_hChildStd_IN_Wr, buf, 2, NULL, NULL)) {
			External_Stop();
			return false;
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

/// Channels for sound effects and unit speech
struct SoundChannel {
	Origin *Unit = NULL;          /// pointer to unit, who plays the sound, if any
	void (*FinishedCallback)(int channel) = NULL; /// Callback for when a sample finishes playing
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
	if (channel < 0 || channel >= MaxChannels) {
		fprintf(stderr, "ERROR: Out of bounds channel (how?)\n");
		return;
	}
	if (Channels[channel].FinishedCallback != NULL) {
		SDL_Event event;
		SDL_zero(event);
		event.type = SDL_SOUND_FINISHED;
		event.user.code = channel;
		event.user.data1 = (void*) Channels[channel].FinishedCallback;
		SDL_PeepEvents(&event, 1, SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
	}
	delete Channels[channel].Unit;
	Channels[channel].Unit = NULL;
}

void HandleSoundEvent(SDL_Event &event)
{
	((void (*)(int channel))(event.user.data1))((int) event.user.code);
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
 * Free a sample loaded with LoadSample.
 * 
 * @param sample 
 */
void FreeSample(Mix_Chunk *sample)
{
#ifdef DYNAMIC_LOAD
	if (sample->allocated == 0xcafebeef) {
		return;
	}
#endif
	Mix_FreeChunk(sample);
}

/**
**  Play a sound sample
**
**  @param sample  Sample to play
**
**  @return        Channel number, -1 for error
*/
static int PlaySample(Mix_Chunk *sample, Origin *origin, void (*callback)(int channel))
{
#ifdef DYNAMIC_LOAD
	if (sample->allocated == 0xcafebeef) {
		char *name = (char*)(sample->abuf);
		Mix_Chunk *loadedSample = ForceLoadSample(name);
		if (loadedSample) {
			memcpy(sample, loadedSample, sizeof(Mix_Chunk));
			free(name);
		} else {
			return -1;
		}
	}
#endif
	int channel = -1;
	DebugPrint("play sample %d\n" _C_ sample->volume);
	if (SoundEnabled() && EffectsEnabled && sample) {
		channel = Mix_PlayChannel(-1, sample, 0);
		if (channel >= 0 && channel < MaxChannels) {
			Channels[channel].FinishedCallback = callback;
			if (origin && origin->Base) {
				Origin *source = new Origin;
				source->Base = origin->Base;
				source->Id = origin->Id;
				Channels[channel].Unit = source;
			}
		}
	}
	return channel;
}

int PlaySample(Mix_Chunk *sample, Origin *origin)
{
	return PlaySample(sample, origin, NULL);
}

int PlaySample(Mix_Chunk *sample, void (*callback)(int channel))
{
	return PlaySample(sample, NULL, callback);
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
		return 0;
	}

	Mix_Music *music = LoadMusic(file);
	if (music) {
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
	return Mix_PlayingMusic() || External_IsPlaying();
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
	SDL_SOUND_FINISHED = SDL_RegisterEvents(1);
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
