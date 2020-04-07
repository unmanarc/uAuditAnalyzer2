#ifndef STATUS_H
#define STATUS_H

#include <string>

class Status
{
public:
    Status();
    static void startThreaded();
    static void run();

private:
    static std::string outputDir;
};

#endif // STATUS_H
