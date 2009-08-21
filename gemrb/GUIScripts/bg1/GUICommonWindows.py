# -*-python-*-
# GemRB - Infinity Engine Emulator
# Copyright (C) 2003 The GemRB Project
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# $Id$


# GUICommonWindows.py - functions to open common
# windows in lower part of the screen
###################################################

import GemRB
from GUIDefines import *
from ie_stats import *
from ie_modal import *
from ie_action import *
from GUICommon import *
from BGCommon import *
from LUCommon import *

FRAME_PC_SELECTED = 0
FRAME_PC_TARGET   = 1

PortraitWindow = None
OptionsWindow = None
ActionsWindow = None
DraggedPortrait = None

def SetupMenuWindowControls (Window, Gears, ReturnToGame):
	global OptionsWindow

	OptionsWindow = Window
	# FIXME: add "(key)" to tooltips!

	# Return to Game
	Button = Window.GetControl (0)
	Button.SetTooltip (16313)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	# enabled BAM isn't present in .chu, defining it here
	Button.SetSprites ("GUILSOP", 0,16,17,28,16)
	Button.SetVarAssoc ("SelectedWindow", 0)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, ReturnToGame)
	Button.SetFlags (IE_GUI_BUTTON_CANCEL, OP_OR)

	# Map
	Button = Window.GetControl (1)
	Button.SetTooltip (16310)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	Button.SetSprites ("GUILSOP", 0,0,1,20,0)
	Button.SetVarAssoc ("SelectedWindow", 1)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenMapWindow")

	# Journal
	Button = Window.GetControl (2)
	Button.SetTooltip (16308)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	Button.SetSprites ("GUILSOP", 0,4,5,22,4)
	Button.SetVarAssoc ("SelectedWindow", 2)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenJournalWindow")

	# Inventory
	Button = Window.GetControl (3)
	Button.SetTooltip (16307)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	Button.SetSprites ("GUILSOP", 0,2,3,21,2)
	Button.SetVarAssoc ("SelectedWindow", 3)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenInventoryWindow")

	# Records
	Button = Window.GetControl (4)
	Button.SetTooltip (16306)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	Button.SetSprites ("GUILSOP", 0,6,7,23,6)
	Button.SetVarAssoc ("SelectedWindow", 4)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenRecordsWindow")

	# Mage
	Button = Window.GetControl (5)
	Button.SetTooltip (16309)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	Button.SetSprites ("GUILSOP", 0,8,9,24,8)
	Button.SetVarAssoc ("SelectedWindow", 5)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenMageWindow")

	# Priest
	Button = Window.GetControl (6)
	Button.SetTooltip (14930)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	Button.SetSprites ("GUILSOP", 0,10,11,25,10)
	Button.SetVarAssoc ("SelectedWindow", 6)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenPriestWindow")

	# Options
	Button = Window.GetControl (7)
	Button.SetTooltip (16311)
	#Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON, OP_OR)
	Button.SetSprites ("GUILSOP", 0,12,13,26,12)
	Button.SetVarAssoc ("SelectedWindow", 7)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenOptionsWindow")

	# Party mgmt
	Button = Window.GetControl (8)
	Button.SetTooltip (16312)
	Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "OpenPartyWindow")
	
	#gears
	if Gears:
		# Pendulum, gears, sun/moon dial  (time)
		# FIXME: display all animations: CPEN, CGEAR, CDIAL
		Button = Window.GetControl (9)
		Button.SetAnimation ("CGEAR")
		Button.SetState (IE_GUI_BUTTON_ENABLED)
		Button.SetFlags (IE_GUI_BUTTON_PICTURE|IE_GUI_BUTTON_ANIMATED|IE_GUI_BUTTON_NORMAL, OP_SET)
		Button.SetEvent(IE_GUI_BUTTON_ON_PRESS, "GearsClicked")
		SetGamedaysAndHourToken()
		Button.SetTooltip(16041)

	MarkMenuButton (Window)

	return

def MarkMenuButton (WindowIndex):
	Pressed = WindowIndex.GetControl( GemRB.GetVar ("SelectedWindow") )

	for button in range (9):
		Button = WindowIndex.GetControl (button)
		Button.SetState (IE_GUI_BUTTON_ENABLED)

	if Pressed:
		Button = Pressed
	else: # highlight return to game
		Button = WindowIndex.GetControl (0)

	Button.SetState (IE_GUI_BUTTON_SELECTED)

