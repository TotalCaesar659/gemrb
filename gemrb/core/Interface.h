/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

/**
 * @file Interface.h
 * Declaration of Interface class, central interconnect for various GemRB parts
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include "SClassID.h"
#include "exports.h"

#include "Audio.h" // needed for _MSC_VER and SoundHandle (everywhere)
#include "Cache.h"
#include "Callback.h"
#include "GameData.h"
#include "GlobalTimer.h"
#include "GUI/Control.h"
#include "GUI/Tooltip.h"
#include "GUI/Window.h"
#include "GUI/GUIFactory.h"
#include "Holder.h"
#include "ImageMgr.h"
#include "InterfaceConfig.h"
#include "Resource.h"
#include "Timer.h"
#include "System/VFS.h"

#include <array>
#include <map>
#include <string>
#include <vector>

// No SFINAE
#if defined(_MSC_VER) || defined(__sgi)
#include "DataFileMgr.h"
#include "MusicMgr.h"
#include "SaveGame.h"
#include "ScriptEngine.h"
#include "StringMgr.h"
#include "SymbolMgr.h"
#include "Video.h"
#endif

namespace GemRB {

class Actor;
class Audio;
class CREItem;
class Calendar;
class Container;
class DataFileMgr;
struct Effect;
class EffectQueue;
class EffectDesc;
class Factory;
class Font;
class Game;
class GameControl;
class GlobalTimer;
class ITMExtHeader;
class Image;
class Item;
class KeyMap;
class Label;
class Map;
class MusicMgr;
class Palette;
using PaletteHolder = Holder<Palette>;
class ProjectileServer;
class Resource;
class SPLExtHeader;
class SaveGame;
class SaveGameIterator;
class ScriptEngine;
class ScriptedAnimation;
class Spell;
class Sprite2D;
class Store;
class StringMgr;
#ifndef __sgi
class SymbolMgr;
class TableMgr;
#endif
class TextArea;
class Variables;
class Video;
class WindowManager;
class WorldMap;
class WorldMapArray;

struct Symbol {
	Holder<SymbolMgr> sm;
	char ResRef[8];
};

struct SlotType {
	ieDword slot;
	ieDword slottype;
	ieDword slottip;
	ieDword slotid;
	ieDword sloteffects;
	ieDword slotflags;
	ieResRef slotresref;
};

struct DamageInfoStruct {
	unsigned int strref;
	unsigned int resist_stat;
	unsigned int value;
	int iwd_mod_type;
	int reduction;
	// maybe also add the ac bonus and/or the DL_ constants
};

struct TimeStruct {
	unsigned int round_sec;
	unsigned int turn_sec;
	unsigned int round_size; // in ticks
	unsigned int rounds_per_turn;
	unsigned int attack_round_size;
	unsigned int hour_sec;
	unsigned int hour_size;
	unsigned int day_sec;
	unsigned int day_size;

	int GetHour(unsigned int time) { return (time/AI_UPDATE_TIME)%day_sec/hour_sec; }
};

struct EncodingStruct
{
	std::string encoding;
	bool widechar;
	bool multibyte;
	bool zerospace;
};

struct SpellDescType {
	ieResRef resref;
	ieStrRef value;
};

struct SpecialSpellType {
	ieResRef resref;
	int flags;
	int amount;
	int bonus_limit;
};
#define SP_IDENTIFY  1      //any spell that cannot be cast from the menu
#define SP_SILENCE   2      //any spell that can be cast in silence
#define SP_SURGE     4      //any spell that cannot be cast during a wild surge
#define SP_REST      8      //any spell that is cast upon rest if memorized
#define SP_HEAL_ALL  16     //any healing spell that is cast upon rest at more than one target (healing circle, mass cure)

struct SurgeSpell {
	ieResRef spell;
	ieStrRef message;
};

class ItemList {
public:
	ieResRef *ResRefs;
	unsigned int Count;
	//if count is odd and the column titles start with 2, the random roll should be 2d((c+1)/2)-1
	bool WeightOdds;

	ItemList(unsigned int size, int label) {
		ResRefs = (ieResRef *) calloc(size, sizeof(ieResRef) );
		Count = size;
		if ((size&1) && (label==2)) {
			WeightOdds=true;
		} else {
			WeightOdds=false;
		}
	}
	~ItemList() {
		if (ResRefs) {
			free(ResRefs);
		}
	}
};

//quitflags
#define QF_NORMAL        0
#define QF_QUITGAME      1
#define QF_EXITGAME      2
#define QF_CHANGESCRIPT  4
#define QF_LOADGAME      8
#define QF_ENTERGAME     16
#define QF_KILL			32

//events that are called out of drawwindow
//they wait until the condition is right
#define EF_CONTROL       1        //updates the game window statuses
#define EF_SHOWMAP       2        //starts worldmap
#define EF_PORTRAIT      4        //updates portraits
#define EF_ACTION        8        //updates the actions bar
#define EF_UPDATEANIM    16       //updates avatar animation
#define EF_SEQUENCER     32       //starts sequencer/contingency creation
#define EF_IDENTIFY      64       //starts identify screen
#define EF_SELECTION     128      //selection changed
#define EF_OPENSTORE     256      //open store window
#define EF_EXPANSION     512      //upgrade game request
#define EF_CREATEMAZE    1024     //call the maze generator
#define EF_RESETTARGET   2048     //reset the mouse cursor
#define EF_TARGETMODE    4096     //update the mouse cursor
#define EF_TEXTSCREEN    8192     //start a textscreen

//autopause
#define AP_UNUSABLE      0
#define AP_ATTACKED      1
#define AP_HIT           2
#define AP_WOUNDED       3
#define AP_DEAD          4
#define AP_NOTARGET      5
#define AP_ENDROUND      6
#define AP_ENEMY         7
#define AP_TRAP          8
#define AP_SPELLCAST     9
#define AP_GENERIC       10  //needed for Android stuff

//pause flags
#define PF_QUIET  1        //no feedback
#define PF_FORCED 2        //pause even in cutscene/dialog

/** ea relations (derivated from 2 actor's EA value) */
#define EAR_FRIEND  0
#define EAR_NEUTRAL 1
#define EAR_HOSTILE 2

