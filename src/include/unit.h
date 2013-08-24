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
/**@name unit.h - The unit headerfile. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

#ifndef __UNIT_H__
#define __UNIT_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>

#ifndef __UNITTYPE_H__
#include "unittype.h"
#endif

#ifndef __PLAYER_H__
#include "player.h"
#endif

#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CAnimation;
class CBuildRestrictionOnTop;
class CConstructionFrame;
class CFile;
class Missile;
class CMapField;
class COrder;
class CPlayer;
class CUnit;
class CUnitColors;
class CUnitPtr;
class CUnitStats;
class CUnitType;
class CUpgrade;
class CVariable;
class CViewport;
class PathFinderData;
class SpellType;
struct lua_State;

typedef COrder *COrderPtr;

/*
** Configuration of the small (unit) AI.
*/
#define PRIORITY_FACTOR   0x10000000
#define HEALTH_FACTOR     0x00000001
#define DISTANCE_FACTOR   0x00010000
#define INRANGE_FACTOR    0x00001000
#define INRANGE_BONUS     0x00100000
#define CANATTACK_BONUS   0x00010000
#define AIPRIORITY_BONUS  0x01000000


/// Called whenever the selected unit was updated
extern void SelectedUnitChanged();

/// Returns the map distance between to unittype as locations
extern int MapDistanceBetweenTypes(const CUnitType &src, const Vec2i &pos1,
								   const CUnitType &dst, const Vec2i &pos2);

/**
**  Unit/Missile headings.
**          N
**  NW              NE
**  W                E
**  SW              SE
**          S
*/
enum _directions_ {
	LookingN  = 0 * 32,      /// Unit looking north
	LookingNE = 1 * 32,      /// Unit looking north east
	LookingE  = 2 * 32,      /// Unit looking east
	LookingSE = 3 * 32,      /// Unit looking south east
	LookingS  = 4 * 32,      /// Unit looking south
	LookingSW = 5 * 32,      /// Unit looking south west
	LookingW  = 6 * 32,      /// Unit looking west
	LookingNW = 7 * 32       /// Unit looking north west
};

#define NextDirection 32        /// Next direction N->NE->E...
#define UnitNotSeen 0x7fffffff  /// Unit not seen, used by CUnit::SeenFrame

/// The big unit structure
class CUnit
{
public:
	CUnit() : tilePos(-1, -1), pathFinderData(NULL), SavedOrder(NULL), NewOrder(NULL), CriticalOrder(NULL) { Init(); }

	void Init();

	COrder *CurrentOrder() const { return Orders[0]; }

	unsigned int CurrentAction() const;

	bool IsIdle() const;

	void ClearAction();

	/// Increase a unit's reference count
	void RefsIncrease();
	/// Decrease a unit's reference count
	void RefsDecrease();

	/// Initialize unit structure with default values
	void Init(const CUnitType &type);
	/// Assign unit to player
	void AssignToPlayer(CPlayer &player);

	/// Draw a single unit
	void Draw(const CViewport &vp) const;
	/// Place a unit on map
	void Place(const Vec2i &pos);

	/// Move unit to tile(pos). (Do special stuff : vision, cachelist, pathfinding)
	void MoveToXY(const Vec2i &pos);
	/// Add a unit inside a container. Only deal with list stuff.
	void AddInContainer(CUnit &host);
	/// Change owner of unit
	void ChangeOwner(CPlayer &newplayer);

	/// Remove unit from map/groups/...
	void Remove(CUnit *host);

	void AssignWorkerToMine(CUnit &mine);
	void DeAssignWorkerFromMine(CUnit &mine);

	/// Release a unit
	void Release(bool final = false);

	bool RestoreOrder();
	bool CanStoreOrder(COrder *order);

	// Cowards and invisible units don't attack unless ordered.
	bool IsAgressive() const {
		return (Type->CanAttack && !Type->Coward
				&& Variable[INVISIBLE_INDEX].Value == 0);
	}

	/// Returns true, if unit is directly seen by an allied unit.
	bool IsVisible(const CPlayer &player) const;

