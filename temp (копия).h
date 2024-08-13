#include <unistd.h>
#include <string.h>
#include <list>
#include <iterator>
#include <vector>
#include <ncurses.h>
#include <pthread.h>

#define MAX_PLAYER_COUNT 4
#define N_MAX_X 82
#define N_MAX_Y 31

using namespace std;

class Point{
    public:
        uint32_t x;
        uint32_t y;
    Point(){
        x=0;y=0;
    }
    Point(int x, int y){
        this->x=x;
        this->y=y;
    }
    void point_copy(Point p){
        this->x=p.x;
        this->y=p.y;
    }
};

class Player{
    uint32_t start_x;
    uint32_t start_y;
    uint32_t player_name_len;
    uint32_t direction;
    list <uint8_t> name;
    public:
        int socket;
        Player(){
            start_x =0;
            start_y = 0;
            player_name_len =0;
            direction = 4;
            socket = -1;
        }

        int set_socket(int sd){ this->socket = sd; return sd;}
        int get_socket(){return this->socket;}

        bool player_init_check(){
            if(start_x>N_MAX_X){perror("Wrong start_x for player");return 0;}
            if(start_y>N_MAX_Y){perror("Wrong start_y for player");return 0;}
            if(player_name_len < 0 ){perror("player_name_len must be > 0");return 0;}
            if(player_name_len>256){perror("player_name_len must be <= 256");return 0;}
            if(direction<0){perror("Direction must be >= 0");return 0;}
            if(direction>3){perror("Direction must be <= 3");return 0;}
            int temp = 0;
            for(list <uint8_t> :: iterator it = name.begin(); it != name.end(); it++){
                temp += *it;
                if((*it>0)&&(*it<32)){*it=32;}
            }
            if(temp == 0){perror("Name must contein not only 0(Ptype = 1)"); return 0;}
            return 1;
        }

        bool player_constructor(uint32_t x, uint32_t y, uint32_t name_len, uint32_t dir, list <uint8_t> name){
            start_x =x;
            start_y = y;
            player_name_len =name_len;
            direction = dir;
            this->name.merge(name);
            return player_init_check();
        }
        void set_name(list <uint8_t> name){
            this->name = name;
        }
        void show_info(){
            cout<<"x:"<<start_x<<endl;
            cout<<"y:"<<start_y<<endl;
            cout<<"player_name_len:"<<player_name_len<<endl;
            cout<<"direction"<<direction<<endl;
            for(list <uint8_t> :: iterator it = name.begin();it!=name.end();it++){
                cout<<*it;
            }
            cout<<endl;
        }
        void show_info_ncurses(){
            int x=90, y=0;
            mvprintw(y,x,"x:%d",start_x);
            mvprintw(++y,x,"y:%d",start_y);
            mvprintw(++y,x,"direction:%d",direction);
            refresh();
        }

        void show_info_ncurses(int y){
            int x=90;
            mvprintw(y,x,"x:%d",start_x);
            mvprintw(++y,x,"y:%d",start_y);
            mvprintw(++y,x,"direction:%d",direction);
            refresh();
        }

        uint32_t get_x(){return start_x;}
        uint32_t get_y(){return start_y;}
        void set_x(uint32_t x){
            if(x>80){perror("Too big x");}
        //if(y>30){perror("Too big y");}
            start_x = x;
        }
        void set_y(uint32_t y){
            if(y>31){perror("Too big y");}
            if(y>0 && 31>y)
                start_y = y;
        }
        void player_printer(){
            mvprintw(start_y,start_x,"A ");
        }
        uint32_t get_dir(){
            return direction;
        }
        void set_dir(uint32_t dir){
            if(dir != 0 && dir != 1 && dir != 2 && dir != 3){
                perror("Wrong dir");
            }
            direction = dir;
        }
};

