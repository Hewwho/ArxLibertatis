/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code').

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Text
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Text Management
//
// Updates: (date) (person) (update)
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "gui/Text.h"

#include <cassert>
#include <sstream>

#include "core/Localization.h"
#include "core/Core.h"

#include "gui/Interface.h"

#include "graphics/Draw.h"
#include "graphics/Frame.h"
#include "graphics/Renderer.h"
#include "graphics/effects/Fog.h"

#include "io/Filesystem.h"
#include "io/Logger.h"

using std::string;

//-----------------------------------------------------------------------------
TextManager * pTextManage;
TextManager * pTextManageFlyingOver;

//-----------------------------------------------------------------------------
Font* hFontInBook	= NULL;
Font* hFontRedist	= NULL;
Font* hFontMainMenu = NULL;
Font* hFontMenu		= NULL;
Font* hFontControls = NULL;
Font* hFontCredits	= NULL;
Font* hFontInGame	= NULL;
Font* hFontInGameNote = NULL;
 

extern long CHINESE_VERSION;


//-----------------------------------------------------------------------------
string FontError() {
	LPVOID lpMsgBuf;
	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
	    GetLastError(),
	    0, // Default language
	    (LPTSTR) &lpMsgBuf,
	    0,
	    NULL);
	return string("Font Error: ") + (LPCSTR)lpMsgBuf;
}

//-----------------------------------------------------------------------------
void ARX_UNICODE_FormattingInRect(Font* pFont, const std::string& text, RECT & _rRect, COLORREF col, long* textHeight = 0, long* numChars = 0, bool computeOnly = false)
{
	std::string::const_iterator itLastLineBreak = text.begin();
	std::string::const_iterator itLastWordBreak = text.begin();
	std::string::const_iterator it = text.begin();
	
	int maxLineWidth = _rRect.right - _rRect.left;
	int penY = _rRect.top;

	if(textHeight)
		*textHeight = 0;

	if(numChars)
		*numChars = 0;

	// Ensure we can at least draw one line...
	if(penY + pFont->GetLineHeight() > _rRect.bottom)
		return;

	for(it = text.begin(); it != text.end(); ++it)
	{
		bool bDrawLine = false;

		// Line break ?
		if((*it == '\n') || (*it == '*'))
		{
			bDrawLine = true;
		}
		else
		{
			// Word break ?
			if((*it == ' ') || (*it == '\t'))
			{
				itLastWordBreak = it;
			}

			// Check length of string up to this point
			Vector2i size = pFont->GetTextSize(itLastLineBreak, it+1);
			if(size.x > maxLineWidth)	// Too long ?
			{
				bDrawLine = true;		// Draw a line from the last line break up to the last word break
				it = itLastWordBreak;
			}			
		}
		
		// If we have to draw a line 
		//  OR
		// This is the last character of the string
		if(bDrawLine || (it+1 == text.end()))
		{
			// Draw the line
			if(!computeOnly)
				pFont->Draw(_rRect.left, penY, itLastLineBreak, it+1, col);
			
			itLastLineBreak = it+1;

			penY += pFont->GetLineHeight();

			// Validate that the new line will fit inside the rect...
			if(penY + pFont->GetLineHeight() > _rRect.bottom)
				break;
		}
	}

	// Return text height
	if(textHeight)
		*textHeight = penY - _rRect.top;

	// Return num characters displayed
	if(numChars)
		*numChars = it - text.begin();
}

//-----------------------------------------------------------------------------
long ARX_UNICODE_ForceFormattingInRect(Font* pFont, const std::string& text, RECT _rRect)
{
	long numChars;
	ARX_UNICODE_FormattingInRect(pFont, text, _rRect, 0, 0, &numChars, true);

	return numChars;
}

