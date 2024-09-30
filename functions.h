#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

#include <ctime>
#include <string>
#include <iostream>
#include <argp.h>

/*SDR channel*/
constexpr int channel = 0;

/*argp stuff*/
/*list of arguments and fucntion for argp parsing*/
struct arguments
{
    bool showSettings = false;
    bool measure = true;
    bool plot = false;
    bool settingsUpdated = false;
    bool customFileName = false;
        //default settings
    double sampleRate = 250000.0;       //use -s to change it
    double freq = 105500000.0;          //use -f to change it
    int gain = 30;                      //use -g to change it
    int bandwidth = 40000;              //use -b to change it
    int blockLenght = 1024;             //use -l to change it
    int numberOfBlocks = 100;           //use -n to change it
    std::string fileName = "";
};
int parse_opt(int key, char* arg, struct argp_state* state);

const std::string getTimeString();
bool isFilenameValid(const std::string&);
bool isIQextensionValid(const std::string&);
void saveSettingToFile(const struct arguments&);
void measure(const struct arguments&);
void plot(const struct arguments&);

#endif