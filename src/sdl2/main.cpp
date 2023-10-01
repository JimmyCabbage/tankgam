#include "sys/Console.h"
#include "sys/File.h"
#include "sys/Net.h"
#include "Server.h"
#include "Client.h"

#include "SDL.h"

int main(int /*argc*/, char** /*argv*/)
{
    {
        Console console{};

        FileManager fileManager{console};
        fileManager.loadAssetsFile("dev.assets");
        fileManager.loadAssetsFile("tank.assets");

        Net net{};

        Server server{console, fileManager, net};
        Client client{console, fileManager, net};

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
    }
    
    //According to the docs I should call this even if I call SDL_QuitSubSystem for every SDL_InitSubSystem call I make
    SDL_Quit();
    
    return 0;
}
