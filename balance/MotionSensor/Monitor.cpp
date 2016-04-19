/* 
 * File:   Monitor.cpp
 * Author: John
 * 
 * Created on 06 March 2016, 06:51
 */

#include "Monitor.h"
#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
#include <climits>
#include <fstream>
#include <unistd.h> //close
#include <wiringPi.h>
#include "json/json.hpp"

using json = nlohmann::json;

#define BUFLEN 512  //Max length of buffer
#define PORT 25045 // = "JBP" in base 36!

bool Monitor::running;
std::vector<Watchvar> Monitor::watchvars;
std::map<std::string, Controlvar> Monitor::controlvars;
struct sockaddr_in Monitor::socket_other;
socklen_t Monitor::slen = sizeof (socket_other);

Monitor::Monitor() {
    running = true;
    socket_other.sin_addr.s_addr = 0;
    
    static int sock = newSocket();
    pthread_create(&listenerThread, NULL, listener, (void *)&sock);
    pthread_create(&watchThread, NULL, watchLoop, (void *)&sock);
}

Monitor::~Monitor() {
    running = false;
    pthread_join(listenerThread, NULL);
}

void Monitor::watch(int * varref, std::string name) 
{
    Watchvar newwatch;
    
    newwatch.ref = varref;
    newwatch.name = name;
    newwatch.oldval = *varref;
    
    watchvars.push_back(newwatch);
}

void Monitor::control(int * varref, std::string name, int maxval, int minval)
{
    Controlvar newcontrol;

    //watch(varref, name);
    
    newcontrol.ref = varref;
    newcontrol.maxval = maxval;
    newcontrol.minval = minval;
    newcontrol.oldval = 0;
    controlvars[name] = newcontrol;
}

int Monitor::newSocket()
{
    struct sockaddr_in socket_me;
    int socket_fd;
    int option = 1;

    //create a UDP socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd == -1) {
        die("socket");
    }

    // zero out the structure
    memset((char *) &socket_me, 0, sizeof (socket_me));

    socket_me.sin_family = AF_INET;
    socket_me.sin_port = htons(PORT);
    socket_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if (bind(socket_fd, (struct sockaddr*) &socket_me, sizeof (socket_me)) == -1) {
        die("bind");
    }
    
    return socket_fd;
}


//======= Thread to watch variables for changes

#define UPDATEINTERVAL 200

void * Monitor::watchLoop(void * param)
{
    json j;
    bool dataToSend;
    unsigned int loopcount = 0;
    int watchSocket_fd = *((int*) param);
   
    while (running)
    {
        usleep(2000);  // loop once per 2ms
        loopcount++;
        j.clear();
        dataToSend = false;
        j["millis"] = millis();
        int len = watchvars.size();
        for (int i = 0; i < len; i++) {
            if ((loopcount % UPDATEINTERVAL) == 0 || watchvars[i].oldval != *watchvars[i].ref) {
                // Add the watched variable to the report
                j["variables"][watchvars[i].name] = *watchvars[i].ref;
                watchvars[i].oldval = *watchvars[i].ref;
                dataToSend = true;
            }
        }
        if (dataToSend) {
            std::string dataout(j.dump());
            if (socket_other.sin_addr.s_addr != 0) {
                if (sendto(watchSocket_fd, dataout.c_str(), dataout.length(), 0, (struct sockaddr *) &socket_other, slen) == -1) {
                    die("sendto()");
                }
            }
        }
    }
    close(watchSocket_fd);
    return 0;

}

//===== Thread to listen for incoming requests, including pings

void Monitor::die(std::string s)
{
    std::cout << std::endl << "Error : " << s << " : " << errno << std::endl;
    running = false;
}

void * Monitor::listener(void * param) {
    
    int socket_fd = *((int*) param);
    int recv_len;
    char buf[BUFLEN];
    json j;

    //keep listening for data
    while (running) {

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(socket_fd, buf, BUFLEN, 0, (struct sockaddr *) &socket_other, &slen)) == -1) {
            die("recvfrom()");
        }
        // Parse out the JSON
        std::string datain(buf, recv_len);
        j = json::parse(datain);

        // Look for any "set"s of control variables
        if (j.count("set") > 0) {
            for (json::iterator it = j["set"].begin(); it != j["set"].end(); ++it) {
                if (controlvars.count(it.key()) > 0) {
                    *(controlvars[it.key()].ref) = it.value();
                }
            }
        }

        // Send back the current set of control variables
        for (std::map<std::string, Controlvar>::iterator it = controlvars.begin(); it != controlvars.end(); ++it) {
            j["control"][it->first]["min"] = it->second.minval;
            j["control"][it->first]["max"] = it->second.maxval;
            j["control"][it->first]["val"] = *(it->second.ref);
        }
        std::string dataout(j.dump());

        //Reply to the client
        if (sendto(socket_fd, dataout.c_str(), dataout.length(), 0, (struct sockaddr*) &socket_other, slen) == -1) {
            die("sendto()");
        }
    }

    close(socket_fd);
    return 0;
}
