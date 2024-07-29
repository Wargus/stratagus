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
/**@name sound.cpp - The sound. */
//
//      (c) Copyright 1998-2015 by Lutz Sammer, Fabrice Rossi,
//		Jimmy Salmon and Andrettin
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

#include "sound.h"

#include "action/action_resource.h"
#include "map.h"
#include "missile.h"
#include "sound_server.h"
#include "ui.h"
#include "unit.h"
#include "video.h"
#include "widgets.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static constexpr unsigned char MaxVolume = 255;

/**
**  Various sounds used in game.
*/
GameSound GameSounds;

/**
**  Selection handling
*/
struct SelectionHandling {
	Origin Source;         /// origin of the sound
	CSound *Sound;         /// last sound played by this unit
	unsigned char HowMany; /// number of sound played in this group
};

static int ViewPointOffset;      /// Distance to Volume Mapping
int DistanceSilent;              /// silent distance

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  "Randomly" choose a sample from a sound group.
*/
static Mix_Chunk *SimpleChooseSample(const CSound &sound)
{
	if (auto *chunks = std::get_if<std::vector<sdl2::ChunkPtr>>(&sound.Sound)) {
		Assert(!chunks->empty());
		return (*chunks)[FrameCounter % chunks->size()].get();
	} else {
		return nullptr;
	}
}

/**
**  Choose the sample to play
*/
static Mix_Chunk *ChooseSample(CSound &sound, bool selection, Origin &source)
{
	if (!SoundEnabled()) {
		return nullptr;
	}

	Mix_Chunk *result = nullptr;
	static SelectionHandling SelectionHandler{};

	if (auto* p = std::get_if<std::pair<CSound *, CSound*>>(&sound.Sound)) {
		// handle a special sound (selection)
		if (SelectionHandler.Sound != nullptr && (SelectionHandler.Source.Base == source.Base && SelectionHandler.Source.Id == source.Id)) {
			if (SelectionHandler.Sound == p->first) {
				result = SimpleChooseSample(*SelectionHandler.Sound);
				SelectionHandler.HowMany++;
				if (SelectionHandler.HowMany >= 3) {
					SelectionHandler.HowMany = 0;
					SelectionHandler.Sound = p->second;
				}
			} else {
				//FIXME: checks for error
				// check whether the second group is really a group
				auto *chunks =
					std::get_if<std::vector<sdl2::ChunkPtr>>(&SelectionHandler.Sound->Sound);
				Assert(SelectionHandler.HowMany < chunks->size());
				result = (*chunks)[SelectionHandler.HowMany].get();
				SelectionHandler.HowMany++;
				if (SelectionHandler.HowMany >= chunks->size()) {
					SelectionHandler.HowMany = 0;
					SelectionHandler.Sound = p->first;
				}
			}
		} else {
			SelectionHandler.Source = source;
			SelectionHandler.Sound = p->first;
			result = SimpleChooseSample(*SelectionHandler.Sound);
			SelectionHandler.HowMany = 1;
		}
	} else {
		Assert(std::holds_alternative<std::vector<sdl2::ChunkPtr>>(sound.Sound));
		// normal sound/sound group handling
		result = SimpleChooseSample(sound);
		if (SelectionHandler.Source.Base == source.Base && SelectionHandler.Source.Id == source.Id) {
			SelectionHandler.HowMany = 0;
			SelectionHandler.Sound = nullptr;
		}
		if (selection) {
			SelectionHandler.Source = source;
		}
	}

	return result;
}