/** Max size of actor's ground circle (PST) */
#define MAX_CIRCLE_SIZE  3

/** Summoning */
#define EAM_SOURCEALLY 0
#define EAM_SOURCEENEMY 1
#define EAM_ENEMY   2
#define EAM_ALLY    3
#define EAM_NEUTRAL 4
#define EAM_DEFAULT 5
//
#define STAT_CON_HP_NORMAL   0
#define STAT_CON_HP_WARRIOR  1
#define STAT_CON_HP_MIN      2
#define STAT_CON_HP_REGEN    3
#define STAT_CON_FATIGUE     4
#define STAT_CON_TNO_REGEN   4

#define STAT_DEX_REACTION    0
#define STAT_DEX_MISSILE     1
#define STAT_DEX_AC          2

#define STAT_INT_LEARN       0
#define STAT_INT_MAXLEVEL    1
#define STAT_INT_MAXNUMBER   2

//sloteffects (querysloteffect returns it)
#define SLOT_EFFECT_NONE     0
#define SLOT_EFFECT_ITEM     1 //normal equipped item
#define SLOT_EFFECT_FIST     2 //fist slot
#define SLOT_EFFECT_MAGIC    3 //magic weapon slot
#define SLOT_EFFECT_MELEE    4 //normal weapon slot
#define SLOT_EFFECT_MISSILE  5 //quiver slots
#define SLOT_EFFECT_LEFT     6 //shield (left hand) slot
#define SLOT_EFFECT_HEAD     7 //head slot

enum PluginFlagsType {
	PLF_NORMAL,
	PLF_SKIP,
	PLF_DELAY
};

enum PauseSetting {
	PAUSE_OFF = 0,
	PAUSE_ON = 1
};

