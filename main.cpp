#include <iostream>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <ncurses.h>
#include <pthread.h>
#include <csignal>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include "temp.h"

int PLAYER_COUNT = 0;
vector<Player> players;
Player host_player;
int socket_base[] = {0, 0, 0};
bool e = false;
int dir = 0;
Map m;
bool CONSOLE = true;
bool server_mode = true;

std::ofstream info;
std::ofstream info_;
std::ofstream info__;

// Func def
int server(int port);
int client(int port, char *ip);
static void *input(void *);
int game(bool server);
static void *key_catcher_server(void *);
static void *key_catcher_client(void *);

// void from_32_to_8(list<uint8_t> &vec, uint32_t value)
// {
//     vec.push_back((value >> 24) & 0xFF);
//     vec.push_back((value >> 16) & 0xFF);
//     vec.push_back((value >> 8) & 0xFF);
//     vec.push_back(value & 0xFF);
// }

// vector<uint32_t> from_8_to_32(list<uint8_t> &vec)
// {
//     vector<uint32_t> ans;
//     uint32_t val;
//     int i = 0;
//     for (list<uint8_t>::iterator it = vec.begin(); it != vec.end(); it++)
//     {
//         if (i < 4)
//         {
//             val = val << 8;
//             val += *it;
//             i++;
//         }
//         else
//         {
//             ans.push_back(val);
//             val = 0;
//             i = 0;
//         }
//     }
//     ans.push_back(val);
//     return ans;
// }

int main(int argc, char *argv[])
{
    int port; //=8080;
    char *ip; //= "127.0.0.1";
    info.open("info.txt", std::ios_base::app);

    if (argc < 3)
    {
        printf("Usage: ./client -p <port> -a <ip> or ./server -p <port> -c <count>\n");
        exit(0);
    }
    int arg;

    while ((arg = getopt(argc, argv, "p:a:c:")) != -1)
    {
        switch (arg)
        {
        case 'p':
            port = atoi(optarg);
            break;
        case 'c':
            PLAYER_COUNT = std::atoi(optarg);
            break;
        case 'a':
            ip = optarg;
            server_mode = false;
            break;
        default:
            break;
        }
    }

    cout << "port:" << port << endl;
    cout << "PLAYER_COUNT:" << PLAYER_COUNT << endl;
    info << server_mode << endl;
    if (!server_mode)
        cout << "IP:" << ip << endl;

    if (server_mode)
    {
        server(port);
    }
    else
    {
        client(port, ip);
    }

    return 0;
}

int server(int port)
{
    Player temp_;
    for (int i = 0; i < PLAYER_COUNT; i++)
        players.push_back(temp_);
    if (PLAYER_COUNT > 5)
    {
        printf("Too many players(%d)\n", PLAYER_COUNT);
        exit(0);
    }
    if (PLAYER_COUNT < 1)
    {
        printf("Too few players(%d)\n", PLAYER_COUNT);
        exit(0);
    }
    cout << port << " " << PLAYER_COUNT << endl;

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, PLAYER_COUNT) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        if ((players[i].socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        // cout << i << endl;
    }
    cout << players[0].socket << endl;
    players.resize(PLAYER_COUNT);
    // читаю имя хоста
    host_player.eneter_name();
    // list<uint8_t> nm;
    // nm.push_back('1');
    // nm.push_back('1');
    // host_player.set_name(nm);
    host_player.show_info();
    host_player.set_body('A');
    // читаю имена players
    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        Packet buffer;
        buffer.read_from_socket(players[i].socket, 1, "server");
        cout << "\n\n\n\n";
        buffer.show_info();
        players[i].set_name(buffer.get_data());
        players[i].show_info();

        buffer.set_packet(16, 20 * 15, m.get_map_to_list());
        buffer.send_to_socket(players[i].socket);

        buffer.read_from_socket(players[i].socket, 1, "server");
        players[i].set_status(1);
        players[i].set_body('G');
    }
    m.map_init(m.get_map_to_list());

    // host_player.show_info();
    // for (int i = 0; i < PLAYER_COUNT; i++)
    //     players[i].show_info();
    //  m.showInfo();
    //  for (int i = 0; i < PLAYER_COUNT; i++)
    //{
    //      Packet buffer;
    //      buffer.set_packet(16, 20 * 15, m.get_map_to_list());
    //      buffer.send_to_socket(players[i].socket);
    //      buffer.show_info();
    //  }

    host_player.set_x_y(m.find_first_user());
    // m.show_full_map();

    players.push_back(host_player);
    // cout << players.size() << endl;
    // cout << endl;
    // players[1].show_info();
    // cout << "host_name" << players[1].get_name().size() << endl;

    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        // list<uint8_t> data;
        uint32_t frame_timeout = 2000;
        Msg msg(PLAYER_COUNT + 1, frame_timeout);
        // from_32_to_8(data, PLAYER_COUNT);
        // from_32_to_8(data, frame_timeout);
        Packet buffer; //(0x20, sizeof(uint32_t) * 6 + 256 * msg.player_count, data);
        buffer.send_to_socket(players[i].get_socket(), 0x20, sizeof(uint32_t) * 6 + 256 * msg.player_count, msg);
        //  buffer.send_to_socket(players[i].get_socket());
        players[i].set_x_y(m.find_first_user());
        buffer.send_to_socket(players[i].get_socket(), players);
        // cout << endl;
        // players[i].show_info();
    }
    cout << endl;
    cout << endl;
    for (Player p : players)
    {
        p.show_info();
        cout << endl;
    }
    cout << endl;
    players.pop_back();
    Point p;
    // m.show_full_map();
    while (!p.is_simple())
    {
        p.point_copy(m.find_first_user());
        cout << p.x << " " << p.y << endl;
        if (!p.is_simple())
        {
            m.set_pos(p, 0xAA);
        }
    }
    players.push_back(host_player);
    host_player.show_info();
    usleep(200000);
    //  m.show_full_map();
    // players.push_back(host_player);
    cout << endl;
    for (int i = 0; i < players.size(); i++)
    {
        players[i].name_cut();
        players[i].show_info();
        cout << "\n"
             << i << endl;
    }

    game(1);

    return 0;
}

