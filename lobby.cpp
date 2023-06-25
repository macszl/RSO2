#include <Ice/Ice.h>
#include "chat.h"
#include <memory>
#include "chatImplementations.h"


int
main (int argc, char *argv[])
{
    int status = 0;
    Ice::CommunicatorPtr ic;
    try
    {
        ic = Ice::initialize (argc, argv);
        Ice::PropertiesPtr properties = ic->getProperties();
        int serverPort = 50000;
        Ice::ObjectAdapterPtr adapter
                = ic->createObjectAdapterWithEndpoints ("ChatAdapter -t",
                                                        "tcp -p " + std::to_string(serverPort));
        Ice::ObjectPtr user = new UserI;
        Ice::ObjectPtr lobby = new LobbyI;
        adapter->add (user, Ice::stringToIdentity ("Chat.User"));
        adapter->add (lobby, Ice::stringToIdentity ("Chat.Lobby"));
        adapter->activate ();
        ic->waitForShutdown ();
    } catch (const Ice::Exception & e)
    {
        std::cerr << e << std::endl;
        status = 1;
    } catch (const char *msg)
    {
        std::cerr << msg << std::endl;
        status = 1;
    }
    if (ic)
    {
        try
        {
            ic->destroy ();
        }
        catch (const Ice::Exception & e)
        {
            std::cerr << e << std::endl;
            status = 1;
        }
    }
    return status;
}