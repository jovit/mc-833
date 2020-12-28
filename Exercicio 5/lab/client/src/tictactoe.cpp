#include "tictactoe.h"

#define MAXLINE 4096

string get_local_socket_port(int fd)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    char address_name[128];

    SocketUtils::Getsockname(fd, (struct sockaddr *)&addr, &len);
    SocketUtils::Inet_ntop(AF_INET, &(addr.sin_addr), address_name, sizeof(address_name));

    stringstream ss;
    ss << ntohs(addr.sin_port);

    return ss.str();
}

void TicTacToe::Game::send_message_to_opponent(string to_send)
{
    SocketUtils::Sendto(this->sockfd, to_send.c_str(), to_send.length(), 0, (struct sockaddr *)&(this->opponent_addr), sizeof(this->opponent_addr));
}

void TicTacToe::Game::init_udp_socket()
{
    struct sockaddr_in addr;
    int sockfd = SocketUtils::Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET; // use IPv4
    addr.sin_port = 0;         // allow system to choose port

    SocketUtils::Bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    this->sockfd = sockfd;
}

void TicTacToe::Game::init_opponent_addr(const char *ip, int port)
{
    struct sockaddr_in addr;

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET; // use IPv4
    addr.sin_port = htons(port);

    SocketUtils::Inet_pton(AF_INET, ip, &addr.sin_addr);

    this->opponent_addr = addr;
}

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
        }
        else if (i.find("play") != string::npos)
        {
            cout << "Requesting to play with user, please wait" << endl;
            this->state = State::PLAY_REQUESTED;
            return i;
        }
        break;
    case State::ACCEPTING_PLAY:
        if (i == "Y")
        {
            return "accept";
        }
        else if (i == "N")
        {
            this->state = State::FETCHING_USER_LIST;
            return "deny";
        }
        else
        {
            cout << "Invalid command, do you wish to play? (Y/N)" << endl;
        }
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

    if (i.find("play_request") != string::npos)
    { // play request can come at any moment
        cout << "An user has requested to play with you, do you wish to accept? (Y/N)" << endl;
        this->state = State::ACCEPTING_PLAY;
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
        if (i == "failed")
        {
            cout << "The requested player is unavailable, please try again." << endl;
            this->state = State::FETCHING_USER_LIST;
            return "user_list";
        }
        else if (i == "success")
        {
            cout << "Request to play sent" << endl;
            return "";
        }
        else if (i == "deny")
        {
            cout << "The player has denied to play, please try again." << endl;
            this->state = State::FETCHING_USER_LIST;
            return "user_list";
        }
        else if (i == "game_accepted")
        {
            cout << "The game has been accepted, initializing the connection" << endl;
            this->state = State::INITIALIZING_CONNECTION;
            return "game_port " + get_local_socket_port(this->sockfd);
        }
        break;
    case State::ACCEPTING_PLAY:
        if (i == "game_accepted")
        {
            cout << "The game has been accepted, initializing the connection" << endl;
            this->state = State::INITIALIZING_CONNECTION;
            return "game_port " + get_local_socket_port(this->sockfd);
        }
        break;
    case State::INITIALIZING_CONNECTION:
        if (i.find("game_ip") != string::npos)
        {
            i.erase(0, i.find(" ") + 1);
            string ip = string(i.substr(0, i.find(" ")));
            i.erase(0, i.find(" ") + 1);
            string port = string(i.substr(0, i.find(" ")));

            this->init_opponent_addr(ip.c_str(), atoi(port.c_str()));

            char received_line[MAXLINE];
            string to_send = "Hello\n";
            this->send_message_to_opponent("Hello");

            SocketUtils::Recvfrom(this->sockfd, received_line, MAXLINE, 0, NULL, NULL);

            cout << "Connection established! Starting up the game!" << endl;

            this->state = State::PLAYING;

            int score = this->start_game();
            this->state = State::SAVING_SCORE;
            return "score " + to_string(score);
        }
        break;
    case State::SAVING_SCORE:
        if (i.find("score") != string::npos)
        {
            i.erase(0, i.find(" ") + 1);
            string score = string(i.substr(0, i.find(" ")));

            cout << "Your current score is: " << score << endl;

            this->state = State::FETCHING_USER_LIST;
            return "user_list";
        }
        break;
    default:
        return "";
    }

    return "";
}

