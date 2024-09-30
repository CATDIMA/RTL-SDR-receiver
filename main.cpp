/*to compile this code you need to install following dependencies:
1. libgnuplot-iostream-dev
2. SoapySDR
3. fftw3
*/

#include <fstream>
#include "functions.h"

/*check functions.h file for the default settings declared in struct arguments*/

int main(int argc, char** argv)
{
    /*all possible arguments*/
    argp_option options[] =
    {
        {0, 'o', "OUTPUT_FILE_NAME", OPTION_ARG_OPTIONAL, "save measurements in OUTPUT_FILE_NAME file"},
        {0, 'p', "FILE_NAME", 0, "plot a graph based on values from FILE_NAME file"},
        {0, 's', "SAMPLE/SEC", 0, "set the sample rate, samples/sec"},
        {0, 'f', "FREQ", 0, "set the frequency, Hz"},
        {0, 'g', "GAIN", 0, "set the gain, db"},
        {0, 'b', "BANDWTH", 0, "set the bandwidth, Hz"},
        {0, 'l', "LENGHT", 0, "set the L block lenght of measurements"},
        {0, 'n', "NUM_OF_BLOCKS", 0, "set the N number of blocks"},
        {0, 'S', 0, 0, "see current settings"},
        {0}
    };
    struct arguments arguments;
    struct argp argpStruct = {options, parse_opt};

    /*load saved settings*/
    std::fstream settingsFile;
    settingsFile.open("settings.conf", std::ios::in);
    if(settingsFile.is_open())
    {
        /*
        a the file with the settings must follow this structure:
        1st line. sample rate
        2nd line. frequency
        3rd line. gain
        4th line. bandwidth
        5ht line. size of each block
        6th line. number of blocks    */
        std::string setting;

        /*there is no exception catching because file MUST have correct values*/

        /*1st line*/
        std::getline(settingsFile, setting);
        arguments.sampleRate = std::stod(setting);

        /*2nd line*/
        std::getline(settingsFile, setting);
        arguments.freq = std::stod(setting);

        /*3rd line*/
        std::getline(settingsFile, setting);
        arguments.gain = std::stoi(setting);

        /*4th line*/
        std::getline(settingsFile, setting);
        arguments.bandwidth = std::stoi(setting);

        /*5th line*/
        std::getline(settingsFile, setting);
        arguments.blockLenght = std::stoi(setting);

        /*6th line*/
        std::getline(settingsFile, setting);
        arguments.numberOfBlocks = std::stoi(setting);

        settingsFile.close();
    }
    else
    {
        /*
        file with settings doesn't exist. 
        Create a new settings file and put default settings in it   */
        std::cout << "Warning: Cannot find file with settings." << std::endl;
        std::cout << "Creating a new one..." << std::endl;
        try
        {
            saveSettingToFile(arguments);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    
    /*Checking arguments:*/
    argp_parse(&argpStruct, argc, argv, 0, 0, &arguments);

    /*need to set default file name*/
    if(!arguments.customFileName)
    {
        arguments.fileName = getTimeString() + "_results.iq";
    }
    
    /*show settings*/
    if(arguments.showSettings)
    {
        std::cout << "Current sample rate: " << static_cast<long>(arguments.sampleRate) << " samples/sec" << std::endl;
        std::cout << "Current frequency: " << static_cast<long>(arguments.freq) << " Hz" << std::endl;
        std::cout << "Current gain: " << arguments.gain << " dB" << std::endl;
        std::cout << "Current bandwidth: " << arguments.bandwidth << " Hz" << std::endl;
        std::cout << "Current block lenght: " << arguments.blockLenght << " values" << std::endl;
        std::cout << "Current number of blocks: " << arguments.numberOfBlocks << std::endl;
    }

    /*saving the settings*/
    if(arguments.settingsUpdated)
    {
        try
        {
            saveSettingToFile(arguments);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    
    if(arguments.measure)
    {
        try
        {
            measure(arguments);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }

    /*plot the graph*/
    if(arguments.plot)
    {
        try
        {
            plot(arguments);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }

    return 0;
}