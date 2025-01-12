#pragma once
#include <iostream>
#include "Log.hpp"
#include <string>

class Console
{
public:
    static void print_error(std::string error);
    static void print_warning(std::string warning);
    static void print_info(std::string print_info);
    static void print_success(std::string success);

private:
    Log logger;

    static std::string greenColor;
    static std::string yellowColor;
    static std::string redColor;
    static std::string blueColor;
    static std::string whiteColor;
};