enum RESOURCE_DIRECTORY {
	DIRECTORY_CHR_PORTRAITS,
	DIRECTORY_CHR_SOUNDS,
	DIRECTORY_CHR_EXPORTS,
	DIRECTORY_CHR_SCRIPTS
};

enum FeedbackType {
	FT_TOHIT = 1,
	FT_COMBAT = 2,
	FT_ACTIONS = 4, // handled by Actor::CommandActor
	FT_STATES = 8,
	FT_SELECTION = 16, // handled by Actor::PlaySelectionSound
	FT_MISC = 32,
	FT_CASTING = 64
};

enum DebugModeBits {
	ID_REFERENCE = 1,
	ID_CUTSCENE = 2,
	ID_VARIABLES = 4,
	ID_ACTIONS = 8,
	ID_TRIGGERS = 16,
	ID_VIEWS = 32,
	ID_WINDOWS = 64,
	ID_FONTS = 128,
	ID_TEXT = 256
};

template<int SIZE>
using ColorPal = std::array<Color, SIZE>;

// TODO: there is no reason why this can't be generated directly from
// the inventory button drag event using the button value as the slot id
// to get the appropriate CREItem
struct ItemDragOp : public Control::ControlDragOp {
	CREItem* item;

	ItemDragOp(CREItem* item);
	
private:
	static Control dragDummy;
};

/**
 * @class Interface
 * Central interconnect for all GemRB parts, driving functions and utility functions possibly belonging to a better place
 */

class GEM_EXPORT Interface
{
private:
	// dirvers must be deallocated last (keep them at the top)
	// we hold onto resources (sprites etc) in Interface that must be destroyed prior to the respective driver
	Holder<Video> video;
	Holder<Audio> AudioDriver;

	std::string VideoDriverName;
	std::string AudioDriverName;
	ProjectileServer * projserv;

	WindowManager* winmgr;
	Holder<GUIFactory> guifact;
	Holder<ScriptEngine> guiscript;
	GameControl* gamectrl = nullptr;
	SaveGameIterator *sgiterator;
	Variables * vars;
	Variables * tokens;
	Variables * lists;
	Holder<MusicMgr> music;
	std::vector<Symbol> symbols;
	Holder<DataFileMgr> INIparty;
	Holder<DataFileMgr> INIbeasts;
	Holder<DataFileMgr> INIquests;
	Holder<DataFileMgr> INIresdata;
	Game * game;
	Calendar * calendar;
	WorldMapArray* worldmap;
	ieDword GameFeatures[(GF_COUNT+31)/32];
	ResRef MainCursorsImage;
	ResRef TextCursorBam;
	ResRef ScrollCursorBam;
	ieResRef GroundCircleBam[MAX_CIRCLE_SIZE];
	int GroundCircleScale[MAX_CIRCLE_SIZE];

	std::map<ResRef, Font*> fonts;
	ResRef ButtonFontResRef;
	ResRef MovieFontResRef;
	ResRef TextFontResRef;
	ResRef TooltipFontResRef;
	std::string DefaultWindowTitle;

	TooltipBackground* TooltipBG;
	ieResRef *DefSound; //default sounds
	int DSCount;

	ResRef Palette16;
	ResRef Palette32;
	ResRef Palette256;
	std::vector<ColorPal<256>> palettes256;
	std::vector<ColorPal<32>>  palettes32;
	std::vector<ColorPal<16>>  palettes16;

	ieDword* slotmatrix; //itemtype vs slottype
	std::vector<std::vector<int> > itemtypedata; //armor failure, critical multiplier, critical range
	SlotType* slottypes;
	int ItemTypes;

	// Currently dragged item or NULL
	std::unique_ptr<ItemDragOp> DraggedItem;
	// Current Store
	Store* CurrentStore;
	// Index of current container
	Container* CurrentContainer;
	bool UseContainer;
	// Scrolling speed
	int mousescrollspd;
	bool update_scripts;
	/** Next Script Name */
	char NextScript[64];

