#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
#define SIZE 8
#define max_depth 5
#define MAX 2147483647
#define MIN -2147483647
#define C 3
struct Point {
	int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};
class OthelloBoard {
public:
	enum SPOT_STATE {
		EMPTY = 0,
		BLACK = 1,
		WHITE = 2
	};
	const std::array<Point, 8> directions{ {
		Point(-1, -1), Point(-1, 0), Point(-1, 1),
		Point(0, -1), /*{0, 0}, */Point(0, 1),
		Point(1, -1), Point(1, 0), Point(1, 1)
	} };
	std::array<std::array<int, SIZE>, SIZE> board;
	std::vector<Point> next_valid_spots;
	std::array<int, 3> disc_count;
	int cur_player;
	bool done;
	int winner;
	int value;
private:
	int get_next_player(int player) const {
		return 3 - player;
	}
	bool is_spot_on_board(Point p) const {
		return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
	}
	int get_disc(Point p) const {
		return board[p.x][p.y];
	}
	void set_disc(Point p, int disc) {
		board[p.x][p.y] = disc;
	}
	bool is_disc_at(Point p, int disc) const {
		if (!is_spot_on_board(p))
			return false;
		if (get_disc(p) != disc)
			return false;
		return true;
	}
	bool is_spot_valid(Point center) const {
		if (get_disc(center) != EMPTY)
			return false;
		for (Point dir : directions) {
			// Move along the direction while testing.
			Point p = center + dir;
			if (!is_disc_at(p, get_next_player(cur_player)))
				continue;
			p = p + dir;
			while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
				if (is_disc_at(p, cur_player))
					return true;
				p = p + dir;
			}
		}
		return false;
	}
	void flip_discs(Point center) {
		for (Point dir : directions) {
			// Move along the direction while testing.
			Point p = center + dir;
			if (!is_disc_at(p, get_next_player(cur_player)))
				continue;
			std::vector<Point> discs({ p });
			p = p + dir;
			while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
				if (is_disc_at(p, cur_player)) {
					for (Point s : discs) {
						set_disc(s, cur_player);
					}
					disc_count[cur_player] += discs.size();
					disc_count[get_next_player(cur_player)] -= discs.size();
					break;
				}
				discs.push_back(p);
				p = p + dir;
			}
		}
	}
public:
	OthelloBoard() {
		reset();
	}
	void reset() {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				board[i][j] = EMPTY;
			}
		}
		board[3][4] = board[4][3] = BLACK;
		board[3][3] = board[4][4] = WHITE;
		cur_player = BLACK;
		disc_count[EMPTY] = 8 * 8 - 4;
		disc_count[BLACK] = 2;
		disc_count[WHITE] = 2;
		next_valid_spots = get_valid_spots();
		done = false;
		winner = -1;
		value = 0;
	}
	std::vector<Point> get_valid_spots() const {
		std::vector<Point> valid_spots;
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				Point p = Point(i, j);
				if (board[i][j] != EMPTY)
					continue;
				if (is_spot_valid(p))
					valid_spots.push_back(p);
			}
		}
		return valid_spots;
	}
	bool put_disc(Point p) {
		if (!is_spot_valid(p)) {
			winner = get_next_player(cur_player);
			done = true;
			return false;
		}
		set_disc(p, cur_player);
		disc_count[cur_player]++;
		disc_count[EMPTY]--;
		flip_discs(p);
		// Give control to the other player.
		cur_player = get_next_player(cur_player);
		next_valid_spots = get_valid_spots();
		return true;
	}
};
int maxmin[max_depth];
int player;
array<array<int, SIZE>, SIZE> board;
vector<Point> next_valid_spots;
OthelloBoard curr;
const int spotvalue[SIZE][SIZE] =
{
	 90, -60,  30,  30,  30,  30, -60,  90,
	-60, -80,  -5,  -5,  -5,  -5, -80, -60,
	 30,  -5,   1,   1,   1,   1,  -5,  30,
	 30,  -5,   1,   1,   1,   1,  -5,  30,
	 30,  -5,   1,   1,   1,   1,  -5,  30,
	 30,  -5,   1,   1,   1,   1,  -5,  30,
	-60, -80,  -5,  -5,  -5,  -5, -80, -60,
	 90, -60,  30,  30,  30,  30, -60,  90
};
void read_board(std::ifstream& fin) {
	fin >> player;
	curr.cur_player = player;
	curr.disc_count[0] = curr.disc_count[1] = curr.disc_count[2] = 0;
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			fin >> board[i][j];
			curr.disc_count[board[i][j]]++;
		}
	}
}

