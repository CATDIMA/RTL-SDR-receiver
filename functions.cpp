#include <vector>
#include <fstream>
#include <numeric>
#include <fftw3.h>
#include <math.h>
#include <stdexcept>
#include <gnuplot-iostream.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include "functions.h"

const std::string getTimeString()
{
    time_t now = time(0);
    tm* time = localtime(&now);

    std::string currentTime = std::to_string(time->tm_hour) + std::to_string(time->tm_min) + 
                            std::to_string(time->tm_sec) + "_" + std::to_string(time->tm_mday) + 
                            std::to_string(time->tm_mon) + std::to_string(time->tm_year + 1900);

    return currentTime;
}

bool isFilenameValid(const std::string& name)
{
    bool valid = true;
    for(int i = 0; i < name.size(); i++)
    {
        if(name[i] == '\0' || name[i] == '\\' || name[i] == '/' || name[i] == ':' ||
        name[i] == '*' || name[i] == '?' || name[i] == '"' || name[i] == '<' ||
        name[i] == '>' || name[i] == '|')
        {
            valid = false;
            std::cout << "Error: file name contains forbidden character: " << name[i] << std::endl;
            break;
        }
        if(name[i] == '.' && (i + 1 < name.size()))
        {
            if(name[i + 1] == '.')
            {
                valid = false;
                std::cout << "Error: file name contains .." << std::endl;
                break;
            }
        }
    }
    return valid;
}

bool isIQextensionValid(const std::string& fileName)
{
    bool isOK = false;
    for(int i = 0; i < fileName.size(); i++)
    {
        //avoiding of out of range
        if(fileName[i] == '.' && (fileName.size() - 2 == i))
        {
            //if we will still in range, check the extension
            if(fileName[i+1] == 'i' && fileName[i+2] == 'q')
            {
                isOK = true;
            }
        }
    }
    return isOK;
}

void saveSettingToFile(const arguments& arguments)
{
    std::fstream settingsFile;
    settingsFile.open("settings.conf", std::ios::out | std::ios::trunc);
    if(settingsFile.is_open())
    {
        /*write default settings*/
        settingsFile << arguments.sampleRate << std::endl;
        settingsFile << arguments.freq << std::endl;
        settingsFile << arguments.gain << std::endl;
        settingsFile << arguments.bandwidth << std::endl;
        settingsFile << arguments.blockLenght << std::endl;
        settingsFile << arguments.numberOfBlocks << std::endl;
        settingsFile.close();
        std::cout << "Done." <<std::endl;
    }   
    else
    {
        throw std::runtime_error{"Saving to settings.conf failed >_<"};
    }    
}

