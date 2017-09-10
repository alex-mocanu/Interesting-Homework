#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <time.h>
#include <unordered_map>
#include <stdexcept>
using namespace std;

const int INF = 1<<30;

//Structure representing big square with user id
struct Square {
    int sq[9];
    int id;

    Square(int sq[9], int id): id(id){
        for(int i = 0; i < 9; ++i)
            this->sq[i] = sq[i];
    }

    bool operator==(const Square &s) const {
        if(id != s.id)
            return false;
        for(int i = 0; i < 9; ++i)
            if(sq[i] != s.sq[i])
                return false;
        return true;
    }
};

std::size_t myHash(const Square& k) {
  int res = k.id, p = 3;
  for(int i = 0; i < 9; ++i) {
    res += p * k.sq[i];
    p *= 3;
  }

  return res;
}

//Hash containing values for big squares
std::unordered_map<Square, int, decltype(&myHash)> squareValue(50000, myHash);

//Returns the j-th element in big square i
int cell(int i, int j){
    return 27 * (i / 3) + 9 * (j / 3) + 3 * (i % 3) + (j % 3);
}

//Structure representing a game state
typedef struct State {
    int t[9][9];
    int bigBoard[9];
    std::pair<int, int> nextMove;

    State(int t[9][9]){
        for(int i = 0; i < 9; ++i)
            for(int j = 0; j < 9; ++j)
                this->t[i][j] = t[i][j];
    }

    State(std::vector<int> field) {
        for(int i = 0; i < 9; ++i){
            for(int j = 0; j < 9; ++j)
                t[i][j] = field[cell(i, j)];
        }
    }

    int evaluateState(int my_id);
} State;

int won(int table[9]);
bool isFree(State& s, int square);

//Function that decides if we can win the match by taking square
bool decisiveMove(State &s, int square, int my_id) {
    s.bigBoard[square] = my_id;
    if(won(s.bigBoard) == my_id){
        std::vector<int> freeCells;
        for(int i = 0; i < 9; ++i)
            if(s.t[square][i] == 0)
                freeCells.push_back(i);
            
        for(int i = 0; i < freeCells.size(); ++i){
            s.t[square][freeCells[i]] = my_id;
            if(won(s.t[square]))
                return 1;
            s.t[square][freeCells[i]] = 0;
        }
    }
    s.bigBoard[square] = -1;
    return 0;
}

//Tells if you are able to win a big square
bool ableToBeWon(int v[9], int his_id) {
    //Rows and columns
    for(int i = 0; i < 3; ++i)
        if(v[3 * i] != his_id && v[3 * i + 1] != his_id && 
            v[3 * i + 2] != his_id)
            return true;
        else if(v[i] != his_id && v[i + 3] != his_id && v[i + 6] != his_id)
            return true;
    //Diagonals
    if(v[0] != his_id && v[4] != his_id && v[8] != his_id)
        return true;
    if(v[2] != his_id && v[4] != his_id && v[6] != his_id)
        return true;
    return false;
}

