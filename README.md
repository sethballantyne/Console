# Console

Console is a Quake-style console written in C++ for SDL 1.2. It's designed to be lightweight and easy to integrate into existing
projects; there's no libraries to link against, just drop the source files into your existing code base and you're ready to go.

Including the console into a project is fairly simple:

**1. Create an instance of the Console structure**
```
#include "console.h"
using namespce console;

Console con;
```

**2. Initialize it:**
```
int result = Console_Init(con, screen, nullptr, nullptr, nullptr)
if(result != CONSOLE_RET_SUCCESS)
{
  ..... // error handling here
}
```
screen is obviously the `SDL_Surface` instance you're using to render to the screen. The `nullptr`'s are telling the Console
to use the default colours for the console's background, font colour and colour key.

**3. Enable Unicode in SDL.** 
The console uses this to retrieve the ASCII values of keys pressed. 
```
SDL_EnableUNICODE(1);
```

**4. In your main loop, call `Console_ProcessInput`.**
I do this when an `SDL_KEYDOWN` event is being handled:

```
..... // inside an event handler
case SDL_KEYDOWN:
  if(con.enabled)
  {
    // event is the SDL_Event instance you're using to handle events. 
    Console_ProcessInput(con, event.key.keysym.unicode);
  }
break;
... // continue handling other events

case SDL_KEYUP:
 switch(event.key.keysym.sym)
 {
   case SDLK_TAB:
     con.enabled = !con.enabled;
   break;
   ... // handle other keys
 }
```

Note that you don't have to use the `enabled` member of the Console structure; it's simply provided for your convenience and the console never directly makes use of it. Regardless of which method you use, don't forget to bind the console to a key!

**5. Render!**
```
Console_Render(con, screen);
```

EASY! It also has facilities to register and execute commands; it's a console after all.

To register a command, you have to pass a name for the command and the address of the function implementing
the command, like so:

```
// functions that implement commands must have the signature
// void <func_name>(Console&, vector<string>&)
void CustomCommandFunc(Console& console, vector<string>& args)
{
	Console_Print(console, "HACK THE PLANET!!!!");
}

// when the user types custom_command into the console,
// CustomCommandFunc will be executed. If the user entered any arguments,
// they'll be present in the functions vector argument.
Console_RegisterCommand(con, "custom_command", CustomCommandFunc);
```

You can programmatically execute a command by calling `Console_ExecuteCommand`, though typically you won't need to do this because part of the job `Console_ProcessInput` is to detect whether commands have been entered and executing them.

v1.1 also introduced the ability to use external bitmap fonts and backgrounds via `Console_SetFont` and `Console_SetBackground`.

A fully working example exists in the documentation. For more information on the API and how the console works, see the documentation (HINT: read the docs!). 
