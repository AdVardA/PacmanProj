#include <unistd.h>
#include <string.h>
#include <list>
#include <iterator>
#include <vector>
#include <ncurses.h>
#include <pthread.h>
#include "new_cons.h"
#include <fstream>

#define MAX_PLAYER_COUNT 4
#define N_MAX_X 40
#define N_MAX_Y 30

std::ofstream info_class;

// Error_ er_;

using namespace std;
std::ofstream out;
std::ofstream history;
std::ofstream send_to;

int error_int = 0;
void perror_(string error)
{
    mvprintw(10 + error_int, 110, "%s", error.c_str());
    refresh();
    error_int++;
    error_int %= 10;
}

void perror__(string error, string who, string when)
{
    // mvprintw(35 + error_int, 15, "%s", error);
    // refresh();
    if (!out.is_open())
    {
        out.open("error.txt", std::ios_base::app);
        out.clear();
    }
    out << who + "(" + when + ")" + ":" << error << endl;
    out.close();
    error_int++;
    error_int %= 10;
    exit(0);
}

void perror_(string error, int t)
{
    mvprintw(error_int, 110 + t, "%s", error.c_str());
    refresh();
    error_int++;
    error_int %= 5;
}

class Point
{
public:
    uint32_t x;
    uint32_t y;
    Point()
    {
        x = 0;
        y = 0;
    }
    Point(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
    void point_copy(Point p)
    {
        this->x = p.x;
        this->y = p.y;
    }
    Point operator++()
    {
        Point t(x + 1, y);
        return t;
    }
    Point operator++(int)
    {
        Point t(x, y + 1);
        return t;
    }
    Point operator--()
    {
        Point t(x - 1, y);
        return t;
    }
    Point operator--(int)
    {
        Point t(x, y - 1);
        return t;
    }
    bool is_simple()
    {
        if (x == 1 && y == 1)
            return true;
        return false;
    }
    bool is_eql(Point temp)
    {
        return (temp.x == this->x && temp.y == this->y);
    }
};

class Player
{
    uint32_t start_x;
    uint32_t start_y;
    uint32_t player_name_len;
    uint32_t direction;
    uint32_t score;
    string body;
    list<uint8_t> name;
    bool status;

public:
    int socket;
    Player()
    {
        start_x = 0;
        start_y = 0;
        player_name_len = 0;
        direction = 0;
        score = 0;
        socket = -1;
    }

    void set_body(char c)
    {
        body = c;
        body += " ";
    }

    int set_socket(int sd)
    {
        this->socket = sd;
        return sd;
    }
    int get_socket() { return this->socket; }

    uint32_t get_len_name() { return player_name_len; }

    bool player_init_check()
    {
        if (start_x > N_MAX_X)
        {
            perror_("Wrong start_x for player");
            return 0;
        }
        if (start_y > N_MAX_Y)
        {
            perror_("Wrong start_y for player");
            return 0;
        }
        if (player_name_len < 0)
        {
            perror_("player_name_len must be > 0");
            return 0;
        }
        if (player_name_len > 256)
        {
            perror_("player_name_len must be <= 256");
            return 0;
        }
        if (direction < 0)
        {
            perror_("Direction must be >= 0");
            return 0;
        }
        if (direction > 3)
        {
            perror_("Direction must be <= 3");
            return 0;
        }
        int temp = 0;
        for (list<uint8_t>::iterator it = name.begin(); it != name.end(); it++)
        {
            temp += *it;
            if ((*it > 0) && (*it < 32))
            {
                *it = 32;
            }
        }
        if (temp == 0)
        {
            perror_("Name must contein not only 0(Ptype = 1)");
            return 0;
        }
        return 1;
    }