int sock = 0;
int client(int port, char *ip)
{

    int valread;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    printf("%d", port);
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        perror("invalid address / address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    Packet buffer;
    uint32_t ptype = 1;
    uint32_t datasize = 256;

    host_player.eneter_name();
    host_player.set_body('H');

    // before start
    buffer.set_packet(ptype, datasize, host_player.get_name());
    buffer.send_to_socket(sock);
    // buffer.show_info();

    // get map
    buffer.read_from_socket(sock, 0, "client");
    // perror("map");
    m.map_init(buffer.get_data());
    // m.show_full_map();

    // ready
    buffer.set_packet(2, 0);
    buffer.send_to_socket(sock);
    buffer.clear();

    // 0x20
    Msg msg;
    msg.msg_init(buffer.read_msg(sock));
    // cout << msg.player_count << endl;
    // cout << endl;
    //  players.resize(msg.player_count);
    //  msg.show_info();
    for (int i = 0; i < msg.player_count; i++)
    {
        players.push_back(buffer.read_player(sock));
        players[players.size() - 1].set_body('G');
    }
    for (Player p : players)
    {
        m.set_pos(p.get_x_y(), 0);
        // p.show_info();
    }
    Point p;
    while (!p.is_simple())
    {
        p.point_copy(m.find_first_user());

        if (!p.is_simple())
        {
            m.set_pos(p, 0xAA);
        }
    }
    // cout << players.size();

    // m.show_full_map();
    host_player.name_cut();
    host_player.show_info();
    cout << endl;

    for (Player p : players)
    {
        p.show_info();
        cout << endl;
    }

    Player temp;
    perror("C");
    for (vector<Player>::iterator it = players.begin(); it != players.end(); it++)
    {
        (*it).name_cut();
        (*it).show_info();
        if ((*it).eql(host_player))
        {
            host_player.set_x((*it).get_x());
            host_player.set_y((*it).get_y());
            host_player.set_dir((*it).get_dir());
            players.erase(it);
            perror("T");
        }
    }
    perror("add_host");
    players.push_back(host_player);
    perror("game_start");

    cout << "\n\n";
    for (Player p : players)
    {
        p.name_cut();
        p.show_info(1);
        cout << endl;
    }
    game(0);

    return 0;
}

string if_server()
{
    if (server_mode)
        return " server";
    return " client";
}
clock_t startTime = 0;
clock_t endTime;
int game(bool server)
{
    startTime = clock();
    info_.open("info_.txt", std::ios_base::app);
    info__.open("info__.txt", std::ios_base::app);
    string stop;
    // cin >> stop;
    srand(time(NULL));
    initscr();
    noecho();
    int x = 0, y = 0;
    getmaxyx(stdscr, y, x);
    m.ncurs_print_map();
    pthread_t pid;
    void *retval;
    pthread_create(&pid, NULL, input, NULL);
    if (server_mode)
    {
        info << "server_start" + if_server() << endl;
        pthread_create(&pid, NULL, key_catcher_server, NULL);
    }
    else
    {
        info << "client_start" + if_server() << endl;
        pthread_create(&pid, NULL, key_catcher_client, NULL);
    }

    curs_set(0);

    while (!e)
    {
        int player_score_y = 0;
        for (int i = 0; i < players.size(); i++)
        {
            refresh();
            players[i].show_info_ncurses();
            players[i].player_printer();
            vector<Player> players_copy(players);
            vector<Player>::iterator it = players_copy.begin();
            advance(it, i);
            players_copy.erase(it);
            Point temp(players[i].get_x(), players[i].get_y());
            switch (players[i].get_dir())
            {
            case 0:

                if (m.wall_checker(temp--) && m.in_map(temp--) && m.is_player((temp--), players_copy))
                {
                    players[i].set_y(players[i].get_y() - 1);
                    players[i].player_printer();
                    m.set_pos((temp--), 0, players[i].get_score());
                }
                break;

            case 1:
                if (m.wall_checker((++temp)) && m.in_map((++temp)) && m.is_player((++temp), players_copy))
                {
                    players[i].set_x(players[i].get_x() + 1);
                    players[i].player_printer();
                    m.set_pos((++temp), 0, players[i].get_score());
                }
                break;

            case 2:
                if (m.wall_checker((temp++)) && m.in_map((temp++)) && m.is_player((temp++), players_copy))
                {
                    players[i].set_y(players[i].get_y() + 1);
                    players[i].player_printer();
                    m.set_pos((temp++), 0, players[i].get_score());
                }
                break;

            case 3:
                if (m.wall_checker((--temp)) && m.in_map((--temp)) && m.is_player((--temp), players_copy))
                {
                    players[i].set_x(players[i].get_x() - 1);
                    players[i].player_printer();
                    m.set_pos((--temp), 0, players[i].get_score());
                }
                break;
            case 10:
                e = true;
                break;

            default:
                break;
            }
            info << if_server() << *(players[players.size() - 1].get_score()) << endl;
            // players[i].score_printer(i);
            m.ncurs_print_map();
            players[i].player_printer();
            for (int l = 0; l < players.size(); l++)
            {
                players[l].player_printer(l);
                refresh();
            }
            refresh();
            // usleep(90000);
            for (int j = 0; j < players.size(); j++)
                players[j].show_info_ncurses(35 + j * 5);
            refresh();
        }
        usleep(550000);
    }
    // usleep(20000000);
    endwin();
    info.close();
    info_.close();
    info__.close();
    return 0;
}

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
        // mvprintw(18, 95, "dir:%d", c);
        players[players.size() - 1].set_dir(dir);
        Packet buffer;
        buffer.CONSOLE = false;
        if (!server_mode)
        {
            info << "send to server" << endl;
            list<uint8_t> t;
            t.push_back(dir);
            buffer.set_magic();
            buffer.set_packet(0, 1, t);
            buffer.send_to_socket(sock, "client", "input");
        }
        else
        {
            for (int i = 0; i < players.size() - 1; i++)
            {
                for (int j = 0; j < players.size(); j++)
                {
                    if (i == j)
                        continue;
                    info_ << "to   " << players[i].get_name(1) << "(" << i << ")" << endl;
                    info_ << "what " << players[j].get_name(1) << "(" << j << ") " << players[j].get_dir() << endl;
                    double kt = (double)(clock() - startTime) / CLOCKS_PER_SEC;

                    info_ << kt << endl;

                    Packet buffer;
                    buffer.set_magic();
                    buffer.CONSOLE = false;
                    list<uint8_t> temp;
                    temp.push_back(players[j].get_dir());
                    buffer.set_packet(0, 1, temp);
                    buffer.send_to_socket(players[i].get_socket(), "server", "key_catcher_server");
                    info_ << "---------------" << endl;
                }
            }
            // usleep(100);
        }
        for (Player i : players)
        {
            i.player_printer();
        }
    }

    return NULL;
}