	std::deque<Timer> timers;
	int SpecialSpellsCount;
	SpecialSpellType *SpecialSpells;
	KeyMap *keymap;
	std::string Encoding;
	Scriptable *CutSceneRunner;
	int debugMode = 0;

public:
	const char * SystemEncoding;
	EncodingStruct TLKEncoding;
	Holder<StringMgr> strings;
	Holder<StringMgr> strings2;
	GlobalTimer timer;
	Color InfoTextColor;
	int SaveAsOriginal; //if true, saves files in compatible mode
	int QuitFlag;
	int EventFlag;
	Holder<SaveGame> LoadGameIndex;
	int VersionOverride;
	unsigned int SlotTypes; //this is the same as the inventory size
	ResRef GlobalScript;
	ResRef WorldMapName[2];
	Variables * AreaAliasTable;
	std::vector<Holder<Sprite2D> > Cursors;
	Holder<Sprite2D> FogSprites[16] {};
	Holder<Sprite2D> GroundCircles[MAX_CIRCLE_SIZE][6] {};
	std::vector<char *> musiclist;
	std::multimap<ieDword, DamageInfoStruct> DamageInfoMap;
	TimeStruct Time;
	std::vector<SurgeSpell> SurgeSpells;
public:
	Interface();
	~Interface(void);
	
	Interface(const Interface&) = delete;
	
	int Init(InterfaceConfig* config);
	//TODO: Core Methods in Interface Class
	void SetFeature(int value, int position);
	/* don't rely on the exact return value of this function */
	ieDword HasFeature(int position) const;
	bool IsAvailable(SClass_ID filetype) const;
	const char * TypeExt(SClass_ID type) const;
	ProjectileServer* GetProjectileServer() const;
	Video * GetVideoDriver() const;
	/* create or change a custom string */
	ieStrRef UpdateString(ieStrRef strref, const char *text) const;
	/* returns a newly created c string */
	char* GetCString(ieStrRef strref, ieDword options = 0) const;
	/* returns a newly created string */
	String* GetString(ieStrRef strref, ieDword options = 0) const;
	/* sets the floattext color */
	void SetInfoTextColor(const Color &color);
	/** returns a gradient set */
	const ColorPal<16>& GetPalette16(uint8_t idx) const { return (idx >= palettes16.size()) ? palettes16[0] : palettes16[idx]; }
	const ColorPal<32>& GetPalette32(uint8_t idx) const { return (idx >= palettes32.size()) ? palettes32[0] : palettes32[idx]; }
	const ColorPal<256>& GetPalette256(uint8_t idx) const { return (idx >= palettes256.size()) ? palettes256[0] : palettes256[idx]; }
	/** Returns a preloaded Font */
	Font* GetFont(const ResRef&) const;
	Font* GetTextFont() const;
	/** Returns the button font */
	Font * GetButtonFont() const;
	/** Get GUI Script Manager */
	ScriptEngine * GetGUIScriptEngine() const;
	/** core for summoning creatures, returns the last created Actor
	may apply a single fx on the summoned creature normally an unsummon effect */
	Actor *SummonCreature(const ieResRef resource, const ieResRef vvcres, Scriptable *Owner, Actor *target, const Point &position, int eamod, int level, Effect *fx, bool sexmod=1);
	/** Get the Window Manager */
	WindowManager *GetWindowManager() const { return winmgr; };
	/** Loads a Window in the Window Manager */
	Window* LoadWindow(ScriptingId WindowID, const ResRef& ref, Window::WindowPosition = Window::PosCentered);
	/** Creates a Window in the Window Manager */
#undef CreateWindow // Win32 might define this, so nix it
	Window* CreateWindow(unsigned short WindowID, const Region&);
	void ToggleViewsVisible(bool visible, const ResRef& group);
	void ToggleViewsEnabled(bool enabled, const ResRef& group);

