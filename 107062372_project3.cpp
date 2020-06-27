#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <map>
#include <algorithm>
#define POS_INF 2147483647
#define NEG_INF -2147483647
#define SIZE 8
using namespace std;

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(Point& rhs){
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(Point& rhs){
		return !operator==(rhs);
	}
	Point operator+(Point& rhs){
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(Point& rhs){
		return Point(x - rhs.x, y - rhs.y);
	}
};

array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
}};

int curPlayer;
bool done;
int winner;

//array<int, 3> disc_count;
array<array<int, SIZE>, SIZE> curBoard;
vector<Point> next_valid_spots;

int get_disc(Point p, array<array<int, SIZE>, SIZE> board){
    return board[p.x][p.y];
}
bool is_spot_on_board(Point p){
    return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
}
bool is_disc_at(Point p, array<array<int, SIZE>, SIZE> board, int disc){
    if (!is_spot_on_board(p))
        return false;
    if (get_disc(p, board) != disc)
        return false;
    return true;
}
int get_next_player(int player){
    return 3 - player;
}
bool is_spot_valid(Point center, array<array<int, SIZE>, SIZE> board, int player){
    if (get_disc(center, board) != 0)
        return false;
    for (Point dir: directions) {
        // Move along the direction while testing.
        Point p = center + dir;
        if (!is_disc_at(p, board, get_next_player(player)))
            continue;
        p = p + dir;
        while (is_spot_on_board(p) && get_disc(p, board) != 0) {
            if (is_disc_at(p, board, player))
                return true;
            p = p + dir;
        }
    }
    return false;
}
vector<Point> get_valid_spots(array<array<int, SIZE>, SIZE> board, int player){
    vector<Point> valid_spots;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            Point p = Point(i, j);
            if (board[i][j] != 0)
                continue;
            if (is_spot_valid(p, board, player))
                valid_spots.push_back(p);
        }
    }
    return valid_spots;
}
array<array<int, SIZE>, SIZE> copyBoard(array<array<int, SIZE>, SIZE> board)
{
    array<array<int, SIZE>, SIZE> newBoard;
    for (int i=0; i<SIZE; i++){
        for (int j=0; j<SIZE; j++)
            newBoard[i][j] = board[i][j];
    }
    return newBoard;
}
void flip_discs(Point center, array<array<int, SIZE>, SIZE> &board, int player) {
    for (Point dir: directions) {
        // Move along the direction while testing.
        Point p = center + dir;
        if (!is_disc_at(p, board, get_next_player(player)))
            continue;
        vector<Point> discs({p});
        p = p + dir;
        while (is_spot_on_board(p) && get_disc(p, board) != 0) { // get_disc(p) != 0 => get_disc(p) != EMPTY
            if (is_disc_at(p, board, player)) {
                for (Point s: discs) {
                    board[s.x][s.y] = player;
                }
                break;
            }
            discs.push_back(p);
            p = p + dir;
        }
    }
}
void put_disc(Point p, array<array<int, SIZE>, SIZE> &board, int disc) {
    board[p.x][p.y] = disc;
    flip_discs(p, board, disc);
    return;
}

