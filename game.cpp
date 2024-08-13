#include <iostream>
#include <time.h>
#include <errno.h>
#include <ncurses.h>
#include <pthread.h>
#include <csignal>
#include "temp.h"

using namespace std;

bool e = false;
int dir = 9;
int PLAYER_COUNT = 0;

// void sigint_handler(int signal)
//
//{
//
//	cout << "Interrupted by signal " << signal << endl;
//	endwin();
// }

static void *input(void *)
{
	char last;
	char c;
	while (!e)
	{
		c = getch();
		switch (c)
		{
		case 'w':
			dir = 0;

			break;
		case 'd':
			dir = 1;

			break;
		case 's':

			dir = 2;

			break;
		case 'a':

			dir = 3;

			break;
		case 'p':
			dir = 10;
			break;
		}
		mvprintw(18, 95, "dir:%d", c);
	}

	return NULL;
}

void test()
{

	endwin();
	e = false;
}

int main()
{
	// signal(SIGINT, sigint_handler);
	srand(time(NULL));
	initscr();
	noecho();
	int x = 0, y = 0;
	getmaxyx(stdscr, y, x);
	Map m;
	m.map_init(m.get_map_to_list());
	m.ncurs_print_map();

	Player player;
	Point start_pos;
	start_pos.point_copy(m.find_first_user());
	list<uint8_t> name;
	name.push_back(65);
	mvprintw(10, 90, "%d,%d", start_pos.x, start_pos.y);
	refresh();

	player.player_constructor((start_pos.x), start_pos.y, name.size(), 0, name);
	player.show_info_ncurses();

	pthread_t pid;
	void *retval;
	pthread_create(&pid, NULL, input, NULL);
	player.show_info_ncurses(4);
	int k = 0;
	curs_set(0);

	while (!e)
	{
		refresh();
		k = dir;
		player.set_dir(dir);
		player.show_info_ncurses();
		player.player_printer();
		Point temp(player.get_x(), player.get_y());
		switch (k)
		{
		case 0:
			// mvprintw(13, 90, "wall_checker:%d", (m.wall_checker((temp--))));
			// mvprintw(14, 90, "in_map:%d", (m.in_map((temp--))));
			// mvprintw(7, 90, "x y:%d %d", (temp--).x, (temp--).y);

			if (((m.wall_checker(temp--))) && ((m.in_map(temp--))))
			{
				player.set_y(player.get_y() - 1);
				player.player_printer();
				m.set_pos((temp--), 0, player.get_score());
			}
			break;

		case 1:
			if (((m.wall_checker((++temp)))) && ((m.in_map((++temp)))))
			{
				player.set_x(player.get_x() + 1);
				player.player_printer();
				m.set_pos((++temp), 0, player.get_score());
			}
			break;

		case 2:
			if (((m.wall_checker((temp++)))) && ((m.in_map((temp++)))))
			{
				player.set_y(player.get_y() + 1);
				player.player_printer();
				m.set_pos((temp++), 0, player.get_score());
			}
			break;

		case 3:
			if (((m.wall_checker((--temp)))) && ((m.in_map((--temp)))))
			{
				player.set_x(player.get_x() - 1);
				player.player_printer();
				m.set_pos((--temp), 0, player.get_score());
			}
			break;
		case 10:
			e = true;
			break;

		default:
			break;
		}
		player.score_printer(0);
		m.ncurs_print_map();
		player.player_printer();
		refresh();
		usleep(90000);
		player.show_info_ncurses();
	}
	// usleep(20000000);
	endwin();
	return 0;
}