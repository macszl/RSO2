.PHONY: all clean

all: lobby user

chat.cpp chat.h:
	slice2cpp chat.ice

lobby: lobby.cpp chat.cpp chatImplementations.h chat_db_helper.h chat.h
	c++ -std=c++11 -I. $< chat.cpp -o $@ -lpthread -lIce
	chmod +x $@

user: user.cpp chat.cpp chatImplementations.h chat_db_helper.h chat.h
	c++ -std=c++11 -I. $< chat.cpp -o $@ -lpthread -lIce
	chmod +x $@

clean:
	-rm lobby user lobby.o user.o