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

//int console::BitmapFont_CreateSurface(SDL_Surface *surface, SDL_Surface *consoleSurface, int numCharacters, int characterWidth, int characterHeight)
//{
//	int fontSheetWidth = characterWidth * numCharacters;
//
//	surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, fontSheetWidth, 
//								   characterHeight, consoleSurface->format->BitsPerPixel, consoleSurface->format->Rmask,
//								   consoleSurface->format->Gmask, consoleSurface->format->Bmask, consoleSurface->format->Amask);
//	if(!surface)
//	{
//		return CONSOLE_RET_CREATE_SURFACE_FAIL;
//	}
//
//	return CONSOLE_RET_SUCCESS;
//}

int console::BitmapFont_Init(BitmapFont& bitmapfont, SDL_Surface *consoleSurface, int characterWidth, int characterHeight, SDL_Colour fontColour, SDL_Colour transparencyColour)
{
	int fontSheetWidth = DEFAULT_FONT_WIDTH * DEFAULT_FONT_NUM_GLYPHS;
	bitmapfont.characterWidth = characterWidth;
	bitmapfont.characterHeight = characterHeight;

	bitmapfont.fontSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, fontSheetWidth,
												  DEFAULT_FONT_HEIGHT, consoleSurface->format->BitsPerPixel, consoleSurface->format->Rmask,
								   consoleSurface->format->Gmask, consoleSurface->format->Bmask, consoleSurface->format->Amask);
	if(!bitmapfont.fontSurface)
	{
		return CONSOLE_RET_CREATE_SURFACE_FAIL;
	}

	bitmapfont.fontColour = SDL_MapRGB(consoleSurface->format, fontColour.r, fontColour.g, fontColour.b);
	bitmapfont.transparencyColour = SDL_MapRGB(consoleSurface->format, transparencyColour.r, 
											   transparencyColour.g, transparencyColour.b);
	SDL_SetColorKey(bitmapfont.fontSurface, SDL_SRCCOLORKEY, bitmapfont.transparencyColour);
	int xBasePosition = 0;

	for(int i = 0; i < DEFAULT_FONT_NUM_GLYPHS; i++)
	{
		for(int j = 0; j < DEFAULT_FONT_HEIGHT; j++)
		{
			for(int k = 0; k < DEFAULT_FONT_WIDTH; k++)
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
		xBasePosition = DEFAULT_FONT_WIDTH * (i + 1);
	}

	return CONSOLE_RET_SUCCESS;
}

void console::BitmapFont_RenderLine(Console& console, std::string line, int x, int y)
{
	int lineLength = line.length();
	SDL_Rect srcRect;
	SDL_Rect destRect;

	srcRect.x = srcRect.y = 0;
	srcRect.w = console.defaultBitmapFont.characterWidth;
	srcRect.h = console.defaultBitmapFont.characterHeight;
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
		srcRect.h = DEFAULT_FONT_HEIGHT - srcRect.y;
		destRect.y = 0;
	}

	for(int i = 0; i < lineLength; i++)
	{
		srcRect.x = (line[i] - DEFAULT_FONT_STARTING_CHARACTER) * console.defaultBitmapFont.characterWidth;
		
		SDL_BlitSurface(console.defaultBitmapFont.fontSurface, &srcRect, console.consoleSurface,  &destRect);

		destRect.x += console.defaultBitmapFont.characterWidth;
		//destRect.y -= console.defaultBitmapFont.characterHeight;
	}

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
	console.inputBuffer.y = console.consoleSurface->h - console.defaultBitmapFont.characterHeight;
	console.inputBuffer.maxBufferLength = console.consoleSurface->w / console.defaultBitmapFont.characterWidth;
}

void console::InputBuffer_Render(Console& console)
{
	BitmapFont_RenderLine(console, console.inputBuffer.buffer, console.inputBuffer.x, console.inputBuffer.y);
}

void console::OutputBuffer_Init(Console& console)
{
	console.outputBuffer.startX = console.inputBuffer.x;

	// minus 2 to factor in a gap between the bottom line of input and the input prompt.
	console.outputBuffer.startY = console.inputBuffer.y - console.defaultBitmapFont.characterHeight - 2;

	float integral;
	
	// character height is being added to startY because if we divide from startY,
	// we end up 1 line short.
	int temp = console.outputBuffer.startY + console.defaultBitmapFont.characterHeight;

	float result = (float)temp / (float)console.defaultBitmapFont.characterHeight;
	float fractional = modf(result, &integral);
	console.outputBuffer.maxNumLinesOnScreen = integral;
	if(fractional > 0)
	{
		// there's not enough room to render the top most line, so it'll be partially
		// obscured. It's still considered a visible line.
		console.outputBuffer.maxNumLinesOnScreen++;
	}

	console.outputBuffer.maxLineLength = console.consoleSurface->w / console.defaultBitmapFont.characterWidth;
}