    bool player_constructor(uint32_t x, uint32_t y, uint32_t name_len, uint32_t dir, list<uint8_t> name)
    {
        start_x = x;
        start_y = y;
        player_name_len = name_len;
        direction = dir;
        this->name.merge(name);
        return player_init_check();
    }
    void player_copy(Player p)
    {
        start_x = p.get_x();
        start_y = p.get_y();
        player_name_len = p.get_len_name();
        direction = p.get_dir();
        this->name.clear();
        this->name.merge(p.get_name());
    }
    void set_name(list<uint8_t> name)
    {
        this->name = name;
        this->player_name_len = 256;
    }
    void show_info()
    {
        cout << "x:" << start_x << endl;
        cout << "y:" << start_y << endl;
        cout << "player_name_len:" << player_name_len << " " << name.size() << endl;
        cout << "direction:" << direction << endl;
        for (list<uint8_t>::iterator it = name.begin(); it != name.end(); it++)
        {
            cout << *it;
        }
        cout << endl;
    }
    void show_info(int a)
    {
        if (!info_class.is_open())
        {
            info_class.open("info.txt", std::ios_base::app);
        }
        info_class << "\nplayer" << endl;
        info_class << "\tx:" << start_x << endl;
        info_class << "\ty:" << start_y << endl;
        info_class << "\tplayer_name_len:" << player_name_len << " " << name.size() << endl;
        info_class << "\tdirection:" << direction << endl;
        info_class << "\t";
        for (list<uint8_t>::iterator it = name.begin(); it != name.end(); it++)
        {
            info_class << *it;
        }
        info_class << "\n----------------------------------" << endl;
    }

    void show_info_ncurses()
    {
        int x = 90, y = 0;
        int i = 0;
        for (uint8_t c : name)
        {
            mvprintw(y, x + i, "name:%c", c);
            i++;
        }
        mvprintw(++y, x, "x:%d", start_x);
        mvprintw(++y, x, "y:%d", start_y);
        mvprintw(++y, x, "direction:%d", direction);
        refresh();
    }

    void show_info_ncurses(int y)
    {
        int x = 0;
        int i = 0;
        mvprintw(y, x + i, "name:");

        for (list<uint8_t>::iterator it = name.begin(); it != name.end(); it++)
        {
            mvprintw(y, x + 5 + i, "%c", *it);
            i++;
        }
        mvprintw(++y, x, "x:%d", start_x);
        mvprintw(++y, x, "y:%d", start_y);
        mvprintw(++y, x, "direction:%d", direction);
        refresh();
    }

    uint32_t get_x() { return start_x; }
    uint32_t get_y() { return start_y; }
    Point get_x_y()
    {
        Point temp(start_x, start_y);
        return temp;
    }
    void set_x_y(Point t)
    {
        start_x = t.x;
        start_y = t.y;
    }
    void set_x(uint32_t x)
    {
        if (x > N_MAX_X)
        {
            perror_("Too big x");
            x = 0;
        }
        // if(y>30){perror_("Too big y");}
        if (N_MAX_X > x)
            start_x = x % N_MAX_X;
    }
    void set_y(uint32_t y)
    {
        if (y > N_MAX_Y)
        {
            perror_("Too big y");
            y = 0;
        }
        if (N_MAX_Y > y)
            start_y = y % N_MAX_Y;
    }
    void player_printer()
    {
        if (start_y != 0)
            mvprintw(start_y + 1, (start_x + 1) * 2, "%s", body.c_str());
        else //(start_y==0)
            mvprintw(1, (start_x + 1) * 2, "%s", body.c_str());
        if (start_x != 0)
            mvprintw(start_y + 1, (start_x + 1) * 2, "%s", body.c_str());
        else //(start_x==1)
            mvprintw(start_y + 1, 2, "%s", body.c_str());
    }

    void player_printer(int a)
    {
        if (start_y != 0)
            mvprintw(start_y + 1, (start_x + 1) * 2, "%s", body.c_str());
        else //(start_y==0)
            mvprintw(1, (start_x + 1) * 2, "%s", body.c_str());
        if (start_x != 0)
            mvprintw(start_y + 1, (start_x + 1) * 2, "%s", body.c_str());
        else //(start_x==1)
            mvprintw(start_y + 1, 2, "%s", body.c_str());
        int i = 0;
        for (uint8_t t : name)
        {
            mvprintw(40 + a, 8 + i, "%c", t);
            i++;
        }
        mvprintw(40 + a, 8 + i, ":%d", score);
    }
    uint32_t get_dir()
    {
        return direction;
    }
    void set_dir(uint32_t dir)
    {
        if (dir != 0 && dir != 1 && dir != 2 && dir != 3)
        {
            perror_("Wrong dir");
        }
        direction = dir;
    }

    uint32_t *get_score()
    {
        return &score;
    }

