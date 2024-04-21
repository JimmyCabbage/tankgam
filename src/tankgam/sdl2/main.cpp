#include "sys/Console.h"
#include "Net.h"
#include "Server.h"
#include "Client.h"
#include "Version.h"

#include "SDL.h"

#include <util/FileManager.h>

int main(int argc, char** argv)
{
    {
        Console console{};
        console.logf("tankgam engine version %s", TANKGAM_VERSION);

        FileManager fileManager{ console };
        fileManager.loadAssetsFile("dev.assets");
        fileManager.loadAssetsFile("tank.assets");
        
        bool initClient = false;
        bool initServer = false;
        if (argc > 1)
        {
            if (strcmp(argv[1], "--client") == 0)
            {
                initClient = true;
            }
            if (strcmp(argv[1], "--server") == 0)
            {
                initServer = true;
            }
        }
        
        if (!initClient && !initServer)
        {
            initClient = true;
            initServer = true;
        }
        
        Net net{ initClient, initServer };
        
        std::unique_ptr<Server> server;
        std::unique_ptr<Client> client;
        
        if (initServer)
        {
            server = std::make_unique<Server>(console, fileManager, net);
        }
        
        if (initClient)
        {
            client = std::make_unique<Client>(console, fileManager, net);
        }
       
        try
        {
            for (;;)
            {
                if (server && !server->runFrame())
                {
                    break;
                }
                
                if (client && !client->runFrame())
                {
                    break;
                }
            }
        }
        catch (const std::exception& e)
        {
            SDL_Log("Run Loop Exception: %s", e.what());
            return 1;
        }
    }
    
    //According to the docs I should call this even if I call SDL_QuitSubSystem for every SDL_InitSubSystem call I make
    SDL_Quit();
    
    return 0;
}
