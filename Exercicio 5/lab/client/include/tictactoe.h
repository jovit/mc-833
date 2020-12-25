#pragma once
#include <string>
#include <iostream>

using namespace std;

namespace TicTacToe {

    enum class State { AWAITING_ID, CONFIRMING_ID, FETCHING_USER_LIST, PLAY_REQUESTED };

    class Game {       
    private:            
        State state {State::AWAITING_ID}; 
        string id {""};
    public:
        void input_id();
        string handle_stdin(string i);
        string handle_server_response(string i);
    };
}
