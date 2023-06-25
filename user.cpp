//
// Created by maciek on 4/15/23.
//
#include <Ice/Ice.h>
#include <Ice/Application.h>
#include <memory>
#include "chatImplementations.h"
#include <map>
#include <functional>
#include <algorithm>
#include <Ice/ProxyHandle.h>

class UserApplication : Ice::Application {
public:
    Ice::CommunicatorPtr communicatorPtr;
    Ice::ObjectAdapterPtr adapterPtr;

    int serverPort;

    LobbyPrx lobbyPrx;
    IceInternal::ProxyHandle<IceProxy::Chat::User> userPrx;
    RoomPrx roomPrx = ICE_NULLPTR;

    UserApplication(int argc, char *argv[])
    {
        communicatorPtr = Ice::initialize (argc, argv);
        serverPort = 10000;
        //get base proxy to lobby
        std::cout << "ProxyHandle" << std::endl;
        IceInternal::ProxyHandle<IceProxy::Ice::Object> baseLobby =
                communicatorPtr->stringToProxy ("Lobby:tcp -h 192.168.171.216 -p " + std::to_string(serverPort));
        //cast it to proper proxy
        std::cout << "CheckedCast" << std::endl;
        lobbyPrx = LobbyPrx ::checkedCast(baseLobby);
        std::cout << "After CheckedCast" << std::endl;

        //TODO - make this a command line argument
        std::cout << "CommunicatorPtr" << std::endl;
        int port = 57576;
        adapterPtr = communicatorPtr->createObjectAdapterWithEndpoints(
                "user_adapter",
                "tcp -p " + std::to_string(port) );
        adapterPtr->activate();
    }


    // Login function
    void login() {
        try {
            std::string username, password;

            std::cout << "Enter username: ";
            std::getline(std::cin, username);

            std::cout << "Enter password: ";
            std::getline(std::cin, password);

            UserPtr userPtr = new UserI(username);
            userPrx = UserPrx::uncheckedCast(adapterPtr->addWithUUID(userPtr));

            std::string token = lobbyPrx->userLogin(userPrx, password);
            Ice::Context ctx;
            ctx["token"] = token;
            ctx["username"] = userPrx->getName();

            userPrx = userPrx->ice_context(ctx);
            lobbyPrx = lobbyPrx->ice_context(ctx);

            std::cout << "Successfully logged in." << std::endl;
        } catch (const Ice::Exception& ex) {
            std::cerr << "Login error: " << ex << std::endl;
        }
    }

    // Get all users function
    void getUsers() {
        try {
            std::vector<UserPrx> users = lobbyPrx->getUsers();

            for (const auto &user: users) {
                std::cout << user->getName() << std::endl;
            }
        } catch (const Ice::UnknownUserException& ex) {
            std::cout << "Unknown user!" << std::endl;
        } catch (const Ice::Exception& ex) {
            std::cerr << "Error retrieving users: " << ex << std::endl;
        }
    }

    // Get all rooms function
    void getRooms() {
        try {
            std::vector<RoomPrx> rooms = lobbyPrx->getRooms();
            for (const auto &room: rooms) {
                std::cout << room->getName() << std::endl;

            }
        } catch (const Ice::UnknownUserException& ex) {
            std::cout << "Unknown user!" << std::endl;
        } catch (const Chat::AccessDenied& ex) {
            std::cout << "Access denied!" << std::endl;
        } catch (const Ice::Exception& ex) {
            std::cerr << "Error retrieving rooms: " << ex << std::endl;
        }
    }



    // Send private message function
    void sendPrivateMessage() {
        try {
            std::string username, message;

            std::cout << "Enter recipient's username: ";
            std::getline(std::cin, username);

            std::cout << "Enter message: ";
            std::getline(std::cin, message);

            std::vector<UserPrx> users = lobbyPrx->getUsers();

            for (const auto &user: users) {
                if (user != ICE_NULLPTR && user->getName() == username) {  // Check if user is not null before accessing its properties
                    user->receivePrivateMessage(userPrx, message);
                    return;
                }
            }

            std::cout << "Recipient not found." << std::endl;
        }
        catch (const Chat::AccessDenied& ex) {
            std::cerr << "Access denied!" << std::endl;
        }
        catch (const Ice::Exception& ex) {
            std::cerr << "Error sending private message: " << ex << std::endl;
        }
    }