def AIPress ():
	Button = PortraitWindow.GetControl (6)
	AI = GemRB.GetMessageWindowSize () & GS_PARTYAI

	if AI:
		GemRB.GameSetScreenFlags (GS_PARTYAI, OP_NAND)
		Button.SetTooltip (15918)
	else:
		GemRB.GameSetScreenFlags (GS_PARTYAI, OP_OR)
		Button.SetTooltip (15917)
	return

def RestPress ():
	GemRB.RestParty(0,0,0)
	return

def EmptyControls ():
	global ActionsWindow

	GemRB.SetVar ("ActionLevel", 0)
	Window = ActionsWindow
	for i in range (12):
		Button = Window.GetControl (i)
		Button.SetFlags (IE_GUI_BUTTON_NO_IMAGE, OP_SET)
		Button.SetPicture ("")
	return

def SelectFormationPreset ():
	GemRB.GameSetFormation (GemRB.GetVar ("Value"), GemRB.GetVar ("Formation") )
	GroupControls ()
	return

def SetupFormation ():
	global ActionsWindow

	Window = ActionsWindow
	for i in range (12):
		Button = Window.GetControl (i)
		Button.SetFlags (IE_GUI_BUTTON_NORMAL, OP_SET)
		Button.SetSprites ("GUIBTBUT",0,0,1,2,3)
		Button.SetBAM ("FORM%x"%i,0,0,-1)
		Button.SetVarAssoc ("Value", i)
		Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "SelectFormationPreset")
	return

def SelectFormation ():
	GemRB.GameSetFormation ( GemRB.GetVar ("Formation") )
	return

def GroupControls ():
	global ActionsWindow

	GemRB.SetVar ("ActionLevel", 0)
	Window = ActionsWindow
	Button = Window.GetControl (0)
	Button.SetActionIcon (7)
	Button = Window.GetControl (1)
	Button.SetActionIcon (15)
	Button = Window.GetControl (2)
	Button.SetActionIcon (21)
	Button = Window.GetControl (3)
	Button.SetActionIcon (-1)
	Button = Window.GetControl (4)
	Button.SetActionIcon (-1)
	Button = Window.GetControl (5)
	Button.SetActionIcon (-1)
	Button = Window.GetControl (6)
	Button.SetActionIcon (-1)
	GemRB.SetVar ("Formation", GemRB.GameGetFormation ())
	for i in range (5):
		Button = Window.GetControl (7+i)
		Button.SetState (IE_GUI_BUTTON_ENABLED)
		idx = GemRB.GameGetFormation (i)
		Button.SetFlags (IE_GUI_BUTTON_RADIOBUTTON|IE_GUI_BUTTON_NORMAL, OP_SET)
		Button.SetSprites ("GUIBTBUT",0,0,1,2,3)
		Button.SetBAM ("FORM%x"%idx,0,0,-1)
		Button.SetVarAssoc ("Formation", i)
		Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "SelectFormation")
		Button.SetEvent (IE_GUI_BUTTON_ON_RIGHT_PRESS, "SetupFormation")
		str = GemRB.GetString (4935)
		Button.SetTooltip ("F%d - %s"%(8+i,str) )
	return

def OpenActionsWindowControls (Window):
	global ActionsWindow

	ActionsWindow = Window
	# Gears (time) when options pane is down
	#Button = Window.GetControl (62)
	#Button.SetAnimation ("CGEAR")
	#Button.SetFlags (IE_GUI_BUTTON_PICTURE | IE_GUI_BUTTON_ANIMATED, OP_SET)
	#Button.SetState (IE_GUI_BUTTON_LOCKED)
	UpdateActionsWindow ()
	return