    void score_printer(uint8_t t)
    {
        int i = 0;
        for (list<uint8_t>::iterator it = name.begin(); it != name.end(); it++)
        {
            mvprintw(30 + t, 110 + i, "%c", *it);
            i++;
        }
        mvprintw(30 + t, 110 + i, ": %d", score);
    }

    void eneter_name()
    {
        string buf;
        bool flag = true;
        while (flag)
        {
            cout << "\nPlease enter name:";
            cin >> buf;
            cout << endl;
            // cout << buf << buf.size() << (buf.size() < 257) << (buf.size() != 0) << endl;
            if ((buf.size() < 257) && (buf.size() != 0)) //&& (buf.size() != 1 && buf[0] == '\n')
            {
                name.resize(buf.size());
                int i = 0;
                for (list<uint8_t>::iterator it = name.begin(); it != name.end(); it++)
                {
                    *it = buf[i];
                    i++;
                }
                name.resize(256);
                player_name_len = 256;
                flag = false;
                // er_.name = buf;
                break;
            }
            if (buf.size() != 0)
            {
                cout << "ERROR: name.size() can't be eqw 0" << endl;
            }
            if (buf.size() < 257)
            {
                cout << "ERROR: name.size() too large" << endl;
            }
        }
    }

    list<uint8_t> get_name()
    {
        return name;
    }

    string get_name(int a)
    {
        string str = "";
        for (char t : name)
        {
            str += t;
        }
        return str;
    }

    void set_status(bool stat)
    {
        status = stat;
    }

    bool get_status()
    {
        return status;
    }

    void name_cut()
    {
        int s = 0;
        for (uint8_t c : name)
        {
            if (c == 0)
            {
                break;
            }
            s++;
        }
        name.resize(s);
        player_name_len = s;
    }
    vector<uint8_t> vector_name()
    {
        vector<uint8_t> temp;
        for (uint8_t c : name)
        {
            temp.push_back(c);
        }
        return temp;
    }

    bool eql(Player p)
    {
        int flag = 0;
        vector<uint8_t> local(vector_name());
        vector<uint8_t> second(p.vector_name());
        if (local.size() != second.size())
            return true;
        // perror("start_loop");

        for (int i = 0; i < local.size(); i++)
        {
            // perror("loop");
            // cout << i << " " << local[i] << " " << second[i] << endl;
            // perror("cout");
            if ((local[i]) == (second[i]))
            {
                // perror("if");
                flag++;
            }
            // perror("end_if");
        }
        if (this->name.size() == flag)
            return true;
        return false;
    }
};

class Msg
{
public:
    uint32_t player_count;
    uint32_t frame_timeout;
    Msg()
    {
        player_count = 0;
        frame_timeout = 0;
    }
    Msg(uint32_t p, uint32_t f)
    {
        player_count = p;
        frame_timeout = f;
    }
    bool msg_check()
    {
        if (frame_timeout == 0)
        {
            perror_("frame_timeout != 0");
            return 0;
        }
        if (player_count > MAX_PLAYER_COUNT)
        {
            perror_("too much palyers");
            return 0;
        }
        // for (list<Player>::iterator it = player.begin(); it != player.end(); it++)
        //{
        //     if ((*it).player_init_check())
        //     {
        //         perror_("Player");
        //         return 0;
        //     }
        // }
        return 1;
    }
    void msg_init(Msg m)
    {
        player_count = m.player_count;
        frame_timeout = m.frame_timeout;
    }
    void show_info()
    {
        cout << frame_timeout << " " << player_count << endl;
    }
};

class Packet
{
    uint32_t magic;
    uint32_t ptype;
    uint32_t datasize;
    list<uint8_t> data;
    bool flag_init;

public:
    bool CONSOLE = true;
    Packet()
    {
        magic = 0;
        ptype = 0;
        datasize = 0;
        list<uint8_t> t;
    }

    bool magic_check()
    {
        return (magic == 0xabcdfe01);
    }

    void set_magic()
    {
        magic = 0xabcdfe01;
    }