//-----------------------------------------------------------------------------
long ARX_UNICODE_DrawTextInRect(Font* font,
                                float x, float y,
                                float maxx,
                                const std::string& _text,
                                COLORREF col,
                                RECT* pClipRect
                               )
{
	Renderer::Viewport previousViewport;

	if (pClipRect)
	{
		previousViewport = GRenderer->GetViewport();

		Renderer::Viewport clippedViewport;
		clippedViewport.x = pClipRect->left;
		clippedViewport.y = pClipRect->top;
		clippedViewport.width = pClipRect->right - pClipRect->left;
		clippedViewport.height = pClipRect->bottom - pClipRect->top;
		GRenderer->SetViewport(clippedViewport); 
	}

	RECT rect;
	rect.top	= (long)y;
	rect.left	= (long)x;
	rect.right	= (long)maxx;
	rect.bottom	= SHRT_MAX;

	long height;
	ARX_UNICODE_FormattingInRect(font, _text, rect, col, &height);

	if (pClipRect)
	{
		GRenderer->SetViewport(previousViewport);
	}

	return height;
}

void ARX_TEXT_Draw(Font* ef,
                   float x, float y,
                   const std::string& car,
                   COLORREF col) {
	
	if (car.empty() || car[0] == 0)
		return;

	ARX_UNICODE_DrawTextInRect(ef, x, y, 9999.f, car, col);
}

long ARX_TEXT_DrawRect(Font* ef,
                       float x, float y,
                       float maxx,
                       const string & car,
                       COLORREF col,
                       RECT* pClipRect) {
	
	col = RGB((col >> 16) & 255, (col >> 8) & 255, (col) & 255);
	return ARX_UNICODE_DrawTextInRect(ef, x, y, maxx, car, col, pClipRect);
}

float DrawBookTextInRect(Font* font, float x, float y, float maxx, const std::string& text, COLORREF col) {
	return (float)ARX_TEXT_DrawRect(font, (BOOKDECX + x) * Xratio, (BOOKDECY + y) * Yratio, (BOOKDECX + maxx) * Xratio, text, col);
}

//-----------------------------------------------------------------------------
void DrawBookTextCenter( Font* font, float x, float y, const std::string& text, COLORREF col )
{
	UNICODE_ARXDrawTextCenter(font, (BOOKDECX + x)*Xratio, (BOOKDECY + y)*Yratio, text, col);
}

//-----------------------------------------------------------------------------

long UNICODE_ARXDrawTextCenter( Font* font, float x, float y, const std::string& str, COLORREF col )
{
	Vector2i size = font->GetTextSize(str);
	int drawX = ((int)x) - (size.x / 2);
	int drawY = (int)y;

	font->Draw(drawX, drawY, str, col);

	return size.x;
}



long UNICODE_ARXDrawTextCenteredScroll( Font* font, float x, float y, float x2, const std::string& str, COLORREF col, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut)
{
	RECT rRect;
	ARX_CHECK_LONG(y);
	ARX_CHECK_LONG(x + x2);   //IF OK, x - x2 cannot overflow
	rRect.left = ARX_CLEAN_WARN_CAST_LONG(x - x2);
	rRect.top = ARX_CLEAN_WARN_CAST_LONG(y);
	rRect.right = ARX_CLEAN_WARN_CAST_LONG(x + x2);

	if (pTextManage)
	{
		pTextManage->AddText(font,
							 str,
							 rRect,
							 col,
							 iTimeOut,
							 iTimeScroll,
							 fSpeed,
							 iNbLigne
							);

		return ARX_CLEAN_WARN_CAST_LONG(x2);
	}

	return 0;
}

//-----------------------------------------------------------------------------
void ARX_Allocate_Text( std::string& dest, const std::string& id_string) {
	std::string output;
	PAK_UNICODE_GetPrivateProfileString(id_string, "default", output);
	dest = output;
}