    // Leave room function
    void leaveRoom() {
        try {
            if (roomPrx != ICE_NULLPTR) {  // Check if roomPrx is not null before accessing it
                roomPrx->leave(userPrx);
                roomPrx = ICE_NULLPTR;  // Set roomPrx to null after leaving the room
                std::cout << "Left the room." << std::endl;
            } else {
                std::cout << "You are currently not in any of the rooms!" << std::endl;
            }
        } catch (const Ice::Exception& ex) {
            std::cerr << "Error leaving room: " << ex << std::endl;
        }
    }

    // Logout function
    void logout() {
        try {
            if (roomPrx != ICE_NULLPTR) {  // Check if roomPrx is not null before leaving the room
                roomPrx->leave(userPrx);
                roomPrx = ICE_NULLPTR;  // Set roomPrx to null after leaving the room
            }
            if (lobbyPrx != ICE_NULLPTR) {  // Check if lobbyPrx is not null before logging out
                lobbyPrx->logout();
            }
            if (userPrx != ICE_NULLPTR) {  // Check if userPrx is not null before removing it
                adapterPtr->remove(userPrx->ice_getIdentity());
            }
            userPrx = ICE_NULLPTR;  // Set userPrx to null after removing it
            std::cout << "Successfully logged out." << std::endl;
        } catch (const Chat::AccessDenied & ex) {
            std::cerr << "Access denied!" << std::endl;
        }catch (const Ice::Exception& ex) {
            std::cerr << "Error logging out: " << ex << std::endl;
        }
    }


    // Send message function
    void sendMessage() {
        try {
            std::string message;

            std::cout << "Enter message: ";
            std::getline(std::cin, message);

            if (roomPrx != ICE_NULLPTR) {
                roomPrx->sendMessage(userPrx, message);
            } else {
                std::cout << "You are not in a room currently!" << std::endl;
            }
        } catch (const Ice::Exception& ex) {
            std::cerr << "Error sending message: " << ex << std::endl;
        }
        std::cout<<"Message sent" << std::endl;
    }

    // Create room function
    void createRoom() {
        try {
            std::string roomName;

            std::cout << "Enter room name: ";
            std::getline(std::cin, roomName);

            if (lobbyPrx != ICE_NULLPTR) {  // Check if lobbyPrx is not null before creating the room
                lobbyPrx->createRoom(roomName);
                std::cout << "Room " + roomName + " successfully created" << std::endl;
            } else {
                std::cout << "You are not connected to a lobby!" << std::endl;
            }
        } catch (const Chat::AccessDenied& ex) {
            std::cout << "Access denied! You are most likely not logged in!" << std::endl;
        } catch (const Ice::Exception& ex) {
            std::cerr << "Error creating room: " << ex << std::endl;
        }
    }

    // Join room function
    void joinRoom() {
        try {

            if (roomPrx != ICE_NULLPTR) {  // Check if roomPrx is not null before leaving the current room
                roomPrx->leave(userPrx);
            }


            std::string roomName;

            std::cout << "Enter room name: ";
            std::getline(std::cin, roomName);

            if (lobbyPrx != ICE_NULLPTR) {  // Check if lobbyPrx is not null before finding the room
                roomPrx = lobbyPrx->findRoom(roomName);
                if (roomPrx != ICE_NULLPTR) {  // Check if roomPrx is not null after finding the room
                    roomPrx->join(userPrx);
                    std::cout << "Joined room " + roomName + "!" << std::endl;
                } else {
                    std::cout << "Something went wrong, findRoom returned null!" << std::endl;
                }
            } else {
                std::cout << "Something went wrong, lobby is null!" << std::endl;
            }
        } catch(const Chat::AccessDenied &ex ) {
            std::cerr << " Access denied!" << std::endl;
        } catch (const Ice::Exception &ex) {
            std::cerr << "Error joining room: " << ex << std::endl;
        }
    }



    void registerUser() {
        try {
            std::string username, password;

            std::cout << "Enter username: ";
            getline(std::cin, username);

            std::cout << "Enter password: ";
            getline(std::cin, password);

            UserPtr userPtr = new UserI(username);
            userPtr->setName(username);

            UserPrx userPrxRegister = UserPrx::uncheckedCast(adapterPtr->addWithUUID(userPtr));
            lobbyPrx->userRegister(userPrxRegister, password);

            std::cout << "User registered successfully." << std::endl;
        } catch (const Ice::Exception& ex) {
            std::cerr << "Error registering user: " << ex << std::endl;
        }
    }