    void read_from_socket(int socket, bool is_server, string func)
    {
        this->data.resize(0);
        string who = "client";
        if (is_server)
        {
            who = "server";
        }
        if (!history.is_open())
        {
            history.open("buffer_hist.txt", std::ios_base::app);
            history.clear();
        }

        if (read(socket, &magic, sizeof(uint32_t)) == -1)
        {
            if (CONSOLE)
                perror("magic");
            else
            {
                char err[1024];
                strcpy(err, strerror(errno));
                string er(err);
                er = "magic" + er;
                perror__(er, who, func);
                // er_.show_error(err);
            }
        }

        if (magic_check())
        {
            if (read(socket, &ptype, sizeof(uint32_t)) == -1)
            {
                if (CONSOLE)
                    perror("ptype");
                else
                {
                    char err[1024];
                    strcpy(err, strerror(errno));
                    string er(err);
                    er = "ptype" + er;
                    perror__(er, who, func);
                    // er_.show_error(err);
                }
            }
            if (read(socket, &datasize, sizeof(uint32_t)) == -1)
            {
                if (CONSOLE)
                    perror("datasize");
                else
                {
                    char err[1024];
                    strcpy(err, strerror(errno));
                    string er(err);
                    er = "datasize" + er;
                    perror__(er, who, func);
                    // er_.show_error(err);
                }
            }
            if (ptype != 0x20)
                for (int i = 0; i < datasize; i++)
                {
                    uint8_t temp;
                    if (read(socket, &temp, sizeof(uint8_t)) == -1)
                    {
                        if (CONSOLE)
                            perror("data");
                        else
                        {
                            char err[1024];
                            strcpy(err, strerror(errno));
                            string er(err);
                            er = "data" + er;
                            perror__(er, who, func);
                            // er_.show_error(err);
                        }
                    }
                    // if(temp==255){cout<<1<<endl;}
                    // if(temp==0xAA){cout<<0<<endl;}
                    data.push_back(temp);
                }
            flag_init = true;
        }
        else
        {
            if (CONSOLE)
                cout << "Error: wrong pacekt " << std::hex << magic;
            else
            {
                char err[1024];
                string er;
                er = "Error: wrong pacekt = " + std::to_string(magic);
                perror__(er, who, func);

                // er_.show_error(err);
            }
        }
        history << who << endl;
        history << magic << endl;
        history << ptype << endl;
        history << datasize << endl;
        for (char c : data)
        {
            history << c << " ";
        }
        history << "\n------------------------------------------------------------------------------------\n";
    }

    Msg read_msg(int socket)
    {
        this->data.resize(0);

        if (read(socket, &magic, sizeof(uint32_t)) == -1)
            perror("magic");
        // cout << magic << endl;
        if (magic_check())
        {
            if (read(socket, &ptype, sizeof(uint32_t)) == -1)
                perror("ptype");
            // cout << ptype << endl;
            if (read(socket, &datasize, sizeof(uint32_t)) == -1)
                perror("datasize");
            // cout << datasize << endl;
            Msg temp;

            if (read(socket, &temp.frame_timeout, sizeof(uint32_t)) == -1)
                perror("frame_timeout");
            // cout << val << "frame_timeout" << endl;
            if (read(socket, &temp.player_count, sizeof(uint32_t)) == -1)
                perror("player_count");
            // cout << val << " player_count " << endl;
            return temp;
        }
        else
        {
            perror("Error: wrong pacekt");
            Msg t;
            return t;
        }
    }

    void send_to_socket(int sock, uint32_t ptype, uint32_t datasize, Msg msg)
    {
        set_magic();
        // cout << magic << " " << pt << " " << dt << endl;
        if (send(sock, &magic, sizeof(uint32_t), 0) == -1)
            perror("magic");
        if (send(sock, &ptype, sizeof(uint32_t), 0) == -1)
            perror("ptype");
        if (send(sock, &datasize, sizeof(uint32_t), 0) == -1)
            perror("datasize");
        // datasize = sizeof(uint32_t) * 6 + 256 * msg.player_count;
        if (send(sock, &msg.frame_timeout, sizeof(uint32_t), 0) == -1)
            perror("frame_timeout");

        if (send(sock, &msg.player_count, sizeof(uint32_t), 0) == -1)
            perror("player_count");
    }

