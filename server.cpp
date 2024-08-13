#include <iostream>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include "temp.h"

using namespace std;

int PLAYER_COUNT = 0;
vector <Player> players;
int socket_base[]={0,0,0};

typedef struct p{
        uint32_t magic;
        uint32_t ptype;
        uint32_t datasize;
};




//void *to_client(int ind){
//    for (size_t i = 0; i < PLAYER_COUNT; i++){
//        recv(socket_base[ind], &buffer, buffer.start_packet_size(),0);
//    }
//    
//}

int main(int argc, char * argv[]){
    int port;//=8080;
    char *ip;//= "127.0.0.1";
    PLAYER_COUNT = 1;
    pthread_t pid;
	void *retval;
    if (argc < 3) {
        printf("Usage: ./client -p <port> -a <ip> or ./server -p <port> -a <count>\n");
        exit(0);
    }
    int arg;

    while ((arg = getopt(argc, argv, "p:a:")) != EOF) {
        switch (arg) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'a':
            PLAYER_COUNT = atoi(optarg);
            break;
        }
        
    }
    Player temp_;
    for(int i=0;i<PLAYER_COUNT;i++)
        players.push_back(temp_);
    if(PLAYER_COUNT>5){
        printf("Too many players(%d)\n",PLAYER_COUNT);
        exit(0);
    }
    if(PLAYER_COUNT<1){
        printf("Too few players(%d)\n", PLAYER_COUNT);
        exit(0);
    } 
    cout<<port<<" "<<PLAYER_COUNT<<endl;

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
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

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, PLAYER_COUNT+1) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    Map m;
    for(int i=0;i<PLAYER_COUNT;i++){
        if ((players[i].socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
                perror("accept");
                exit(EXIT_FAILURE);
        }
        cout<<i<<endl;
    }
    cout<<players[0].socket<<endl;
    
    players.resize(PLAYER_COUNT);
    for(int i=0;i<PLAYER_COUNT;i++){
        Packet buffer;
        buffer.read_from_socket(players[i].socket);
        players[i].set_name(buffer.get_data());
    }
    m.showInfo();
    for(int i=0;i<PLAYER_COUNT;i++){
        Packet buffer;
        buffer.set_packet(16, 20*15, m.get_map_to_list());
        buffer.send_to_socket(players[i].socket);
        buffer.show_info();
    }
    
    list <uint8_t> temp;
    temp.merge(m.get_map_to_list());
    int i=0;
        for(list <uint8_t> :: iterator it2 = (temp).begin(); it2!=(temp).end();it2++ ){
            if(i<20){
            i++;
            }
            else{
            i=1;
            cout<<endl;
            }
            if(*it2==0xFF){cout<<"##";}
            if(*it2==0x22){cout<<"0 ";}
            if(*it2==0xAA){cout<<". ";}
        }
    
    cout<<endl;
    Map n;
    n.map_init(m.get_map_to_list());
    
    return 0;
}