static void *key_catcher_server(void *)
{
    info_ << "server_key_catcher_server" + if_server() << endl;
    while (!e)
    {
        // players[players.size() - 1].set_dir(host_player.get_dir());
        for (int i = 0; i < players.size() - 1; i++)
        {
            info_ << i << " " << players[i].get_name(1) << " " << players[i].get_dir() << endl;
            Packet buffer;
            buffer.CONSOLE = false;
            buffer.read_from_socket(players[i].get_socket(), 1, "key_catcher_server");
            players[i].set_dir(buffer.get_key());
        }

        for (int i = 0; i < players.size() - 1; i++)
        {
            for (int j = 0; j < players.size(); j++)
            {
                if (i == j)
                    continue;
                info_ << "to   " << players[i].get_name(1) << "(" << i << ")" << endl;
                info_ << "what " << players[j].get_name(1) << "(" << j << ") " << players[j].get_dir() << endl;
                double kt = (double)(clock() - startTime) / CLOCKS_PER_SEC;

                info_ << kt << endl;

                Packet buffer;
                buffer.set_magic();
                buffer.CONSOLE = false;
                list<uint8_t> temp;
                temp.push_back(players[j].get_dir());
                buffer.set_packet(0, 1, temp);
                buffer.send_to_socket(players[i].get_socket(), "server", "key_catcher_server");
                info_ << "---------------" << endl;
            }
        }
        // usleep(15000);

        // usleep(9000000);
    }
    return NULL;
}

static void *key_catcher_client(void *)
{
    info__ << "key_catcher_client" + if_server() << endl;

    while (!e)
    {

        for (int i = 0; i < players.size() - 1; i++)
        {
            info__ << ";)" << endl;
            Packet buffer;
            buffer.set_magic();

            buffer.CONSOLE = false;
            buffer.read_from_socket(sock, 0, "key_catcher_client");
            players[i].set_dir(buffer.get_key());
            buffer.show_info(1);
            // players[i].show_info(1);
            double kt = (double)(clock() - startTime) / CLOCKS_PER_SEC;
            info__ << kt << endl;

            info__ << i << " " << buffer.get_key() + 48 << "\n"
                   << endl;
        }
        // usleep(20000);
    }
    return NULL;
}