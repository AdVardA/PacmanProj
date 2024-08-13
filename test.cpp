#include <iostream>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <vector>
#include <list>
#include <cstring>
#include <ctime>

// #include "temp.h"

using namespace std;

int PLAYER_COUNT = 0;
int socket_base[] = {0, 0, 0};

// void printer(Point p){
//     cout<<p.x<<" ";
//     cout<<p.y<<endl;
//     cout<<endl;
// }

void from_32_to_8(list<uint8_t> &vec, uint32_t value)
{
    vec.push_back((value >> 24) & 0xFF);
    vec.push_back((value >> 16) & 0xFF);
    vec.push_back((value >> 8) & 0xFF);
    vec.push_back(value & 0xFF);
}

vector<uint32_t> from_8_to_32(list<uint8_t> &vec)
{
    vector<uint32_t> ans;
    uint32_t val;
    int i = 0;
    for (list<uint8_t>::iterator it = vec.begin(); it != vec.end(); it++)
    {
        if (i < 4)
        {
            val = val << 8;
            val += *it;
            i++;
        }
        else
        {
            ans.push_back(val);
            val = 0;
            i = 0;
        }
    }
    ans.push_back(val);
    return ans;
}

int main()
{
    // uint32_t a, b;
    // a = 0x11234567;
    // b = 0x89ABCDEF;
    // list<uint8_t> l;
    // from_32_to_8(l, a);
    // from_32_to_8(l, b);
    // // memcpy((void *)I, (void *)l, sizeof(uint8_t) * 8);
    // cout << l.size() << endl;

    // for (uint8_t byte : l)
    // {
    //     std::cout << std::hex << static_cast<int>(byte) << " ";
    // }
    // cout << endl;
    // for (uint32_t byte : from_8_to_32(l))
    // {
    //     std::cout << std::hex << static_cast<int>(byte) << " ";
    // }
    // cout << endl;
    // cout << from_8_to_32(l).size() << endl;

    // for (int i = 0; i < 10; i++)
    // {
    //     if (i == 5)
    //         continue;
    //     cout << i << endl;
    // }
    // cout << sizeof(uint32_t) * 6 + 256 * 1 << endl;

    // cout << "\n\n\n\n";

    // vector<int> arr_;
    // for (int i = 0; i < 5; i++)
    //     arr_.push_back(i);
    // vector<int> arr_2(arr_);
    // for (int i : arr_2)
    // {
    //     cout << i;
    // }
    // cout << endl;
    clock_t start = clock();
    cout << start / CLOCKS_PER_SEC << endl;
    int a = 0;
    for (long long int i = 0; i < 100000000; i++)
    {
        a++;
    }
    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;

    return 0;
}