    Player read_player(int socket)
    {
        this->data.resize(0);
        Player temp;
        uint32_t val;

        if (read(socket, &val, sizeof(uint32_t)) == -1)
            perror("start_x");

        // cout << "\t" << val << endl;
        temp.set_x(val);
        if (read(socket, &val, sizeof(uint32_t)) == -1)
            perror("start_y");
        // cout << "\t" << val << endl;

        temp.set_y(val);
        if (read(socket, &val, sizeof(uint32_t)) == -1)
            perror("start_direction");
        // cout << "\t" << val << endl;
        temp.set_dir(val);
        if (read(socket, &val, sizeof(uint32_t)) == -1)
            perror("player_name_len");
        // cout << "\t" << val << endl;

        for (int i = 0; i < 256; i++)
        {
            uint8_t t;
            if (read(socket, &t, sizeof(uint8_t)) == -1)
                perror("name");
            // if(temp==255){cout<<1<<endl;}
            // if(temp==0xAA){cout<<0<<endl;}
            data.push_back(t);
        }

        temp.set_name(data);
        return temp;
    }

    void send_to_socket(int sock, vector<Player> players)
    {

        for (Player p : players)
        {
            // cout << "-----------------------" << endl;
            // data.resize(0);
            uint32_t val = p.get_x();
            if (send(sock, &val, sizeof(uint32_t), 0) == -1)
                perror("get_x");

            val = p.get_y();
            if (send(sock, &val, sizeof(uint32_t), 0) == -1)
                perror("get_y");

            val = p.get_dir();
            if (send(sock, &val, sizeof(uint32_t), 0) == -1)
                perror("get_dir");

            val = p.get_len_name();
            if (send(sock, &val, sizeof(uint32_t), 0) == -1)
                perror("get_len_name");
            data.clear();
            data.merge(p.get_name());
            for (list<uint8_t>::iterator it = data.begin(); it != data.end(); it++)
            {
                if (send(sock, &(*it), sizeof(uint8_t), 0) == -1)
                    perror("data");
                // std::cout << std::hex << static_cast<int>(*it) << "(" << *it << ") ";
            }
            // cout << endl;
        }
    }

    void send_to_socket(int sock, string who = "not game", string when = "not_at_game_fun")
    {
        set_magic();
        if (!send_to.is_open())
        {
            send_to.open("send_history.txt", std::ios_base::app);
            send_to.clear();
        }
        send_to << who + "(" + when + ")" + ":" << magic << "\n\tptype: " << ptype << "\n\tdatasize: " << datasize << endl;
        send_to << "\tdata:";
        for (uint8_t t : data)
        {
            send_to << t + 48 << " ";
        }
        send_to << "\n------------------------------------------------------------------------------------\n";

        if (datasize != data.size())
        {
            data.resize(datasize);
        }
        if (send(sock, &magic, sizeof(uint32_t), 0) == -1)
            perror("magic");
        if (send(sock, &ptype, sizeof(uint32_t), 0) == -1)
            perror("ptype");
        if (send(sock, &datasize, sizeof(uint32_t), 0) == -1)
            perror("datasize");
        for (list<uint8_t>::iterator it = data.begin(); it != data.end(); it++)
        {
            if (send(sock, &(*it), sizeof(uint8_t), 0) == -1)
                perror("data");
        }
    }
    Packet(uint32_t ptype, uint32_t datasize, list<uint8_t> data)
    {
        this->magic = 0xabcdfe01;
        this->ptype = ptype;
        this->datasize = datasize;
        this->data.merge(data);
    }
    void set_packet(uint32_t ptype, uint32_t datasize, list<uint8_t> data)
    {
        this->magic = 0xabcdfe01;
        this->ptype = ptype;
        this->datasize = datasize;
        this->data.resize(0);
        this->data.merge(data);
        if (ptype == 1)
            (this->data).resize(256, 0);
    }

    void set_packet(uint32_t ptype, uint32_t datasize)
    {
        this->magic = 0xabcdfe01;
        this->ptype = ptype;
        this->datasize = datasize;
    }

    int start_packet_size()
    {
        return sizeof(uint32_t) * 3;
    }
    void show_info()
    {
        cout << "magic: " << magic << endl;
        cout << "ptype: " << ptype << endl;
        cout << "datasize: " << datasize << endl;
        // cout << strlen((char*)data) << endl;
        if (!data.empty())
        {
            // copy(data.begin(), data.end(), ostream_iterator(cout, " ")); cout << endl;
            for (list<uint8_t>::iterator it = data.begin(); it != data.end(); it++)
            {
                if (ptype != 1)
                    cout << (*it) << " ";
                else
                    cout << (*it);
            }
            cout << endl;
        }
        else
        {
            cout << "List is empty" << endl;
        }
    }

