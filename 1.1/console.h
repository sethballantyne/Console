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

#pragma once

#include <SDL.h>
#include <string>
#include <map>
#include <vector>
#include "defaults.h"

// return values used by the console
#define CONSOLE_RET_SUCCESS                 0
#define CONSOLE_RET_CREATE_SURFACE_FAIL     1
#define CONSOLE_RET_COMMAND_DOESNT_EXIST    2
#define CONSOLE_RET_COMMAND_EXISTS          3
#define CONSOLE_RET_NULLPTR_ARGUMENT        4
#define CONSOLE_RET_FILL_RECT_FAILED        5
#define CONSOLE_RET_BLIT_FAILED             6
#define CONSOLE_RET_COMMAND_INVALID	        7

#define CONSOLE_SCROLL_DIR_UP               0
#define CONSOLE_SCROLL_DIR_DOWN             1

#define CONSOLE_VERSION_MAJOR               1
#define CONSOLE_VERSION_MINOR               1

// the gap in pixels between the output buffer and the input buffer
#define CONSOLE_GAP_BETWEEN_BUFFERS         2

#define CONSOLE_GAP_BELOW_INPUT_BUFFER      1


namespace console
{
	struct BitmapFont
	{
		SDL_Surface *fontSurface = nullptr;

		Uint32 fontColour;

		Uint32 transparencyColour;

		// the height of each character in pixels. 
		int characterHeight;

		// the witdth of each character in pixels
		int characterWidth;

		int numberOfChars;

		unsigned char firstChar;

		unsigned char lastChar;

		~BitmapFont()
		{
			if(nullptr != fontSurface)
			{
				SDL_FreeSurface(fontSurface);
			}
		}
	};

	struct InputBuffer
	{
		std::string buffer;
		int x;
		int y;
		int maxBufferLength;
	};

	struct OutputBuffer
	{
		std::vector<std::string> buffer;
		SDL_Rect clippingRect;
		int x;
		int y;
		int bottomLineIndex = 0;
		int topLineIndex = 0;

		// the maximum number of lines that can be visible to the user.
		// This is based on the size of the console and the font's height.
		// pre-calcuated in OutputBuffer_Init()
		int maxNumLinesOnScreen; 
		int maxLineLength;
		int yOffset;
		bool applyOffsetToY = false;
	};

	struct Console;
	typedef void(*command_func_ptr)(Console&, std::vector<std::string> &);

	struct Console
	{
		// the available commands that the console can execute
		std::map<std::string, command_func_ptr> commands;

		// the built in bitmap font
		BitmapFont defaultBitmapFont;

		// font loaded in via Console_SetFont
		BitmapFont externalBitmapFont;

		// the background, output buffer and input buffers render to this surface.
		SDL_Surface *consoleSurface = nullptr;

		// surface for the background image.
		SDL_Surface *backgroundSurface = nullptr;

		SDL_Surface *cursorSurface = nullptr;

		// stores the users input and determines how the buffer is rendered to the console surface
		InputBuffer inputBuffer;

		// stores all the output resulting from user input and controls how it's all rendered
		OutputBuffer outputBuffer;

		// the colour of the consoles background
		Uint32 defaultConsoleColour;

		bool enabled;

		~Console()
		{
			if(nullptr != consoleSurface)
			{
				SDL_FreeSurface(consoleSurface);
			}
		}
	};

	/**********************************************************************************
	 * INTERNAL FUNCTIONS - GENERALLY NOT MEANT FOR EXTERNAL USE
	 **********************************************************************************/

	/// BUILT-IN COMMANDS /////////////////////////////////////////////////////////////
	void console_command_version(Console& console, std::vector<std::string>& args);
	void console_command_list_commands(Console& console, std::vector<std::string>& args);
	void console_command_clear(Console& console, std::vector<std::string>& args);

	void RegisterBuiltInCommand(Console& console, const std::string& command, command_func_ptr func);

	/// INTERNAL BITMAP FONT FUNCTIONS ////////////////////////////////////////////////
	int BitmapFont_InitBuiltInFont(BitmapFont& bitmapFont, SDL_Surface *screen, int characterWidth, 
								   int characterHeight, unsigned char firstChar, unsigned char lastChar, SDL_Colour* fontColour, SDL_Colour* transparency);
	int BitmapFont_RenderLine(Console& console, BitmapFont& font, std::string& line, int x, int y);

	/// INTERNAL INPUTBUFFER FUNCTIONS ////////////////////////////////////////////////
	void InputBuffer_SplitInput(InputBuffer& inputBuffer, std::string& command, std::vector<std::string>& args);
	void InputBuffer_Init(Console& console);
	int InputBuffer_Render(Console& console, BitmapFont& font);
	void InputBuffer_SubmitBuffer(Console& console);
	/// INTERNAL OUTPUTBUFFER FUNCTIONS //////////////////////////////////////////////

	void OutputBuffer_Clear(Console& console);

	// call this AFTER the inputbuffer has been initialized!
	void OutputBuffer_Init(Console& console);

	int OutputBuffer_Render(Console& console, BitmapFont& font);
	void OutputBuffer_ResizeText(Console& console);
	void OutputBuffer_Scroll(Console& console, int numberOfLines, int direction);

	/// INTERNAL CURSOR FUNCTIONS ////////////////////////////////////////////////////
	void Cursor_Render(Console& console, BitmapFont& font);

	/********************************************************************************
     * END INTERNAL FUNCTIONS
	 ********************************************************************************/

	/********************************************************************************
	 * CONSOLE FUNCTIONS FOR HAX0RS
	 ********************************************************************************/
	int Console_Init(Console& console, SDL_Surface* screen, SDL_Colour* consoleColour, 
					SDL_Colour* fontColour, SDL_Colour* transparencyColour);

	void Console_ProcessInput(Console& console, SDL_Event* event);
	int Console_Render(Console& console, SDL_Surface *screen);
	void Console_Print(Console& console, std::string line);
	bool Console_IsCommand(Console& console, std::string command);
	int Console_ExecuteCommand(Console& console, std::string command, std::vector<std::string>& args);
	int Console_RegisterCommand(Console& console, std::string command, command_func_ptr commandFuncPtr);
	void Console_SetBackground(Console& console, SDL_Surface* imageSurface);
	int Console_SetFont(Console& console, SDL_Surface* fontSurface,  
						 unsigned int charWidth, unsigned int charHeight, unsigned char firstChar, unsigned char lastChar,
						 SDL_Colour* cursorColour);
	int Console_CreateCursor(Console& console, SDL_Colour* colour);
	int Console_ResolutionChanged(Console& console, SDL_Surface *screen);
}

