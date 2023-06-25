//
// Created by maciek on 4/15/23.
//

#ifndef RSO2_CHATIMPL_H
#define RSO2_CHATIMPL_H

#include "chat.h"
#include <unordered_set>
#include "chat_db_helper.h"


using namespace Chat;


class RoomI : public Room {
    Users users;
    std::string name;

    std::string getName(const ::Ice::Current &current) {
        return name;
    }

    Users getUsers(const ::Ice::Current &current) {
        return users;
    }

    void join(const UserPrx &user, const ::Ice::Current &current) {

        std::cout << user->getName() << " is currently trying to join the room " << this->getName(current) << std::endl;
        if (checkAllUsersForName(user)) {
            return;
        }

        users.push_back(user);
        std::cout << user->getName() << " has joined the room " << this->getName(current) << std::endl;
    }

    void leave(const UserPrx &user, const ::Ice::Current &current) {
        auto userIt = std::find(users.begin(), users.end(), user);

        if (userIt == users.end()) {
            // If the user is not found, safely return.
            return;
        }
        // Remove the user from the room.
        users.erase(userIt);
    }


    void sendMessage(const UserPrx &fromUser, const std::string &message, const Ice::Current &current) {

        RoomPrx roomPrx = Ice::uncheckedCast<RoomPrx>(current.adapter->createProxy(current.id));
        std::string roomName = roomPrx->getName();
        // Iterate over all users in the room and send them the message
        std::cout << users.size() << std::endl;
        for (const auto &user: users) {
            std::cout<<"sending to User: "<<user->getName()<<std::endl;
            user->receiveMessage(roomName, user, message);
        }
    }


public:
    RoomPrx proxyConnection;

    RoomI() {}

    RoomI(const std::string &name) {
        this->name = name;
    }

    bool checkAllUsersForName(const UserPrx &user) {
        for (const auto &u: users) {
            if (u != ICE_NULLPTR) {
                try {
                    if (u->getName() == user->getName()) {
                        return true;
                    }
                } catch (Ice::ConnectionRefusedException) {
                    users.erase(std::remove(users.begin(), users.end(), u), users.end());
                }
            }
        }
        return false;
    }

};

class LobbyI : public Lobby {

    Rooms rooms;
    Users users;
    std::vector<RoomFactoryPrx> roomFactories;

    void userRegister(const UserPrx &user, const ::std::string &password, const ::Ice::Current &current) {
        if (user_exists_in_json_file(user->getName())) {
            throw Chat::UserExists();
        }
        add_user_to_json_file(user->getName(), password, "");
    }

    std::string userLogin(const UserPrx &user, const ::std::string &password, const ::Ice::Current &current) {
        user->ice_fixed(current.con);
        if (!user_exists_in_json_file(user->getName())) {
            throw AccessDenied();
        }
        for (const auto &u: users) {
            if (u != ICE_NULLPTR) {
                try {
                    if (u->getName() == user->getName()) {
                        throw AccessDenied();
                    }
                } catch (Ice::ConnectionRefusedException) {
                    users.erase(std::remove(users.begin(), users.end(), u), users.end());
                }
            }
        }

        if (get_user_pwd_in_json_file(user->getName()) == password) {
            std::string token = createToken(user->getName(), password);
            replace_user_in_json_file(user->getName(), password, token);
            users.push_back(user);
            return token;
        }

        // If no user with the given name was found, throw an exception
        throw AccessDenied();
    }

    void logout(const Ice::Current &current) {
        checkLoggedIn(current);
        Ice::Context ctx = current.ctx;
        std::string username = ctx.find("username")->second;
        for (const auto &u: users) {
            if (u != ICE_NULLPTR) {
                try {
                    if (u->getName() == username) {
                        users.erase(std::remove(users.begin(), users.end(), u), users.end());
                    }
                } catch (Ice::ConnectionRefusedException) {
                    users.erase(std::remove(users.begin(), users.end(), u), users.end());
                }
            }
        }
    }


    Rooms getRooms(const ::Ice::Current &current) {

        checkLoggedIn(current);

        return rooms;
    }

    Users getUsers(const ::Ice::Current &current) {

        checkLoggedIn(current);

        return users;
    }