    void show_info(int o)
    {
        if (!info_class.is_open())
        {
            info_class.open("info.txt", std::ios_base::app);
        }
        info_class << "\nbuffer" << endl;
        info_class << "\t" << "magic: " << magic << endl;
        info_class << "\t" << "ptype: " << ptype << endl;
        info_class << "\t" << "datasize: " << datasize << endl;
        // cout << strlen((char*)data) << endl;
        if (!data.empty())
        {
            // copy(data.begin(), data.end(), ostream_iterator(cout, " ")); cout << endl;
            for (list<uint8_t>::iterator it = data.begin(); it != data.end(); it++)
            {
                if (ptype != 1)
                    info_class << "\t" << (*it) << " ";
                else
                    info_class << "\t" << (*it) + 48;
            }
            info_class << "\t" << endl;
        }
        else
        {
            info_class << "\t" << "List is empty" << endl;
        }
        info_class << "\n----------------------------------------------\n";
    }

    void rand_filds()
    {
        magic = rand() % 100;
        ptype = rand() % 100;
        datasize = 4;
        for (int i = 0; i < datasize; i++)
            data.push_back(48 + i);
    }

    bool type_check()
    {

        if (ptype == 1)
        {
            if (datasize < 0)
            {
                perror("Datasize must be < 0(Ptype = 1)");
                return 0;
            }
            if (datasize > 256)
            {
                perror("Datasize must be >= 256(Ptype = 1)");
                return 0;
            }
            int temp = 0;
            for (list<uint8_t>::iterator it = data.begin(); it != data.end(); it++)
            {
                temp += *it;
                if ((*it > 0) && (*it < 32))
                {
                    *it = 32;
                }
            }
            if (temp == 0)
            {
                perror("Data must contein not only 0(Ptype = 1)");
                return 0;
            }

            return 1;
        }

        else if (ptype == 16)
        {
            if (datasize != 20 * 15)
            {
                perror("Wrong datasize(Ptype = 16)");
                return 0;
            }
            for (list<uint8_t>::iterator it = data.begin(); it != data.end(); it++)
            {
                if ((*it != 0xFF) && (*it != 0xAA) && (*it != 0xAA))
                {
                    perror("Wrong object(Ptype = 16)");
                    return 0;
                }
            }
        }
    }

    list<uint8_t> get_data()
    {
        return data;
    }

    void clear()
    {
        ptype = -1;
        datasize = 0;
        data.clear();
    }

    uint8_t get_key()
    {
        list<uint8_t>::iterator it = data.begin();
        return *it;
    }
};

#define H 15
#define W 20
#define MAX_WALL_COUNT 117
#define LOGIC_POINT 2

class Map
{
private:
    vector<vector<uint8_t>> map;

public:
    Map()
    {
        srand(time(NULL));
        vector<uint8_t> temp;
        temp.resize(20, 0xAA);
        map.resize(15, temp);

        int wall_count;
        while ((wall_count = (rand()) % 200 + 50) > MAX_WALL_COUNT)
            ;

        int i = 0;

        while (i != wall_count)
        {
            int x;
            int y;
            x = (rand() % (W - 3)) + 2;
            y = (rand() % (H - 3)) + 2;
            map[y][x] = 0xFF;
            i++;
        }

        for (int i = 1; i < H - 1; i++)
        {
            for (int j = 1; j < W - 1; j++)
            {
                if (map[i][j] == 0xAA)
                {
                    int sum = 0;
                    if (map[i - 1][j] == 0xFF)
                        sum += 1;
                    if (map[i][j - 1] == 0xFF)
                        sum += 1;
                    if (map[i + 1][j] == 0xFF)
                        sum += 1;
                    if (map[i][j + 1] == 0xFF)
                        sum += 1;
                    if (sum > 2)
                    {
                        if (sum == 3)
                        {
                            if (map[i + 1][j])
                                map[i + 1][j] = 0xAA;
                            else
                            {
                                map[i][j + 1] = 0xAA;
                            }
                        }
                        if (sum == 4)
                        {
                            map[i + 1][j] = 0xAA;
                            map[i][j + 1] = 0xAA;
                        }
                    }
                }
            }
        }
        bool pers = true;
        while (pers)
        {
            int x = rand() % 20;
            int y = rand() % 15;
            if (map[y][x] != 0xff)
            {
                map[y][x] = 0x22;
                pers = false;
            }
        }

        // for(list <list <int>> :: iterator it = map.begin(); it != data.end(); it++)
    }

