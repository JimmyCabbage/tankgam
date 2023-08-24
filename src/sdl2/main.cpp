#include "sys/Console.h"
#include "Server.h"
#include "Client.h"

#include "SDL.h"

int main(int /*argc*/, char** /*argv*/)
{
    Console console{};

    Server server{ console };
    Client client{ console };
    
    for (;;)
    {
        if (!server.runFrame())
        {
            break;
        }

        if (!client.runFrame())
        {
            break;
        }
    }
    
    //According to the docs I should call this even if I call SDL_QuitSubSystem for every SDL_InitSubSystem call I make
    SDL_Quit();
    
    return 0;
}
