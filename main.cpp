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

// Longest time considered a "swipe"
long int holdLength = 1000000L;


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
            // Reported horizontal distance from right
            if(swipe.needX) {
                swipe.initX = ie.value;
                swipe.needX = false;
            }
            else swipe.x = ie.value;

            break;

        case ABS_MT_POSITION_Y:
            // Reported vertical distance from bottom
            if(swipe.needY) {
                swipe.initY = ie.value;
                swipe.needY = false;
            }
            else swipe.y = ie.value;

            break;

        case 57:
        {
            // release
            struct timeval ctime;
            gettimeofday(&ctime,NULL);

            // Calculate length of hold
            long usecs = ((ctime.tv_sec   -  swipe.pressTime.tv_sec  )*holdLength
                          +ctime.tv_usec) -  swipe.pressTime.tv_usec;


            if(usecs < holdLength && swipe.initX < 400 && swipe.x > 400) {
                cout << "PING" << endl;
                // Write a RIGHT keypress to the button file (106)
                button.code = 106;
                gettimeofday(&button.time, NULL);
                button.type = 1;
                button.value = 1;
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);

                usleep(20000);

                // Write a RIGHT keyrelease to the button file (106)
                gettimeofday(&button.time, NULL);
                button.value = 0;
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);
            }

            if(usecs < holdLength && swipe.initX > 400 && swipe.x <=400) {
                cout << "PONG" << endl;
                button.code = 105;
                gettimeofday(&button.time, NULL);
                button.type = 1;
                button.value = 1;
                // Write a LEFT keypress to the button file (106)
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);

                usleep(20000);

                // Write a LEFT keyrelease to the button file (106)
                gettimeofday(&button.time, NULL);
                button.value = 0;
                buttonfile.write((char*)&button, sie);
                buttonfile.write((char*)&empty, sie);
            }

            // Unset the current swipe data.
            swipe.x = swipe.y = swipe.initX = swipe.initY = -1;
            swipe.needX = swipe.needY = true;
            gettimeofday(&swipe.pressTime,NULL);

            break;
        }
        case 0:
        default: break;
        }

    }

    eventsfile.close();
    buttonfile.close();

    return 0;
}