    void map_init(list<uint8_t> data)
    {
        copy_vector(data);
        showInfo();
        for (int i = map.size() - 1; i > -1; i--)
        {
            map.push_back(map[i]);
        }

        // for(vector <vector<int>> :: iterator it = (map.begin()); it != map.end(); it++){
        //     for(vector <int> :: iterator it2 = ((*it).end()); it2 != (*it).begin(); it2--){
        //         (*it).push_back(*it2);
        //     }
        // }
        for (int i = 0; i < map.size(); i++)
        {
            for (int j = map[i].size(); j > -1; j--)
            {
                map[i].push_back(map[i][j]);
            }
            vector<uint8_t>::iterator it = map[i].begin();
            for (int i = 0; i < 20; i++)
                it++;
            map[i].erase(it);
        }
    }

    vector<uint8_t> revers(vector<uint8_t> data)
    {
        vector<uint8_t> temp;
        for (vector<uint8_t>::iterator it = (data.end()); it != data.begin(); it--)
        {
            temp.push_back(*it);
        }
        return temp;
    }

    void copy_vector(list<uint8_t> data)
    {
        vector<uint8_t> temp;
        int i = 0;
        map.resize(0);
        for (list<uint8_t>::iterator it = (data.begin()); it != data.end(); it++)
        {
            if (temp.size() != 20)
                temp.push_back(*it);
            else
            {
                map.push_back(temp);
                temp.resize(1, *it);
                i++;
            }
        }
        map.push_back(temp);
        cout << i << endl;
    }

    int map_sum()
    {
        int sum = 0;
        for (vector<vector<uint8_t>>::iterator it = (map.begin()); it != map.end(); it++)
        {
            if (it == map.begin())
                advance(it, 1);
            for (vector<uint8_t>::iterator it2 = ((*it).begin()); it2 != (*it).end(); it2++)
            {
                if (it2 == (*it).begin())
                    advance(it2, 1);
                sum += *it2;
            }
        }
        cout << sum << endl;
        return sum;
    }

    void showInfo()
    {
        int i = 0;
        for (vector<vector<uint8_t>>::iterator it = map.begin(); it != map.end(); it++)
        {
            for (vector<uint8_t>::iterator it2 = (*it).begin(); it2 != (*it).end(); it2++)
            {
                if (*it2 == 0xFF)
                    cout << "##";
                else if (*it2 == 0x22)
                    cout << "0 ";
                else
                    cout << ". ";
            }
            cout << " " << i << endl;
            i++;
        }
        cout << "size:" << map.size() << endl;
        cout << "len:" << map[0].size() << endl;
    }

    vector<uint8_t> get_map()
    {
        vector<uint8_t> temp;
        for (vector<vector<uint8_t>>::iterator it = map.begin(); it != map.end(); it++)
        {
            for (vector<uint8_t>::iterator it2 = ((*it).begin()); it2 != (*it).end(); it2++)
            {
                temp.push_back(*it2);
            }
        }
        return temp;
    }

    list<uint8_t> get_map_to_list()
    {
        list<uint8_t> temp;
        for (vector<vector<uint8_t>>::iterator it = map.begin(); it != map.end(); it++)
        {
            for (vector<uint8_t>::iterator it2 = ((*it).begin()); it2 != (*it).end(); it2++)
            {
                temp.push_back(*it2);
            }
        }
        return temp;
    }