/**
**  Maps a EUnitVoice to a CSound*.
**
**  @param unit    Sound initiator
**  @param voice   Type of sound wanted
**
**  @return        Sound identifier
*/
static std::shared_ptr<CSound> ChooseUnitVoiceSound(const CUnit &unit, EUnitVoice voice)
{
	switch (voice) {
		case EUnitVoice::Acknowledging: return unit.Type->MapSound.Acknowledgement.Sound;
		case EUnitVoice::Attack:
			return unit.Type->MapSound.Attack.Sound ? unit.Type->MapSound.Attack.Sound
			                                        : unit.Type->MapSound.Acknowledgement.Sound;
		case EUnitVoice::Build: return unit.Type->MapSound.Build.Sound;
		case EUnitVoice::Ready: return unit.Type->MapSound.Ready.Sound;
		case EUnitVoice::Selected: return unit.Type->MapSound.Selected.Sound;
		case EUnitVoice::HelpMe: return unit.Type->MapSound.Help.Sound;
		case EUnitVoice::Dying:
			return unit.Type->MapSound.Dead[unit.DamagedType].Sound
			         ? unit.Type->MapSound.Dead[unit.DamagedType].Sound
			         : unit.Type->MapSound.Dead[ANIMATIONS_DEATHTYPES].Sound;
		case EUnitVoice::WorkCompleted:
			return unit.Type->MapSound.WorkComplete.Sound
			         ? unit.Type->MapSound.WorkComplete.Sound
			         : GameSounds.WorkComplete[ThisPlayer->Race].Sound;
		case EUnitVoice::Building: return GameSounds.BuildingConstruction[ThisPlayer->Race].Sound;
		case EUnitVoice::Docking: return GameSounds.Docking.Sound;
		case EUnitVoice::Repairing:
			return unit.Type->MapSound.Repair.Sound ? unit.Type->MapSound.Repair.Sound
			                                        : unit.Type->MapSound.Acknowledgement.Sound;
		case EUnitVoice::Harvesting:
			if (auto it = ranges::find(unit.Orders, UnitAction::Resource, &COrder::Action);
				it != unit.Orders.end()) {
				const COrder_Resource &order = dynamic_cast<COrder_Resource &>(**it);
				const auto resIndex = order.GetCurrentResource();

				return unit.Type->MapSound.Harvest[resIndex].Sound
				         ? unit.Type->MapSound.Harvest[resIndex].Sound
				         : unit.Type->MapSound.Acknowledgement.Sound;
			} else {
				return nullptr;
			}
	}
	return nullptr;
}

/**
**  Compute a suitable volume for something taking place at a given
**  distance from the current view point.
**
**  @param d      distance
**  @param range  range
**
**  @return       volume for given distance (0..??)
*/
unsigned char VolumeForDistance(unsigned short d, unsigned char range)
{
	// FIXME: THIS IS SLOW!!!!!!!
	if (d <= ViewPointOffset || range == INFINITE_SOUND_RANGE) {
		return MaxVolume;
	} else {
		if (range) {
			d -= ViewPointOffset;
			int d_tmp = d * MAX_SOUND_RANGE;
			int range_tmp = DistanceSilent * range;
			if (d_tmp > range_tmp) {
				return 0;
			} else {
				return (unsigned char)((range_tmp - d_tmp) * MAX_SOUND_RANGE / range_tmp);
			}
		} else {
			return 0;
		}
	}
}

/**
**  Calculate the stereo value for a unit
*/
static char CalculateStereo(const CUnit &unit)
{
	int stereo = ((unit.tilePos.x * PixelTileSize.x + unit.Type->TileWidth * PixelTileSize.x / 2 +
				   unit.IX - UI.SelectedViewport->MapPos.x * PixelTileSize.x) * 256 /
				  ((UI.SelectedViewport->MapWidth - 1) * PixelTileSize.x)) - 128;
	clamp(&stereo, -128, 127);
	return stereo;
}

