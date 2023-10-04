#pragma once
#include <iostream>
#include "Log.hpp"
#include <string>

class Console
{
public:
   
    static void printError(std::string error);
    static void printWarning(std::string warning);
    static void printInfo(std::string printInfo);
    static void printSuccess(std::string success);

private:
    
    Log logger;

    static std::string greenColor;
    static std::string yellowColor;
    static std::string redColor;
    static std::string blueColor;
    static std::string whiteColor;
    
};
