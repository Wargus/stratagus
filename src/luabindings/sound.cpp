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
/**@name sound.cpp. Bindings for sound related code to lua */
//
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
#include "sound_server.h"
#include "script.h"
#include "script_sol.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
/**

extern int GetEffectsVolume(void);
extern void SetEffectsVolume(int volume);
extern int GetMusicVolume(void);
extern void SetMusicVolume(int volume);

extern void SetEffectsEnabled(bool enabled);
extern bool IsEffectsEnabled(void);
extern void SetMusicEnabled(bool enabled);
extern bool IsMusicEnabled(void);

extern int PlayFile(const std::string name, LuaActionListener *listener = NULL);
$[
function PlaySoundFile(file, callback)
  return PlayFile(file, LuaActionListener:new(callback))
end
$]

extern int PlayMusic(const std::string name);
extern void StopMusic();
extern bool IsMusicPlaying();

extern int SetChannelVolume(int channel, int volume);
extern void SetChannelStereo(int channel, int stereo);
extern void StopChannel(int channel);
extern void StopAllChannels();

**/

void ToLuaBind_Sound()
{
	sol::state_view luaSol(Lua);

	luaSol["GetEffectsVolume"] = GetEffectsVolume;
	luaSol["SetEffectsVolume"] = SetEffectsVolume;
	luaSol["GetMusicVolume"] = GetMusicVolume;
	luaSol["SetMusicVolume"] = SetMusicVolume;
	
	luaSol["SetEffectsEnabled"] = SetEffectsEnabled;
	luaSol["IsEffectsEnabled"] = IsEffectsEnabled;
	luaSol["SetMusicEnabled"] = SetMusicEnabled;
	luaSol["IsMusicEnabled"] = IsMusicEnabled;
	
	luaSol["PlayFile"] = PlayFile;
	luaSol.script(R"(
		function PlaySoundFile(file, callback)
			return PlayFile(file, LuaActionListener:new(callback))
		end
	)");
	
	luaSol["PlayMusic"] = PlayMusic;
	luaSol["StopMusic"] = StopMusic;
	luaSol["IsMusicPlaying"] = IsMusicPlaying;
	
	luaSol["SetChannelVolume"] = SetChannelVolume;
	luaSol["SetChannelStereo"] = SetChannelStereo;
	luaSol["StopChannel"] = StopChannel;
	luaSol["StopAllChannels"] = StopAllChannels;
}
//@}