TicTacToe::GameEndState TicTacToe::Game::check_endgame()
{
    // check rows and columns
    for (int i = 0; i < 3; i++)
    {
        if (this->board[i][0] == this->board[i][1] && this->board[i][1] == this->board[i][2])
        {
            if (this->board[i][0] == "X")
            {
                return TicTacToe::GameEndState::WON;
            }
            else if (this->board[i][0] == "O")
            {
                return TicTacToe::GameEndState::LOST;
            }
        }
        if (this->board[0][i] == this->board[1][i] && this->board[1][i] == this->board[2][i])
        {
            if (this->board[0][i] == "X")
            {
                return TicTacToe::GameEndState::WON;
            }
            else if (this->board[i][0] == "O")
            {
                return TicTacToe::GameEndState::LOST;
            }
        }
    }

    // check the diagonals
    if (this->board[0][0] == this->board[1][1] && this->board[1][1] == this->board[2][2])
    {
        if (this->board[0][0] == "X")
        {
            return TicTacToe::GameEndState::WON;
        }
        else if (this->board[0][0] == "O")
        {
            return TicTacToe::GameEndState::LOST;
        }
    }
    if (this->board[2][0] == this->board[1][1] && this->board[1][1] == this->board[2][0])
    {
        if (this->board[2][0] == "X")
        {
            return TicTacToe::GameEndState::WON;
        }
        else if (this->board[2][0] == "O")
        {
            return TicTacToe::GameEndState::LOST;
        }
    }

    // check for full board
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (this->board[i][j] == "-")
            {
                return TicTacToe::GameEndState::NONE; // not full, stil has plays to be done
            }
        }
    }

    return TicTacToe::GameEndState::DRAW; // if not any other case, it's a draw
}

void TicTacToe::Game::show_board()
{
    cout << endl
         << endl;
    cout << "   1     2     3" << endl;
    cout << "      |     |     " << endl;
    cout << "1  " << this->board[0][0] << "  |  " << this->board[0][1] << "  |  " << this->board[0][2] << "  " << endl;
    cout << " _____|_____|_____" << endl;
    cout << "      |     |     " << endl;
    cout << "2  " << this->board[1][0] << "  |  " << this->board[1][1] << "  |  " << this->board[1][2] << "  " << endl;
    cout << " _____|_____|_____" << endl;
    cout << "      |     |     " << endl;
    cout << "3  " << this->board[2][0] << "  |  " << this->board[2][1] << "  |  " << this->board[2][2] << "  " << endl;
    cout << "      |     |     " << endl;
    cout << endl
         << endl;

    cout << "Your plays will be displayed on the board with (X) and the opponent's with (O)" << endl;
    if (this->my_turn)
    {
        cout << "To play type \"<row> <column>\"" << endl;
        cout << "Eg: \"1 1\"" << endl;
    }
    else
    {
        cout << "Waiting for him to play" << endl;
    }
}

int TicTacToe::Game::start_game()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            this->board[i][j] = "-";
        }
    }

    char received_line[MAXLINE];

    cout << "Deciding who will start first!" << endl;

    srand(time(NULL) + atoi(get_local_socket_port(this->sockfd).c_str()));
    const int lucky_number = rand() % 100;

    cout << "Your lucky number is: " << lucky_number << endl;

    this->send_message_to_opponent(to_string(lucky_number));
    SocketUtils::Recvfrom(this->sockfd, received_line, MAXLINE, 0, NULL, NULL);

    int opponent_lucky_number = atoi(received_line);

    cout << "The opponent lucky number is: " << opponent_lucky_number << endl;

    if (lucky_number > opponent_lucky_number)
    {
        cout << "You're starting first!" << endl;
        this->my_turn = true;
    }
    else
    {
        cout << "Your opponent is starting first!" << endl;
        this->my_turn = false;
    }

    this->show_board();

    while (true)
    {
        string play;
        if (!this->my_turn)
        {
            SocketUtils::Recvfrom(this->sockfd, received_line, MAXLINE, 0, NULL, NULL);
            play = string(received_line);
        }
        else
        {
            getline(cin, play);
        }

        int row = atoi(play.substr(0, play.find(" ")).c_str()) - 1;
        play.erase(0, play.find(" ") + 1);
        int column = atoi(play.substr(0, play.find(" ")).c_str()) - 1;

        if (this->board[row][column] != "-")
        {
            cout << "Invalid play!" << endl;
            continue;
        }
        else
        {
            if (this->my_turn)
            {
                this->board[row][column] = "X";
                this->send_message_to_opponent(to_string(row + 1) + " " + to_string(column + 1)); // transmit the play to the opponent
            }
            else
            {
                this->board[row][column] = "O";
            }
        }

        this->my_turn = !this->my_turn;

        TicTacToe::GameEndState game_status = this->check_endgame();
        switch (game_status)
        {
        case TicTacToe::GameEndState::NONE:
            this->show_board();
            break;
        case TicTacToe::GameEndState::WON:
            cout << "You won the game! Congratulations!" << endl;
            return 1;
            break;
        case TicTacToe::GameEndState::LOST:
            cout << "You lost :(" << endl;
            return -1;
            break;
        case TicTacToe::GameEndState::DRAW:
            cout << "It's a draw!" << endl;
            return 0;
            break;
        default:
            break;
        }
    }
}