/**
**  Ask to the sound server to play a sound attached to a unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit   Sound initiator, unit speaking
**  @param voice  Type of sound wanted (Ready,Die,Yes,...)
*/
void PlayUnitSound(const CUnit &unit, EUnitVoice voice, bool sampleUnique)
{
	auto sound = ChooseUnitVoiceSound(unit, voice);
	if (!sound) {
		return;
	}

	const bool selection = (voice == EUnitVoice::Selected || voice == EUnitVoice::Building);
	Origin source = {&unit, unsigned(UnitNumber(unit))};

	if (!sampleUnique && UnitSoundIsPlaying(source)) {
		return;
	}

	Mix_Chunk *sample = ChooseSample(*sound, selection, source);

	if (sampleUnique && SampleIsPlaying(sample)) {
		return;
	}

	int channel = PlaySample(sample, &source);
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, VolumeForDistance(ViewPointDistanceToUnit(unit), sound->Range));
	SetChannelStereo(channel, CalculateStereo(unit));
#ifdef USE_MNG
	const CUnitType &type = *unit.Type;
	if (!type.Portrait.Mngs.empty() && type.Portrait.Talking && type.Portrait.Mngs[0]) {
		type.Portrait.Mngs[type.Portrait.CurrMng]->Reset();
		type.Portrait.CurrMng = (MyRand() % (type.Portrait.Mngs.size() - type.Portrait.Talking)) + type.Portrait.Talking;
		type.Portrait.NumIterations = 1;
	}
#endif
}

/**
**  Ask to the sound server to play a sound attached to a unit. The
**  sound server may discard the sound if needed (e.g., when the same
**  unit is already speaking).
**
**  @param unit   Sound initiator, unit speaking
**  @param sound  Sound to be generated
*/
void PlayUnitSound(const CUnit &unit, CSound *sound)
{
	if (!sound) {
		return;
	}
	Origin source = {&unit, unsigned(UnitNumber(unit))};
	const unsigned char volume = VolumeForDistance(ViewPointDistanceToUnit(unit), sound->Range);
	if (volume == 0) {
		return;
	}

	int channel = PlaySample(ChooseSample(*sound, false, source));
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, volume);
	SetChannelStereo(channel, CalculateStereo(unit));
}

/**
**  Ask the sound server to play a sound for a missile.
**
**  @param missile  Sound initiator, missile exploding
**  @param sound    Sound to be generated
*/
void PlayMissileSound(const Missile &missile, CSound *sound)
{
	if (!sound) {
		return;
	}
	int stereo = ((missile.position.x + (missile.Type->G ? missile.Type->G->Width / 2 : 0) +
				   UI.SelectedViewport->MapPos.x * PixelTileSize.x) * 256 /
				  ((UI.SelectedViewport->MapWidth - 1) * PixelTileSize.x)) - 128;
	clamp(&stereo, -128, 127);

	Origin source = {nullptr, 0};
	const unsigned char volume =
		VolumeForDistance(ViewPointDistanceToMissile(missile), sound->Range);
	if (volume == 0) {
		return;
	}

	int channel = PlaySample(ChooseSample(*sound, false, source));
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, volume);
	SetChannelStereo(channel, stereo);
}

/**
**  Play a game sound
**
**  @param sound   Sound to play
**  @param volume  Volume level to play the sound
*/
void PlayGameSound(CSound *sound, unsigned char volume, bool always)
{
	if (!sound) {
		return;
	}
	Origin source = {nullptr, 0};

	Mix_Chunk *sample = ChooseSample(*sound, false, source);

	if (!sample || (!always && SampleIsPlaying(sample))) {
		return;
	}

	int channel = PlaySample(sample);
	if (channel == -1) {
		return;
	}
	SetChannelVolume(channel, std::min(MaxVolume, volume));
}

static std::map<int, LuaActionListener *> ChannelMap;
static std::map<int, sdl2::ChunkPtr> SampleMap;

/**
**  Callback for PlaySoundFile
*/
static void PlaySoundFileCallback(int channel)
{
	LuaActionListener *listener = ChannelMap[channel];
	ChannelMap[channel] = nullptr;
	// free any previously loaded sample that was playing on this channel before
	SampleMap[channel] = nullptr;
	if (listener != nullptr) {
		listener->action(gcn::ActionEvent{nullptr, ""});
	}
}


