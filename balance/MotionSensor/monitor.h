/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Monitor.h
 * Author: John
 *
 * Created on 06 March 2016, 06:51
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>
#include <string>
#include <vector>
#include <map>
#include <climits>
#include <arpa/inet.h>
#include <sys/socket.h>


struct Watchvar {
    int * ref;
    int oldval;
    std::string name;
};

struct Controlvar {
    int * ref;
    int oldval;
    int maxval;
    int minval;
};

class Monitor {
public:
    Monitor();
    virtual ~Monitor();
    
    void watch(int * watchvar, std::string name);
    void control(int * controlvar, std::string name, int maxval = INT_MAX, int minval = INT_MIN);

private:
    pthread_t listenerThread;
    pthread_t watchThread;
    static int newSocket();
    static void * listener(void * param);
    static void * watchLoop(void * param);
    static void die(std::string s);
    static bool running;
    static std::vector<Watchvar> watchvars;
    static std::map<std::string, Controlvar> controlvars;
    static struct sockaddr_in socket_other;
    static socklen_t slen;
};

#endif /* MONITOR_H */

