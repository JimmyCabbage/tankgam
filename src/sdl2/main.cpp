#include "sys/Console.h"
#include "sys/Net.h"
#include "Server.h"
#include "Client.h"

#include "SDL.h"

int main(int /*argc*/, char** /*argv*/)
{
    Console console{};
    Net net{};

    Server server{ console, net };
    Client client{ console, net };
    
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