def UpdateActionsWindow ():
	global ActionsWindow
	global level, TopIndex

	if ActionsWindow == -1:
		return

	if ActionsWindow == None:
		return

	pc = 0
	for i in range (PARTY_SIZE):
		if GemRB.GameIsPCSelected (i+1):
			if pc == 0:
				pc = i+1
			else:
				pc = -1
				break

	#setting up the disabled button overlay (using the second border slot)
	for i in range (12):
		Button = ActionsWindow.GetControl (i)
		Button.SetBorder (0,6,6,4,4,0,254,0,255)
		Button.SetBorder (1, 0, 0, 0, 0, 50,30,10,120, 0, 1)
		Button.SetFont ("NUMBER")
		Button.SetText ("")

	if pc == 0:
		EmptyControls ()
		return
	if pc == -1:
		GroupControls ()
		return
	#this is based on class

	level = GemRB.GetVar ("ActionLevel")
	TopIndex = GemRB.GetVar ("TopIndex")
	if level == 0:
		ActionsWindow.SetupControls (pc)
	elif level == 1:
		ActionsWindow.SetupEquipmentIcons(pc, TopIndex)
	elif level == 2: #spells
		GemRB.SetVar ("Type", 3)
		ActionsWindow.SetupSpellIcons(pc, 3, TopIndex)
	elif level == 3: #innates
		GemRB.SetVar ("Type", 4)
		ActionsWindow.SetupSpellIcons(pc, 4, TopIndex)
	return

def OpenFloatMenuWindow ():
	GemRB.GameControlSetTargetMode (TARGET_MODE_NONE)

def ActionTalkPressed ():
	GemRB.GameControlSetTargetMode (TARGET_MODE_TALK)

def ActionAttackPressed ():
	GemRB.GameControlSetTargetMode (TARGET_MODE_ATTACK)

def ActionDefendPressed ():
	GemRB.GameControlSetTargetMode (TARGET_MODE_DEFEND)

def ActionThievingPressed ():
	GemRB.GameControlSetTargetMode (TARGET_MODE_PICK)

def ActionQWeaponPressed (which):
	pc = GemRB.GameGetFirstSelectedPC ()

	if GemRB.GetEquippedQuickSlot (pc,1)==which and GemRB.GameControlGetTargetMode() != TARGET_MODE_ATTACK:
		GemRB.GameControlSetTargetMode (TARGET_MODE_ATTACK)
	else:
		GemRB.GameControlSetTargetMode (TARGET_MODE_NONE)
		GemRB.SetEquippedQuickSlot (pc, which)

	ActionsWindow.SetupControls (pc)
	UpdateActionsWindow ()
	return

def ActionQWeapon1Pressed ():
	ActionQWeaponPressed(0)

def ActionQWeapon2Pressed ():
	ActionQWeaponPressed(1)

def ActionQWeapon3Pressed ():
	ActionQWeaponPressed(2)

def ActionQWeapon4Pressed ():
	ActionQWeaponPressed(3)

def ActionStopPressed ():
	for i in range (PARTY_SIZE):
		if GemRB.GameIsPCSelected(i + 1):
			GemRB.ClearActions(i + 1)
	return

#no check needed because the button wouldn't be drawn if illegal
def ActionLeftPressed ():
	TopIndex = GemRB.GetVar ("TopIndex")
	if TopIndex>10:
		TopIndex -= 10
	else:
		TopIndex = 0
	GemRB.SetVar ("TopIndex", TopIndex)
	UpdateActionsWindow ()
	return

#no check needed because the button wouldn't be drawn if illegal
def ActionRightPressed ():
	pc = GemRB.GameGetFirstSelectedPC ()
	TopIndex = GemRB.GetVar ("TopIndex")
	Type = GemRB.GetVar ("Type")
	Max = GemRB.GetMemorizedSpellsCount(pc, Type)
	TopIndex += 10
	if TopIndex > Max - 10:
		if Max>10:
			TopIndex = Max-10
		else:
			TopIndex = 0

	GemRB.SetVar ("TopIndex", TopIndex)
	UpdateActionsWindow ()
	return

def ActionBardSongPressed ():
	pc = GemRB.GameGetFirstSelectedPC ()
	GemRB.SetModalState (pc, MS_BATTLESONG)
	UpdateActionsWindow ()
	return

def ActionSearchPressed ():
	pc = GemRB.GameGetFirstSelectedPC ()
	GemRB.SetModalState (pc, MS_DETECTTRAPS)
	UpdateActionsWindow ()
	return