	inline bool IsInvisibile(const CPlayer &player) const {
		return (&player != Player && !!Variable[INVISIBLE_INDEX].Value
				&& !player.IsBothSharedVision(*Player));
	}

	/**
	**  Returns true if unit is alive.
	**  Another unit can interact only with alive map units.
	**
	**  @return        True if alive, false otherwise.
	*/
	bool IsAlive() const;

	/**
	**  Returns true if unit is alive and on the map.
	**  Another unit can interact only with alive map units.
	**
	**  @return        True if alive, false otherwise.
	*/
	inline bool IsAliveOnMap() const {
		return !Removed && IsAlive();
	}

	/**
	**  Returns true, if unit is visible as an action goal for a player on the map.
	**
	**  @param player  Player to check for.
	**
	**  @return        True if visible, false otherwise.
	*/
	inline bool IsVisibleAsGoal(const CPlayer &player) const {
		// Invisibility
		if (IsInvisibile(player)) {
			return false;
		}
		// Don't attack revealers
		if (this->Type->Revealer) {
			return false;
		}
		if ((player.Type == PlayerComputer && !this->Type->PermanentCloak)
			|| IsVisible(player) || IsVisibleOnRadar(player)) {
			return IsAliveOnMap();
		} else {
			return Type->VisibleUnderFog
				   && (Seen.ByPlayer & (1 << player.Index))
				   && !(Seen.Destroyed & (1 << player.Index));
		}
	}

	/**
	**  Returns true, if unit is visible for this player on the map.
	**  The unit has to be out of fog of war and alive
	**
	**  @param player  Player to check for.
	**
	**  @return        True if visible, false otherwise.
	*/
	inline bool IsVisibleOnMap(const CPlayer &player) const {
		return IsAliveOnMap() && !IsInvisibile(player) && IsVisible(player);
	}

	/// Returns true if unit is visible on minimap. Only for ThisPlayer.
	bool IsVisibleOnMinimap() const;

	// Returns true if unit is visible under radar (By player, or by shared vision)
	bool IsVisibleOnRadar(const CPlayer &pradar) const;

	/// Returns true if unit is visible in a viewport. Only for ThisPlayer.
	bool IsVisibleInViewport(const CViewport &vp) const;

	bool IsEnemy(const CPlayer &player) const;
	bool IsEnemy(const CUnit &unit) const;
	bool IsAllied(const CPlayer &player) const;
	bool IsAllied(const CUnit &unit) const;
	bool IsSharedVision(const CPlayer &player) const;
	bool IsSharedVision(const CUnit &unit) const;
	bool IsBothSharedVision(const CPlayer &player) const;
	bool IsBothSharedVision(const CUnit &unit) const;
	bool IsTeamed(const CPlayer &player) const;
	bool IsTeamed(const CUnit &unit) const;

	bool IsUnusable(bool ignore_built_state = false) const;

	/**
	 **  Returns the map distance between this unit and dst units.
	 **
	 **  @param dst  Distance to this unit.
	 **
	 **  @return     The distance between in tiles.
	 */
	int MapDistanceTo(const CUnit &dst) const {
		return MapDistanceBetweenTypes(*Type, tilePos, *dst.Type, dst.tilePos);
	}

	int MapDistanceTo(const Vec2i &pos) const;

	/**
	**  Test if unit can move.
	**  For the moment only check for move animation.
	**
	**  @return true if unit can move.
	*/
	bool CanMove() const { return Type->CanMove(); }

	int GetDrawLevel() const;

	PixelPos GetMapPixelPosTopLeft() const;
	PixelPos GetMapPixelPosCenter() const;

public:
	class CUnitManagerData
	{
		friend class CUnitManager;
	public:
		CUnitManagerData() : slot(-1), unitSlot(-1) {}

		int GetUnitId() const { return slot; }
	private:
		int slot;           /// index in UnitManager::unitSlots
		int unitSlot;       /// index in UnitManager::units
	};
public:
	// @note int is faster than shorts
	unsigned int     Refs;         /// Reference counter
	unsigned int     ReleaseCycle; /// When this unit could be recycled
	CUnitManagerData UnitManagerData;
	size_t PlayerSlot;  /// index in Player->Units