void measure(const arguments& arguments)
{
    /*get all sdr devices*/
    SoapySDR::KwargsList results = SoapySDR::Device::enumerate();
    SoapySDR::Kwargs::iterator itr;

    /*show the results*/
    if(results.empty())
    {
        throw std::runtime_error{"No devices connected"};
    }

    std::cout << "\nStarting measurent..." << std::endl;
    std::cout << "You can find the results in " << arguments.fileName << " file" << std::endl;  
    /*create a file*/
    std::fstream outputFile;
    outputFile.open(arguments.fileName, std::ios::trunc | std::ios::binary | std::ios::out);

    /*write frequency, sample rate amount of blocks and its size*/
    outputFile.write(reinterpret_cast<const char*>(&arguments.freq), sizeof(arguments.freq));
    outputFile.write(reinterpret_cast<const char*>(&arguments.sampleRate), sizeof(arguments.sampleRate));
    outputFile.write(reinterpret_cast<const char*>(&arguments.numberOfBlocks), sizeof(arguments.numberOfBlocks));
    outputFile.write(reinterpret_cast<const char*>(&arguments.blockLenght), sizeof(arguments.blockLenght));
    outputFile.write(reinterpret_cast<const char*>(&arguments.gain), sizeof(arguments.gain));
    outputFile.write(reinterpret_cast<const char*>(&arguments.bandwidth), sizeof(arguments.bandwidth));

    for( int i = 0; i < results.size(); ++i)
    {
        std::cout << "Found device #" << i << " ";
        for( itr = results[i].begin(); itr != results[i].end(); ++itr)
        {
            std::cout << itr->first.c_str() << " = " << itr->second.c_str() <<std::endl;
        }
        std::cout << std::endl;
    }

    /*get only one device*/
    SoapySDR::Kwargs arg = results[0];
    SoapySDR::Device* device = SoapySDR::Device::make(arg);
    if(device == nullptr || device == NULL)
    {
        throw std::runtime_error{"Cannot make a device"};
    }

    /*query device info*/
    std::vector<std::string> str_list;

    /*antennas*/
    str_list = device->listAntennas(SOAPY_SDR_RX, channel);
    std::cout << "RX antennas: ";
    for(int i = 0; i < str_list.size(); ++i)
    {
        std::cout << str_list[i].c_str() << ", ";
    }
    std::cout << std::endl;

    /*gains*/
    str_list = device->listGains(SOAPY_SDR_RX, channel);
    std::cout << "RX gains: ";
    for(int i = 0; i < str_list.size(); ++i)
    {
        std::cout << str_list[i].c_str() << ", ";
    }
    std::cout << std::endl;

    /*frequency ranges*/
    SoapySDR::RangeList ranges = device->getFrequencyRange(SOAPY_SDR_RX, channel);
    std::cout << "RX ranges: ";
    for(int i = 0; i < ranges.size(); ++i)
    {
        std::cout << ranges[i].minimum() << "->" << ranges[i].maximum();
    }
    std::cout << std::endl;

    /*set the settings*/
    device->setSampleRate(SOAPY_SDR_RX, channel, arguments.sampleRate);
    device->setFrequency(SOAPY_SDR_RX, channel, arguments.freq);
    device->setGain(SOAPY_SDR_RX, channel, arguments.gain);
    device->setBandwidth(SOAPY_SDR_RX, channel, arguments.bandwidth);

    /*setup a stream*/
    SoapySDR::Stream* rx_stream = device->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CS8);
    if(rx_stream == NULL || rx_stream == nullptr)
    {
        SoapySDR::Device::unmake(device);
        throw std::runtime_error{"Cannot create a stream"};
    }
    device->activateStream(rx_stream, 0, 0, 0);

    std::complex<char> buff[arguments.blockLenght];
    for(int i = 0; i < arguments.numberOfBlocks; ++i)
    {
        void* buffs[] = {buff};
        int flags;
        long long time_ns;
        int ret = device->readStream(rx_stream, buffs, arguments.blockLenght, flags, time_ns, 1000000);
    
        for(int j = 0; j < arguments.blockLenght; j++)
        {
            outputFile << buff[j].real() << buff[j].imag();
        }
    }
    outputFile.close();

    device->deactivateStream(rx_stream, 0, 0);
    device->closeStream(rx_stream);

    SoapySDR::Device::unmake(device);
    std::cout << "\nDone." << std::endl;
}

