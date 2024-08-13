#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;

#define H  16
#define W  21
#define MAX_WALL_COUNT 117
#define LOGIC_POINT 2



int map[H][W]={0};

void map_printer_left_high(){
    for(int i=0;i<H;i++){
        for(int j=0;j<W;j++){
            if(map[i][j])
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<"#";
            else
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<" ";
        }
        cout<<endl;
    }
}

void map_printer_left_low(){
    for(int i=H-1;i>-1;i--){
        for(int j=0;j<W;j++){
            if(map[i][j])
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<"#";
            else
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<" ";
        }
        cout<<endl;
    }
}

void map_printer_right_low(){
    for(int i=H-1;i>-1;i--){
        for(int j=W-1;j>-1;j--){
            if(map[i][j])
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<"#";
            else
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<" ";
        }
        cout<<endl;
    }
}

void map_printer_right_high(){
    for(int i=0;i<H;i++){
        for(int j=W-1;j>-1;j--){
            if(map[i][j])
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<"#";
            else
            for(int l=0;l<LOGIC_POINT;l++)
                cout<<" ";
        }
        cout<<endl;
    }
}

int map_sum(){
    int sum = 0;
    for(int i=1;i<H;i++){
        for(int j=1;j<W;j++){
            sum+=map[i][j];
        }
    }
    return sum;
}

int main(){
    srand(time(NULL));
    for(int i=0;i<W;i++){
        map[0][i] = 1;
    }
    for(int i=0;i<H;i++){
        map[i][0] = 1;
    }

    //map_printer();

    int wall_count;
    while((wall_count = (rand())%200 + 50)>MAX_WALL_COUNT);

    while(map_sum()!=wall_count){
        int x;
        int y;
        x= (rand()%(W-2))+1;
        y= (rand()%(H-2))+1;
        cout<<endl;
        cout<<x<<" "<<y<<endl;
        //for(int i=y-1;i<x+2;i++){
            //for(int j=x-1;j<y+2;j++){
                if((y>1) && (y<H) && (x>1) && (x<W)){
                    map[y][x] = 1;
                }
           // }
        //}
        //map_printer();
        //usleep(3000000);
    }
        //map_printer_left_high();
        cout<<endl;

    for(int i = 1;i<H-1;i++){
        for(int j=1;j<W-1;j++){
            if(!map[i][j]){
            int sum = map[i-1][j]+map[i][j-1] + map[i+1][j] +map[i][j+1];
                if(sum>2){
                    if(sum==3){
                        if(map[i+1][j])
                            map[i+1][j]=0;
                        else{
                            map[i][j+1]=0;
                        }
                    }
                    if(sum==4){
                        map[i+1][j]=0;
                        map[i][j+1]=0;
                    }
                }
            }
            
        }
    }
    map_printer_left_high();
    map_printer_left_low();
            cout<<endl;

    map_printer_right_high();
    map_printer_right_low();

    return 0; 
}