	int    InsideCount;   /// Number of units inside.
	int    BoardCount;    /// Number of units transported inside.
	CUnit *UnitInside;    /// Pointer to one of the units inside.
	CUnit *Container;     /// Pointer to the unit containing it (or 0)
	CUnit *NextContained; /// Next unit in the container.
	CUnit *PrevContained; /// Previous unit in the container.

	CUnit *NextWorker; //pointer to next assigned worker to "Goal" resource.
	struct {
		CUnit *Workers; /// pointer to first assigned worker to this resource.
		int Assigned; /// how many units are assigned to harvesting from the resource.
		int Active; /// how many units are harvesting from the resource.
	} Resource; /// Resource still

	Vec2i tilePos; /// Map position X

	unsigned int Offset;/// Map position as flat index offset (x + y * w)

	const CUnitType  *Type;        /// Pointer to unit-type (peon,...)
	CPlayer    *Player;            /// Owner of this unit
	const CUnitStats *Stats;       /// Current unit stats
	int         CurrentSightRange; /// Unit's Current Sight Range

	// Pathfinding stuff:
	PathFinderData *pathFinderData;

	// DISPLAY:
	int         Frame;      /// Image frame: <0 is mirrored
	CUnitColors *Colors;    /// Player colors

	signed char IX;         /// X image displacement to map position
	signed char IY;         /// Y image displacement to map position
	unsigned char Direction; //: 8; /// angle (0-255) unit looking
	unsigned char CurrentResource;
	int ResourcesHeld;      /// Resources Held by a unit

	unsigned char DamagedType;   /// Index of damage type of unit which damaged this unit
	unsigned long Attacked;      /// gamecycle unit was last attacked
	unsigned Blink : 3;          /// Let selection rectangle blink
	unsigned Moving : 1;         /// The unit is moving
	unsigned ReCast : 1;         /// Recast again next cycle
	unsigned AutoRepair : 1;     /// True if unit tries to repair on still action.

	unsigned Burning : 1;        /// unit is burning
	unsigned Destroyed : 1;      /// unit is destroyed pending reference
	unsigned Removed : 1;        /// unit is removed (not on map)
	unsigned Selected : 1;       /// unit is selected

	unsigned Constructed : 1;    /// Unit is in construction
	unsigned Active : 1;         /// Unit is active for AI
	unsigned Boarded : 1;        /// Unit is on board a transporter.
	unsigned CacheLock : 1;      /// Unit is on lock by unitcache operations.

	unsigned Summoned : 1;       /// Unit is summoned using spells.

	unsigned TeamSelected;  /// unit is selected by a team member.
	CPlayer *RescuedFrom;        /// The original owner of a rescued unit.
	/// NULL if the unit was not rescued.
	/* Seen stuff. */
	int VisCount[PlayerMax];     /// Unit visibility counts
	struct _seen_stuff_ {
		_seen_stuff_() : CFrame(NULL), Type(NULL), tilePos(-1, -1) {}
		const CConstructionFrame  *CFrame;  /// Seen construction frame
		int         Frame;                  /// last seen frame/stage of buildings
		const CUnitType  *Type;             /// Pointer to last seen unit-type
		Vec2i       tilePos;                /// Last unit->tilePos Seen
		signed char IX;                     /// Seen X image displacement to map position
		signed char IY;                     /// seen Y image displacement to map position
		unsigned    Constructed : 1;        /// Unit seen construction
		unsigned    State : 3;              /// Unit seen build/upgrade state
		unsigned    Destroyed : PlayerMax;  /// Unit seen destroyed or not
		unsigned    ByPlayer : PlayerMax;   /// Track unit seen by player
	} Seen;

	CVariable *Variable; /// array of User Defined variables.

	unsigned long TTL;  /// time to live