class Msg{
    uint32_t player_count;
    list <Player> player;
    uint32_t frame_timeout;
    public:
    Msg(){
        player_count=0;
        frame_timeout =0;
    }
    bool msg_check(){
        if(frame_timeout == 0){perror("frame_timeout != 0"); return 0;}
        if(player_count > MAX_PLAYER_COUNT){perror("too much palyers"); return 0;}
        for( list <Player> :: iterator it = player.begin(); it != player.end(); it++){
            if((*it).player_init_check()){perror("Player"); return 0;}
        }
        return 1;
    }
    

};

class Packet{
    uint32_t magic;
    uint32_t ptype;
    uint32_t datasize;
    list <uint8_t> data;
    bool flag_init;
    public:

    Packet(){
        magic=0;
        ptype=0;
        datasize = 0;
        list <uint8_t> t;
    }

    bool magic_check(){
        return (magic==0xabcdef);
    }

    void read_from_socket(int socket){
        this->data.resize(0);

        if(read(socket, &magic, sizeof(uint32_t))==-1)
            perror("magic");
        if(magic_check()){
            if(read(socket, &ptype, sizeof(uint32_t))==-1)
                perror("ptype");
            if(read(socket, &datasize, sizeof(uint32_t))==-1)
                perror("datasize");
            if(ptype!=0x20)
                for(int i =0;i<datasize;i++){
                    uint8_t temp;
                    if(read(socket, &temp, sizeof(uint8_t))==-1)
                        perror("data");
                    //if(temp==255){cout<<1<<endl;}
                    //if(temp==0xAA){cout<<0<<endl;}
                    data.push_back(temp);
            }
            flag_init = true;
        }
        else{
            perror("Error: wrong pacekt");
        }
    }

    void send_to_socket(int sock){
        //if(send(sock, &magic, sizeof(uint32_t),0) == -1)
        //    perror("magic");
        //if(send(sock, &ptype, sizeof(uint32_t),0) == -1)
        //    perror("ptype");
        //if(send(sock, &datasize, sizeof(uint32_t),0) == -1)
        //    perror("datasize");
        //for(list <uint8_t> :: iterator it = data.begin(); it != data.end(); it++){
        //    if(send(sock, &(*it), sizeof(uint8_t),0) == -1)
        //        perror("data");
        //}
    }  
    Packet(uint32_t ptype, uint32_t datasize, list <uint8_t> data){
        this->magic=0xabcdef;
        this->ptype=ptype;
        this->datasize = datasize;
        this->data.merge(data);

    }
    void set_packet(uint32_t ptype, uint32_t datasize, list <uint8_t> data){
        this->magic=0xabcdef;
        this->ptype=ptype;
        this->datasize = datasize;
        this->data.resize(0);
        this->data.merge(data);
        if(ptype==1)
        (this->data).resize(256,0);
    }
    int start_packet_size(){
        return sizeof(uint32_t)*3;
    }
    void show_info(){
        cout<<"magic: "<<magic<<endl;
        cout<<"ptype: "<<ptype<<endl;
        cout<<"datasize: "<<datasize<<endl;
        //cout << strlen((char*)data) << endl;
        if(!data.empty()){
        //copy(data.begin(), data.end(), ostream_iterator(cout, " ")); cout << endl;
        for(list <uint8_t> :: iterator it = data.begin(); it != data.end(); it++){
            if(ptype!=1)
            cout<< (*it) << " ";
            else
            cout<< (*it);
        }
        cout << endl;
        }
        else{
            cout<< "List is empty"<<endl;
        }
    }
    void rand_filds(){
        magic=rand()%100;
        ptype=rand()%100;
        datasize = 4;
        for(int i=0; i<datasize;i++)
            data.push_back(48+i);
    }

    bool type_check(){
        
        if(ptype==1){
            if(datasize<0){perror("Datasize must be < 0(Ptype = 1)"); return 0;}
            if(datasize>256){perror("Datasize must be >= 256(Ptype = 1)"); return 0;}
            int temp=0;
            for(list <uint8_t> :: iterator it = data.begin(); it != data.end(); it++){
                temp+=*it;
                if((*it>0)&&(*it<32)){*it=32;}
            }
            if(temp == 0){perror("Data must contein not only 0(Ptype = 1)"); return 0;}

            return 1;
        }

        else if (ptype==16){
            if(datasize != 20*15){perror("Wrong datasize(Ptype = 16)"); return 0;}
            for(list <uint8_t> :: iterator it = data.begin(); it != data.end(); it++){
                if((*it!=0xFF)&&(*it!=0xAA)&&(*it!=0xAA)){perror("Wrong object(Ptype = 16)"); return 0;}
            }
        }
        
    }