//Function that counts the rows, columns and diagonals that are about to be won
int winnable(int v[9], int my_id) {
    int res = 0;
    int his_id = (my_id == 1 ? 2 : 1);
    //Rows
    for(int i = 0; i < 3; ++i){
        int aux1 = 0, aux2 = 0;
        for(int j = 0; j < 3; ++j)
            if(v[3 * i + j] == my_id)
                ++aux1;
            else if(v[3 * i + j] == his_id)
                ++aux2;
        if(aux1 == 2 && aux2 == 0)
            ++res;
        else if(aux2 == 2 && aux1 == 0)
            --res;
    }
    //Columns
    for(int i = 0; i < 3; ++i){
        int aux1 = 0, aux2 = 0;
        for(int j = 0; j < 3; j++)
            if(v[3 * j + i] == my_id)
                ++aux1;
            else if(v[3 * j + i] == his_id)
                ++aux2;
        if(aux1 == 2 && aux2 == 0)
            ++res;
        else if(aux2 == 2 && aux1 == 0)
            --res;
    }
    //Diagonals
    int aux1 = (v[0] == my_id ? 1 : 0) + (v[4] == my_id ? 1 : 0) + 
                (v[8] == my_id ? 1 : 0);
    int aux2 = (v[0] == his_id ? 1 : 0) + (v[4] == his_id ? 1 : 0) + 
                (v[8] == his_id ? 1 : 0);
    if(aux1 == 2 && aux2 == 0)
        ++res;
    else if(aux2 == 2 && aux1 == 0)
        --res;
    aux1 = (v[2] == my_id ? 1 : 0) + (v[4] == my_id ? 1 : 0) + 
            (v[6] == my_id ? 1 : 0);
    aux2 = (v[2] == his_id ? 1 : 0) + (v[4] == his_id ? 1 : 0) + 
            (v[6] == his_id ? 1 : 0);
    if(aux1 == 2 && aux2 == 0)
        ++res;
    else if(aux2 == 2 && aux1 == 0)
        --res;
    return res;
}

/*Function that counts the rows, columns and diagonals that could be won on 
the big board*/
int winnableBig(State &s, int my_id) {
    int res = 0;
    int his_id = (my_id == 1 ? 2 : 1);
    bool toBeWon[9] = {0}, cellValue[9] = {0};
    //See what big squares can be won
    for(int i = 0; i < 9; ++i)
        if(s.bigBoard[i] == 0 || s.bigBoard[i] == -1){
            toBeWon[i] = ableToBeWon(s.t[i], his_id);
            if(toBeWon[i])
                cellValue[i] = winnable(s.t[i], my_id);
        }

    //Rows
    for(int i = 0; i < 3; ++i){
        int aux1 = 0, aux2 = 0, valToBeWon = 0;
        for(int j = 0; j < 3; ++j)
            if(s.bigBoard[3 * i + j] == my_id)
                ++aux1;
            else if(toBeWon[3 * i + j]){
                ++aux2;
                valToBeWon += cellValue[3 * i + j];
            }
        if(aux1 == 2 && aux2 == 1)
            res += 3 * (1 + valToBeWon);
        else if(aux1 == 1 && aux2 == 2)
            res += 2 * (1 + valToBeWon);
        else if(aux2 == 3)
            res += 1 + valToBeWon;
    }
    //Columns
    for(int i = 0; i < 3; ++i){
        int aux1 = 0, aux2 = 0, valToBeWon = 0;
        for(int j = 0; j < 3; ++j)
            if(s.bigBoard[3 * j + i] == my_id)
                ++aux1;
            else if(toBeWon[3 * j + i]){
                ++aux2;
                valToBeWon += cellValue[3 * j + i];
            }
        if(aux1 == 2 && aux2 == 1)
            res += 3 * (1 + valToBeWon);
        else if(aux1 == 1 && aux2 == 2)
            res += 2 * (1 + valToBeWon);
        else if(aux2 == 3)
            res += 1 + valToBeWon;
    }
    //Diagonals
    int aux1 = (s.bigBoard[0] == my_id ? 1 : 0) + 
                (s.bigBoard[4] == my_id ? 1 : 0) + 
                (s.bigBoard[8] == my_id ? 1 : 0);
    int aux2 = 0, valToBeWon = 0;
    if(toBeWon[0]){
        ++aux2;
        valToBeWon += cellValue[0];
    }
    if(toBeWon[4]){
        ++aux2;
        valToBeWon += cellValue[4];
    } 
    if(toBeWon[8]){
        ++aux2;
        valToBeWon += cellValue[8];
    }
    if(aux1 == 2 && aux2 == 1)
        res += 3 * (1 + valToBeWon);
    else if(aux1 == 1 && aux2 == 2)
        res += 2 * (1 + valToBeWon);
    else if(aux2 == 3)
        res += 1 + valToBeWon;

    valToBeWon = 0;
    aux1 = (s.bigBoard[2] == my_id ? 1 : 0) + 
                (s.bigBoard[4] == my_id ? 1 : 0) + 
                (s.bigBoard[6] == my_id ? 1 : 0);
    aux2 = 0;
    if(toBeWon[2]){
        ++aux2;
        valToBeWon += cellValue[2];
    }
    if(toBeWon[4]){
        ++aux2;
        valToBeWon += cellValue[4];
    } 
    if(toBeWon[6]){
        ++aux2;
        valToBeWon += cellValue[6];
    }
    if(aux1 == 2 && aux2 == 1)
        res += 3 * (1 + valToBeWon);
    else if(aux1 == 1 && aux2 == 2)
        res += 2 * (1 + valToBeWon);
    else if(aux2 == 3)
        res += 1 + valToBeWon;
    return res;
}