    void printCommands()
    {
        std::map<std::string, std::function<void()>> printCommandMap{{"/login",  []() {
            std::cout << "Login" << std::endl;
        }},
                                                                     {"/register",    []() {
                                                                         std::cout << "Register" << std::endl;
                                                                     }},
                                                                     {"/create", []() {
                                                                         std::cout << "Create Room" << std::endl;
                                                                     }},
                                                                     {"/join",   []() {
                                                                         std::cout << "Join Room" << std::endl;
                                                                     }},
                                                                     {"/say",    []() {
                                                                         std::cout << "Send Message" << std::endl;
                                                                     }},
                                                                     {"/msg",     []() {
                                                                         std::cout << "Send Private Message"
                                                                                   << std::endl;
                                                                     }},
                                                                     {"/leave",  []() {
                                                                         std::cout << "Leave Room" << std::endl;
                                                                     }},
                                                                     {"/logout", []() {
                                                                         std::cout << "Logout" << std::endl;
                                                                     }},
                                                                     {"/rooms",  []() {
                                                                         std::cout << "Get Rooms" << std::endl;
                                                                     }},
                                                                     {"/users",  []() {
                                                                         std::cout << "Get Users" << std::endl;
                                                                     }},
                                                                     {"/list", []() {
                                                                         std::cout << "List commands" << std::endl;
                                                                     }}};

        for (auto it: printCommandMap) {
            std::cout << it.first << std::endl;
        }
    }
    int run(int argc, char* argv[]) override {
        std::string command;
        std::map<std::string, std::function<void()>> printCommandMap{{"/login",  []() {
                                                                        std::cout << "Login" << std::endl;
                                                                    }},
                                                                     {"/register",    []() {
                                                                         std::cout << "Register" << std::endl;
                                                                     }},
                                                                     {"/create", []() {
                                                                         std::cout << "Create Room" << std::endl;
                                                                     }},
                                                                     {"/join",   []() {
                                                                         std::cout << "Join Room" << std::endl;
                                                                     }},
                                                                     {"/say",    []() {
                                                                         std::cout << "Send Message" << std::endl;
                                                                     }},
                                                                     {"/msg",     []() {
                                                                         std::cout << "Send Private Message"
                                                                                   << std::endl;
                                                                     }},
                                                                     {"/leave",  []() {
                                                                         std::cout << "Leave Room" << std::endl;
                                                                     }},
                                                                     {"/logout", []() {
                                                                         std::cout << "Logout" << std::endl;
                                                                     }},
                                                                     {"/rooms",  []() {
                                                                         std::cout << "Get Rooms" << std::endl;
                                                                     }},
                                                                     {"/users",  []() {
                                                                         std::cout << "Get Users" << std::endl;
                                                                     }},
                                                                     {"/list", []() {
                                                                         std::cout << "List commands" << std::endl;
                                                                     }}};

        std::map<std::string, std::function<void()>> commandMap{{"/login",  std::bind(&UserApplication::login, this)},
                                                                {"/list", std::bind(&UserApplication::printCommands, this)},
                                                                {"/register",    std::bind(&UserApplication::registerUser,
                                                                                      this)},
                                                                {"/create", std::bind(&UserApplication::createRoom,
                                                                                      this)},
                                                                {"/join",   std::bind(&UserApplication::joinRoom,
                                                                                      this)},
                                                                {"/say",    std::bind(&UserApplication::sendMessage,
                                                                                      this)},
                                                                {"/msg",     std::bind(
                                                                        &UserApplication::sendPrivateMessage, this)},
                                                                {"/leave",  std::bind(&UserApplication::leaveRoom,
                                                                                      this)},
                                                                {"/logout", std::bind(&UserApplication::logout, this)},
                                                                {"/rooms",  std::bind(&UserApplication::getRooms,
                                                                                      this)},
                                                                {"/users",  std::bind(&UserApplication::getUsers,
                                                                                      this)}};


        std::cout << "Commands: " << std::endl;
        for (auto it: printCommandMap) {
            std::cout << it.first << std::endl;
        }

        while (true) {
            getline(std::cin, command);

            auto it = commandMap.find(command);
            if (it != commandMap.end()) {
                it->second();
            } else if (command == "/exit") {
                break;
            } else {
                std::cout << "Invalid command" << std::endl;
            }
        }

        return 0;
    }
};

int main(int argc, char *argv[]) {

    int status = 0;
    Ice::CommunicatorPtr ic;
    try
    {

        srand(time(ICE_NULLPTR));
        UserApplication userApplication(argc, argv);
        userApplication.run(argc, argv);


        ic->shutdown();
        ic->waitForShutdown();

    }
    catch (const Ice::Exception & ex)
    {
        std::cerr << ex << std::endl;
        status = 1;
    } catch (const char *msg)
    {
        std::cerr << msg << std::endl;
        status = 1;
    }
    if (ic)
        ic->destroy ();
    return status;

}