def ActionStealthPressed ():
	pc = GemRB.GameGetFirstSelectedPC ()
	GemRB.SetModalState (pc, MS_STEALTH)
	UpdateActionsWindow ()
	return
	
def ActionTurnPressed ():
	pc = GemRB.GameGetFirstSelectedPC ()
	GemRB.SetModalState (pc, MS_TURNUNDEAD)
	UpdateActionsWindow ()
	return

def ActionUseItemPressed ():
	GemRB.SetVar ("TopIndex", 0)
	GemRB.SetVar ("ActionLevel", 1)
	UpdateActionsWindow ()
	return

def ActionCastPressed ():
	GemRB.SetVar ("TopIndex", 0)
	GemRB.SetVar ("ActionLevel", 2)
	UpdateActionsWindow ()
	return

def ActionQItemPressed (action):
	pc = GemRB.GameGetFirstSelectedPC ()
	#quick slot
	GemRB.UseItem (pc, -2, action)
	return
	
def ActionQItem1Pressed ():
	ActionQItemPressed (ACT_QSLOT1)
	return

def ActionQItem2Pressed ():
	ActionQItemPressed (ACT_QSLOT2)
	return

def ActionQItem3Pressed ():
	ActionQItemPressed (ACT_QSLOT3)
	return

def ActionQItem4Pressed ():
	ActionQItemPressed (ACT_QSLOT4)
	return

def ActionQItem5Pressed ():
	ActionQItemPressed (ACT_QSLOT5)
	return

def ActionInnatePressed ():
	GemRB.SetVar ("TopIndex", 0)
	GemRB.SetVar ("ActionLevel", 3)
	UpdateActionsWindow ()
	return

def SpellPressed ():
	pc = GemRB.GameGetFirstSelectedPC ()

	GemRB.GameControlSetTargetMode (TARGET_MODE_CAST)
	Spell = GemRB.GetVar ("Spell")
	Type = GemRB.GetVar ("Type")
	GemRB.SpellCast (pc, Type, Spell)
	GemRB.SetVar ("ActionLevel", 0)
	UpdateActionsWindow ()
	return

def EquipmentPressed ():
	pc = GemRB.GameGetFirstSelectedPC ()

	GemRB.GameControlSetTargetMode (TARGET_MODE_CAST)
	Item = GemRB.GetVar ("Equipment")
	#equipment index
	GemRB.UseItem (pc, -1, Item)
	GemRB.SetVar ("ActionLevel", 0)
	UpdateActionsWindow ()
	return

def GetActorClassTitle (actor):
	"""Returns the string representation of the actors class."""

	ClassTitle = GemRB.GetPlayerStat (actor, IE_TITLE1)

	if ClassTitle == 0:
		Class = GemRB.GetPlayerStat (actor, IE_CLASS)
		ClassIndex = ClassTable.FindValue ( 5, Class )
		KitIndex = GetKitIndex (actor)
		Multi = ClassTable.GetValue (ClassIndex, 4)
		Dual = IsDualClassed (actor, 1)

		if Multi and Dual[0] == 0: # true multi class
			ClassTitle = ClassTable.GetValue (ClassIndex, 2)
			ClassTitle = GemRB.GetString (ClassTitle)
		else:
			if Dual[0]: # dual class
				# first (previous) kit or class of the dual class
				if Dual[0] == 1:
					ClassTitle = KitListTable.GetValue (Dual[1], 2)
				elif Dual[0] == 2:
					ClassTitle = ClassTable.GetValue (Dual[1], 2)
				ClassTitle = GemRB.GetString (ClassTitle) + " / "
				ClassTitle += GemRB.GetString (ClassTable.GetValue (Dual[2], 2))
			else: # ordinary class or kit
				if KitIndex:
					ClassTitle = KitListTable.GetValue (KitIndex, 2)
				else:
					ClassTitle = ClassTable.GetValue (ClassIndex, 2)
				ClassTitle = GemRB.GetString (ClassTitle)

	if ClassTitle == "*":
		return 0
	return ClassTitle

def GetActorPaperDoll (actor):
	PortraitTable = GemRB.LoadTableObject ("PDOLLS")
	anim_id = GemRB.GetPlayerStat (actor, IE_ANIMATION_ID)
	level = GemRB.GetPlayerStat (actor, IE_ARMOR_TYPE)
	row = "0x%04X" %anim_id
	which = "LEVEL%d" %(level+1)
	return PortraitTable.GetValue (row, which)

