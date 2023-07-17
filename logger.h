#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <stdexcept>
#include <stack>

class Logger
{
    static std::stack<std::string> logStack;

public:
    explicit Logger(const std::string & str);
    ~Logger();

public:
    static const std::stack<std::string> & getStack()
    {
        return logStack;
    };
};

class LogException : std::exception
{
    std::string str;

public:
    LogException(const std::string & )
    {
       // auto & st = Logger::getStack();
        //while(st) {}
    }
    virtual const char * what() const noexcept
    {
        return str.c_str();
    }
};

#endif // LOGGER_H
