/*
* MIT License
*
* Copyright (c) 2020 Seth Ballantyne
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <string>
#include <cmath>
#include <iostream>
#include <sstream>
#include "console.h"


using namespace std;

// coverts any characters in str to lowercase and returns a new string
string ToLower(string& str)
{
	int length = str.length();

	for(int i = 0; i < length; i++)
	{
		str[i] = tolower(str[i]);
	}
	
	return str;
}

bool StringContainsWhiteSpace(const string& str)
{
	if(str.find(' ') == string::npos && str.find('\t') == string::npos)
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------
// BUILT-IN COMMANDS
//
// These are registered in Console_Init.
//---------------------------------------------------------------------------
void console::console_command_version(Console& console, vector<string>& args)
{
	Console_Print(console, "CONSOLE VERSION " + to_string(CONSOLE_VERSION_MAJOR) + "." + to_string(CONSOLE_VERSION_MINOR));
}

void console::console_command_list_commands(Console& console, vector<string>& args)
{
	string commands;
	int count = 0;

	for(auto command : console.commands)
	{
		commands += command.first + " ";
		count++;
	}

	Console_Print(console, commands);
	Console_Print(console, to_string(count) + " commands available.");
}

void console::console_command_clear(Console& console, vector<string>& args)
{
	console.outputBuffer.buffer.clear();
	console.outputBuffer.topLineIndex = console.outputBuffer.bottomLineIndex = 0;
	
}

//---------------------------------------------------------------------------
// END BUILT-IN COMMANDS
//---------------------------------------------------------------------------

void console::RegisterBuiltInCommand(Console& console, const string& command, command_func_ptr func)
{
	if(CONSOLE_RET_SUCCESS != Console_RegisterCommand(console, command, func))
	{
		Console_Print(console, "Console_Init: failed to register command " + command);
	}
}

void draw_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
		case 1:
			*p = pixel;
			break;

		case 2:
			*(Uint16 *)p = pixel;
			break;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			}
			else {
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
			break;

		case 4:
			*(Uint32 *)p = pixel;
			break;
	}
}

int console::BitmapFont_InitBuiltInFont(BitmapFont& bitmapfont, SDL_Surface* consoleSurface, int characterWidth, int characterHeight, unsigned char firstChar, unsigned char lastChar, SDL_Colour* fontColour, SDL_Colour* transparencyColour)
{
	SDL_Colour defaultFontColour = { DEFAULT_FONT_COLOUR_R, DEFAULT_FONT_COLOUR_G, DEFAULT_FONT_COLOUR_B, 0 };
	SDL_Colour defaultTransparencyColour = { DEFAULT_COLOUR_KEY_R, DEFAULT_COLOUR_KEY_G, DEFAULT_COLOUR_KEY_B, 0 };

	bitmapfont.characterWidth = characterWidth;
	bitmapfont.characterHeight = characterHeight;
	bitmapfont.firstChar = firstChar;
	bitmapfont.lastChar = lastChar;
	bitmapfont.numberOfChars = (lastChar - firstChar) + 1;

	int fontSheetWidth = characterWidth * bitmapfont.numberOfChars;

	bitmapfont.fontSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, fontSheetWidth,
												  characterHeight, consoleSurface->format->BitsPerPixel, consoleSurface->format->Rmask,
								   consoleSurface->format->Gmask, consoleSurface->format->Bmask, consoleSurface->format->Amask);
	if(!bitmapfont.fontSurface)
	{
		return CONSOLE_RET_CREATE_SURFACE_FAIL;
	}

	SDL_Colour colour = (nullptr == fontColour) ? defaultFontColour : *fontColour;
	bitmapfont.fontColour = SDL_MapRGB(consoleSurface->format, colour.r, colour.g, colour.b);

	colour = (nullptr == transparencyColour) ? defaultTransparencyColour : *transparencyColour;
	bitmapfont.transparencyColour = SDL_MapRGB(consoleSurface->format, colour.r, colour.g, colour.b);

	int xBasePosition = 0;

	for(int i = 0; i < bitmapfont.numberOfChars; i++)
	{
		for(int j = 0; j < characterHeight; j++)
		{
			for(int k = 0; k < characterWidth; k++)
			{
				if('.' == defaultFont[i][j][k]) // transparency pixel
				{
					draw_pixel(bitmapfont.fontSurface, xBasePosition + k, j, bitmapfont.transparencyColour);
				}
				else if('#' == defaultFont[i][j][k]) // character pixel
				{
					draw_pixel(bitmapfont.fontSurface, xBasePosition + k, j, bitmapfont.fontColour);
				}
			}
		}

		// starting on a new character, move to the appropriate position on the surface.
		xBasePosition = characterWidth * (i + 1);
	}
	
	// not checking for errors here, because the console can still function without the colour key.
	SDL_SetColorKey(bitmapfont.fontSurface, SDL_SRCCOLORKEY, bitmapfont.transparencyColour);

	return CONSOLE_RET_SUCCESS;
}

int console::BitmapFont_RenderLine(Console& console, BitmapFont& font, string& line, int x, int y)
{
	int lineLength = line.length();

	if(lineLength == 0)
	{
		return CONSOLE_RET_SUCCESS;
	}

	SDL_Rect srcRect;
	SDL_Rect destRect;

	srcRect.x = srcRect.y = 0;
	srcRect.w = font.characterWidth;
	srcRect.h = font.characterHeight;
	destRect.x = x;
	destRect.y = y;
	destRect.w = srcRect.w;
	destRect.h = srcRect.h;

	// check to see if the line is partially off-screen
	// assumes the top of the console is the top edge of the window.
    if(y < 0)
	{
		// only rendering a portion of the character because the rest of it
		// has scrolled off the screen. Multiplying by 1 because we need to
		// make the negative number positive.
		srcRect.y = 0 + (y * -1);
		srcRect.h = font.characterHeight - srcRect.y;
		destRect.y = 0;
	}

	for(char c : line)
	{
		srcRect.x = (c - font.firstChar) * font.characterWidth;

		int result = SDL_BlitSurface(font.fontSurface, &srcRect, console.consoleSurface, &destRect);
		if(result != 0)
		{
			return CONSOLE_RET_BLIT_FAILED;
		}

		destRect.x += font.characterWidth;
	}

	return CONSOLE_RET_SUCCESS;
}

void console::InputBuffer_SplitInput(InputBuffer& inputBuffer, string& command, vector<string>& args)
{
	string word = "";
	vector<string> tempArgs;

	for(int i = 0; i <= inputBuffer.buffer.length(); i++)
	{
		if(inputBuffer.buffer[i] != ' ')
		{
			word += inputBuffer.buffer[i];
		}
		else
		{
			args.push_back(word);
			word.clear();
		}
	}

	command = args[0];
	args.erase(args.begin());
}

void console::InputBuffer_Init(Console& console)
{
	console.inputBuffer.x = 0;
	BitmapFont& font = (console.externalBitmapFont.fontSurface != nullptr) ? console.externalBitmapFont : console.defaultBitmapFont;

	console.inputBuffer.y = console.consoleSurface->h - font.characterHeight - CONSOLE_GAP_BELOW_INPUT_BUFFER;
	console.inputBuffer.maxBufferLength = console.consoleSurface->w / font.characterWidth;

	console.inputBuffer.buffer.reserve(console.inputBuffer.maxBufferLength);
}

int console::InputBuffer_Render(Console& console, BitmapFont& font)
{
	return BitmapFont_RenderLine(console, font, console.inputBuffer.buffer, console.inputBuffer.x, console.inputBuffer.y);
}

void console::InputBuffer_SubmitBuffer(Console& console)
{
	string command;
	vector<string> args;

	// delimit the end of the string
	console.inputBuffer.buffer += ' ';

	InputBuffer_SplitInput(console.inputBuffer, command, args);

	console.inputBuffer.buffer.clear();

	command = ToLower(command);
	Console_ExecuteCommand(console, command, args);
}


// call this AFTER the inputbuffer has been initialized!
void console::OutputBuffer_Init(Console& console)
{
	BitmapFont& font = (console.externalBitmapFont.fontSurface != nullptr) ? console.externalBitmapFont : console.defaultBitmapFont;

	console.outputBuffer.x = console.inputBuffer.x;

	console.outputBuffer.y = console.inputBuffer.y - font.characterHeight - CONSOLE_GAP_BETWEEN_BUFFERS;
	
	console.outputBuffer.clippingRect.x = 0;
	console.outputBuffer.clippingRect.y = 0;
	console.outputBuffer.clippingRect.w = console.consoleSurface->w;
	console.outputBuffer.clippingRect.h = console.inputBuffer.y - CONSOLE_GAP_BETWEEN_BUFFERS;

	// character height is being added to Y because if we divide from Y,
	// we end up 1 line short.
	int temp = console.outputBuffer.y + font.characterHeight;

	int result = temp / font.characterHeight;
	console.outputBuffer.maxNumLinesOnScreen = result;

	result = temp % font.characterHeight;
	console.outputBuffer.yOffset = 0;

	console.outputBuffer.yOffset = (console.outputBuffer.maxNumLinesOnScreen * font.characterHeight) - console.outputBuffer.y;

	if(result > 0)
	{
		// there's not enough room to render the top most line, so it'll be partially
		// obscured. It's still considered a visible line.
		console.outputBuffer.maxNumLinesOnScreen++;
	}

	//console.outputBuffer.yOffset = (console.outputBuffer.maxNumLinesOnScreen * font.characterHeight) - console.outputBuffer.y;

	console.outputBuffer.maxLineLength = console.consoleSurface->w / font.characterWidth;

}

int console::OutputBuffer_Render(Console& console, BitmapFont& font)
{
	int xPos = console.outputBuffer.x;
	int yPos = console.outputBuffer.y;
	
	SDL_SetClipRect(console.consoleSurface, &console.outputBuffer.clippingRect);

	// ensures the top most line is completely visible when scrolling.
	// this will only have any effect if the top visible line is normally
	// partially obscured when text fills the console.
	if(console.outputBuffer.applyOffsetToY)
	{
		yPos += console.outputBuffer.yOffset;
	}
	
	if(console.outputBuffer.buffer.size() > 0)
	{
		for(int i = console.outputBuffer.bottomLineIndex; i >= console.outputBuffer.topLineIndex; i--)
		{
			int result = BitmapFont_RenderLine(console, font, console.outputBuffer.buffer[i], xPos, yPos);
			if(result != CONSOLE_RET_SUCCESS)
			{
				return result;
			}

			if(yPos > 0)
			{
				yPos -= font.characterHeight;
			}
			else
			{
				break;
			}
		}
	}

	SDL_SetClipRect(console.consoleSurface, NULL);

	return CONSOLE_RET_SUCCESS;
}

void console::OutputBuffer_ResizeText(Console& console)
{
	vector<string> newBuffer = console.outputBuffer.buffer;
	OutputBuffer_Clear(console);

	for(auto line : newBuffer)
	{
		Console_Print(console, line);
	}	
}

void console::OutputBuffer_Scroll(Console& console, int numberOfLines, int direction)
{
	int numberOfLinesScrolled;

	// dont bother attempting to scroll if there's not enough lines to fill the screen
	// because there's nothing to scroll, sucker!
	if(console.outputBuffer.buffer.size() <= console.outputBuffer.maxNumLinesOnScreen)
	{
		return;
	}

	switch(direction)
	{
		case CONSOLE_SCROLL_DIR_UP:
			if(numberOfLines < console.outputBuffer.topLineIndex)
			{
				numberOfLinesScrolled = numberOfLines;
			}
			else
			{
				// the very first that was entered is now visible on the screen 
				// due to scrolling up. 
				// Make sure the entirety of the first line is visible and not
				// partially obscured.
				console.outputBuffer.applyOffsetToY = true;
				numberOfLinesScrolled = console.outputBuffer.topLineIndex;
			}

			console.outputBuffer.topLineIndex -= numberOfLinesScrolled;
			console.outputBuffer.bottomLineIndex -= numberOfLinesScrolled;

		break;

		case CONSOLE_SCROLL_DIR_DOWN:
			// don't scroll down if we're already at the bottom of the buffer.
			if(console.outputBuffer.bottomLineIndex < console.outputBuffer.buffer.size() - 1)
			{
				if(console.outputBuffer.bottomLineIndex + numberOfLines < console.outputBuffer.buffer.size())
				{
					numberOfLinesScrolled = numberOfLines;
				}
				else
				{
					numberOfLinesScrolled = (console.outputBuffer.buffer.size() - 1) - console.outputBuffer.bottomLineIndex;					
				}

				console.outputBuffer.applyOffsetToY = false;
				console.outputBuffer.topLineIndex += numberOfLinesScrolled;
				console.outputBuffer.bottomLineIndex += numberOfLinesScrolled;

				if(console.outputBuffer.bottomLineIndex >= console.outputBuffer.buffer.size())
				{
					Console_Print(console, "dd");
				}
			}
			
			break;

		default:
			break;
	}
}

void console::OutputBuffer_Clear(Console& console)
{
	console.outputBuffer.buffer.clear();
	console.outputBuffer.bottomLineIndex = console.outputBuffer.topLineIndex = 0;
}

void console::Cursor_Render(Console& console, BitmapFont& font)
{
	int x = (console.inputBuffer.buffer.length() * font.characterWidth) + console.inputBuffer.x;
	int y = console.inputBuffer.y;
	SDL_Rect dest;

	dest.x = x;
	dest.y = y;
	dest.h = console.cursorSurface->h;
	dest.w = console.cursorSurface->w;

	SDL_BlitSurface(console.cursorSurface, NULL, console.consoleSurface, &dest);
}

int console::Console_Init(Console& console, SDL_Surface* screen, SDL_Colour* consoleColour,
						  SDL_Colour* fontColour, SDL_Colour* transparencyColour)
{
	SDL_Colour defaultConsoleColour = { DEFAULT_BACKGROUND_COLOUR_R, DEFAULT_BACKGROUND_COLOUR_G, 
		DEFAULT_BACKGROUND_COLOUR_B, DEFAULT_BACKGROUND_COLOUR_A };

	console.consoleSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, screen->w, screen->h / 2,
												  screen->format->BitsPerPixel, screen->format->Rmask,
												  screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	if(nullptr == console.consoleSurface)
	{
		return CONSOLE_RET_CREATE_SURFACE_FAIL;
	}

	int result = BitmapFont_InitBuiltInFont(console.defaultBitmapFont, console.consoleSurface, 
											DEFAULT_FONT_WIDTH, DEFAULT_FONT_HEIGHT, DEFAULT_FONT_FIRST_CHARACTER,
											DEFAULT_FONT_LAST_CHARACTER, fontColour, transparencyColour);
	if(CONSOLE_RET_SUCCESS != result)
	{
		return result;
	}

	SDL_Colour colour = (nullptr == consoleColour) ? defaultConsoleColour : *consoleColour;
	console.defaultConsoleColour = SDL_MapRGB(console.consoleSurface->format, colour.r, colour.g, colour.b);

	Console_CreateCursor(console, fontColour);

	InputBuffer_Init(console);
	OutputBuffer_Init(console);

	RegisterBuiltInCommand(console, "cv", console_command_version);
	RegisterBuiltInCommand(console, "clist", console_command_list_commands);
	RegisterBuiltInCommand(console, "cls", console_command_clear);
	
	return CONSOLE_RET_SUCCESS;
}

int console::Console_Render(Console& console, SDL_Surface *screen)
{
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = console.consoleSurface->w;
	rect.h = console.consoleSurface->h;

	int result;

	if(console.backgroundSurface != nullptr)
	{
		result = SDL_BlitSurface(console.backgroundSurface, &rect, console.consoleSurface, &rect);
		if(result != 0)
		{
			return CONSOLE_RET_BLIT_FAILED;
		}
	}
	else
	{
		result = SDL_FillRect(console.consoleSurface, &rect, console.defaultConsoleColour);
		if(-1 == result)
		{
			return CONSOLE_RET_FILL_RECT_FAILED;
		}

	}

	BitmapFont& font = (console.externalBitmapFont.fontSurface != nullptr) ? console.externalBitmapFont : console.defaultBitmapFont;

	result = InputBuffer_Render(console, font);
	if(result != CONSOLE_RET_SUCCESS)
	{
		return result;
	}

	Cursor_Render(console, font);
	result = OutputBuffer_Render(console, font);
	if(result != CONSOLE_RET_SUCCESS)
	{
		return result;
	}

	result = SDL_BlitSurface(console.consoleSurface, &rect, screen, &rect);
	if(result != 0)
	{
		return CONSOLE_RET_BLIT_FAILED;
	}

	return CONSOLE_RET_SUCCESS;
}

void console::Console_ProcessInput(Console& console, SDL_Event* event)
{
	BitmapFont& font = (console.externalBitmapFont.fontSurface != nullptr) ? console.externalBitmapFont : console.defaultBitmapFont;

	// only ASCII characters space to '~' are supported
	if(event->key.keysym.unicode >= font.firstChar && 
	   event->key.keysym.unicode <= font.lastChar &&
	   console.inputBuffer.buffer.length() < console.inputBuffer.maxBufferLength)
	{
		console.inputBuffer.buffer += event->key.keysym.unicode;
	}
	else
	{
		switch(event->type)
		{
			case SDL_KEYDOWN:
				switch(event->key.keysym.sym)
				{
					case SDLK_BACKSPACE:
						if(console.inputBuffer.buffer.length() > 0)
						{
							console.inputBuffer.buffer.erase(console.inputBuffer.buffer.length() - 1);
						}
					break;

					case SDLK_RETURN:
						InputBuffer_SubmitBuffer(console);
					break;

					case SDLK_PAGEUP:
						OutputBuffer_Scroll(console, 5, CONSOLE_SCROLL_DIR_UP);
					break;

					case SDLK_PAGEDOWN:
						OutputBuffer_Scroll(console, 5, CONSOLE_SCROLL_DIR_DOWN);
					break;

					default:
						break;
				}
			break;

			default:
				break;
		}
	}
}

void console::Console_Print(Console& console, string line)
{
	vector<string> lines;

	int i = 0;
	int j = 1;
	string s;
	while(i < line.length())
	{
		s += line[i];
		if(j == console.outputBuffer.maxLineLength)
		{
			lines.push_back(s);
			s.clear();
			j = 1;
		}
		else
		{
			j++;
		}

		i++;
	}

	if(!s.empty())
	{
		lines.push_back(s);
	}

	for(string b : lines)
	{
		console.outputBuffer.buffer.push_back(b);
		if(console.outputBuffer.buffer.size() > console.outputBuffer.maxNumLinesOnScreen)
		{
			// we're showing the max number of lines that can appear on the screen at any one time.
			// incrementing this gives the appearance that the text is scrolling.
			console.outputBuffer.topLineIndex++;
			console.outputBuffer.bottomLineIndex++;
		}
		else
		{
			console.outputBuffer.bottomLineIndex = console.outputBuffer.buffer.size() - 1;
		}
	}
}

bool console::Console_IsCommand(Console& console, string command)
{
	auto result = console.commands.find(command);
	if(console.commands.end() != result)
	{
		return true;
	}

	return false;
}

int console::Console_ExecuteCommand(Console& console, std::string command, std::vector<std::string>& args)
{
	if(!Console_IsCommand(console, command))
	{
		stringstream s;
		s << "INVALID COMMAND " << "\'" << command << "\'.";
		
		Console_Print(console, s.str());

		return CONSOLE_RET_COMMAND_DOESNT_EXIST;
	}

	auto cmd = console.commands.find(command);
	if(nullptr == cmd->second)
	{
		return CONSOLE_RET_NULLPTR_ARGUMENT;
	}

	cmd->second(console, args);

	return CONSOLE_RET_SUCCESS;
}

int console::Console_RegisterCommand(Console& console, string command, command_func_ptr commandFuncPtr)
{
	if(command.length() == 0 || StringContainsWhiteSpace(command))
	{
		return CONSOLE_RET_COMMAND_INVALID;
	}
	
	if(nullptr == commandFuncPtr)
	{
		return CONSOLE_RET_NULLPTR_ARGUMENT;
	}

	command = ToLower(command);

	if(Console_IsCommand(console, command))
	{
		return CONSOLE_RET_COMMAND_EXISTS;
	}

	console.commands[command] = commandFuncPtr;

	return CONSOLE_RET_SUCCESS;
}

void console::Console_SetBackground(Console& console, SDL_Surface* imageSurface)
{
	console.backgroundSurface = imageSurface;

	if(imageSurface != nullptr)
	{
		SDL_SetAlpha(console.backgroundSurface, 0, 255);
	}
}

int console::Console_SetFont(Console& console, SDL_Surface* fontSurface, 
							  unsigned int charWidth, unsigned int charHeight, 
							  unsigned char firstChar, unsigned char lastChar,
							  SDL_Colour* cursorColour)
{
	console.externalBitmapFont.fontSurface = fontSurface;
	console.externalBitmapFont.characterHeight = charHeight;
	console.externalBitmapFont.characterWidth = charWidth;
	console.externalBitmapFont.firstChar = firstChar;
	console.externalBitmapFont.lastChar = lastChar;
	console.externalBitmapFont.numberOfChars = (lastChar - firstChar) + 1;
	console.externalBitmapFont.transparencyColour = console.defaultBitmapFont.transparencyColour;

	// changing fonts requires a few things to be recalculated
	// so the font renders properly; even if fontSurface evaluates to nullptr
	// this still needs to be done, because if it does, we're falling back to the default font.
	// It's not obvious here, but the two calls below will detect if we've passed a nullptr
	// for the font argument and reconfigure the console to use the built-in font.
	InputBuffer_Init(console);
	OutputBuffer_Init(console);

	if(console.outputBuffer.buffer.size() > 0)
	{
		// the buffer is currently in use, so the indices for the buffer need to be recalculated.
		// if we've been scrolling through the text, this will scroll the text so we're
		// back at the bottom, viewing the last line that was entered.
		if(console.outputBuffer.buffer.size() > console.outputBuffer.maxNumLinesOnScreen)
		{
			// if we were at the very top of the output buffer when the font changed,
			// applyOffsetToY was probably true, meaning the bottom line was likely partially
			// obscured. Setting it to false makes sure the bottom line renders properly 
			// (ie, we can see the entire line) when we reset the indices. 
			console.outputBuffer.applyOffsetToY = false;

			console.outputBuffer.bottomLineIndex = console.outputBuffer.buffer.size() - 1;
			console.outputBuffer.topLineIndex = (console.outputBuffer.bottomLineIndex - (console.outputBuffer.maxNumLinesOnScreen - 1));
		}
	}

	if(fontSurface != nullptr)
	{
		SDL_SetColorKey(console.externalBitmapFont.fontSurface, SDL_SRCCOLORKEY,
						console.externalBitmapFont.transparencyColour);
	}

	// no need to set the colour key for the default font, this is done during initilization

	Console_CreateCursor(console, cursorColour);

	if(fontSurface != nullptr)
	{
		return CONSOLE_RET_SUCCESS;
	}
	else
	{
		return CONSOLE_RET_NULLPTR_ARGUMENT;
	}
}

int console::Console_CreateCursor(console::Console& console, SDL_Colour* colour)
{
	int width, height;

	if(console.externalBitmapFont.fontSurface != nullptr)
	{ 
		width = console.externalBitmapFont.characterWidth;
		height = console.externalBitmapFont.characterHeight;
	}
	else
	{
		width = console.defaultBitmapFont.characterWidth;
		height = console.defaultBitmapFont.characterHeight;
	}

	if(console.cursorSurface != nullptr)
	{
		SDL_FreeSurface(console.cursorSurface);
		console.cursorSurface = nullptr;
	}

	console.cursorSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, console.consoleSurface->format->BitsPerPixel,
												 console.consoleSurface->format->Rmask, console.consoleSurface->format->Gmask, 
												 console.consoleSurface->format->Bmask, console.consoleSurface->format->Amask);
	if(!console.cursorSurface)
	{
		return CONSOLE_RET_CREATE_SURFACE_FAIL;
	}

	SDL_Colour defaultColour = { DEFAULT_FONT_COLOUR_R, DEFAULT_FONT_COLOUR_G, DEFAULT_FONT_COLOUR_B, 0 };
	Uint32 convertedColour;
	if(colour != nullptr)
	{
		convertedColour = SDL_MapRGB(console.consoleSurface->format, colour->r, colour->g, colour->b);
	}
	else
	{
		convertedColour = SDL_MapRGB(console.consoleSurface->format, defaultColour.r, defaultColour.g, defaultColour.b);
	}
	
	int result = SDL_FillRect(console.cursorSurface, NULL, convertedColour);
	if(result != 0)
	{
		return CONSOLE_RET_FILL_RECT_FAILED;
	}

	return CONSOLE_RET_SUCCESS;
}

int console::Console_ResolutionChanged(Console& console, SDL_Surface *screen)
{
    SDL_FreeSurface(console.consoleSurface);
	console.consoleSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, screen->w, screen->h / 2, screen->format->BitsPerPixel,
												  screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	if(!console.consoleSurface)
	{
		return CONSOLE_RET_CREATE_SURFACE_FAIL;
	}

	InputBuffer_Init(console);
	OutputBuffer_Init(console);

	OutputBuffer_ResizeText(console);

	return CONSOLE_RET_SUCCESS;
}