Font* _CreateFont(std::string fontFace, std::string fontProfileName, unsigned int fontSize, float scaleFactor = Yratio)
{
	std::stringstream ss;

	std::string szFontSize;
	ss << fontSize;
	ss >> szFontSize;
	ss.clear();

	std::string szUT;
	PAK_UNICODE_GetPrivateProfileString(fontProfileName, szFontSize, szUT);
	ss << szUT;
	ss >> fontSize;
	ss.clear();

	fontSize *= scaleFactor;

	Font* newFont = FontCache::GetFont(fontFace, fontSize);
	if(!newFont) {
		LogError << "error loading font: " << fontFace << " of size " << fontSize;
	}
	
	return newFont;
}

string getFontFile() {
	string tx= "misc" PATH_SEPERATOR_STR "Arx.ttf";
	if(!FileExist(tx.c_str())) {
		tx = "misc" PATH_SEPERATOR_STR "ARX_default.ttf"; // Full path
	}
	return tx;
}

//-----------------------------------------------------------------------------
void ARX_Text_Init()
{	
	ARX_Text_Close();

	Localisation_Init();
	
	std::string strInGameFont = getFontFile();
	std::string strInMenuFont = strInGameFont;

	pTextManage = new TextManager();
	pTextManageFlyingOver = new TextManager();

	FontCache::Initialize();

	hFontMainMenu = _CreateFont(strInMenuFont, "system_font_mainmenu_size", 58);
	LogInfo << "Created hFontMainMenu, size " << hFontMainMenu->GetSize();

	hFontMenu	  = _CreateFont(strInMenuFont, "system_font_menu_size", 32);
	LogInfo << "Created hFontMenu, size " << hFontMenu->GetSize();

	hFontControls = _CreateFont(strInMenuFont, "system_font_menucontrols_size", 22);
	LogInfo << "Created hFontControls, size " << hFontControls->GetSize();

	hFontCredits  = _CreateFont(strInMenuFont, "system_font_menucredits_size", 36);
	LogInfo << "Created hFontCredits, size " << hFontCredits->GetSize();

	hFontRedist   = _CreateFont(strInGameFont, "system_font_redist_size", 18);
	LogInfo << "Created hFontRedist, size " << hFontRedist->GetSize();

	// Keep small font small when increasing resolution
	float smallFontRatio = Yratio > 1.0f ? Yratio * 0.8f : Yratio;

	hFontInGame     = _CreateFont(strInGameFont, "system_font_book_size", 18, smallFontRatio);
	LogInfo << "Created hFontInGame, size " << hFontInGame->GetSize();

	hFontInGameNote = _CreateFont(strInGameFont, "system_font_note_size", 18, smallFontRatio);
	LogInfo << "Created hFontInGameNote, size " << hFontInGameNote->GetSize();

	hFontInBook		= _CreateFont(strInGameFont, "system_font_book_size", 18, smallFontRatio);
	LogInfo << "Created InBookFont, size " << hFontInBook->GetSize();
}

//-----------------------------------------------------------------------------
void ARX_Text_Close()
{
	Localisation_Close();

	delete pTextManage;
	pTextManage = NULL;

	delete pTextManageFlyingOver;
	pTextManageFlyingOver = NULL;

	FontCache::ReleaseFont(hFontInBook);
	hFontInBook = NULL;
	
	FontCache::ReleaseFont(hFontRedist);
	hFontRedist = NULL;
	
	FontCache::ReleaseFont(hFontMainMenu);
	hFontMainMenu = NULL;

	FontCache::ReleaseFont(hFontMenu);
	hFontMenu = NULL;
	
	FontCache::ReleaseFont(hFontControls);
	hFontControls = NULL;
	
	FontCache::ReleaseFont(hFontCredits);
	hFontCredits = NULL;
	
	FontCache::ReleaseFont(hFontInGame);
	hFontInGame = NULL;
	
	FontCache::ReleaseFont(hFontInGameNote);
	hFontInGameNote = NULL;

	FontCache::Shutdown();
}
