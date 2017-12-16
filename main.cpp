#include <iostream>
#include <linux/input.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <sys/time.h>

using namespace std;

//Keeping track of presses.
struct PressRecord {
    bool pressed = false;
    struct timeval pressTime;
    string name = "Unknown";
    PressRecord (string name) : name(name) {}
    PressRecord(){}
};
struct TouchRecord {
    bool pressed = false;
    struct timeval pressTime;
    string name = "Touch";
    int initX = -1;
    int initY = -1;
    int x = -1;
    int y = -1;
    bool needX = true;
    bool needY = true;
    TouchRecord (string name) : name(name) {}
    TouchRecord(){}
};


int main()
{
    // Mapping the correct button IDs.
    unordered_map<int, PressRecord> map;
    map[105] = PressRecord("Left");
    map[102] = PressRecord("Middle");
    map[106] = PressRecord("Right");
    map[116] = PressRecord("Power");

    TouchRecord swipe;

    // Open the touch device.
    ifstream eventsfile;
    eventsfile.open("/dev/input/event1", ios::in);
    if(eventsfile.is_open()) {
        cout << "File open for reading." << endl;
    }
    else {
        cout << "File couldn't be opened." << endl;
        return 0;
    }


    // Open the buttons device.
    ofstream buttonfile;
    buttonfile.open("/dev/input/event2", ios::out);
    if(buttonfile.is_open()) {
        cout << "File open for writing." << endl;
    }
    else {
        cout << "File couldn't be opened." << endl;
        return 0;
    }

    // Get the size of an input event in the right format!
    input_event ie;
    streamsize sie = static_cast<streamsize>(sizeof(struct input_event));

    timeval tadd{300000};

    input_event button;
    input_event empty{0};

    while(eventsfile.read((char*)&ie, sie)) {
        switch(ie.code) {
        case ABS_MT_POSITION_X :
            // horizontal distance from RHS
            if(swipe.needX) {
                swipe.initX = ie.value;
                swipe.needX = false;
            }
            else swipe.x = ie.value;

            //cout << "X Value " << ie.value << endl;
            break;

        case ABS_MT_POSITION_Y:
            // vertical distance from bottom
            if(swipe.needY) {
                swipe.initY = ie.value;
                swipe.needY = false;
            }
            else swipe.y = ie.value;

            //cout << "Y Value " << ie.value << endl;
            break;

        case 57:
        {
            // release
            struct timeval ctime;
            gettimeofday(&ctime,NULL);

            // Calculate length of hold
            long usecs = ((ctime.tv_sec   -  swipe.pressTime.tv_sec  )*1000000L
                          +ctime.tv_usec) -  swipe.pressTime.tv_usec;

            cout << swipe.initX << " " << swipe.initY << " " << swipe.x << " " << swipe.y << " " << usecs << endl;

            if(usecs < 1000000L && swipe.initX < 400 && swipe.x > 400) {
                cout << "PING" << endl;
                button.code = 106;
                gettimeofday(&button.time, NULL);
                button.type = 1;
                button.value = 1;
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);
                buttonfile.flush();

                usleep(20000);

                gettimeofday(&button.time, NULL);
                button.value = 0;
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);
                buttonfile.flush();
            }

            if(usecs < 1000000L && swipe.initX > 400 && swipe.x <=400) {
                cout << "PONG" << endl;
                button.code = 105;
                gettimeofday(&button.time, NULL);
                button.type = 1;
                button.value = 1;
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);
                buttonfile.flush();

                usleep(20000);

                gettimeofday(&button.time, NULL);
                button.value = 0;
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);
                buttonfile.flush();
            }

            swipe.x = swipe.y = swipe.initX = swipe.initY = -1;
            swipe.needX = swipe.needY = true;
            gettimeofday(&swipe.pressTime,NULL);

            break;
        }
        case 0:
            //cout << endl;
            break;

        default:
            //cout << "Code " << ie.code << "  Type " << ie.type << "  Value " << ie.value << endl;
            break;
        }
        /*
        // Read for non-zero event codes.
        if(ie.code != 0) {

            // Toggle the button state.
            map[ie.code].pressed = !map[ie.code].pressed;

            // On press
            if(map[ie.code].pressed) {
                gettimeofday(&map[ie.code].pressTime,NULL);
                cout << map[ie.code].name << " " << "DOWN" << endl;
            }

            // On release
            else {
                struct timeval ctime;
                gettimeofday(&ctime,NULL);

                // Calculate length of hold
                long usecs = ((ctime.tv_sec   -  map[ie.code].pressTime.tv_sec  )*1000000L
                              +ctime.tv_usec) -  map[ie.code].pressTime.tv_usec;

                // Print out press information
                cout << map[ie.code].name << " " << "UP (" << usecs << " microseconds)" << endl;

                // Check if MIDDLE was held for > 1 second
                if(map[ie.code].name == "Middle" && usecs > 1000000L) {
                    ifstream termfile;
                    // Then execute the contents of /etc/draft/.terminate
                    termfile.open("/etc/draft/.terminate", ios::in);
                    if(termfile.is_open()) {
                        cout << "Termfile exists and can be read." << endl;
                        termfile.close();
                        system("/bin/bash /etc/draft/.terminate");
                    }
                    else {
                        cout << "Termfile couldn't be read." << endl;
                    }

                }

            }
        }
            */
    }

    eventsfile.close();
    buttonfile.close();

    return 0;
}