SelectionChangeHandler = None

def SetSelectionChangeHandler (handler):
	global SelectionChangeHandler

	# Switching from walking to non-walking environment:
	# set the first selected PC in walking env as a selected
	# in nonwalking env
	if (not SelectionChangeHandler) and handler:
		sel = GemRB.GameGetFirstSelectedPC ()
		if not sel:
			sel = 1
		GemRB.GameSelectPCSingle (sel)

	SelectionChangeHandler = handler

	# redraw selection on change main selection | single selection
	SelectionChanged ()

def RunSelectionChangeHandler ():
	if SelectionChangeHandler:
		SelectionChangeHandler ()

def OpenPortraitWindow (needcontrols):
	global PortraitWindow

	PortraitWindow = Window = GemRB.LoadWindowObject (1)

	# AI
	if needcontrols:
		Button = Window.GetControl (6)
		GSFlags = GemRB.GetMessageWindowSize ()&GS_PARTYAI
		Button.SetFlags (IE_GUI_BUTTON_CHECKBOX, OP_OR)
		#this control is crippled
		Button.SetSprites ("GUIBTACT", 0, 46, 47, 48, 49)
		Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "AIPress")
		Button.SetVarAssoc ("", GSFlags)
		if GSFlags:
			Button.SetTooltip (15917)
		else:
			Button.SetTooltip (15918)

		#Select All
		Button = Window.GetControl (7)
		Button.SetTooltip (10485)
		Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "SelectAllOnPress")

	pc = GemRB.GameGetSelectedPCSingle ()
	Inventory = GemRB.GetVar ("Inventory")

	for i in range (PARTY_SIZE):
		Button = Window.GetControl (i)
		Button.SetVarAssoc ("PressedPortrait", i)

		if (needcontrols):
			Button.SetEvent (IE_GUI_BUTTON_ON_RIGHT_PRESS, "OpenInventoryWindowClick")
		else:
			Button.SetEvent (IE_GUI_BUTTON_ON_RIGHT_PRESS, "PortraitButtonOnPress")

		Button.SetEvent (IE_GUI_BUTTON_ON_PRESS, "PortraitButtonOnPress")
		Button.SetEvent (IE_GUI_BUTTON_ON_SHIFT_PRESS, "PortraitButtonOnShiftPress")
		Button.SetEvent (IE_GUI_BUTTON_ON_DRAG_DROP, "OnDropItemToPC")
		Button.SetEvent (IE_GUI_BUTTON_ON_DRAG, "PortraitButtonOnDrag")
		Button.SetEvent (IE_GUI_MOUSE_ENTER_BUTTON, "PortraitButtonOnMouseEnter")
		Button.SetEvent (IE_GUI_MOUSE_LEAVE_BUTTON, "PortraitButtonOnMouseLeave")
		if Inventory and pc != i+1:
			Button.SetFlags (IE_GUI_BUTTON_NO_IMAGE, OP_SET)
			Button.SetState (IE_GUI_BUTTON_DISABLED)
			Button.SetText ("")
			Button.SetTooltip ("")

		Button.SetBorder (FRAME_PC_SELECTED, 1, 1, 2, 2, 0, 255, 0, 255)
		Button.SetBorder (FRAME_PC_TARGET, 3, 3, 4, 4, 255, 255, 0, 255)

	UpdatePortraitWindow ()
	SelectionChanged ()
	return Window

