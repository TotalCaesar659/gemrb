/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "GUI/Button.h"

#include "GUI/GameControl.h"
#include "GUI/EventMgr.h"
#include "GUI/ScrollBar.h"
#include "GUI/Window.h"

#include "defsounds.h"
#include "ie_cursors.h"

#include "Game.h"
#include "GameData.h"
#include "Palette.h"

#include <cmath>

#define IS_PORTRAIT (Picture && ((flags&IE_GUI_BUTTON_PORTRAIT) == IE_GUI_BUTTON_PORTRAIT))

namespace GemRB {

Button::Button(Region& frame)
	: Control(frame),
	buttonImages()
{
	ControlType = IE_GUI_BUTTON;
	State = IE_GUI_BUTTON_UNPRESSED;
	HotKeyCallback = METHOD_CALLBACK(&Button::HandleHotKey, this);

	hasText = false;
	SetFont(core->GetButtonFont());
	SetFlags(IE_GUI_BUTTON_NORMAL, OP_OR);
	ToggleState = false;
	pulseBorder = false;
	Picture = NULL;
	Clipping = 1.0;

	PushOffset = Point(2, 2);
}

Button::~Button()
{
	SetImage(BUTTON_IMAGE_NONE, NULL);
	ClearPictureList();

	if (hotKey.global) {
		UnregisterHotKey();
	}
}

void Button::UnregisterHotKey()
{
	if (hotKey) {
		if (hotKey.global) {
			EventMgr::UnRegisterHotKeyCallback(HotKeyCallback, hotKey.key, hotKey.mod);
		} else {
			window->UnRegisterHotKeyCallback(HotKeyCallback, hotKey.key);
		}
	}
}

/** Sets the 'type' Image of the Button to 'img'.
 see 'BUTTON_IMAGE_TYPE' */
void Button::SetImage(BUTTON_IMAGE_TYPE type, Holder<Sprite2D> img)
{
	if (type >= BUTTON_IMAGE_TYPE_COUNT) {
		Log(ERROR, "Button", "Trying to set a button image index out of range: %d", type);
		return;
	}

	if (type <= BUTTON_IMAGE_NONE) {
		for (int i = 0; i < BUTTON_IMAGE_TYPE_COUNT; i++) {
			buttonImages[i] = nullptr;
		}
		flags &= IE_GUI_BUTTON_NO_IMAGE;
	} else {
		buttonImages[type] = img;
		/*
		 currently IE_GUI_BUTTON_NO_IMAGE cannot be infered from the presence or lack of images
		 leaving this here commented out in case it may be useful in the future.

		if (img) {
			Flags &= ~IE_GUI_BUTTON_NO_IMAGE;
		} else {
			// check if we should set IE_GUI_BUTTON_NO_IMAGE

			int i=0;
			for (; i < BUTTON_IMAGE_TYPE_COUNT; i++) {
				if (buttonImages[i] != NULL) {
					break;
				}
			}
			if (i == BUTTON_IMAGE_TYPE_COUNT) {
				// we made it to the end of the list without breaking so we have no images
				Flags &= IE_GUI_BUTTON_NO_IMAGE;
			}
		}*/
	}
	MarkDirty();
}

void Button::WillDraw(const Region& /*drawFrame*/, const Region& /*clip*/)
{
	if (overlayAnim) {
		overlayAnim.Next(GetTicks());
	}
}

/** Draws the Control on the Output Display */
void Button::DrawSelf(Region rgn, const Region& /*clip*/)
{
	Video * video = core->GetVideoDriver();

	// Button image
	if (!( flags & IE_GUI_BUTTON_NO_IMAGE )) {
		Holder<Sprite2D> Image;

		switch (State) {
			case IE_GUI_BUTTON_FAKEPRESSED:
			case IE_GUI_BUTTON_PRESSED:
				Image = buttonImages[BUTTON_IMAGE_PRESSED];
				break;
			case IE_GUI_BUTTON_SELECTED:
				Image = buttonImages[BUTTON_IMAGE_SELECTED];
				break;
			case IE_GUI_BUTTON_DISABLED:
			case IE_GUI_BUTTON_FAKEDISABLED:
				Image = buttonImages[BUTTON_IMAGE_DISABLED];
				break;
			default:
				Image = buttonImages[BUTTON_IMAGE_UNPRESSED];
				break;
		}
		if (Image) {
			// FIXME: maybe it's useless...
			Point offset((frame.w / 2) - (Image->Frame.w / 2), (frame.h / 2) - (Image->Frame.h / 2));
			video->BlitSprite(Image, rgn.origin + offset);
		}
	}

	if (State == IE_GUI_BUTTON_PRESSED) {
		//shift the writing/border a bit
		rgn.x += PushOffset.x;
		rgn.y += PushOffset.y;
	}

	// Button picture
	Point picPos;
	if (Picture && (flags & IE_GUI_BUTTON_PICTURE) ) {
		// Picture is drawn centered
		picPos.x = (rgn.w / 2) - (Picture->Frame.w / 2) + rgn.x;
		picPos.y = (rgn.h / 2) - (Picture->Frame.h / 2) + rgn.y;
		if (flags & IE_GUI_BUTTON_HORIZONTAL) {
			picPos += Picture->Frame.origin;

			// Clipping: 0 = overlay over full button, 1 = no overlay
			int overlayHeight = Picture->Frame.h * (1.0 - Clipping);
			if (overlayHeight < 0)
				overlayHeight = 0;
			if (overlayHeight >= Picture->Frame.h)
				overlayHeight = Picture->Frame.h;
			int buttonHeight = Picture->Frame.h - overlayHeight;

			if (overlayHeight) {
				// TODO: Add an option to add BlitFlags::GREY to the flags
				const Color& col = overlayAnim.Current();
				video->BlitGameSprite(Picture, picPos, BlitFlags::COLOR_MOD, col);
			}

			Region rb = Region(picPos.x, picPos.y, Picture->Frame.w, buttonHeight);
			video->BlitSprite( Picture, rb.origin, &rb );
		} else {
			Region r(picPos.x, picPos.y, (Picture->Frame.w * Clipping), Picture->Frame.h);
			video->BlitSprite(Picture, Picture->Frame.origin + r.origin, &r);
		}
	}

	// Button animation
	if (AnimPicture) {
		int xOffs = ( frame.w / 2 ) - ( AnimPicture->Frame.w / 2 );
		int yOffs = ( frame.h / 2 ) - ( AnimPicture->Frame.h / 2 );
		Region r( rgn.x + xOffs, rgn.y + yOffs, int(AnimPicture->Frame.w * Clipping), AnimPicture->Frame.h );

		if (flags & IE_GUI_BUTTON_CENTER_PICTURES) {
			video->BlitSprite( AnimPicture, r.origin + AnimPicture->Frame.origin, &r );
		} else {
			video->BlitSprite( AnimPicture, r.origin, &r );
		}
	}

	// Composite pictures (paperdolls/description icons)
	if (!PictureList.empty() && (flags & IE_GUI_BUTTON_PICTURE) ) {
		auto iter = PictureList.begin();
		Point offset;
		if (flags & IE_GUI_BUTTON_CENTER_PICTURES) {
			// Center the hotspots of all pictures
			offset.x = frame.w / 2;
			offset.y = frame.h / 2;
		} else if (flags & IE_GUI_BUTTON_BG1_PAPERDOLL) {
			// Display as-is
		} else {
			// Center the first picture, and align the rest to that
			offset.x = frame.w / 2 - (*iter)->Frame.w/2 + (*iter)->Frame.x;
			offset.y = frame.h / 2 - (*iter)->Frame.h/2 + (*iter)->Frame.y;
		}

		for (; iter != PictureList.end(); ++iter) {
			video->BlitSprite(*iter, rgn.origin + offset);
		}
	}

	// Button label
	if (hasText && ! ( flags & IE_GUI_BUTTON_NO_TEXT )) {
		// FIXME: hopefully there's no button which sinks when selected
		//   AND has text label
		//else if (State == IE_GUI_BUTTON_PRESSED || State == IE_GUI_BUTTON_SELECTED) {

		ieByte align = 0;
		if (flags & IE_GUI_BUTTON_ALIGN_LEFT)
			align |= IE_FONT_ALIGN_LEFT;
		else if (flags & IE_GUI_BUTTON_ALIGN_RIGHT)
			align |= IE_FONT_ALIGN_RIGHT;
		else
			align |= IE_FONT_ALIGN_CENTER;

		if (flags & IE_GUI_BUTTON_ALIGN_TOP)
			align |= IE_FONT_ALIGN_TOP;
		else if (flags & IE_GUI_BUTTON_ALIGN_BOTTOM)
			align |= IE_FONT_ALIGN_BOTTOM;
		else
			align |= IE_FONT_ALIGN_MIDDLE;

		Region r = rgn;
		if (IS_PORTRAIT) {
			// constrain the label (status icons) to the picture bounds
			// FIXME: we have to do +1 because the images are 1 px too small to fit 3 icons...
			r = Region(picPos.x, picPos.y, Picture->Frame.w + 1, Picture->Frame.h);
		} else if (flags&IE_GUI_BUTTON_ANCHOR) {
			r.x += Anchor.x;
			r.y += Anchor.y;
			r.w -= Anchor.x;
			r.h -= Anchor.y;
		} else {
			Font::StringSizeMetrics metrics {r.size, 0, 0, false};
			font->StringSize(Text, &metrics);

			if (metrics.numLines == 1 && (IE_GUI_BUTTON_ALIGNMENT_FLAGS & flags)) {
				// FIXME: I'm unsure when exactly this adjustment applies...
				// we do know that if a button is multiline it should not have margins
				// I'm actually wondering if we need this at all anymore
				// I suspect its origins predate the fixing of font baseline alignment
				r.ExpandAllSides(-5);
			}
		}
		
		Color c = textColor;
		if (State == IE_GUI_BUTTON_DISABLED || IsDisabled()) {
			c.r *= 0.66;
			c.g *= 0.66;
			c.b *= 0.66;
		}

		Font::PrintColors colors {c, ColorBlack};
		font->Print(r, Text, align, colors);
	}

	if (! (flags&IE_GUI_BUTTON_NO_IMAGE)) {
		for (int i = 0; i < MAX_NUM_BORDERS; i++) {
			ButtonBorder *fr = &borders[i];
			if (! fr->enabled) continue;

			const Region& frRect = fr->rect;
			Region r = Region( rgn.origin + frRect.origin, frRect.size );
			if (pulseBorder && !fr->filled) {
				Color mix = GlobalColorCycle.Blend(ColorWhite, fr->color);
				video->DrawRect( r, mix, fr->filled, BlitFlags::BLENDED );
			} else {
				video->DrawRect( r, fr->color, fr->filled, BlitFlags::BLENDED );
			}
		}
	}
}
/** Sets the Button State */
void Button::SetState(unsigned char state)
{
	if (state > IE_GUI_BUTTON_LOCKED_PRESSED) {// If wrong value inserted
		return;
	}

	// FIXME: we should properly consolidate IE_GUI_BUTTON_DISABLED with the view Disabled flag
	SetDisabled(state == IE_GUI_BUTTON_DISABLED);

	if (State != state) {
		MarkDirty();
		State = state;
	}
}

bool Button::IsAnimated() const
{
	if (overlayAnim) {
		return true;
	}

	if (pulseBorder) {
		return true;
	}

	return Control::IsAnimated();
}

bool Button::IsOpaque() const
{
	return Picture != NULL
		&& !(flags&IE_GUI_BUTTON_NO_IMAGE)
		&& Picture->HasTransparency() == false;
}

void Button::SetBorder(int index, const Region& rgn, const Color &color, bool enabled, bool filled)
{
	if (index >= MAX_NUM_BORDERS)
		return;

	ButtonBorder fr = { rgn, color, filled, enabled };
	borders[index] = fr;

	MarkDirty();
}

void Button::EnableBorder(int index, bool enabled)
{
	if (index >= MAX_NUM_BORDERS)
		return;

	if (borders[index].enabled != enabled) {
		borders[index].enabled = enabled;
		MarkDirty();
	}
}

void Button::SetFont(Font* newfont)
{
	font = newfont;
}

String Button::TooltipText() const
{
	if (IsDisabled() || flags & IE_GUI_BUTTON_NO_TOOLTIP) {
		return L"";
	}

	ieDword showHotkeys = 0;
	core->GetDictionary()->Lookup("Hotkeys On Tooltips", showHotkeys);

	if (showHotkeys && hotKey) {
		String s;
		switch (hotKey.key) {
			// FIXME: these arent localized...
			case GEM_ESCAPE:
				s += L"Esc";
				break;
			case GEM_RETURN:
				s += L"Enter";
				break;
			case GEM_PGUP:
				s += L"PgUp";
				break;
			case GEM_PGDOWN:
				s += L"PgDn";
				break;
			default:
				// TODO: check if there are more possible keys
				if (hotKey.key >= GEM_FUNCTIONX(1) && hotKey.key <= GEM_FUNCTIONX(16)) {
					s.push_back(L'F');
					int offset = hotKey.key - GEM_FUNCTIONX(0);
					if (hotKey.key > GEM_FUNCTIONX(9)) {
						s.push_back(L'1');
						offset -= 10;
					}
					s.push_back(L'0' + offset);
				} else {
					s.push_back(toupper(hotKey.key));
				}
				break;
		}
		String tt = ((tooltip.length()) ? tooltip : QueryText());
		tt = (tt.length()) ? s + L": " + tt : s;
		return tt;
	}
	return Control::TooltipText();
}

Holder<Sprite2D> Button::Cursor() const
{
	if (IS_PORTRAIT) {
		GameControl* gc = core->GetGameControl();
		if (gc) {
			Holder<Sprite2D> cur = gc->GetTargetActionCursor();
			if (cur) return cur;
		}
	}
	return Control::Cursor();
}

bool Button::AcceptsDragOperation(const DragOp& dop) const
{
	// FIXME: this implementation is obviously not future proof
	// portrait buttons accept other portraits and dropped items
	if (IS_PORTRAIT) {
		return true;
	}
	
	return Control::AcceptsDragOperation(dop);
}

void Button::CompleteDragOperation(const DragOp& dop)
{
	if (dop.dragView == this) {
		// this was the dragged view
		EnableBorder(1, false);
	}
	
	Control::CompleteDragOperation(dop);
}

Holder<Sprite2D> Button::DragCursor() const
{
	if (IS_PORTRAIT) {
		// TODO: would it be an enhancement to actually use the portrait for the drag icon?
		return core->Cursors[IE_CURSOR_SWAP];
	} else if (Picture) {
		return Picture;
	}
	
	return Control::DragCursor();
}

/** Mouse Button Down */
bool Button::OnMouseDown(const MouseEvent& me, unsigned short mod)
{
	ActionKey key(Action::DragDropDest);
    if (core->GetDraggedItem() && !SupportsAction(key)) {
        return true;
    }
    
	if (me.button == GEM_MB_ACTION) {
		if (State == IE_GUI_BUTTON_LOCKED) {
			SetState( IE_GUI_BUTTON_LOCKED_PRESSED );
			return true;
		}
		SetState( IE_GUI_BUTTON_PRESSED );
		if (flags & IE_GUI_BUTTON_SOUND) {
			core->PlaySound(DS_BUTTON_PRESSED, SFX_CHAN_GUI);
		}
	}
	return Control::OnMouseDown(me, mod);
}

/** Mouse Button Up */
bool Button::OnMouseUp(const MouseEvent& me, unsigned short mod)
{
	bool drag = core->GetDraggedItem () != NULL;

	if (drag && me.repeats == 1) {
		ActionKey key(Action::DragDropDest);
		if (SupportsAction(key)) {
			return PerformAction(key);
		} else {
			//if something was dropped, but it isn't handled here: it didn't happen
			return false;
		}
	}

	switch (State) {
	case IE_GUI_BUTTON_PRESSED:
		if (ToggleState) {
			SetState( IE_GUI_BUTTON_SELECTED );
		} else {
			SetState( IE_GUI_BUTTON_UNPRESSED );
		}
		break;
	case IE_GUI_BUTTON_LOCKED_PRESSED:
		SetState( IE_GUI_BUTTON_LOCKED );
		break;
	}

	DoToggle();
	return Control::OnMouseUp(me, mod);
}

bool Button::OnMouseOver(const MouseEvent& me)
{
	if (State == IE_GUI_BUTTON_LOCKED) {
		return true;
	}

	return Control::OnMouseOver(me);
}

void Button::OnMouseEnter(const MouseEvent& me, const DragOp* dop)
{
	Control::OnMouseEnter(me, dop);

	if (IsFocused() && me.ButtonState(GEM_MB_ACTION)) {
		SetState( IE_GUI_BUTTON_PRESSED );
	}

	for (int i = 0; i < MAX_NUM_BORDERS; i++) {
		ButtonBorder *fr = &borders[i];
		if (fr->enabled) {
			pulseBorder = !fr->filled;
			MarkDirty();
			break;
		}
	}
}

void Button::OnMouseLeave(const MouseEvent& me, const DragOp* dop)
{
	Control::OnMouseLeave(me, dop);

	if (State == IE_GUI_BUTTON_PRESSED && (dop == nullptr || dop->dragView == this)) {
		SetState( IE_GUI_BUTTON_UNPRESSED );
	}

	if (pulseBorder) {
		pulseBorder = false;
		MarkDirty();
	}
}

/** Sets the Text of the current control */
void Button::SetText(const String& string)
{
	Text = string;
	if (string.length()) {
		if (flags&IE_GUI_BUTTON_LOWERCASE)
			StringToLower( Text );
		else if (flags&IE_GUI_BUTTON_CAPS)
			StringToUpper( Text );
		hasText = true;
	} else {
		hasText = false;
	}
	MarkDirty();
}

/** Refresh a button from a given radio button group */
void Button::UpdateState(unsigned int Sum)
{
	if (IsDisabled()) {
		return;
	}

	if (flags & IE_GUI_BUTTON_RADIOBUTTON) {
		//radio button, exact value
		ToggleState = ( Sum == GetValue() );
	} else if (flags & IE_GUI_BUTTON_CHECKBOX) {
		//checkbox, bitvalue
		ToggleState = !!( Sum & GetValue() );
	} else {
		//other buttons, nothing to redraw
		return;
	}

	if (ToggleState) {
		SetState(IE_GUI_BUTTON_SELECTED);
	} else {
		SetState(IE_GUI_BUTTON_UNPRESSED);
	}
}

void Button::DoToggle()
{
	if (flags & IE_GUI_BUTTON_CHECKBOX) {
		//checkbox
		ToggleState = !ToggleState;
		if (ToggleState)
			SetState( IE_GUI_BUTTON_SELECTED );
		else
			SetState( IE_GUI_BUTTON_UNPRESSED );
		if (VarName[0] != 0) {
			ieDword tmp = 0;
			core->GetDictionary()->Lookup( VarName, tmp );
			tmp ^= GetValue();
			core->GetDictionary()->SetAt( VarName, tmp );
			window->RedrawControls( VarName, tmp );
		}
	} else {
		if (flags & IE_GUI_BUTTON_RADIOBUTTON) {
			//radio button
			ToggleState = true;
			SetState( IE_GUI_BUTTON_SELECTED );
		}
		if (VarName[0] != 0) {
			ieDword val = GetValue();
			core->GetDictionary()->SetAt( VarName, val );
			window->RedrawControls( VarName, val );
		}
	}
}

/** Sets the Picture */
void Button::SetPicture(Holder<Sprite2D> newpic)
{
	ClearPictureList();
	Picture = newpic;
	if (Picture) {
		// try fitting to width if rescaling is possible, otherwise we automatically crop
		unsigned int ratio = round((float) Picture->Frame.w / (float) frame.w);
		if (ratio > 1) {
			Holder<Sprite2D> img = core->GetVideoDriver()->SpriteScaleDown(Picture, ratio);
			Picture = img;
		}
		flags |= IE_GUI_BUTTON_PICTURE;
	} else {
		flags &= ~IE_GUI_BUTTON_PICTURE;
	}
	MarkDirty();
}

/** Clears the list of Pictures */
void Button::ClearPictureList()
{
	PictureList.clear();
	MarkDirty();
}

/** Add picture to the end of the list of Pictures */
void Button::StackPicture(Holder<Sprite2D> Picture)
{
	PictureList.push_back(Picture);
	MarkDirty();
	flags |= IE_GUI_BUTTON_PICTURE;
}

bool Button::HitTest(const Point& p) const
{
	bool hit = Control::HitTest(p);
	if (hit) {
		// some buttons have hollow Image frame filled w/ Picture
		// some buttons in BG2 are text only (if BAM == 'GUICTRL')
		Holder<Sprite2D> Unpressed = buttonImages[BUTTON_IMAGE_UNPRESSED];
		if (Picture || PictureList.size() || !Unpressed) return true;

		Point off;
		off.x = (frame.w / 2) - (Unpressed->Frame.w / 2) + Unpressed->Frame.x;
		off.y = (frame.h / 2) - (Unpressed->Frame.h / 2) + Unpressed->Frame.y;
		hit = !Unpressed->IsPixelTransparent(p - off);
	}
	return hit;
}

// Set palette used for drawing button label in normal state
void Button::SetTextColor(const Color &color)
{
	textColor = color;
	MarkDirty();
}

void Button::SetHorizontalOverlay(double clip, const Color& src, const Color &dest)
{
	if ((Clipping>clip) || !(flags&IE_GUI_BUTTON_HORIZONTAL) ) {
		flags |= IE_GUI_BUTTON_HORIZONTAL;

		overlayAnim = ColorAnimation(src, dest, false);
	}
	Clipping = clip;
	MarkDirty();
}

void Button::SetAnchor(ieWord x, ieWord y)
{
	Anchor = Point(x,y);
}

void Button::SetPushOffset(ieWord x, ieWord y)
{
	PushOffset = Point(x,y);
}

bool Button::SetHotKey(KeyboardKey key, short mod, bool global)
{
	UnregisterHotKey();
	
	if (key == 0) {
		return true;
	}

	if (global) {
		if (EventMgr::RegisterHotKeyCallback(HotKeyCallback, key, mod)) {
			hotKey.key = key;
			hotKey.mod = mod;
			hotKey.global = true;
			return true;
		}
	} else if (window->RegisterHotKeyCallback(HotKeyCallback, key)) { // FIXME: this doesnt respect mod param
		hotKey.key = key;
		hotKey.mod = mod;
		return true;
	}
	return false;
}

bool Button::HandleHotKey(const Event& e)
{
	if (IsReceivingEvents() && e.type == Event::KeyDown) {
		// only run once on keypress (or should it be KeyRelease?)
		// we could support both; key down = left mouse down, key up = left mouse up
		DoToggle();
		return PerformAction();
	}
	return false;
}

}