    RoomPrx createRoom(const ::std::string& name, const ::Ice::Current& current) {
        std::cout <<"Checking login" << std::endl;
        checkLoggedIn(current);
        std::cout <<"Logged in!" << std::endl;


        std::cout<<"Checking room existence" << std::endl;
        // Check if a room with the given name already exists
        for (const auto &room: rooms) {
            if (room->getName() == name) {
                std::cout<<"Room with the name exists!" << std::endl;
                throw RoomExists();
            }
        }

        std::cout<<"Room with the name doesnt exist!" << std::endl;
        // Create room

        std::cout<<"Creating room..." << std::endl;
        auto room = new RoomI(name);
        std::cout<<"Created room!" << std::endl;

        std::cout<<"Getting room proxy..." << std::endl;
        RoomPrx roomPrx = Ice::uncheckedCast<RoomPrx>(current.adapter->addWithUUID(room));;
        std::cout<<"Room proxy achieved!" << std::endl;
        room->proxyConnection = roomPrx;
        rooms.push_back(roomPrx);
        // Return a proxy to the new room
        return roomPrx;
    }

    RoomPrx findRoom(const ::std::string &name, const ::Ice::Current &current) {

        checkLoggedIn(current);

        // Find the room with the given name
        for (const auto &r: rooms) {
            if (r->getName() == name) {
                // Return the proxy to the found room
                return r;
            }
        }

        // If no room with the given name was found, throw an exception
        throw NoSuchRoom();
    }

    void registerRoomFactory(const Chat::RoomFactoryPrx &roomFactory, const ::Ice::Current &current) {
//    // check if the room factory is already registered
//    for (const auto& rf : roomFactories) {
//        if (rf == roomFactory) {
//            throw RoomFactoryExists();
//        }
//    }
//
//    // register the new room factory
//    roomFactories.push_back(roomFactory);
    }

    void unregisterRoomFactory(const Chat::RoomFactoryPrx &roomFactory, const ::Ice::Current &current) {
//    // check if the room factory is registered
//    auto it = std::find(roomFactories.begin(), roomFactories.end(), roomFactory);
//    if (it == roomFactories.end()) {
//        throw Chat::NoSuchRoomFactory();
//    }
//
//    // unregister the room factory
//    roomFactories.erase(it);
    }


    std::string createToken(const ::std::string &username, const ::std::string &password) {
        return "THIS_WOULD_BE_A_REAL_TOKEN_IN_A_REAL_WORLD_APPLICATION";
    }

    void checkLoggedIn(const ::Ice::Current &current) {
        const auto p = current.ctx.find("token");

        cleanUpDanglingConnections();

        if (p == current.ctx.end()) {
            throw AccessDenied();
        }
        if (p != current.ctx.end() && p->second != "THIS_WOULD_BE_A_REAL_TOKEN_IN_A_REAL_WORLD_APPLICATION") {
            throw AccessDenied();
        }
    }

    void cleanUpDanglingConnections() {
        bool found = false;
        for (const auto &u: users) {
            if (u != ICE_NULLPTR) {
                try {
                    std::cout << "User:" << u->getName() << " still exists." << std::endl;
                } catch (Ice::ConnectionRefusedException) {
                    users.erase(std::remove(users.begin(), users.end(), u), users.end());
                }
            }
        }
    }

};

class UserI : public User {
    UserStatus status;
    std::string name;


    std::string getName(const ::Ice::Current &current) {
        return name;
    }

    void setName(const std::string &username, const Ice::Current &) {
        this->name = username;
    }

    UserStatus getStatus(const ::Ice::Current &current) {
        return status;
    }


    void receivePrivateMessage(const UserPrx &fromUser, const ::std::string &message, const ::Ice::Current &current) {
        std::cout << "[" << fromUser->getName() << "]:" << std::endl << fromUser->getName() << ": " << message
                  << std::endl;
    }


    void receiveMessage(const ::std::string &fromRoom, const UserPrx &fromUser, const ::std::string &message,
                        const ::Ice::Current &current) {
        std::cout << "[ROOM]:" << fromRoom << std::endl << fromUser->getName() << ": " << message << std::endl;
    }

public:
    UserI(const std::string &name) {
        this->name = name;
    }


    UserI() {}
};
//
//class RoomFactoryI : public RoomFactory {
//    std::vector<RoomPtr> rooms;
//
//public:
//    double getServerLoad(const ::Ice::Current &current) ;
//
//public:
//    RoomPrx createRoom(const ::std::string &name, const ::Ice::Current &current) throw(RoomExists) ;
//
//};


#endif //RSO2_CHATIMPL_H