void read_valid_spots(std::ifstream& fin) {
	int n_valid_spots;
	fin >> n_valid_spots;
	int x, y;
	for (int i = 0; i < n_valid_spots; i++) {
		fin >> x >> y;
		next_valid_spots.push_back(Point(x, y));
	}
	curr.next_valid_spots = next_valid_spots;
}
bool corner(Point a)
{
	int i = a.x;
	int j = a.y;
	if ((i == 0 && j == 0) || (i == 0 && j == SIZE - 1) || (i == SIZE - 1 && j == SIZE - 1) || (i == SIZE - 1 && j == 0))
		return true;
	return false;
}
int find(OthelloBoard move, int depth, int alpha, int beta)//return maxvalue for my next move
{
	if (depth == max_depth)
	{
		return 0;
	}
	int now_alpha = alpha;
	int now_beta = beta;
	if (move.cur_player == player)
	{
		maxmin[depth] = MIN;
		if (move.next_valid_spots.size())
		{
			for (auto now : move.next_valid_spots)
			{
				OthelloBoard nextmove = move;
				nextmove.put_disc(now);
				int value = C * spotvalue[now.x][now.y] + 4 * C * find(nextmove, depth + 1, now_alpha, now_beta) - nextmove.next_valid_spots.size();//opponent's mobility
				if (value > maxmin[depth])
					maxmin[depth] = value;
				if (now_alpha <= maxmin[depth])
					now_alpha = maxmin[depth];
				if (now_alpha >= now_beta)//prune
					break;
			}
		}
	}
	else
	{
		maxmin[depth] = MAX;
		if (move.next_valid_spots.size())
		{
			for (auto now : move.next_valid_spots)
			{
				OthelloBoard nextmove = move;
				nextmove.put_disc(now);
				int value = -C * spotvalue[now.x][now.y] + 4 * C * find(nextmove, depth + 1, now_alpha, now_beta) + nextmove.next_valid_spots.size();//my mobility
				if (value < maxmin[depth])
					maxmin[depth] = value;
				if (now_beta >= maxmin[depth])
					now_beta = maxmin[depth];
				if (now_alpha >= now_beta)//prune
					break;
			}
		}
	}
	return maxmin[depth];
}
void write_valid_spot(std::ofstream& fout) {
	maxmin[0] = MIN;
	Point p = curr.next_valid_spots[0];
	if (curr.disc_count[0] > 1)//more than 1 blank space
	{
		int alpha = MIN;
		int beta = MAX;
		for (auto now : curr.next_valid_spots)
		{
			if (corner(now))
			{
				p = now;
				break;
			}
			OthelloBoard put;
			put.put_disc(now);
			int nowvalue = C * spotvalue[now.x][now.y] + 4 * C * find(put, 1, alpha, beta) - put.next_valid_spots.size();
			if (nowvalue > maxmin[0])
			{
				maxmin[0] = nowvalue;
				p = now;
			}
			if (alpha <= maxmin[0])
				alpha = maxmin[0];
			if (alpha >= beta)//prune
				break;
		}
	}
	fout << p.x << " " << p.y << std::endl;
	fout.flush();
}

int main(int, char** argv) {
	std::ifstream fin(argv[1]);
	std::ofstream fout(argv[2]);
	read_board(fin);
	read_valid_spots(fin);
	write_valid_spot(fout);
	fin.close();
	fout.close();
	return 0;
}