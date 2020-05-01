# Console
Console is a quake-style console written in C++ using SDL 1.2. It's designed to be lightweight and easy to use: the API only consists of 8 functions and there's no libraries to link against; just drop `console.cpp`, `console.h` and `defaults.h` into your project and you're ready to go. Full HTML documentation also exists if you like to read. :P

Including the console into a project is fairly simple:

1. Create an instance of the Console structure
```
Console con;
```

2. Initialize it:
```
int result = Console_Init(con, screen, nullptr, nullptr, nullptr)
if(result != CONSOLE_RET_SUCCESS)
{
  ..... // error handling here
}
```
screen is obviously the `SDL_Surface` instance you're using to render to the screen. The `nullptr`'s are telling the Console
to use the default colours for the console's background, font colour and colour key.

3. Enable Unicode in SDL. The console uses this to retrieve the ASCII values of keys pressed. The irony of using unicode to retrieve ASCII values is not lost on me. ;)
```
SDL_EnableUNICODE(1);
```

4. In your main loop, call SDL_ProcessInput. I do this when an `SDL_KEYDOWN` event is being handled:

```
..... // inside an event handler
case SDL_KEYDOWN:
  if(con.enabled)
  {
    event is the SDL_Event instance you're using to handle events. 
    Console_ProcessInput(con, event.key.keysym.unicode);
  }
break;
... // continue handling other events`
```

5. Render!
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

A fully working example exists in the documentation. For more information on the API and how the console works, see the documentation (HINT: read the docs!). 