def UpdatePortraitWindow ():
	Window = PortraitWindow

	pc = GemRB.GameGetSelectedPCSingle ()
	Inventory = GemRB.GetVar ("Inventory")

	for i in range (PARTY_SIZE):
		Button = Window.GetControl (i)
		pic = GemRB.GetPlayerPortrait (i+1, 1)
		if Inventory and pc != i+1:
			pic = None
		if not pic:
			Button.SetFlags (IE_GUI_BUTTON_NO_IMAGE, OP_SET)
			Button.SetState (IE_GUI_BUTTON_DISABLED)
			Button.SetText ("")
			Button.SetTooltip ("")
			continue

		Button.SetFlags (IE_GUI_BUTTON_PICTURE|IE_GUI_BUTTON_ALIGN_BOTTOM|IE_GUI_BUTTON_ALIGN_LEFT|IE_GUI_BUTTON_HORIZONTAL|IE_GUI_BUTTON_DRAGGABLE, OP_SET)
		Button.SetState (IE_GUI_BUTTON_ENABLED)
		Button.SetPicture (pic, "NOPORTSM")
		hp = GemRB.GetPlayerStat (i+1, IE_HITPOINTS)
		hp_max = GemRB.GetPlayerStat (i+1, IE_MAXHITPOINTS)
		state = GemRB.GetPlayerStat (i+1, IE_STATE_ID)

		if (hp_max<1):
			ratio = 0.0
		else:
			ratio = (hp+0.0) / hp_max

		if hp<1 or (state & STATE_DEAD):
			Button.SetOverlay (ratio, 64,64,64,200, 64,64,64,200)
		else:
			Button.SetOverlay (ratio, 255,0,0,200, 128,0,0,200)
		Button.SetTooltip (GemRB.GetPlayerName (i+1, 1) + "\n%d/%d" %(hp, hp_max))
	return

def PortraitButtonOnDrag ():
	global DraggedPortrait

	#they start from 1
	DraggedPortrait = GemRB.GetVar ("PressedPortrait")+1
	GemRB.DragItem (DraggedPortrait, -1, "")
	return

def PortraitButtonOnPress ():
	i = GemRB.GetVar ("PressedPortrait")
	
	if GemRB.GameControlGetTargetMode() != TARGET_MODE_NONE:
		GemRB.ActOnPC(i+1)
		return

	if (not SelectionChangeHandler):
		if GemRB.GameIsPCSelected (i+1):
			GemRB.GameControlSetScreenFlags (SF_CENTERONACTOR, OP_OR)
		GemRB.GameSelectPC (i + 1, True, SELECT_REPLACE)
	else:
		GemRB.GameSelectPCSingle (i + 1)
		SelectionChanged ()
		RunSelectionChangeHandler ()
	return

def PortraitButtonOnShiftPress ():
	i = GemRB.GetVar ("PressedPortrait")

	if (not SelectionChangeHandler):
		sel = GemRB.GameIsPCSelected (i + 1)
		sel = not sel
		GemRB.GameSelectPC (i + 1, sel)
	else:
		GemRB.GameSelectPCSingle (i + 1)
		SelectionChanged ()
		RunSelectionChangeHandler ()
	return

def SelectAllOnPress ():
	GemRB.GameSelectPC (0, 1)
	return

# Run by Game class when selection was changed
def SelectionChanged ():
	global PortraitWindow

	GemRB.SetVar ("ActionLevel", 0)
	if (not SelectionChangeHandler):
		UpdateActionsWindow ()
		for i in range (PARTY_SIZE):
			Button = PortraitWindow.GetControl (i)
			Button.EnableBorder (FRAME_PC_SELECTED, GemRB.GameIsPCSelected (i + 1))
	else:
		sel = GemRB.GameGetSelectedPCSingle ()

		for i in range (PARTY_SIZE):
			Button = PortraitWindow.GetControl (i)
			Button.EnableBorder (FRAME_PC_SELECTED, i + 1 == sel)
	return

def PortraitButtonOnMouseEnter ():
	global DraggedPortrait

	i = GemRB.GetVar ("PressedPortrait")

	if DraggedPortrait != None:
		GemRB.DragItem (0, -1, "")
		#this might not work
		GemRB.SwapPCs (DraggedPortrait, i+1)
		DraggedPortrait = None
		return

	if GemRB.IsDraggingItem ():
		Button = PortraitWindow.GetControl (i)
		Button.EnableBorder (FRAME_PC_TARGET, 1)
	return

def PortraitButtonOnMouseLeave ():
	i = GemRB.GetVar ("PressedPortrait")
	Button = PortraitWindow.GetControl (i)
	Button.EnableBorder (FRAME_PC_TARGET, 0)
	return

def GetSavingThrow (SaveName, row, level):
	SaveTable = GemRB.LoadTableObject (SaveName)
	tmp = SaveTable.GetValue (level)
	return tmp

def GearsClicked():
	GemRB.GamePause(2,0)
