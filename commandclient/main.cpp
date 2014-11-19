#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <sstream>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <string>
#include <vector>

using namespace std;

string host = "cpssd4-web.computing.dcu.ie:80/";

char boardChar(int x, char def)
{
    if (x == 2) {
        return 'O';
    } else if (x == 1) {
        return 'X';
    }
    return def;
}

char toChar(int x)
{
    return '0' + x;
}

void printBoard(int b[9], bool forSelection)
{
    cout << "Current board : " << endl;
    for (int i = 0; i < 3; i++) {
        cout << "-------" << endl;
        if (forSelection == false) {
            cout << "|" << boardChar(b[i*3], ' ') << "|" << boardChar(b[i*3 + 1], ' ') << "|" << boardChar(b[i*3 + 2], ' ') << "|" << endl;
        } else {
            cout << "|" << boardChar(b[i*3], toChar(i*3 + 1)) << "|" << boardChar(b[i*3 + 1], toChar(i*3 + 2)) << "|" << boardChar(b[i*3 + 2], toChar(i*3 + 3)) << "|" << endl;
        }
    }
    cout << "-------" << endl;
}

int getWinner(int b[9])
{
    //rows and coloums
    for (int i = 0; i < 3; i++) {
        if (b[i*3] == b[i*3 + 1] && b[i*3] == b[i*3 + 2] && b[i*3] > 0) {
            return b[i*3];
        }
        if (b[i] == b[i + 3] && b[i] == b[i + 6] && b[i] > 0) {
            return b[i];
        }
    }
    //diagnals
    if (b[0] == b[4] && b[0] == b[8] && b[0] > 0) {
        return b[0];
    }
    if (b[2] == b[4] && b[2] == b[6] && b[2] > 0) {
        return b[2];
    }
    //is the game over?
    for (int i = 0; i < 9; i++) {
        if (b[i] == 0) {
            return -1;
        }
    }
    //game is a draw
    return 0;
}

bool validMove(int board[9], int pos)
{
    if (pos >= 0) {
        if (pos < 9) {
            if (board[pos] == 0) {
                return true;
            }
        }
    }
    return false;
}

struct generalRequest
{
    string status;
    int code;
    string message;
    string id;
    string secret;
    int letter;
    int board[9];
    int turn;
    int winner;
};

void getBoard(int b[9], boost::property_tree::ptree pt)
{
    if (pt.count("board") == 0) {
        for (int i = 0; i < 9; i++) {
            b[i] = 0;
        }
        return;
    }
    vector<int> r;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("board")) {
        r.push_back(v.second.get_value<int>());
    }
    for (int i = 0; i < 9; i++) {
        if (i < r.size()) {
            b[i] = r[i];
        } else {
            b[i] = 0;
        }
    }
}

generalRequest processJson(string x)
{
    stringstream ss;
    ss << x;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    generalRequest ret;
    ret.status = pt.get("status", "");
    ret.code = pt.get("code", 0);
    ret.message = pt.get("message", "");
    ret.id = pt.get("id", "");
    ret.secret = pt.get("secret", "");
    ret.letter = pt.get("letter", 0);
    getBoard(ret.board, pt);
    ret.turn = pt.get("turn", 0);
    ret.winner = pt.get("winner", -1);
    return ret;
}

CURL* curl = curl_easy_init();

size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* data)
{
	((std::string*)data)->append((char*)ptr, size * nmemb);
	return size * nmemb;
}

string getData(string request)
{
    string requestAddress = host + request;
    string replyJSON;
	curl_easy_setopt(curl, CURLOPT_URL, requestAddress.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &replyJSON);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
	curl_easy_perform(curl);
	curl_easy_reset(curl);
    return replyJSON;
}

void handleError(generalRequest x)
{
    cout << "status : " << x.status << endl;
    if (x.code > 0) {
        cout << "code : " << x.code << endl;
    }
    if (x.message != "") {
        cout << "message : " << x.message << endl;
    }
}

