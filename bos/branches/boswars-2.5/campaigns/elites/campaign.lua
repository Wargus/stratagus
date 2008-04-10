--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      campaign.lua  -  Define the Elite campaign 1.
--
--      (c) Copyright 2005-2008 by Lois Taulelle, Llearch n'n'daCorna 
--                                 and François Beerten
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--=============================================================================
--  Define the campaign
--

local briefingtext01 = 
   "Level 1: The River... "..
   "You're the last remaining free leader. "..
   "Defend your small outpost in this crucial battle. "..
   "Your first enemy, Captain Szarin, knows that "..
   "his victory will spell the end of yours. You "..
   "must lead your war-weary troops ".. 
   "and stop him at all costs. "
local briefingtext02 =
   "Level 2: Green Valley... "..
   "Your scouts report that a small force of mercenaries is encamped "..
   "in the Green Valley, near Captain Fizzpot's forces. " ..
   "Mercenaries, no doubt, see this conflict as an opportunity to "..
   "expand their military influence. Not to worry, though: Mercenary "..
   "troops are neutral and in fact their presence offers you a "..
   "strategic advantage. If you should so choose, first contact "..
   "the mercenaries, you will have the resources to complete "..
   "your quest easily. "
local briefingtext03 = 
   "Level 3: Buffy Forest... "..
   "Your troops arrive near Buffy Forest, a small forest in the "..
   "South-Eastern Mountain region. "..
   "After your victories over Szarin and Fizzpot, the enemy is "..
   "paying more attention to your area. They have dispatched "..
   "Commander Fzedhia to punish your forces, and restore their "..
   "dictatorship. "..
   "The Commander is known to be a harsh leader, and will show you no "..
   "mercy. You are strongly recommended to resist with all the forces "..
   "you command. "
local briefingtext04 = 
   "Level 4: Shield of Wasteland... "..
   "In the Southern Mountains, your forces have found a bare plateau, "..
   "with many resources. When developed, it will become a jewel in "..
   "the rebellion. However, enemy troops are known to be active "..
   "on the far side of the mountains. "..
   "You must be cautious - if provoked, the leader of the enemy "..
   "troops will react with overwhelming force, and we will lose "..
   "this advantage. "..
   "Once you have built sufficient forces, taking over the enemy "..
   "station should be within the resources you have to hand. "..
   "Good luck. You'll need it."
local briefingtext05 = 
   "Level 5: Rochebrune... "..
   "In a barren corner of the Western Mountains, a scouting party of "..
   "your troops has run into an opposing scouting party. "..
   "Since both parties believe they outnumber the other, a short, "..
   "vicious battle will ensue. "..
   "Ensure your scouts annihilate their counterparts, before "..
   "reinforcements can arrive."
local briefingtext06 = 
   "Level 6: Aiglier... "..
   "In a sparsely resourced pass in the western mountains, your "..
   "scouts spot an enemy brigade setting up camp on the other "..
   "side, but are able to pull back before they are spotted. "..
   "They report that the brigade appears to be preparing to take "..
   "the pass. Take them out, and the mountains will be yours."
local briefingtext07 = 
   "Level 7: Bellegarde... "..
   "A thriving fortified camp has been spotted in the nearby "..
   "lowlands of Bellegarde. Surrounded by a rocky hills, the camp "..
   "appears well prepared to fend off attackers. The enemy has "..
   "sent Lieutenant-Colonel TJ to organize the defenses. "..
   "His reputation as an effective battlefield commander suggests "..
   "that you should not take him lightly. "..
   "Destroy the camp, and send him packing."
local briefingtext08 = 
   "Level 8: Savoy's Steps... "..
   "Having beaten all the troops the enemy can send into your "..
   "beloved mountain home, you've ventured down towards the captured "..
   "capital city. Ripping your way through the enemy troops, you've "..
   "reached the final pass between the hills to reach the capital "..
   "city. "..
   "Colonel TJ is preparing his troops to stop you. "..
   "You just need to tear through him, and you've got the enemy on "..
   "the run."
local briefingtext09 = 
   "Level 9: Savoy's Lake... "..
   "After the hard fighting of the Steps, your troops cheer as "..
   "you reach the Lake. They know they've almost won. "..
   "Before you can proceed to rescue the capital, you must clear "..
   "the area around Savoy's Lake. Lieutenant-General TJ "..
   "is prepared to stop you."
local briefingtext10 = 
   "Level 10: Fort Savoy "..
   "At long last, you have reach the Fort protecting the capital. "..
   "Most of the enemy forces have been destroyed, but there are "..
   "more than enough left here in the Fort to cause you trouble. "..
   "More worrying, General Szarin is here. And yes, he's "..
   "extremely unhappy at his son's death at your hands. "..
   "With luck, you won't give him a chance to explain how much in "..
   "person."

campaign_steps = {
  CreateMapStep("campaigns/elites/level01.smp",
      "Build your base. Drive the enemy out. Kill all enemy units.", 
      briefingtext01),
  CreateMapStep("campaigns/elites/level02.smp", 
      "Gain an ally and drive the enemy out. Kill all enemy units.", 
      briefingtext02),
  CreateMapStep("campaigns/elites/level03.smp",
      "Prepare your defences and destroy the enemy.",
      briefingtext03),
  CreateMapStep("campaigns/elites/level04.smp",
      "Build up your forces and destroy the enemy units.",
      briefingtext04),
  CreateMapStep("campaigns/elites/level05.smp", 
      "Annihilate the enemy scouting party.", 
      briefingtext05),
  CreateMapStep("campaigns/elites/level06.smp",
      "Take the mountain pass and destroy the enemy.",
      briefingtext06),
  CreateMapStep("campaigns/elites/level07.smp",
      "Wipe out the enemy camp.",
      briefingtext07),
  CreateMapStep("campaigns/elites/level08.smp",
      "Clear the final pass before the enemy capital.",
      briefingtext08),
  CreateMapStep("campaigns/elites/level09.smp",
      "Destroy the enemy troops, and take the lake.",
      briefingtext09),
  CreateMapStep("campaigns/elites/level10.smp",
      "The Fort stands between you and victory. Destroy it, and the enemy will fold.",
      briefingtext10)
}