/**
**  Play a sound file
**
**  @param name      Filename of a sound to play
**  @param listener  Optional lua callback
**
**  @return          Channel number the sound is playing on, -1 for error
*/
int PlayFile(const std::string &name, LuaActionListener *listener)
{
	int channel = -1;
	auto sample = LoadSample(name);

	if (sample) {
		channel = PlaySample(sample.get(), PlaySoundFileCallback);
		if (channel != -1) {
			SampleMap[channel] = std::move(sample);
			SetChannelVolume(channel, MaxVolume);
			ChannelMap[channel] = listener;
		}
	}
	return channel;
}

/**
**  Ask the sound server to register a sound (and currently to load it)
**  and to return an unique identifier for it. The unique identifier is
**  memory pointer of the server.
**
**  @param files   An array of wav files.
**  @param number  Number of files belonging together.
**
**  @return        the sound unique identifier
**
**  @todo FIXME: Must handle the errors better.
*/
std::shared_ptr<CSound> RegisterSound(const std::vector<std::string> &files)
{
	std::vector<sdl2::ChunkPtr> chunks;
	for (const auto &file : files) {
		chunks.push_back(LoadSample(file));
		if (chunks.back() == nullptr) {
			return nullptr;
		}
	}
	auto id = CSound::make();
	id->Sound = std::move(chunks);
	id->Range = MAX_SOUND_RANGE;
	return id;
}

/**
**  Ask the sound server to put together two sounds to form a special sound.
**
**  @param first   first part of the group
**  @param second  second part of the group
**
**  @return        the special sound unique identifier
*/
std::shared_ptr<CSound> RegisterTwoGroups(CSound *first, CSound *second)
{
	if (first == nullptr || second == nullptr) {
		return nullptr;
	}
	auto id = CSound::make();
	id->Sound = std::make_pair(first, second);
	id->Range = MAX_SOUND_RANGE;

	return id;
}

/**
**  Lookup the sound id's for the game sounds.
*/
void InitSoundClient()
{
	if (!SoundEnabled()) { // No sound enabled
		return;
	}
	// let's map game sounds, look if already setup in ccl.

	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.PlacementError[i].Sound) {
			GameSounds.PlacementError[i].MapSound();
		}
	}

	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.PlacementSuccess[i].Sound) {
			GameSounds.PlacementSuccess[i].MapSound();
		}
	}

	if (!GameSounds.Click.Sound) {
		GameSounds.Click.MapSound();
	}
	if (!GameSounds.Docking.Sound) {
		GameSounds.Docking.MapSound();
	}

	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.BuildingConstruction[i].Sound) {
			GameSounds.BuildingConstruction[i].MapSound();
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.WorkComplete[i].Sound) {
			GameSounds.WorkComplete[i].MapSound();
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.ResearchComplete[i].Sound) {
			GameSounds.ResearchComplete[i].MapSound();
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		for (unsigned int j = 0; j < MaxCosts; ++j) {
			if (!GameSounds.NotEnoughRes[i][j].Sound) {
				GameSounds.NotEnoughRes[i][j].MapSound();
			}
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.NotEnoughFood[i].Sound) {
			GameSounds.NotEnoughFood[i].MapSound();
		}
	}
	for (unsigned int i = 0; i < PlayerRaces.Count; ++i) {
		if (!GameSounds.Rescue[i].Sound) {
			GameSounds.Rescue[i].MapSound();
		}
	}
	if (!GameSounds.ChatMessage.Sound) {
		GameSounds.ChatMessage.MapSound();
	}

	int MapWidth = (UI.MapArea.EndX - UI.MapArea.X + PixelTileSize.x) / PixelTileSize.x;
	int MapHeight = (UI.MapArea.EndY - UI.MapArea.Y + PixelTileSize.y) / PixelTileSize.y;
	DistanceSilent = 3 * std::max<int>(MapWidth, MapHeight);
	ViewPointOffset = std::max<int>(MapWidth / 2, MapHeight / 2);
}

//@}
