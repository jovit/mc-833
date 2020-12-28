#pragma once
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <future>
#include <thread>
#include <chrono>
#include "poll.h"
#include <time.h>
#include <sstream>

#include "SocketUtils.h"
using namespace std;

namespace TicTacToe {
    enum class State { AWAITING_ID, CONFIRMING_ID, FETCHING_USER_LIST, PLAY_REQUESTED, ACCEPTING_PLAY, INITIALIZING_CONNECTION, PLAYING, SAVING_SCORE };
    enum class GameEndState { WON, LOST, DRAW, NONE };

    class Game {       
    private:
        string board[3][3];
        bool my_turn {false};
        State state {State::AWAITING_ID}; 
        string id {""};
        int sockfd;
        struct sockaddr_in opponent_addr;
        void init_opponent_addr(const char *ip, int port);
        void send_message_to_opponent(string to_send);
        int start_game(); // returns the earned score of the match
        void show_board();
        GameEndState check_endgame();
    public:
        void init_udp_socket();
        void input_id();
        string handle_stdin(string i);
        string handle_server_response(string i);
    };
}