	int GroupId;        /// unit belongs to this group id
	int LastGroup;      /// unit belongs to this last group

	unsigned int Wait;          /// action counter
	int Threshold;              /// The counter while ai unit couldn't change target.

	struct _unit_anim_ {
		const CAnimation *Anim;      /// Anim
		const CAnimation *CurrAnim;  /// CurrAnim
		int Wait;                    /// Wait
		int Unbreakable;             /// Unbreakable
	} Anim;


	std::vector<COrder *> Orders; /// orders to process
	COrder *SavedOrder;         /// order to continue after current
	COrder *NewOrder;           /// order for new trained units
	COrder *CriticalOrder;      /// order to do as possible in breakable animation.

	char *AutoCastSpell;        /// spells to auto cast
	int *SpellCoolDownTimers;   /// how much time unit need to wait before spell will be ready

	CUnit *Goal; /// Generic/Teleporter goal pointer
};

#define NoUnitP (CUnit *)0        /// return value: for no unit found

/**
**  Returns unit number (unique to this unit)
*/
#define UnitNumber(unit) ((unit).UnitManagerData.GetUnitId())

/**
**  User preference.
*/
class CPreference
{
public:
	CPreference() : ShowSightRange(false), ShowReactionRange(false),
		ShowAttackRange(false), ShowMessages(true),
		BigScreen(false), PauseOnLeave(true),  ShowOrders(0), ShowNameDelay(0),
		ShowNameTime(0) {};

	bool ShowSightRange;     /// Show sight range.
	bool ShowReactionRange;  /// Show reaction range.
	bool ShowAttackRange;    /// Show attack range.
	bool ShowMessages;		 /// Show messages.
	bool BigScreen;			 /// If true, shows the big screen(without panels)
	bool PauseOnLeave;       /// If true, game pauses when cursor is gone

	int  ShowOrders;         /// How many second show orders of unit on map.
	int  ShowNameDelay;      /// How many cycles need to wait until unit's name popup will appear.
	int  ShowNameTime;       /// How many cycles need to show unit's name popup.
};

extern CPreference Preference;

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

// in unit_draw.c
/// @todo could be moved into the user interface ?
extern unsigned long ShowOrdersCount;   /// Show orders for some time
extern unsigned long ShowNameDelay;     /// Delay to show unit's name
extern unsigned long ShowNameTime;      /// Show unit's name for some time
extern bool EnableTrainingQueue;               /// Config: training queues enabled
extern bool EnableBuildingCapture;             /// Config: building capture enabled
extern bool RevealAttacker;				       /// Config: reveal attacker enabled
extern int ResourcesMultiBuildersMultiplier;   /// Config: spend resources for building with multiple workers
extern const CViewport *CurrentViewport; /// CurrentViewport
extern void DrawUnitSelection(const CViewport &vp, const CUnit &unit);
extern void (*DrawSelection)(IntColor, int, int, int, int);
extern int MaxSelectable;                  /// How many units could be selected

extern CUnit **Selected;                    /// currently selected units
extern CUnit **TeamSelected[PlayerMax];     /// teams currently selected units
extern int     NumSelected;                 /// how many units selected
extern int     TeamNumSelected[PlayerMax];  /// Number of Units a team member has selected

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/// Mark the field with the FieldFlags.
void MarkUnitFieldFlags(const CUnit &unit);
/// Unmark the field with the FieldFlags.
void UnmarkUnitFieldFlags(const CUnit &unit);
/// Update unit->CurrentSightRange.
void UpdateUnitSightRange(CUnit &unit);
/// Create a new unit
extern CUnit *MakeUnit(const CUnitType &type, CPlayer *player);
/// Create a new unit and place on map
extern CUnit *MakeUnitAndPlace(const Vec2i &pos, const CUnitType &type, CPlayer *player);
/// Handle the loss of a unit (food,...)
extern void UnitLost(CUnit &unit);
/// Remove the Orders of a Unit
extern void UnitClearOrders(CUnit &unit);
/// @todo more docu
extern void UpdateForNewUnit(const CUnit &unit, int upgrade);
/// @todo more docu
extern void NearestOfUnit(const CUnit &unit, const Vec2i &pos, Vec2i *dpos);

