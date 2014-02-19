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

/**
 * @file Font.h
 * Declares Font, class for manipulating images serving as fonts
 * @author The GemRB Project
 */

#ifndef FONT_H
#define FONT_H

#include "globals.h"
#include "exports.h"

namespace GemRB {

enum FontStyle {
	NORMAL = 0x00,
	BOLD = 0x01,
	ITALIC = 0x02,
	UNDERLINE = 0x04
};

class Palette;
class Sprite2D;

#define IE_FONT_ALIGN_LEFT   0x00
#define IE_FONT_ALIGN_CENTER 0x01
#define IE_FONT_ALIGN_RIGHT  0x02
#define IE_FONT_ALIGN_BOTTOM 0x04
#define IE_FONT_ALIGN_TOP    0x10
#define IE_FONT_ALIGN_MIDDLE 0x20
#define IE_FONT_SINGLE_LINE  0x40

// TODO: this is a placeholder struct
// eventually fonts will dynamically create pages of glyphs as a single bit map
// at that time Glyph will have to change to accomodate that.
// It should be able to take a pointer to a location in the 2D bit map coresponding to its location
// and access its data using offsets instead of having to copy and free pixel data in the Glyph
// for now i need something to work with.
struct Glyph {
	wchar_t chr;
	Size dimensions;
	ieByte* pixels;
};

/**
 * @class Font
 * Class for using and manipulating images serving as fonts
 */

class GEM_EXPORT Font {
protected:
	ieResRef* resRefs;
	int numResRefs;
	char name[20];

	Palette* palette;
	Sprite2D* blank;
public:
	int maxHeight;
	int descent;
private:
	void BlitGlyphToCanvas(const Glyph& glyph, int x, int y,
						   ieByte* canvas, const Size& size) const;
	// Blit to the sprite or screen if canvas is NULL
	size_t RenderText(const String&, Region&, Palette*,
				  ieByte alignment, ieByte** canvas = NULL, bool grow = false) const;
public:
	Font(Palette*);
	virtual ~Font(void);

	Sprite2D* RenderTextAsSprite(const String& string, const Size& size, ieByte alignment,
								 Palette* pal = NULL, size_t* numPrinted = NULL) const;
	//allow reading but not setting glyphs
	virtual const Sprite2D* GetCharSprite(ieWord chr) const = 0;

	bool AddResRef(const ieResRef resref);
	bool MatchesResRef(const ieResRef resref);

	const char* GetName() const {return name;};
	void SetName(const char* newName);

	virtual ieWord GetPointSize() const {return 0;};
	virtual FontStyle GetStyle() const {return NORMAL;};

	Palette* GetPalette() const;
	void SetPalette(Palette* pal);

	// Printing methods
	// return the number of glyphs printed
	size_t Print(Region rgn, const char* string,
				 Palette* color, ieByte Alignment) const;
	size_t Print(Region rgn, const String& string,
				 Palette* hicolor, ieByte Alignment) const;

	/** Returns size of the string rendered in this font in pixels */
	Size StringSize(const String&, const Size* = NULL) const;

	virtual int GetKerningOffset(ieWord /*leftChr*/, ieWord /*rightChr*/) const {return 0;};
};

}

#endif
