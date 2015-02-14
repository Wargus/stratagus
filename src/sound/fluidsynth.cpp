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
/**@name fluidsynth.cpp - FluidSynth support */
//
//      (c) Copyright 2014 by cybermind
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

#ifdef USE_FLUIDSYNTH // {

#include "sound.h"
#include "sound_server.h"
#include "iolib.h"
#include "unit.h"

#include <fluidsynth.h>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct FluidSynthData {
	CFile *MIDIFile;       /// MIDI file handle
};

class CSynthesizer {
public:
	CSynthesizer() : Settings(NULL), Synth(NULL), Player(NULL), State(StateCleaned) {}

	fluid_settings_t *Settings;
	fluid_synth_t *Synth;
	fluid_player_t *Player;

	SynthState State;
};

CSynthesizer FluidSynthesizer;

class CSampleFluidSynth : public CSample
{
public:
	~CSampleFluidSynth();
	int Read(void *buf, int len);

	FluidSynthData Data;
};

class CSampleFluidSynthStream : public CSample
{
public:
	~CSampleFluidSynthStream();
	int Read(void *buf, int len);

	FluidSynthData Data;
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Type member function to read from the MIDI file
**
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
int CSampleFluidSynthStream::Read(void *buf, int len)
{
	while (this->Len < SOUND_BUFFER_SIZE / 2 && fluid_player_get_status(FluidSynthesizer.Player) == FLUID_PLAYER_PLAYING) {
		memcpy(this->Buffer, this->Buffer + this->Pos, this->Len);
		this->Pos = 0;
		fluid_synth_write_s16(FluidSynthesizer.Synth, SOUND_BUFFER_SIZE / 8, this->Buffer + this->Len, 0, 2,
			this->Buffer + this->Len, 1, 2);
		this->Len += SOUND_BUFFER_SIZE / 2;
	}

	if (this->Len < len) {
		// EOF
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Len -= len;
	this->Pos += len;

	return len;
}

/**
**  Type member function to free sample
*/
CSampleFluidSynthStream::~CSampleFluidSynthStream()
{
	this->Data.MIDIFile->close();
	delete this->Data.MIDIFile;
	delete[] this->Buffer;
}

/**
**  Type member function to read from the module
**
**  @param buf     Buffer to write data to
**  @param len     Length of the buffer
**
**  @return        Number of bytes read
*/
int CSampleFluidSynth::Read(void *buf, int len)
{
	/* TO-DO: not supported yet*/

	return len;
}

/**
**  Type member function to free sample
*/
CSampleFluidSynth::~CSampleFluidSynth()
{
	delete[] this->Buffer;
}

/**
**  Gets the state of Fluidsynth player
**
*/
SynthState GetFluidSynthState()
{
	return FluidSynthesizer.State;
}

/**
**  Cleans FluidSynth data
**
*/
void CleanFluidSynth(bool reinit)
{
	if (reinit) {
		delete_fluid_player(FluidSynthesizer.Player);
		FluidSynthesizer.Player = new_fluid_player(FluidSynthesizer.Synth);
		FluidSynthesizer.State = StateInitialized;
	} else if (FluidSynthesizer.State != StateCleaned) {
		if (FluidSynthesizer.Player) {
			delete_fluid_player(FluidSynthesizer.Player);
		}
		if (FluidSynthesizer.Synth) {
			delete_fluid_synth(FluidSynthesizer.Synth);
		}
		if (FluidSynthesizer.Settings) {
			delete_fluid_settings(FluidSynthesizer.Settings);
		}
		FluidSynthesizer.State = StateCleaned;
	}
}

/**
**  Inits FluidSynth and loads SF2 soundfont
**
*/
int InitFluidSynth()
{
	// Don't reinit
	if (FluidSynthesizer.State > StateCleaned) {
		return 0;
	}
	FluidSynthesizer.State = StateInitialized;
	// Settings
	FluidSynthesizer.Settings = new_fluid_settings();
	if (FluidSynthesizer.Settings == NULL) {
		fprintf(stderr, "Failed to create the FluidSynth settings\n");
		CleanFluidSynth();
		return -1;
	}
	// Default settings
	fluid_settings_setstr(FluidSynthesizer.Settings, "audio.file.type", "raw");
	fluid_settings_setnum(FluidSynthesizer.Settings, "synth.sample-rate", 44100);
	fluid_settings_setstr(FluidSynthesizer.Settings, "audio.file.format", "s16");
	fluid_settings_setstr(FluidSynthesizer.Settings, "audio.file.endian", "little");
	fluid_settings_setint(FluidSynthesizer.Settings, "audio.period-size", 4096);
	fluid_settings_setint(FluidSynthesizer.Settings, "synth.parallel-render", 1);
	fluid_settings_setnum(FluidSynthesizer.Settings, "synth.gain", 0.2);
	fluid_settings_setstr(FluidSynthesizer.Settings, "synth-midi-bank-select", "gm");
	fluid_settings_setint(FluidSynthesizer.Settings, "synth.synth-polyphony", 24);

	// Synthesizer itself
	FluidSynthesizer.Synth = new_fluid_synth(FluidSynthesizer.Settings);
    if (FluidSynthesizer.Synth == NULL) {
        fprintf(stderr, "Failed to create the SF2 synthesizer\n");
		CleanFluidSynth();
		return -1;
    }
    // Load the soundfont 
    if (fluid_synth_sfload(FluidSynthesizer.Synth, Preference.SF2Soundfont.c_str(), 1) == -1) {
		fprintf(stderr, "Failed to load the SoundFont: %s\n", Preference.SF2Soundfont.c_str());
		CleanFluidSynth();
		return -1;
    }
	// Create player
	FluidSynthesizer.Player = new_fluid_player(FluidSynthesizer.Synth);
	if (FluidSynthesizer.Player == NULL) {
		fprintf(stderr, "Failed to create SF2 player\n");
		CleanFluidSynth();
		return -1;
	}
	return 0;
}

/**
**  Load MIDI file using FluidSynth library.
**
**  @param name   MIDI file.
**  @param flags  Unused.
**
**  @return       Returns the loaded sample.
*/
CSample *LoadFluidSynth(const char *name, int flags)
{
	CSample *sample;
	FluidSynthData *data;
	CFile *f;
	char s[256];

	// If library isn't loaded, load it now
	if (FluidSynthesizer.State == StateCleaned) {
		if (InitFluidSynth() != 0) {
			DebugPrint("Can't init FluidSynth!\n");
			return NULL;
		}
	} else if (FluidSynthesizer.State == StatePlaying) {
		// Reinit the player
		CleanFluidSynth(true);
	}

	strcpy_s(s, sizeof(s), name);
	f = new CFile;
	if (f->open(name, CL_OPEN_READ) == -1) {
		delete f;
		return NULL;
	}
	
	// check if this is a MIDI file
	if (!fluid_is_midifile(name)) {
		fprintf(stderr, "Not a MIDI file: %s\n", name);
		f->close();
		delete f;
		return NULL;
	} else {
		fluid_player_add(FluidSynthesizer.Player, name);
	}

	if (flags & PlayAudioStream) {
		CSampleFluidSynthStream *sampleFluidSynthStream = new CSampleFluidSynthStream;
		sample = sampleFluidSynthStream;
		data = &sampleFluidSynthStream->Data;
	} else {
		CSampleFluidSynth *sampleFluidSynth = new CSampleFluidSynth;
		sample = sampleFluidSynth;
		data = &sampleFluidSynth->Data;
	}
	data->MIDIFile = f;
	sample->Channels = 2;
	sample->SampleSize = 16;
	sample->Frequency = 44100;
	sample->Pos = 0;

	if (flags & PlayAudioStream) {
		sample->Len = 0;
		sample->Buffer = new unsigned char[SOUND_BUFFER_SIZE];

		fluid_player_play(FluidSynthesizer.Player);
		FluidSynthesizer.State = StatePlaying;
	} else {
		/* not supported yet*/
		f->close();
		delete f;
		return NULL;
	}

	return sample;
}

#endif  // } USE_FLUIDSYNTH

//@}
