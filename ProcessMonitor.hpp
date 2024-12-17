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
    time_point<system_clock> start;
    time_point<system_clock> end;

    ProcessMonitor(std::string name)
    {
        this->name = name;
        this->start = system_clock::now();
    };

    static std::string to_csv(time_point<system_clock> start, time_point<system_clock> end, std::string name)
    {
        std::time_t start_time = system_clock::to_time_t(start);
        std::time_t end_time = system_clock::to_time_t(end);
        return name + "," + std::ctime(&start_time) + "," + std::ctime(&end_time) + "\r\n";
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
        std::lock_guard<std::mutex> lock(m);
        file << to_csv(this->start, this->end, this->name);
        file.close();
    }

    ~ProcessMonitor()
    {
        this->end = system_clock::now();
        saveProcess();
    };
};