int set_heuristic(array<array<int, SIZE>, SIZE> board, int player) {
    int my_discs = 0, opp_discs = 0, my_front_discs = 0, opp_front_discs = 0;
    int p = 0, c = 0, l = 0, m = 0, f = 0, d = 0;

    int X1[] = {-1, -1, 0, 1, 1, 1, 0, -1};
	int Y1[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int V[8][8] = {
        {20, -3, 11, 8, 8, 11, -3, 20},
        {-3, -7, -4, 1, 1, -4, -7, -3},
        {11, -4, 2, 2, 2, 2, -4, 11}, 
        {8, 1, 2, -3, -3, 2, 1, 8}, 
        {8, 1, 2, -3, -3, 2, 1, 8}, 
        {11, -4, 2, 2, 2, 2, -4, 11}, 
        {-3, -7, -4, 1, 1, -4, -7, -3},
        {20, -3, 11, 8, 8, 11, -3, 20}
    };

    // Piece difference, frontier disks and disk squares
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            if (board[i][j] == player){
                d += V[i][j];
                my_discs++;
            }else if (board[i][j] == get_next_player(player)){
                d -= V[i][j];
                opp_discs++;
            }
            if (board[i][j] != 0){
                for (int k=0; k<8; k++){
                    int x = i + X1[k];
                    int y = j + Y1[k];
                    if (x >= 0 && x < 8 && y >= 0 && y < 8 && board[i][j] == 0){
                        if (board[i][j] == player) my_front_discs++;
                        else opp_front_discs++;
                        break;
                    }
                }
            }
        }
    }
    p = my_discs - opp_discs;
    f = my_front_discs - opp_front_discs;

    // Corner occupancy 
    my_discs = opp_discs = 0;
    if (board[0][0] == player) my_discs++;
    else if (board[0][0] == get_next_player(player)) opp_discs++;
    if (board[0][7] == player) my_discs++;
    else if (board[0][7] == get_next_player(player)) opp_discs++;
    if (board[7][0] == player) my_discs++;
    else if (board[7][0] == get_next_player(player)) opp_discs++;
    if (board[7][7] == player) my_discs++;
    else if (board[7][7] == get_next_player(player)) opp_discs++;
    c = 25*(my_discs - opp_discs);

    // Corner closeness
    my_discs = opp_discs = 0;
    if (board[0][0] == 0){
        if (board[0][1] == player) my_discs++;
        else if (board[0][1] == get_next_player(player)) opp_discs++;
        if (board[1][1] == player) my_discs++;
        else if (board[1][1] == get_next_player(player)) opp_discs++;
        if (board[1][0] == player) my_discs++;
        else if (board[1][0] == get_next_player(player)) opp_discs++;
    }
    if (board[0][7] == 0){
        if (board[0][6] == player) my_discs++;
        else if (board[0][6] == get_next_player(player)) opp_discs++;
        if (board[1][6] == player) my_discs++;
        else if (board[1][6] == get_next_player(player)) opp_discs++;
        if (board[1][7] == player) my_discs++;
        else if (board[1][7] == get_next_player(player)) opp_discs++;
    }
    if (board[7][0] == 0){
        if (board[7][1] == player) my_discs++;
        else if (board[7][1] == get_next_player(player)) opp_discs++;
        if (board[6][1] == player) my_discs++;
        else if (board[6][1] == get_next_player(player)) opp_discs++;
        if (board[6][0] == player) my_discs++;
        else if (board[6][0] == get_next_player(player)) opp_discs++;
    }
    if (board[7][7] == 0){
        if (board[6][7] == player) my_discs++;
        else if (board[6][7] == get_next_player(player)) opp_discs++;
        if (board[6][6] == player) my_discs++;
        else if (board[6][6] == get_next_player(player)) opp_discs++;
        if (board[7][6] == player) my_discs++;
        else if (board[7][6] == get_next_player(player)) opp_discs++;
    }
    l = (-13) * (my_discs - opp_discs);
    // Mobility
    vector<Point> my_discs_vec = get_valid_spots(board, player);
    vector<Point> opp_discs_vec = get_valid_spots(board, get_next_player(player));
    m = my_discs_vec.size() - opp_discs_vec.size();

    // final weighted score
    int score = (10 * p) + (1000* c) + (500 * l) + (80 * m) + (75 * f) + (10 * d);
    score = score < 0 ? 0 : score;
	return score;
}

int minimax(Point p, array<array<int, SIZE>, SIZE> board, int player, int depth, bool isMaximizingPlayer, int alpha, int beta){
    if (depth == 5){
        int h = set_heuristic(board, player);
        return h;
    }
    if (isMaximizingPlayer){
        int bestValue = NEG_INF;
        vector<Point> valid_spots = get_valid_spots(board, player);
        for(Point c : valid_spots){
            array<array<int, SIZE>, SIZE> newBoard = copyBoard(board);
            put_disc(c, newBoard, player);

            int value = minimax(p, newBoard, player, depth+1, false, alpha, beta);
            bestValue = max(bestValue, value);
            alpha = max(alpha, bestValue);
            if (beta <= alpha) 
                break;
        } 
        return bestValue;
    }
    else{
        int bestValue = POS_INF;
        vector<Point> valid_spots = get_valid_spots(board, player);
        for(Point c : valid_spots){
            array<array<int, SIZE>, SIZE> newBoard = copyBoard(board);
            put_disc(c, newBoard, player);

            int value = minimax(p, newBoard, player, depth+1, true, alpha, beta);
            bestValue = min(bestValue, value);
            beta = min(beta, bestValue);
            if (beta <= alpha) 
                break;
        } 
        return bestValue;
    }
}

void read_board(ifstream& fin) {
    fin >> curPlayer;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> curBoard[i][j];
        }
    }
}

void read_valid_spots(ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(ofstream& fout) {
    int depth = 0;
    bool isMaximizingPlayer = true;
    int alpha = NEG_INF, beta = POS_INF, maxVal = NEG_INF;
    Point maxP;
    // depth, nodeIndex, maximizingPlayer, next_valid_spots, alpha, beta
    for (auto c : next_valid_spots){
        int h = minimax(c, curBoard, curPlayer, depth, isMaximizingPlayer, alpha, beta); 
        if (h >= maxVal){
            maxVal = h;
            maxP.x = c.x;
            maxP.y = c.y;
        }
    }
    // Remember to flush the output to ensure the last action is written to file.
    fout << maxP.x << " " << maxP.y << endl;
    fout.flush();
}

int main(int, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(0);

    ifstream fin(argv[1]);
    ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}