    void ncurs_print_map()
    {
        for (int i = 0; i < 82; i++)
            mvprintw(0, i, "==");
        for (int i = 0; i < 82; i++)
            mvprintw(31, i, "==");
        for (int i = 0; i < 32; i++)
            mvprintw(i, 0, "||");
        for (int i = 0; i < 32; i++)
            mvprintw(i, 82, "||%d", i);
        refresh();

        for (int i = 0; i < map.size(); i++)
        {
            for (int j = 0; j < map[i].size() * 2; j += 2)
            {
                string c;
                // if(j/2!=20){
                if (map[i][j / 2] == 0xFF)
                {
                    mvprintw(i + 1, j + 2, "##");
                }
                // if (map[i][j / 2] == 0x22)
                // {
                //     mvprintw(i + 1, j + 2, "0 ");
                // }
                if (map[i][j / 2] == 0xAA)
                {
                    mvprintw(i + 1, j + 2, ". ");
                }
                if (map[i][j / 2] == 0)
                {
                    mvprintw(i + 1, j + 2, "  ");
                }
                // if(j/2==20){mvprintw(i+1,j+2,". ");}
                // }
                // mvprintw(i+1,j+1,c);
                refresh();
            }
        }

        for (int i = 0; i < 41; i++)
        {
            mvprintw(32, 0, "0 1 2 3 4 5 6 7 8 910111213141516171819");
            refresh();
        }
    }

    void show_full_map()
    {
        for (int i = 0; i < map.size(); i++)
        {
            for (int j = 0; j < map[i].size(); j++)
            {
                if (map[i][j] == 0xAA)
                    cout << ". ";
                if (map[i][j] == 0xFF)
                    cout << "##";
                if (map[i][j] == 0x22)
                    cout << "0 ";
                if (map[i][j] == 0x00)
                    cout << "  ";
            }
            cout << endl;
        }
    }
    uint8_t get_pos(Point p)
    {
        if (p.x > N_MAX_X)
        {
            perror_("Too big x");
        }
        if (p.y > N_MAX_Y)
        {
            perror_("Too big y");
        }
        return map[p.y][p.x];
    }
    void set_pos(Point p, uint8_t var, uint32_t *score)
    {
        if (p.x > N_MAX_X)
        {
            perror_("Too big x");
        } // p.x=0;}
        if (p.y > N_MAX_Y)
        {
            perror_("Too big y");
        } // p.y=0;}
        if ((N_MAX_Y > p.y) && (N_MAX_X > p.x))
        {
            // mvprintw(50, 110, "x/y %d/%d", p.x, p.y);
            if (var != 0x22 && var != 0xff && var != 0xAA && var != 0)
            {
                perror_("Worng var");
            }
            try
            {
                if (map[p.y][p.x] == 0xAA)
                {
                    *score += 1;
                }

                map[p.y][p.x] = var;
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }

    void set_pos(Point p, uint32_t val)
    {
        if (p.x > N_MAX_X)
        {
            perror_("Too big x");
        } // p.x=0;}
        if (p.y > N_MAX_Y)
        {
            perror_("Too big y");
        }
        map[p.y][p.x] = val;
    }

    Point find_first_user()
    {
        for (int i = 0; i < map.size(); i++)
        {
            for (int j = 0; j < map[i].size(); j++)
            {
                if (map[i][j] == 0x22)
                {
                    Point temp(j, i);
                    map[i][j] = 0;
                    // perror("FIND");
                    return temp;
                }
            }
        }
        if (errno != 0)
            perror("Wrong map");

        return Point(1, 1);
    }

    bool wall_checker(Point p)
    {
        bool f = 1;
        if (p.x >= N_MAX_X || p.x < 0)
        {
            perror_("Too big x");
            f = false;
        }
        if (p.y >= N_MAX_Y || p.y < 0)
        {
            perror_("Too big y");
            f = false;
        }
        if (f)
        {
            return (map[p.y][p.x] != 0xFF);
        }
        return NULL;
    }
    int i_t = 0;
    bool in_map(Point p)
    {
        // mvprintw(10 + i_t, 90, "%d,%d", p.x, p.y);
        i_t++;
        // if(p.x>=0){return false;}
        if (p.x > N_MAX_X)
        {
            return false;
        }
        // if(p.y==-1){return false;}
        if (p.y > N_MAX_Y)
        {
            return false;
        }
        return true;
    }

    bool is_player(Point p, vector<Player> plaers)
    {
        for (Player i : plaers)
        {
            Point temp(i.get_x(), i.get_y());
            if (p.is_eql(temp))
            {
                return false;
            }
        }
        return true;
    }
};