	Tooltip CreateTooltip();
	/** returns the label which should receive game messages (overrides messagetextarea) */
	Label *GetMessageLabel() const;
	/** returns the textarea of the main game screen */
	TextArea *GetMessageTextArea() const;
	void SetFeedbackLevel(int level);
	/** returns true if the passed feedback type is enabled */
	bool HasFeedback(int type) const;
	/** Get the SaveGameIterator */
	SaveGameIterator * GetSaveGameIterator() const;
	/** Get the Variables Dictionary */
	Variables * GetDictionary() const;
	/** Get the Token Dictionary */
	Variables * GetTokenDictionary() const;
	/** Get the Music Manager */
	MusicMgr * GetMusicMgr() const;
	/** Loads an IDS Table, returns -1 on error or the Symbol Table Index on success */
	int LoadSymbol(const char * ResRef);
	/** Gets the index of a loaded Symbol Table, returns -1 on error */
	int GetSymbolIndex(const char * ResRef) const;
	/** Gets a Loaded Symbol Table by its index, returns NULL on error */
	Holder<SymbolMgr> GetSymbol(unsigned int index) const;
	/** Frees a Loaded Symbol Table, returns false on error, true on success */
	bool DelSymbol(unsigned int index);
	/** Plays a Movie */
	int PlayMovie(const char * ResRef);
	/** Generates traditional random number xdy+z */
	int Roll(int dice, int size, int add) const;
	/** Loads a Game Compiled Script */
	int LoadScript(const char * ResRef);
	/** store the scriptable running the cutscene */
	void SetCutSceneRunner(Scriptable *runner);
	Scriptable *GetCutSceneRunner() const { return CutSceneRunner; };
	/** Enables/Disables the CutScene Mode */
	void SetCutSceneMode(bool active);
	/** returns true if in cutscene mode */
	bool InCutSceneMode() const;
	/** Updates the Game Script Engine State */
	bool GSUpdate(bool update_scripts);
	/** Get the Party INI Interpreter */
	DataFileMgr * GetPartyINI() const
	{
		return INIparty.get();
	}
	DataFileMgr * GetBeastsINI() const
	{
		return INIbeasts.get();
	}
	DataFileMgr * GetQuestsINI() const
	{
		return INIquests.get();
	}
	DataFileMgr * GetResDataINI() const
	{
		return INIresdata.get();
	}
	/** Gets the Game class */
	Game * GetGame() const
	{
		return game;
	}
	/** Gets the Calendar class */
	Calendar * GetCalendar() const
	{
		return calendar;
	}

	/** Gets the KeyMap class */
	KeyMap * GetKeyMap() const
	{
		return keymap;
	}

	/** Gets the WorldMap class, returns the current worldmap or the first worldmap containing the area*/
	WorldMap * GetWorldMap(const char *area = NULL);
	GameControl *GetGameControl() const { return game ? gamectrl : nullptr; }
	/** if backtomain is not null then goes back to main screen */
	void QuitGame(int backtomain);
	/** sets up load game */
	void SetupLoadGame(Holder<SaveGame> save, int ver_override);
	/** load saved game by index (-1 is default), ver_override is an optional parameter
	to override the saved game's version */
	void LoadGame(SaveGame *save, int ver_override);
	/** reloads the world map from a resource file */
	void UpdateWorldMap(ResRef wmResRef);
	/** fix changes in global script/worldmap*/
	void UpdateMasterScript();

	DirectoryIterator GetResourceDirectory(RESOURCE_DIRECTORY);