void console::OutputBuffer_Render(Console& console)
{
	int xPos = console.outputBuffer.startX;
	int yPos = console.outputBuffer.startY;

	for(int i = console.outputBuffer.bottomLineIndex; i >= console.outputBuffer.topLineIndex; i--)
	{
		BitmapFont_RenderLine(console, console.outputBuffer.buffer[i], xPos, yPos);
		
		if(yPos > 0)
		{
			yPos -= console.defaultBitmapFont.characterHeight;
		}
		else
		{
			break;
		}
	}
}

//void console::OutputBuffer_Scroll(Console& console, int numberOfLines, int direction)
//{
//	int numberOfLinesScrolled;
//
//	switch(direction)
//	{
//		case CONSOLE_SCROLL_DIR_UP:
//			if(numberOfLines < console.outputBuffer.topLineIndex)
//			{
//				numberOfLinesScrolled = numberOfLines;
//			}
//			else
//			{
//				numberOfLinesScrolled = console.outputBuffer.topLineIndex;
//			}
//
//			console.outputBuffer.topLineIndex -= numberOfLinesScrolled;
//			console.outputBuffer.bottomLineIndex -= numberOfLinesScrolled;
//			break;
//
//		case CONSOLE_SCROLL_DIR_DOWN:
//			if(console.outputBuffer.bottomLineIndex + numberOfLines < console.outputBuffer.buffer.size())
//			{
//				numberOfLinesScrolled = numberOfLines;
//			}
//			else
//			{
//				numberOfLinesScrolled = console.outputBuffer.buffer.size() - console.outputBuffer.bottomLineIndex;
//			}
//
//			console.outputBuffer.topLineIndex += numberOfLinesScrolled;
//			console.outputBuffer.bottomLineIndex += numberOfLinesScrolled;
//			break;
//
//		default:
//			break;
//	}
//}

void console::console_command_version(Console& console, vector<string>& args)
{
	Console_Print(console, "Console version " + to_string(CONSOLE_VERSION_MAJOR) + "." + to_string(CONSOLE_VERSION_MINOR));
}

int console::Console_Init(Console& console, SDL_Surface *screen, SDL_Colour& consoleColour,
						  SDL_Colour& fontColour, SDL_Colour& transparencyColour)
{
	console.consoleSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, screen->w, screen->h / 2,
												  screen->format->BitsPerPixel, screen->format->Rmask,
												  screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	if(nullptr == console.consoleSurface)
	{
		return CONSOLE_RET_CREATE_SURFACE_FAIL;
	}

	int result = BitmapFont_Init(console.defaultBitmapFont, console.consoleSurface, DEFAULT_FONT_WIDTH, DEFAULT_FONT_HEIGHT, fontColour, transparencyColour);
	if(CONSOLE_RET_SUCCESS != result)
	{
		return result;
	}

	console.defaultConsoleColour = SDL_MapRGB(console.consoleSurface->format, consoleColour.r, consoleColour.g, consoleColour.b);

	InputBuffer_Init(console);
	OutputBuffer_Init(console);

	if(CONSOLE_RET_SUCCESS != Console_RegisterCommand(console, "cv", console_command_version))
	{
		Console_Print(console, "Console_Init: failed to register command \'cv\'");
	}

	return CONSOLE_RET_SUCCESS;
}

void console::Console_Render(Console& console, SDL_Surface *screen)
{
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = console.surfaceRect.w;
	rect.h = console.surfaceRect.h;

	SDL_FillRect(console.consoleSurface, &rect, console.defaultConsoleColour);

	InputBuffer_Render(console);
	OutputBuffer_Render(console);
	SDL_BlitSurface(console.consoleSurface, &rect, screen, &rect);
}

void console::Console_ProcessInput(Console& console, Uint16 unicode)
{
	SDL_Rect glyph;

	// only ASCII characters space to '~' are supported
	if(unicode >= DEFAULT_FONT_STARTING_CHARACTER &&
	   unicode <= 126 &&
	   console.inputBuffer.buffer.length() < console.inputBuffer.maxBufferLength)
	{
		console.inputBuffer.buffer += unicode;
	}
	else if(8 == unicode)	// backspace
	{
		if(console.inputBuffer.buffer.length() > 0)
		{
			console.inputBuffer.buffer.erase(console.inputBuffer.buffer.length() - 1);
		}
	}
	else if(13 == unicode)	// enter was pressed
	{
		string command;
		vector<string> args;

		// delimit the end of the string
		console.inputBuffer.buffer += ' ';

		InputBuffer_SplitInput(console.inputBuffer, command, args);

		console.inputBuffer.buffer.clear();

		Console_ExecuteCommand(console, command, args);
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
		}

		console.outputBuffer.bottomLineIndex = console.outputBuffer.buffer.size() - 1;
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

int console::Console_RegisterCommand(Console& console, string command, command_func_ptr command_func_ptr)
{
	if(Console_IsCommand(console, command))
	{
		return CONSOLE_RET_COMMAND_EXISTS;
	}

	if(nullptr == command_func_ptr)
	{
		return CONSOLE_RET_NULLPTR_ARGUMENT;
	}

	console.commands[command] = command_func_ptr;

	return CONSOLE_RET_SUCCESS;
}