.PHONY:all
all:ser cli
ser:Epoll.cpp Epoll.h Common.h DealMessage.h DealMessage.cpp
	g++ -g -o $@ $^ -std=c++11
cli:client.c
	g++ -o $@ $^
.PHONY:clean
clean:
	rm -rf ser cli
