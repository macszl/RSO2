module Chat {
    exception AccessDenied {};
    exception UserExists {};
    exception RoomExists {};
    exception NoSuchRoom {};
    exception RoomFactoryExists {};
    exception NoSuchRoomFactory {};

    enum UserStatus {Online, Offline, BeBack};

    interface Room;
    interface User;
    interface RoomFactory;

    sequence<Room*> Rooms;
    sequence<User*> Users;

    interface Lobby {
       void userRegister(User* user, string password) throws UserExists;
       string userLogin(User* user, string password) throws AccessDenied;
       void logout() throws AccessDenied;

       Rooms getRooms() throws AccessDenied;
       Users getUsers();

       Room* createRoom(string name) throws AccessDenied, RoomExists;
       Room* findRoom(string name) throws AccessDenied, NoSuchRoom;

       void registerRoomFactory(RoomFactory* roomFactory) throws RoomFactoryExists;
       void unregisterRoomFactory(RoomFactory* roomFactory) throws NoSuchRoomFactory;
    };

    interface User {
       string getName();

       UserStatus getStatus();
       void setName(string name);

       void receivePrivateMessage(User* fromUser, string message);
       void receiveMessage(string fromRoom, User* fromUser, string message);
    };

    interface Room {
      string getName();

      Users getUsers();

      void join(User* user);
      void leave(User* user);

      void sendMessage(User* fromUser, string message);
    };

    interface RoomFactory {
      double getServerLoad();

      Room* createRoom(string name) throws RoomExists;
    };
};