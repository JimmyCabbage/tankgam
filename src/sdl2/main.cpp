#include "Engine.h"

#include "SDL.h"

int main(int /*argc*/, char** /*argv*/)
{
    Engine engine{};
    
    engine.loop();
    
    //According to the docs I should call this even if I call SDL_QuitSubSystem for every SDL_InitSubSystem call I make
    SDL_Quit();
    
    return 0;
}