//Gives a score to the state of the board
int State::evaluateState(int my_id) {
    int his_id = my_id == 1 ? 2 : 1;
    int priority = 0;
    //Make a list of the -1 big squares
    std::vector<int> availableSquares;
    for(int i = 0; i < 9; ++i)
        if(bigBoard[i] == -1)
            availableSquares.push_back(i);

    //Check if I can make decisive move
    for(int i = 0; i < availableSquares.size(); ++i)
        if(decisiveMove(*this, availableSquares[i], my_id))
            return INF;

    priority += (winnableBig(*this, my_id) - winnableBig(*this, his_id)) * 
                1000;
    for(int i = 0; i < 9; ++i)
        if((bigBoard[i] == 0 && isFree(*this, i)) || bigBoard[i] == -1) {
            int mine = 0, his = 0;
            for(int j = 0; j < 9; ++j){
                if(t[i][j] == my_id)
                    ++mine;
                if(t[i][j] == his_id)
                    ++his;
            }
            if(mine > his)
                priority += 10;
            else
                priority -= 10;
            try{
                priority += squareValue.at(Square(t[i], my_id)) * 100;
            }
            catch(const std::out_of_range& e){
                squareValue[Square(t[i], my_id)] = winnable(t[i], my_id);
                priority += squareValue.at(Square(t[i], my_id)) * 100;
            }
        }

    return priority;
}

