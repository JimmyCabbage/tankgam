#include "sys/Console.h"
#include "sys/File.h"
#include "Net.h"
#include "Server.h"
#include "Client.h"

#include "SDL.h"

int main(int argc, char** argv)
{
    {
        Console console{};

        FileManager fileManager{console};
        fileManager.loadAssetsFile("dev.assets");
        fileManager.loadAssetsFile("tank.assets");
        
        bool onlyClient = false;
        bool onlyServer = false;
        if (argc > 1)
        {
            if (strcmp(argv[1], "--client") == 0)
            {
                onlyClient = true;
            }
            
            if (strcmp(argv[1], "--server") == 0)
            {
                onlyServer = true;
            }
        }
        
        Net net{ !onlyServer, !onlyClient };

        if (!onlyClient && !onlyServer)
        {
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
        else if (onlyClient)
        {
            Client client{console, fileManager, net};
            
            for (;;)
            {
                if (!client.runFrame())
                {
                    break;
                }
            }
        }
        else if (onlyServer)
        {
            Server server{console, fileManager, net};
            
            for (;;)
            {
                if (!server.runFrame())
                {
                    break;
                }
            }
        }
    }
    
    //According to the docs I should call this even if I call SDL_QuitSubSystem for every SDL_InitSubSystem call I make
    SDL_Quit();
    
    return 0;
}
