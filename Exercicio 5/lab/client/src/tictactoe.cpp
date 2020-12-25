#include "tictactoe.h"

void TicTacToe::Game::input_id()
{
    cout << "Input your desired ID" << endl;
    this->state = State::AWAITING_ID;
    return;
}

string TicTacToe::Game::handle_stdin(string i)
{
    if (i.empty())
    {
        return "";
    }

    switch (this->state)
    {
    case State::AWAITING_ID:
        cout << "Is this the id you wish to use? \"" << i << "\" (Y/N)" << endl;
        this->state = State::CONFIRMING_ID;
        this->id = i;
        break;
    case State::CONFIRMING_ID:
        if (i == "Y")
        {
            cout << "Confirming selected id" << endl;
            return "select_id " + this->id;
        }
        else
        {
            this->input_id();
        }
        break;
    case State::FETCHING_USER_LIST:
        if (i == "refresh")
        {
            return "user_list";
        } else if (i.find("play") != string::npos) {
            cout << "Requesting to play with user, please wait" << endl;
            this->state = State::PLAY_REQUESTED;
            return i;
        }
        break;
    default:
        return "";
    }

    return "";
}

string TicTacToe::Game::handle_server_response(string i)
{
    if (i.empty())
    {
        return "";
    }

    switch (this->state)
    {
    case State::CONFIRMING_ID:
        if (i == "ok")
        {
            cout << "Confirmed id \"" << this->id << "\"" << endl;
            cout << "Fetching currently online users" << endl;
            this->state = State::FETCHING_USER_LIST;
            return "user_list";
        }
        else
        {
            cout << "The selected id is already taken, please choose another" << endl;
            this->input_id();
        }
        break;
    case State::FETCHING_USER_LIST:
        if (i == "failed")
        {
            cout << "There are no available players, type 'refresh' to fetch the list again" << endl;
        }
        else
        {
            cout << "These are the available players: " << i << endl;
            cout << "Input 'refresh' to fetch the list again" << endl;
            cout << "Input 'play <id>' to request to play with this user" << endl;
        }
        break;
    case State::PLAY_REQUESTED:
        if (i == "failed") {
            cout << "The requested player is unavailable, please try again." << endl;
            this->state = State::FETCHING_USER_LIST;
            return "user_list";
        } else if (i == "success") {
           cout << "Request to play sent" << endl; 
           return "";
        }
    default:
        return "";
    }

    return "";
}