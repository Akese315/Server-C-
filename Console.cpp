#include "Console.hpp"

std::string Console::greenColor = "\033[32m";
std::string Console::redColor = "\033[31m";
std::string Console::blueColor = "\033[36m";
std::string Console::yellowColor = "\033[93m";
std::string Console::whiteColor = "\033[97m";

void Console::printError(std::string error)
{
    std::string strError = "ERROR : "+ error;

    std::cout <<Console::redColor;
    std::cout << strError;
    std::cout <<Console::whiteColor;
    std::cout << "\n";

    Log::writeLog(strError.c_str());
}


void Console::printInfo(std::string info)
{
    std::string strInfo = "INFO : "+ info;

    std::cout << Console::blueColor;
    std::cout << strInfo;
    std::cout << Console::whiteColor;
    std::cout << "\n";

    Log::writeLog(strInfo.c_str());
}

void Console::printWarning(std::string warning)
{
    std::string strWarning = "WARNING : "+ warning;

    std::cout << Console::yellowColor;
    std::cout << strWarning;
    std::cout << Console::whiteColor;
    std::cout << "\n";

    Log::writeLog(strWarning.c_str());
}

void Console::printSuccess(std::string success)
{
    std::string strSuccess = "SUCCESS : "+ success;

    std::cout << Console::greenColor;
    std::cout << strSuccess;
    std::cout << Console::whiteColor;
    std::cout << "\n";

    Log::writeLog(strSuccess.c_str());
}