extern CUnit *GetFirstContainer(const CUnit &unit);

/// Call when an Unit goes under fog.
extern void UnitGoesUnderFog(CUnit &unit, const CPlayer &player);
/// Call when an Unit goes out of fog.
extern void UnitGoesOutOfFog(CUnit &unit, const CPlayer &player);

/// Does a recount for VisCount
extern void UnitCountSeen(CUnit &unit);

/// Check for rescue each second
extern void RescueUnits();

/// Convert direction (dx,dy) to heading (0-255)
extern int DirectionToHeading(const Vec2i &dir);
/// Convert direction (dx,dy) to heading (0-255)
extern int DirectionToHeading(const PixelDiff &dir);

///Correct directions for placed wall.
extern void CorrectWallDirections(CUnit &unit);
/// Correct the surrounding walls.
extern void CorrectWallNeighBours(CUnit &unit);

/// Update frame from heading
extern void UnitUpdateHeading(CUnit &unit);
/// Heading and frame from delta direction
extern void UnitHeadingFromDeltaXY(CUnit &unit, const Vec2i &delta);


/// @todo more docu
extern void DropOutOnSide(CUnit &unit, int heading, const CUnit *container);
/// @todo more docu
extern void DropOutNearest(CUnit &unit, const Vec2i &goalPos, const CUnit *container);

/// Drop out all units in the unit
extern void DropOutAll(const CUnit &unit);

/// Return the rule used to build this building.
extern CBuildRestrictionOnTop *OnTopDetails(const CUnit &unit, const CUnitType *parent);
/// @todo more docu
extern CUnit *CanBuildHere(const CUnit *unit, const CUnitType &type, const Vec2i &pos);
/// @todo more docu
extern bool CanBuildOn(const Vec2i &pos, int mask);
/// FIXME: more docu
extern CUnit *CanBuildUnitType(const CUnit *unit, const CUnitType &type, const Vec2i &pos, int real);
/// Get the suitable animation frame depends of unit's damaged type.
extern int ExtraDeathIndex(const char *death);

/// Get unit under cursor
extern CUnit *UnitOnScreen(int x, int y);

/// Let a unit die
extern void LetUnitDie(CUnit &unit);
/// Destroy all units inside another unit
extern void DestroyAllInside(CUnit &source);
/// Calculate some value to measure the unit's priority for AI
extern int ThreatCalculate(const CUnit &unit, const CUnit &dest);
/// Hit unit with damage, if destroyed give attacker the points
extern void HitUnit(CUnit *attacker, CUnit &target, int damage, const Missile *missile = NULL);

/// Calculate the distance from current view point to coordinate
extern int ViewPointDistance(const Vec2i &pos);
/// Calculate the distance from current view point to unit
extern int ViewPointDistanceToUnit(const CUnit &dest);

/// Can this unit-type attack the other (destination)
extern int CanTarget(const CUnitType &type, const CUnitType &dest);
/// Can transporter transport the other unit
extern int CanTransport(const CUnit &transporter, const CUnit &unit);

/// Generate a unit reference, a printable unique string for unit
extern std::string UnitReference(const CUnit &unit);
/// Generate a unit reference, a printable unique string for unit
extern std::string UnitReference(const CUnitPtr &unit);

/// save unit-structure
extern void SaveUnit(const CUnit &unit, CFile &file);

/// Initialize unit module
extern void InitUnits();
/// Clean unit module
extern void CleanUnits();

// in unit_draw.c
//--------------------
/// Draw nothing around unit
extern void DrawSelectionNone(IntColor, int, int, int, int);
/// Draw circle around unit
extern void DrawSelectionCircle(IntColor, int, int, int, int);
/// Draw circle filled with alpha around unit
extern void DrawSelectionCircleWithTrans(IntColor, int, int, int, int);
/// Draw rectangle around unit
extern void DrawSelectionRectangle(IntColor, int, int, int, int);
/// Draw rectangle filled with alpha around unit
extern void DrawSelectionRectangleWithTrans(IntColor, int, int, int, int);
/// Draw corners around unit
extern void DrawSelectionCorners(IntColor, int, int, int, int);