    list <uint8_t> get_data(){
        return data;
    }
    
};

#define H  15
#define W  20
#define MAX_WALL_COUNT 117
#define LOGIC_POINT 2

class Map{
    private:
        vector <vector <uint8_t>> map;
    public:
    Map(){
        srand(time(NULL));
        vector <uint8_t> temp;
        temp.resize(20,0xAA);
        map.resize(15,temp);

        int wall_count;
        while((wall_count = (rand())%200 + 50)>MAX_WALL_COUNT);
        

        int i=0;

        while(i!=wall_count){
            int x;
            int y;
            x= (rand()%(W-3))+2;
            y= (rand()%(H-3))+2;
            map[y][x] = 0xFF;
            i++;
        }

        for(int i = 1;i<H-1;i++){
        for(int j=1;j<W-1;j++){
            if(map[i][j]==0xAA){
            int sum = 0;
            if(map[i-1][j]==0xFF)sum+=1;
            if(map[i][j-1]==0xFF)sum+=1;
            if(map[i+1][j]==0xFF)sum+=1;
            if(map[i][j+1]==0xFF)sum+=1;
                if(sum>2){
                    if(sum==3){
                        if(map[i+1][j])
                            map[i+1][j]=0xAA;
                        else{
                            map[i][j+1]=0xAA;
                        }
                    }
                    if(sum==4){
                        map[i+1][j]=0xAA;
                        map[i][j+1]=0xAA;
                    }
                }
            }
            
        }
    }
        bool pers=true;
        while(pers){
            int x=rand()%20;
            int y=rand()%15;
            if(map[y][x]!=0xff){
                map[y][x] = 0x22;
                pers=false;
            }
        }

        //for(list <list <int>> :: iterator it = map.begin(); it != data.end(); it++)
    }

    void map_init(list <uint8_t> data){
        copy_vector(data);
        showInfo();
        for(int i = map.size()-1;i>-1;i--){
            map.push_back(map[i]);
        }
        
        
        //for(vector <vector<int>> :: iterator it = (map.begin()); it != map.end(); it++){
        //    for(vector <int> :: iterator it2 = ((*it).end()); it2 != (*it).begin(); it2--){
        //        (*it).push_back(*it2);
        //    }
        //}
        for(int i=0; i < map.size(); i++){
            for(int j=map[i].size(); j > -1; j--){
                map[i].push_back(map[i][j]);
            }
            vector <uint8_t> :: iterator it = map[i].begin();
            for(int i=0;i<20;i++)
                it++;
            map[i].erase(it);
        }
    }

    vector <uint8_t> revers(vector <uint8_t> data){
        vector <uint8_t> temp;
        for(vector <uint8_t> :: iterator it = (data.end()); it != data.begin(); it--){
            temp.push_back(*it);
        }
        return temp;
    }


    void copy_vector(list <uint8_t> data){
        vector <uint8_t> temp;
        int i=0;
        map.resize(0);
        for(list <uint8_t> :: iterator it = (data.begin()); it != data.end(); it++){
            if(temp.size()!=20)
                temp.push_back(*it);
            else{
                map.push_back(temp);
                temp.resize(1,*it);
                i++;
            }
        }
        map.push_back(temp);
        cout<<i<<endl;
        
    }

    int map_sum(){
        int sum=0;
        for(vector <vector <uint8_t>> :: iterator it = (map.begin()); it != map.end(); it++){
            if(it == map.begin())
                advance(it, 1);
            for(vector <uint8_t> :: iterator it2 = ((*it).begin()); it2 != (*it).end(); it2++){
                if(it2 == (*it).begin())
                advance(it2, 1);
                sum+=*it2;
            }
        }
        cout<<sum<<endl;
        return sum;
    }