	unsigned int GetInventorySize() const { return SlotTypes-1; }
	ieDword FindSlot(unsigned int idx) const;
	ieDword QuerySlot(unsigned int idx) const;
	ieDword QuerySlotType(unsigned int idx) const;
	ieDword QuerySlottip(unsigned int idx) const;
	ieDword QuerySlotID(unsigned int idx) const;
	ieDword QuerySlotFlags(unsigned int idx) const;
	ieDword QuerySlotEffects(unsigned int idx) const;
	const char * QuerySlotResRef(unsigned int idx) const;
	int GetArmorFailure(unsigned int itemtype) const;
	int GetShieldFailure(unsigned int itemtype) const;
	int GetArmorPenalty(unsigned int itemtype) const;
	int GetShieldPenalty(unsigned int itemtype) const;
	int GetCriticalMultiplier(unsigned int itemtype) const;
	int GetCriticalRange(unsigned int itemtype) const;
	/*returns true if an itemtype is acceptable for a slottype, also checks the usability flags */
	int CanUseItemType(int slottype, const Item *item, const Actor *actor = nullptr, bool feedback = false, bool equipped = false) const;
	/*removes single file from cache*/
	void RemoveFromCache(const ieResRef resref, SClass_ID SClassID);
	/*removes all files from directory*/
	void DelTree(const char *path, bool onlysaved);
	/*returns 0,1,2 based on how the file should be saved */
	int SavedExtension(const char *filename);
	/*returns true if the file should never be deleted accidentally */
	bool ProtectedExtension(const char *filename);
	/*returns true if the directory path isn't good as a Cache */
	bool StupidityDetector(const char* Pt);
	bool InDebugMode(int mode) const { return debugMode & mode; };
	void SetDebugMode(int mode) { debugMode = mode; };
	/*handles the load screen*/
	void LoadProgress(int percent);

	void DragItem(CREItem* item, const ieResRef Picture);
	const ItemDragOp* GetDraggedItem() const { return DraggedItem.get(); }
	/* use this only when the dragged item is dropped */
	void ReleaseDraggedItem();
	CREItem *ReadItem(DataStream *str);
	CREItem *ReadItem(DataStream *str, CREItem *itm);
	void SanitizeItem(CREItem *item) const;
	bool ResolveRandomItem(CREItem *itm);
	ieStrRef GetRumour(const ieResRef resname);
	Container *GetCurrentContainer();
	int CloseCurrentContainer();
	void SetCurrentContainer(Actor *actor, Container *arg, bool flag=false);
	Store *GetCurrentStore();
	void CloseCurrentStore();
	Store *SetCurrentStore(const ieResRef resname, ieDword owner);
	void SetMouseScrollSpeed(int speed);
	int GetMouseScrollSpeed();

	//creates a standalone effect with opcode
	Effect *GetEffect(ieDword opcode);
	/** plays stock gui sound referenced by index */
	Holder<SoundHandle> PlaySound(int idx, unsigned int channel);
	/** returns the first selected PC, if forced is set, then it returns
	first PC if none was selected */
	Actor *GetFirstSelectedPC(bool forced);
	Actor *GetFirstSelectedActor();
	/** is an area loaded? (prefer Game::GetCurrentArea if including Game.h makes sense) */
	bool HasCurrentArea() const;
	/** returns a cursor sprite (not cached) */
	Holder<Sprite2D> GetCursorSprite();
	/** returns a scroll cursor sprite */
	Holder<Sprite2D> GetScrollCursorSprite(int frameNum, int spriteNum);
	/** returns 0 for unmovable, -1 for movable items, otherwise it
	returns gold value! */
	int CanMoveItem(const CREItem *item) const;
	int GetRareSelectSoundCount() const;
	int GetMaximumAbility() const;
	int GetStrengthBonus(int column, int value, int ex) const;
	int GetIntelligenceBonus(int column, int value) const;
	int GetDexterityBonus(int column, int value) const;
	int GetConstitutionBonus(int column, int value) const;
	int GetCharismaBonus(int column, int value) const;
	int GetLoreBonus(int column, int value) const;
	int GetWisdomBonus(int column, int value) const;
	int GetReputationMod(int column) const;