char getXOFromInt(int x)
{
    if (x == 1) {
        return 'X';
    } else {
        return 'O';
    }
}

void handleWinner(int player, int winner)
{
    if (winner == 0) {
        cout << "The game has ended in a draw" << endl;
    } else if (winner == player) {
        cout << "You have won the game, congratulations!" << endl;
    } else {
        cout << "You have lost the game, better luck next time" << endl;
    }
}

int getOppsite(int x)
{
    if (x == 1) {
        return 2;
    }
    return 1;
}

bool makeMove(int board[9], string secret)
{
    cout << "It is your turn, select a move to make or enter 10 to end the game : " << endl;
    printBoard(board, true);
    int m;
    while (true) {
        cin >> m;
        if (validMove(board, m - 1)) {
            board[m - 1] = player;
            serverResponse = getData("/move?secret=" + secret + "&position=" + toChar(m - 1));
            response = processJson(serverResponse);
                if (response.status != "okay") {
                    handleError(response);
                }
            printBoard(board, false);
            cout << "Waiting for other player to move" << endl;
            return true;
        } else if (m == 10) {
            serverResponse = getData("/endGame?secret=" + secret);
            response = processJson(serverResponse);
            if (response.status != "okay") {
                handleError(response);
            }
            return false;
        } else {
            cout << "Invalid Move" << endl;
        }
    }
}

int tryNext(int board[9], string secret)
{
    serverResponse = getData("/next?secret=" + secret);
    response = processJson(serverResponse);
    if (response.status == "okay") {
        if (response.winner >= 0) {
            handleWinner(player, response.winner);
            return 3;
        } else {
            if (response.turn == player) {
                for (int i = 0; i < 9; i++) {
                    board[i] = response.board[i];
                }
                return 2;
            } else {
                boost::this_thread::sleep(boost::posix_time::seconds(1));
                return 1;
            }
        }
    } else {
        handleError(response);
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        return 1;
    }
}

void runGame(int board[9], int wMove, string secret)
{
    while (true) {
            if (getWinner(board) >= 0) {
                handleWinner(player, getWinner(board));
                break;
            } else {
                if (wMove == player) {
                    bool moveMade = makeMove(board, secret);
                    if (moveMade == false) {
                        return;
                    }
                    wMove = getOppsite(wMove);
                } else {
                    int meNext = tryNext(board, secret);
                    if (meNext == 2) {
                        wMove = getOppsite(wMove);
                    } else if (meNext == 3) {
                        return;
                    }
                }
            }
        }
}

void startGame(string name)
{
    string serverResponse = getData("/startGame?name=" + name);
    generalRequest response = processJson(serverResponse);
    if (response.status == "okay") {
        string secret = response.secret;
        int player = response.letter;
        cout << "Started game with id " << id << ". You are playing as " << getXOFromInt(response.letter) << endl;
        int wMove = 1;
        int board[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        runGame(board, wMove, secret);
    } else {
        handleError(response);
    }
}

void listGames(string name)
{
    string serverResponse = getData("/listGames");
}

void playGames()
{
    string name;
    cout << "Please enter your name : " << endl;
    cin >> name;
    char anwser;
    while (true) {
        cout << "Would you like to start a new game or list all available games? Enter 1 or 2. Enter 3 to exit." << endl;
        while (true) {
            cin >> anwser;
            if (anwser == '1') {
                startGame(name);
                break;
            } else if (anwser == '2') {
                listGames(name);
                break;
            } else if (anwser == '3') {
                return;
            }
            cout << "invalid response" << endl;
        }
    }
}

int main()
{
    cout << "Would you like to use the defualt host (" << host << ") or enter an alternitive? y/n" << endl;
    char a;
    cin >> a;
    if (a == 'n') {
    	cout << "Enter host name :" << endl;
    	cin >> host;
    }
    playGames();
    return 0;
}