std::vector<std::string> &split(const std::string &s, char delim, 
                                std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    elems.clear();
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


int stringToInt(const std::string &s) {
    std::istringstream ss(s);
    int result;
    ss >> result;
    return result;
}

//Function for deciding if a square was won by either player
int won(int table[9]) {
    //Check the rows
    for(int i = 0; i < 3; ++i)
        if(table[3 * i] == 1 && table[3 * i + 1] == 1 && table[3 * i + 2] == 1)
            return 1;
        else if(table[3 * i] == 2 && table[3 * i + 1] == 2 && 
                table[3 * i + 2] == 2)
            return 2;
    //Check the columns
    for(int i = 0; i < 3; ++i)
        if(table[i] == 1 && table[i + 3] == 1 && table[i + 6] == 1)
            return 1;
        else if(table[i] == 2 && table[i + 3] == 2 && table[i + 6] == 2)
            return 2;
    //Check the diagonals
    if(table[0] == 1 && table[4] == 1 && table[8] == 1)
        return 1;
    if(table[0] == 2 && table[4] == 2 && table[8] == 2)
        return 2;
    if(table[2] == 1 && table[4] == 1 && table[6] == 1)
        return 1;
    if(table[2] == 2 && table[4] == 2 && table[6] == 2)
        return 2;
    return 0;
}

//Function for returning free cells in a state
std::vector<std::pair<int, int> > cells(State& s) {
    std::vector<std::pair<int, int> > res;
    for(int i = 0; i < 9; ++i)
        if(s.bigBoard[i] == -1)
            for(int j = 0; j < 9; ++j)
                if(s.t[i][j] == 0)
                    res.push_back(std::make_pair(i, j));
    return res;
}

//Function that tells if a 0 big cell is accesible
bool isFree(State& s, int square) {
    bool res = 0;
    for(int i = 0; i < 9; ++i)
        if(s.t[square][i] == 0) {
            res = 1;
            break;
        }
    return res;
}

//Function for setting macroboard state
void setMacro(State& s, int move) {
    for(int i = 0; i < 9; ++i)
        s.bigBoard[i] = won(s.t[i]);
    if(s.bigBoard[move] == 0 && isFree(s, move))
        s.bigBoard[move] = -1;
    else
        for(int i = 0; i < 9; ++i)
            if(s.bigBoard[i] == 0 && isFree(s, i))
                s.bigBoard[i] = -1;
}

//The alpha-beta algorithm
int alphaBeta(State& state, int depth, int alpha, int beta, int my_id) {
    int result = won(state.bigBoard);
    //I win
    if(result == my_id)
        return INF;
    //The opponent wins
    if(result != my_id && result)
        return -INF;
    //Draw
    int i;
    for(i = 0; i < 9; ++i)
        if(state.bigBoard[i] == -1)
            break;
    if(i == 9)
        return 0;
    //We reached the maximum depth
    if(depth == 0)
        return state.evaluateState(my_id);

    //Analyse the possible moves from the current player's point of view
    std::vector<std::pair<int, int> > freeCells = cells(state);
    for(int i = 0; i < freeCells.size(); ++i){
        State newState(state.t);
        newState.t[freeCells[i].first][freeCells[i].second] = my_id;
        setMacro(newState, freeCells[i].second);

        int score = -alphaBeta(newState, depth - 1, -beta, -alpha, my_id ^ 3);

        //In case of any change, change the next move from this state
        if(score > alpha){
            alpha = score;
            state.nextMove = freeCells[i];
        }
        if(beta <= alpha)
            break;
    }
    return alpha;
}

/**
 * This class implements all IO operations.
 * Only one method must be realized:
 *
 *      > BotIO::action
 *
 */
class BotIO
{

public:

    /**
     * Initialize your bot here.
     */
    BotIO() {
        srand(static_cast<unsigned int>(time(0)));
        _field.resize(81);
        _macroboard.resize(9);

    }


    void loop() {
        std::string line;
        std::vector<std::string> command;
        command.reserve(256);

        while (std::getline(std::cin, line)) {
            processCommand(split(line, ' ', command));
        }
    }

private:

    /**
     * Implement this function.
     * type is always "move"
     *
     * return value must be position in x,y presentation
     *      (use std::make_pair(x, y))
     */
    std::pair<int, int> action(const std::string &type, int time) {
        return myMove();
    }

    //Returns the move to be made
    std::pair<int, int> myMove() const {
        State s(_field);
        //Initialize the macroboard and make a list of squares for the move
        std::vector<int> minusOne;
        for(int i = 0; i < _macroboard.size(); ++i){
            s.bigBoard[i] = _macroboard[i];
            if(_macroboard[i] == -1)
                minusOne.push_back(i);
        }

        //Test if you can win without alphaBeta
        if(minusOne.size() == 9)
            return std::make_pair(4, 4);
        for(int i = 0; i < minusOne.size(); ++i){
            int square = minusOne[i];
            s.bigBoard[square] = _botId;
            if(won(s.bigBoard) == _botId){
                for(int j = 0; j < 9; ++j)
                    if(s.t[square][j] == 0) {
                        s.t[square][j] = _botId;
                        if(won(s.t[square]) == _botId){
                            int pos = 27 * (square / 3) + 9 * (j / 3) + 3 * (square % 3) + (j % 3);
                            return std::make_pair(pos % 9, pos / 9);
                        }
                        s.t[square][j] = 0;
                    }
            }
            s.bigBoard[square] = -1;
        }

        std::vector<std::pair<int, int> > freeCells = cells(s);
        int occupiedBigCells = 0;
        for(int i = 0; i < 9; ++i)
            if(s.bigBoard[i] == 1 || s.bigBoard[i] == 2)
                ++occupiedBigCells;
            else if(s.bigBoard[i] == 0 && won(s.t[i]))
                ++occupiedBigCells;

        int totalFreeCells = 0;
        for(int i = 0; i < 9; ++i)
            if(s.bigBoard[i] == -1 || (s.bigBoard[i] == 0 && !won(s.t[i])))
                for(int j = 0; j < 9; ++j)
                    if(s.t[i][j] == 0)
                        ++totalFreeCells;

        int res;
        //Apply alpha-beta with a variable depth
        if(occupiedBigCells < 2 || (occupiedBigCells >=4 && occupiedBigCells < 6 && freeCells.size() <= 9))
            res = alphaBeta(s, 8, -INF, INF, _botId);
        else if(occupiedBigCells >= 5 && totalFreeCells < 25)
            res = alphaBeta(s, 11, -INF, INF, _botId);
        else if(occupiedBigCells >= 5 && freeCells.size() < 5)
            res = alphaBeta(s, 10, -INF, INF, _botId);
        else if(occupiedBigCells >= 6)
            res = alphaBeta(s, 9, -INF, INF, _botId);
        else
            res = alphaBeta(s, 7, -INF, INF, _botId);
        if(res == -INF){
            int i;
            for(i = 0; i < freeCells.size(); ++i)
                if(s.bigBoard[freeCells[i].second] == 0 || s.bigBoard[freeCells[i].second] == -1){
                    s.nextMove = freeCells[i];
                    break;
                }
            if(i == freeCells.size())
                s.nextMove = freeCells[0];
        }
        int pos = 27 * (s.nextMove.first / 3) + 9 * (s.nextMove.second / 3) + 3 * (s.nextMove.first % 3) + (s.nextMove.second % 3);
        return std::make_pair(pos % 9, pos / 9);
    }

    void processCommand(const std::vector<std::string> &command) {
        if (command[0] == "action") {
            auto point = action(command[1], stringToInt(command[2]));
            std::cout << "place_move " << point.first << " " << point.second << std::endl << std::flush;
        }
        else if (command[0] == "update") {
            update(command[1], command[2], command[3]);
        }
        else if (command[0] == "settings") {
            setting(command[1], command[2]);
        }
        else {
            debug("Unknown command <" + command[0] + ">.");
        }
    }

    void update(const std::string& player, const std::string& type, const std::string& value) {
        if (player != "game" && player != _myName) {
            // It's not my update!
            return;
        }

        if (type == "round") {
            _round = stringToInt(value);
        }
        else if (type == "move") {
            _move = stringToInt(value);
        }
        else if (type == "macroboard" || type == "field") {
            std::vector<std::string> rawValues;
            split(value, ',', rawValues);
            std::vector<int>::iterator choice = (type == "field" ? _field.begin() : _macroboard.begin());
            std::transform(rawValues.begin(), rawValues.end(), choice, stringToInt);
        }
        else {
            debug("Unknown update <" + type + ">.");
        }
    }

    void setting(const std::string& type, const std::string& value) {
        if (type == "timebank") {
            _timebank = stringToInt(value);
        }
        else if (type == "time_per_move") {
            _timePerMove = stringToInt(value);
        }
        else if (type == "player_names") {
            split(value, ',', _playerNames);
        }
        else if (type == "your_bot") {
            _myName = value;
        }
        else if (type == "your_botid") {
            _botId = stringToInt(value);
        }
        else {
            debug("Unknown setting <" + type + ">.");
        }
    }

    void debug(const std::string &s) const{
        std::cerr << s << std::endl << std::flush;
    }

private:
    // static settings
    int _timebank;
    int _timePerMove;
    int _botId;
    std::vector<std::string> _playerNames;
    std::string _myName;

    // dynamic settings
    int _round;
    int _move;
    std::vector<int> _macroboard;
    std::vector<int> _field;
};

/**
 * don't change this code.
 * See BotIO::action method.
 **/
int main() {
    BotIO bot;
    bot.loop();
    return 0;
}