    void showInfo(){
        int i=0;
        for(vector <vector <uint8_t>> :: iterator it = map.begin(); it != map.end(); it++){
            for(vector <uint8_t> :: iterator it2 = (*it).begin(); it2 != (*it).end(); it2++){
                if(*it2 == 0xFF)
                cout<<"##";
                else if (*it2 == 0x22)                
                    cout<<"0 ";
                else
                    cout<<". ";
            }
            cout<<" "<<i<<endl;
            i++;
        }
        cout<<"size:"<<map.size()<<endl;
        cout<<"len:"<<map[0].size()<<endl;
    }

    vector <uint8_t> get_map(){
        vector <uint8_t> temp;
        for(vector <vector <uint8_t>> ::iterator it = map.begin(); it!=map.end();it++){
            for(vector <uint8_t> :: iterator it2 = ((*it).begin()); it2 != (*it).end(); it2++){
                temp.push_back(*it2);
            }
        }
        return temp;
    }

    list <uint8_t> get_map_to_list(){
        list <uint8_t> temp;
        for(vector <vector <uint8_t>> ::iterator it = map.begin(); it!=map.end();it++){
            for(vector <uint8_t> :: iterator it2 = ((*it).begin()); it2 != (*it).end(); it2++){
                temp.push_back(*it2);
            }
        }
        return temp;
    }

    void ncurs_print_map(){
        for(int i=0; i<N_MAX_X;i++)
            mvprintw(0,i,"==");
        for(int i=0;i<N_MAX_X;i++)
            mvprintw(31,i,"==");
        for(int i=0;i<32;i++)
            mvprintw(i,0,"||");
        for(int i=0;i<32;i++)
            mvprintw(i,N_MAX_X,"||%d",i);
            refresh();

        for(int i=0;i<map.size();i++){
            for(int j =0; j<map[i].size()*2;j+=2){
                string c;
                //if(j/2!=20){
                if(map[i][j/2]==0xFF){mvprintw(i+1,j+2,"##");}
                if(map[i][j/2]==0x22){mvprintw(i+1,j+2,"0 ");}
                if(map[i][j/2]==0xAA){mvprintw(i+1,j+2,". ");}
                if(map[i][j/2]==0){mvprintw(i+1,j+2,"  ");}
                //if(j/2==20){mvprintw(i+1,j+2,". ");}
                //}
                //mvprintw(i+1,j+1,c);
                refresh();
            }
        }
        
        for(int i=0;i<41;i++){
            mvprintw(32,0,"0 1 2 3 4 5 6 7 8 910111213141516171819");
            refresh();
        }
    }

    uint8_t get_pos(unsigned int x, unsigned int  y){
        if(x>40){perror("Too big x");}
        if(y>31){perror("Too big y");}
        return map[y][x];
    }
    void set_pos(unsigned int x, unsigned int  y, uint8_t var){
        if(x>40){perror("Too big x");}
        if(y>31){perror("Too big y");}
        if(y>0 && 31>y && x>1 && 31>x)
        if(var != 0x22 && var != 0xff && var != 0xAA && var != 0){perror("Worng var");}
        map[y][x] = var;
    }

    Point find_first_user(){
        for(int i=0;i<map.size();i++){
            for(int j=0;j<map[i].size();j++){
                if(map[i][j]==0x22){
                    Point temp(j,i);
                    perror("FIND");
                    return temp;
                }
            }
        }
        perror("Wrong map");
        return Point(1, 1);
    }

    bool wall_checker(uint32_t x,uint32_t y){
        if(x>N_MAX_X){perror("Too big x");}
        if(y>31){perror("Too big y");}
        return (map[y-1][x/2]!=0xFF);
    }
    int i_t=0;
    bool in_map(uint32_t x,uint32_t y){
        mvprintw(10+i_t,90,"%d,%d",x,y);
        i_t++;
        if(x==0){return false;}
        if(x>N_MAX_X){return false;}
        if(y==0){return false;}
        if(y>31){return false;}
        return true;
    }

};