	/** applies the spell on the target */
	void ApplySpell(const ieResRef resname, Actor *target, Scriptable *caster, int level);
	/** applies the spell on the area or on a scriptable object */
	void ApplySpellPoint(const ieResRef resname, Map *area, const Point &pos, Scriptable *caster, int level);
	/** applies a single effect on the target */
	int ApplyEffect(Effect *fx, Actor *target, Scriptable *caster);
	/** applies an effect queue on the target */
	int ApplyEffectQueue(EffectQueue *fxqueue, Actor *actor, Scriptable *caster);
	int ApplyEffectQueue(EffectQueue *fxqueue, Actor *actor, Scriptable *caster, Point p);
	Effect *GetEffect(const ieResRef resname, int level, const Point &p);
	/** dumps an area object to the cache */
	int SwapoutArea(Map *map);
	/** saves (exports a character to the characters folder */
	int WriteCharacter(const char *name, Actor *actor);
	/** saves the game object to the destination folder */
	int WriteGame(const char *folder);
	/** saves the worldmap object to the destination folder */
	int WriteWorldMap(const char *folder);
	/** saves the .are and .sto files to the destination folder */
	int CompressSave(const char *folder);
	/** toggles the pause. returns either PAUSE_ON or PAUSE_OFF to reflect the script state after toggling. */
	PauseSetting TogglePause();
	/** returns true the passed pause setting was applied. false otherwise. */
	bool SetPause(PauseSetting pause, int flags = 0);
	/** receives an autopause reason, returns true if autopause was accepted and successful */
	bool Autopause(ieDword flag, Scriptable *target);
	/** registers engine opcodes */
	void RegisterOpcodes(int count, const EffectDesc *opcodes);
	/** reads a list of resrefs into an array, returns array size */
	int ReadResRefTable(const ieResRef tablename, ieResRef *&data);
	/** frees the data */
	void FreeResRefTable(ieResRef *&table, int &count);
	/** Returns the virtual worldmap entry of a sub-area */
	int GetAreaAlias(const ieResRef areaname) const;
	/** Returns up to 3 resources from resref, choosing rows randomly
	unwanted return variables could be omitted */
	void GetResRefFrom2DA(const ieResRef resref, ieResRef resource1, ieResRef resource2 = NULL, ieResRef resource3 = NULL);
	/** returns a numeric list read from a 2da. The 0th element is the number of elements in the list */
	ieDword *GetListFrom2DA(const ieResRef resref);
	/** translates a stat symbol to numeric value */
	ieDword TranslateStat(const char *stat_name);
	/** resolves a stat bonus based on multiple stats */
	int ResolveStatBonus(Actor *actor, const char *tablename, ieDword flags = 0, int value = 0);
	/** Opens CD prompt window and waits for the specified disc */
	void WaitForDisc(int disc_number, const char* path);
	/** Returns the music playlist corresponding to the provided type */
	/** it allows scrapping the entry, hence it isn't const */
	char *GetMusicPlaylist(int SongType) const;
	/** Returns the DeathVarFormat of the day */
	static const char *GetDeathVarFormat();
	int CheckSpecialSpell(const ieResRef resref, Actor *actor);
	int GetSpecialSpell(const ieResRef resref);
	int GetSpecialSpellsCount() { return SpecialSpellsCount; }
	SpecialSpellType *GetSpecialSpells() { return SpecialSpells; }
	/** Saves config variables to a file */
	bool SaveConfig();
private:
	int LoadSprites();
	int LoadFonts();
	bool LoadGemRBINI();
	/** Load the encoding table selected in gemrb.cfg */
	bool LoadEncoding();

	template<int SIZE>
	bool LoadPalette(ResRef resref, std::vector<ColorPal<SIZE>>& palettes)
	{
		static_assert(SIZE == 16 || SIZE == 32 || SIZE == 256, "invalid palette size");

		ResourceHolder<ImageMgr> palim = GetResourceHolder<ImageMgr>(resref);
		if (palim) {
			auto image = palim->GetImage();
			int height = image->GetHeight();
			palettes.resize(height);
			for (int row = 0; row < height; ++row) {
				for (int col = 0; col < SIZE; ++col) {
					palettes[row][col] = image->GetPixel(col, row);
				}
			}
			delete image;
			return true;
		}
		return false;
	}