void plot(const arguments& arguments)
{
    std::vector<char> ic;
    std::vector<char> qc;
    double currentSampleRate = 0;
    double currentFrequency = 0;
    int currentBlockLenght = 0;
    int currentNumberOfBlocks = 0;
    int currentGain = 0;
    int currentBandwidth = 0;

    /*open file with iq counts*/
    std::fstream iqs;
    iqs.open(arguments.fileName, std::ios::in | std::ios::binary);
    if(!iqs.is_open())
    {
        throw std::runtime_error{"Cannot open the " + arguments.fileName + " Did you write its name correctly?"};
    }

    /*read file*/
    int i = 0;
    char temp;
    while(true)
    {
        if(i == 0)
        {
            /*read settings*/
            iqs.read(reinterpret_cast<char*>(&currentFrequency), sizeof(double));
            iqs.read(reinterpret_cast<char*>(&currentSampleRate), sizeof(double));
            iqs.read(reinterpret_cast<char*>(&currentNumberOfBlocks), sizeof(int));
            iqs.read(reinterpret_cast<char*>(&currentBlockLenght), sizeof(int));
            iqs.read(reinterpret_cast<char*>(&currentGain), sizeof(currentGain));
            iqs.read(reinterpret_cast<char*>(&currentBandwidth), sizeof(currentBandwidth));
            i = 1;  /*to make i even and non-zero for next iteration*/
        }
        else if((i % 2) == 0)   /*if i is even it's ic*/
        {
            iqs.read(&temp, sizeof(char));
            if(iqs.eof())
            {
                break;
            }
            ic.emplace_back(temp);
        }
        else /*if i is odd it's qc*/
        {
            iqs.read(&temp, sizeof(char));
            if(iqs.eof())
            {
                break;
            }
            qc.emplace_back(temp);
        }
        i++;
    }
    std::cout << "current frequency: " << currentFrequency << std::endl;
    std::cout << "current sample rate: " << currentSampleRate << std::endl;
    std::cout << "current block lenght: " << currentBlockLenght << std::endl;
    std::cout << "current numbers of blocks: " << currentNumberOfBlocks << std::endl;
    std::cout << "current gain: " << currentGain << std::endl;
    std::cout << "current bandwidth " << currentBandwidth << std::endl;

    /*size of array must to be power of 2*/
    ic.resize(currentBlockLenght * currentNumberOfBlocks);
    qc.resize(currentBlockLenght * currentNumberOfBlocks);

    /*create complex numbers*/
    const int REAL = 0; 
    const int IMAG = 1;
    const int N = currentBlockLenght;
    fftw_complex* inputFFTarray = (fftw_complex*)fftw_malloc(N * sizeof(fftw_complex));
    fftw_complex* outputFFTarray = (fftw_complex*)fftw_malloc(N * sizeof(fftw_complex));
    fftw_plan plan;
    std::vector<fftw_complex>* complexAmplitudesSum = new std::vector<fftw_complex>(currentBlockLenght);

    for(int i = 0; i < currentNumberOfBlocks; i++)
    {
        /*fill input array*/
        for(int j = 0; j < currentBlockLenght; j++)
        {
            inputFFTarray[j][REAL] = static_cast<double>(ic[i * currentBlockLenght + j]);
            inputFFTarray[j][IMAG] = static_cast<double>(qc[i * currentBlockLenght + j]);
            outputFFTarray[j][REAL] = 0;
            outputFFTarray[j][IMAG] = 0;
        }
        /*create plan and execute it*/
        plan = fftw_plan_dft_1d(N, inputFFTarray, outputFFTarray, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan);

        /*sum FFT result*/
        for(int k = 0; k < currentBlockLenght; k++)
        {
            complexAmplitudesSum->at(k)[REAL] += pow(outputFFTarray[k][REAL], 2);
            complexAmplitudesSum->at(k)[IMAG] += pow(outputFFTarray[k][IMAG], 2);
        }
    }

    /*we don't need it anymore*/
    ic.clear();
    qc.clear();
    fftw_destroy_plan(plan);
    fftw_free(inputFFTarray);
    fftw_free(outputFFTarray);

    /*average FFT results*/
    for(int i = 0; i < currentBlockLenght; i++)
    {
        complexAmplitudesSum->at(i)[REAL] /= currentNumberOfBlocks * currentNumberOfBlocks;
        complexAmplitudesSum->at(i)[IMAG] /= currentNumberOfBlocks * currentNumberOfBlocks;
    }
    
    /*compute amplitudes*/
    std::vector<double> amplitudes(currentBlockLenght);
    for(int i = 0; i < currentBlockLenght; i++)
    {
        amplitudes[i] = sqrt(complexAmplitudesSum->at(i)[REAL] + complexAmplitudesSum->at(i)[IMAG]);
    }
    for(int i = 0; i < currentBlockLenght; i++)
    {
        amplitudes[i] = 20 * log10(amplitudes[i]);
    }

    /*free memory*/
    complexAmplitudesSum->clear();
    delete complexAmplitudesSum;

    std::cout << "Now plotting..." <<std::endl;
    std::cout << "Please, wait..." <<std::endl;
    
    //now start plotting
    Gnuplot gp;
    gp << "set nokey\n";
    gp << "set logscale y\n";
    gp << "set title 'Spectrum'\n";
    gp << "set xrange[0:" << currentBlockLenght << "]\n";
    gp << "set yrange[0:]\n";
    gp << "plot '-' smooth acsplines\n";
    gp.send(amplitudes);
}

