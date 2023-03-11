/**
 * 描述：压力测试客户端
 * 作者：guchenfeng
 * 日期：2023/1/17
 * 
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include<fstream>
#include<iostream>
#include<vector>

using namespace std;

//处理消息接收线程
void *handle_recv(void *arg) {
    int sock = *(int *)arg;
    while (1) {
        char recv_buffer[1000];
        memset(recv_buffer, 0, sizeof(recv_buffer));
        int len = recv(sock, recv_buffer, sizeof(recv_buffer), 0);
        if (len ==  0)
            continue;
        if (len ==  -1)
            break;
        string str(recv_buffer);
        cout << str << endl;
    }
    return NULL;
}

//处理消息发送线程
void *handle_send(void *arg) {
    int sock = *(int *)arg;
    while (1) {
        string str;
        //cin >> str;
        getline(cin, str);
        if (str == "exit")
            break;
        if (str.empty())
            continue;
        str = "content:" + str;
        send(sock, str.c_str(), str.length(), 0);
       
    }   
    return NULL;
}

int main(int argc,char * argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("121.37.4.159");
    serv_addr.sin_port = htons(8045);

    //把账号信息读进来，用于压力测试
    vector<string> name_arr;
    ifstream in("account.txt");
    while(!in.eof()){
        string tmp;
        in >> tmp;
        name_arr.push_back(tmp);
    }
    in.close();

    cout << "[测试开始]" << endl;
    clock_t start_clock, end_clock;
    //测试开始时间
    start_clock = clock();
    int link_num = 0;
    //压力测试，建立若干个到服务器的连接
    vector<int> sock_arr;
    int total_test = atoi(argv[1]);
    for(int i = 0; i < total_test; i ++ ) {
        usleep(10000);
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
            //cout<< "第" << i << "个连接connect() error" << endl;
        } else{ //连接成功
            //cout<< "第" << i << "个连接成功建立,套接字描述符为" << sock << endl;
            link_num ++ ;
            sock_arr.push_back(sock);
        }   
    }
    cout << "连接结束" << endl;

    string name, pass, pass1;
    bool if_login = false;//记录是否登录成功
    string login_name;//记录成功登录的用户名

    //压力测试登录
    srand(time(NULL));
    int total_name = name_arr.size();
    int login_num = 0; //记录登录数量

    //测试登录数量
    for(int i = 0; i < sock_arr.size(); i ++ ) {
        //登录
        string name = name_arr[i], pass = name;
        string str = "login" + name + "pass:" + pass;
        send(sock_arr[i], str.c_str(), str.length(), 0);
        char buffer[1000];
        memset(buffer, 0, sizeof(buffer));
        recv(sock_arr[i], buffer, sizeof(buffer), 0);//接收响应
        string recv_str(buffer);
        if(recv_str.substr(0,2) == "ok"){
            login_num ++ ;
            //cout<<"第"<<i<<"个测试登录成功,套接字描述符为"<<sock_arr[i]<<endl;
        }

    }
    //测试结束时间
    end_clock = clock();
    double seconds = (double)(end_clock - start_clock) / CLOCKS_PER_SEC ;
    cout << "测试完成,共用时" << seconds << "s!" << endl;
    cout << "请求建立" << total_test << "个连接, 成功建立" << link_num 
            << "个连接， 失败" << total_test - link_num << "个！" << endl; 
    cout << "平均每秒处理" << link_num / seconds << "个连接请求!" << endl;
    cout << "请求登录个数为：" << total_test << "个, 成功登录" << login_num 
            << "个， 失败" << total_test - login_num << "个！" << endl; 
    //关闭套接字
    for(auto &i : sock_arr)
        close(i);
}