	bool InitializeVarsWithINI(const char * iniFileName);
	bool InitItemTypes();
	bool ReadRandomItems();
	bool ReadItemTable(const ieResRef item, const char *Prefix);
	bool ReadAbilityTables();
	bool ReadAbilityTable(const ieResRef name, ieWordSigned *mem, int cols, int rows);
	bool ReadMusicTable(const ieResRef name, int col);
	bool ReadDamageTypeTable();
	bool ReadReputationModTable();
	bool ReadGameTimeTable();
	bool ReadSpecialSpells();
	bool ReadSoundChannelsTable();
	/** Reads table of area name mappings for WorldMap (PST only) */
	bool ReadAreaAliasTable(const ieResRef name);
	/** handles the QuitFlag bits (main loop events) */
	void HandleFlags();
	/** handles the EventFlag bits (conditional events) */
	void HandleEvents();
	/** handles hardcoded gui behaviour */
	void HandleGUIBehaviour();
	/** Creates a game control, closes all other windows */
	GameControl* StartGameControl();
	/** Executes everything (non graphical) in the main game loop */
	void GameLoop(void);
	/** the internal (without cache) part of GetListFrom2DA */
	ieDword *GetListFrom2DAInternal(const ieResRef resref);

public:
	char GameDataPath[_MAX_PATH];
	char GameOverridePath[_MAX_PATH];
	char GameSoundsPath[_MAX_PATH];
	char GameScriptsPath[_MAX_PATH];
	char GamePortraitsPath[_MAX_PATH];
	char GameCharactersPath[_MAX_PATH];
	char GemRBOverridePath[_MAX_PATH];
	char GemRBUnhardcodedPath[_MAX_PATH];
	ieResRef GameNameResRef;
	ieResRef GoldResRef; //MISC07.itm
	Variables *RtRows;
	char CustomFontPath[_MAX_PATH];
	char GameName[_MAX_PATH];
	char GameType[10];
	int GamepadPointerSpeed = 10;
	char GemRBPath[_MAX_PATH];
	char PluginsPath[_MAX_PATH];
	char CachePath[_MAX_PATH];
	char GUIScriptsPath[_MAX_PATH];
	char SavePath[_MAX_PATH];
	char INIConfig[_MAX_PATH];
	char GamePath[_MAX_PATH];
	std::vector<std::string> CD[MAX_CD];
	std::vector<std::string> ModPath;
	int Width = 640, Height = 480, Bpp = 32;
	int IgnoreOriginalINI;
	bool DitherSprites = true;
	bool CaseSensitive = true, DrawFPS = false;
	bool UseSoftKeyboard; // TODO: reevaluate the need for this, see comments in StartTextInput
	unsigned short NumFingScroll, NumFingKboard, NumFingInfo;
	int MouseFeedback;
	int MaxPartySize;
	bool KeepCache;
	bool MultipleQuickSaves;
	bool UseCorruptedHack;
	int FeedbackLevel;

	Variables *plugin_flags;
	/** The Main program loop */
	void Main(void);
	/** returns true if the game is paused */
	bool IsFreezed();
	void AskAndExit();
	void ExitGemRB(void);
	/** CheatKey support */
	inline void EnableCheatKeys(int Flag)
	{
		CheatFlag=(Flag > 0);
	}

	inline bool CheatEnabled()
	{
		return CheatFlag;
	}

	inline void SetEventFlag(int Flag)
	{
		EventFlag|=Flag;
	}
	inline void ResetEventFlag(int Flag)
	{
		EventFlag&=~Flag;
	}

	static void SanityCheck(const char *ver);

	/** Set Next Script */
	void SetNextScript(const char *script);
	/** Cheats enabled? */
	bool CheatFlag;

	Audio* GetAudioDrv(void) const;

	Timer& SetTimer(const EventHandler&, tick_t interval, int repeats = -1);
};

extern GEM_EXPORT Interface * core;

}

#endif
