#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>

using namespace std::chrono;

class ProcessMonitor
{
public:
    static std::mutex m;
    static std::string file_name;
    static std::ofstream file;
    static bool is_first_process;
    std::string name;
    time_point<high_resolution_clock> start;
    time_point<high_resolution_clock> end;

    ProcessMonitor(std::string name)
    {
        this->name = name;
        this->start = high_resolution_clock::now();
    };

    static std::string to_csv(time_point<system_clock> start, time_point<system_clock> end, std::string name)
    {
        auto start_ms = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch());
        auto end_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch());

        return name + "," + std::to_string(start_ms.count()) + "," + std::to_string(end_ms.count()) + "\n";
    }

    static void first_write()
    {
        std::lock_guard<std::mutex> lock(m);
        file << "Name,Start,End\n";
    }

    void saveProcess()
    {
        if (!file.is_open())
        {
            exit(0);
        }
        if (is_first_process)
        {
            first_write();
            is_first_process = false;
        }
        std::unique_lock<std::mutex> lock(m);
        file << to_csv(this->start, this->end, this->name);
    }

    ~ProcessMonitor()
    {
        this->end = high_resolution_clock::now();
        saveProcess();
    };
};