int parse_opt(int key, char* arg, struct argp_state* state)
{
    struct arguments* arguments = reinterpret_cast<struct arguments*>(state->input);
    std::string strArg;
    if(arg != 0)
    {
        strArg = arg;
    }
    switch (key)
    {
    case 'o':
        if(strArg.empty() || isFilenameValid(strArg))
        {
            arguments->fileName = strArg;
            std::cout << "You can find the results in " << strArg << " file" << std::endl;
            arguments->customFileName = true;
        }
        else
        {
            std::cout << "Error in file naming. Default file name will be used" << std::endl;
        }
        break;
    case 'p':
        if(arg == 0)
        {
            argp_failure(state, 1, 0, "file name missing");
        }
        else
        {
            arguments->fileName = std::string{arg};
            arguments->measure = false;
            arguments->plot = true;
            arguments->customFileName = true;
        }
        break;
    case 's':
        try
        {
            arguments->sampleRate = std::stod(std::string{*arg});
            std::cout << "New sample rate: " << static_cast<long>(arguments->sampleRate) << std::endl;
            arguments->settingsUpdated = true;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "-s: invalid argument" << '\n';
        }
        break;
    case 'f':
        try
        {
            arguments->freq = std::stod(std::string{*arg});
            std::cout << "New frequency: " << static_cast<long>(arguments->freq) << std::endl;
            arguments->settingsUpdated = true;
            arguments->measure = false;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "-f: invalid argument" << '\n';
        }
        break;
    case 'g':
        try
        {
            arguments->gain = std::stoi(std::string{*arg});
            std::cout << "New gain: " << arguments->gain << std::endl;
            arguments->settingsUpdated = true;
            arguments->measure = false;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "-g: invalid argument" << '\n';
        }
        break;
    case 'b':
        try
        {
            arguments->bandwidth = std::stoi(std::string{*arg});
            std::cout << "New bandwidth: " << arguments->bandwidth << std::endl;
            arguments->settingsUpdated = true;
            arguments->measure = false;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "-b: invalid argument" << '\n';
        }
        break;
    case 'l':
        try
        {
            arguments->blockLenght = std::stoi(std::string{*arg});
            std::cout << "New block lenght: " << arguments->blockLenght << std::endl;
            arguments->settingsUpdated = true;
            arguments->measure = false;
        }
        catch(const std::invalid_argument& e)
        {
            std::cerr << "-l: invalid argument" << '\n';
        }
        break;
    case 'n':
        try
        {
            arguments->numberOfBlocks = std::stoi(std::string{*arg});
            std::cout << "New number of blocks: " << arguments->numberOfBlocks << std::endl;
            arguments->settingsUpdated = true;
            arguments->measure = false;
        }
        catch(const std::invalid_argument& e)
        {
            std::cerr << "-n: invalid argument" << '\n';
        }
        break;
    case 'S':
        arguments->showSettings = true;
        break;
    }

    return 0;
};