/// Register CCL decorations features
extern void DecorationCclRegister();
/// Load the decorations (health,mana) of units
extern void LoadDecorations();
/// Clean the decorations (health,mana) of units
extern void CleanDecorations();

/// Draw unit's shadow
extern void DrawShadow(const CUnitType &type, int frame, const PixelPos &screenPos);
/// Draw all units visible on map in viewport
extern int FindAndSortUnits(const CViewport &vp, std::vector<CUnit *> &table);

/// Show a unit's orders.
extern void ShowOrder(const CUnit &unit);

// in groups.c

/// Initialize data structures for groups
extern void InitGroups();
/// Save groups
extern void SaveGroups(CFile &file);
/// Cleanup groups
extern void CleanGroups();

// 2 functions to conseal the groups internal data structures...
/// Get the number of units in a particular group
extern int GetNumberUnitsOfGroup(int num, GroupSelectionMode mode = SELECTABLE_BY_RECTANGLE_ONLY);
/// Get the array of units of a particular group
extern CUnit **GetUnitsOfGroup(int num);

/// Remove all units from a group
extern void ClearGroup(int num);
/// Add the array of units to the group
extern void AddToGroup(CUnit **units, int nunits, int num);
/// Set the contents of a particular group with an array of units
extern void SetGroup(CUnit **units, int nunits, int num);
/// Remove a unit from a group
extern void RemoveUnitFromGroups(CUnit &unit);
/// Register CCL group features
extern void GroupCclRegister();
extern bool IsGroupTainted(int num);

// in selection.c

/// Check if unit is the currently only selected
#define IsOnlySelected(unit) (NumSelected == 1 && Selected[0] == &(unit))

///  Save selection to restore after.
extern void SaveSelection();
///  Restore selection.
extern void RestoreSelection();
/// Clear current selection
extern void UnSelectAll();
/// Changed TeamUnit Selection
extern void ChangeTeamSelectedUnits(CPlayer &player, const std::vector<CUnit *> &units);
/// Add a unit to selection
extern int SelectUnit(CUnit &unit);
/// Select one unit as selection
extern void SelectSingleUnit(CUnit &unit);
/// Remove a unit from selection
extern void UnSelectUnit(CUnit &unit);
/// Add a unit to selected if not already selected, remove it otherwise
extern int ToggleSelectUnit(CUnit &unit);
/// Select units from the same type (if selectable by rectangle)
extern int SelectUnitsByType(CUnit &base);
/// Toggle units from the same type (if selectable by rectangle)
extern int ToggleUnitsByType(CUnit &base);
/// Select the units belonging to a particular group
extern int SelectGroup(int group_number, GroupSelectionMode mode = SELECTABLE_BY_RECTANGLE_ONLY);
/// Add the units from the same group as the one in parameter
extern int AddGroupFromUnitToSelection(CUnit &unit);
/// Select the units from the same group as the one in parameter
extern int SelectGroupFromUnit(CUnit &unit);
/// Select the units in the selection rectangle
extern int SelectUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright);
/// Select ground units in the selection rectangle
extern int SelectGroundUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright);
/// Select flying units in the selection rectangle
extern int SelectAirUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright);
/// Add the units in the selection rectangle to the current selection
extern int AddSelectedUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright);
/// Add ground units in the selection rectangle to the current selection
extern int AddSelectedGroundUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright);
/// Add flying units in the selection rectangle to the current selection
extern int AddSelectedAirUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright);

/// Init selections
extern void InitSelections();
/// Save current selection state
extern void SaveSelections(CFile &file);
/// Clean up selections
extern void CleanSelections();
/// Register CCL selection features
extern void SelectionCclRegister();

// in ccl_unit.c

/// register CCL units features
extern void UnitCclRegister();

//@}

#endif // !__UNIT_H__
