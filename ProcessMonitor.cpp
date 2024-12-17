#include "ProcessMonitor.hpp"

std::mutex ProcessMonitor::m;
std::string ProcessMonitor::file_name = "./ProcessMonitor.txt";
std::ofstream ProcessMonitor::file(ProcessMonitor::file_name, std::ios::out | std::ios::trunc);
bool ProcessMonitor::is_first_process = true;