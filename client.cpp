#include <iostream>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include "temp.h"


using namespace std;


int PLAYER_COUNT = 0;

//class Packet{
//    public:
//        uint32_t magic;
//        uint32_t ptype;
//        uint32_t datasize;
//        uint8_t *data;
//
//
//    Packet(){
//        magic=rand()%100;
//        ptype=rand()%100;
//        datasize = rand()%100;
//        data = NULL;
//    }
//    int start_packet_size(){
//        return sizeof(uint32_t)*3;
//    }
//    void show_info(){
//        cout<<"magic: "<<magic<<endl;
//        cout<<"ptype: "<<ptype<<endl;
//        cout<<"data: "<<datasize<<endl;
//        //for(int i=0;i<datasize;i++){
//        //    cout<<i<<":"<<data[i]<<endl;
//        //}
//    }
//    void data_gen(){
//        uint8_t * t= (uint8_t*) malloc(datasize*sizeof(uint8_t));
//        for(int i=0;i<datasize;i++){
//            t[i]=rand()%128;
//        }
//        data = t;
//    }
//};

typedef struct p{
        uint32_t magic;
        uint32_t ptype;
        uint32_t datasize;
};


int main(int argc, char * argv[]){
    srand(time(NULL));
    int port;
    char *ip;
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
            ip = optarg;
            break;
        }
    }

    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    printf("%d",port);
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        perror("invalid address / address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    Packet buffer;

    //send(sock, &buffer, sizeof(buffer.start_packet_size()), 0);
    //perror("1");
    //buffer.data_gen();
    //perror("2");
    //send(sock, &buffer.data, sizeof(buffer.data),0);
    //perror("3");
    //buffer.show_info();
    //perror("4");
    //cout<<endl;
    //read(sock, &buffer, sizeof(buffer.start_packet_size()));
    //perror("5");
    //read(sock, &buffer.data, sizeof(buffer.data));
    //perror("6");

    //buffer.show_info();
    //buffer.show_info();
    //perror("2");
    //send(sock, &buffer, buffer.start_packet_size(), 0);
    //buffer.show_info();
    //p p1;
    //p1.magic = 1;
    //p1.datasize = 3;
    //p1.ptype = 2;
    uint32_t ptype=1;
    uint32_t datasize=256;
    list <uint8_t> data;

    for(int i=0;i<rand()%256;i++){
        data.push_back(rand()%(128-48)+48);
    }
    buffer.set_packet(ptype,datasize, data);
    buffer.show_info();
    buffer.send_to_socket(sock);
    buffer.read_from_socket(sock);
    buffer.show_info();
    Map m;
    m.map_init(buffer.get_data());
    m.showInfo();


return 0;
}