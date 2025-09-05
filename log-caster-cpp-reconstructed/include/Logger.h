// [SEQUENCE: MVP2-8]
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

// [SEQUENCE: MVP2-9]
// 로깅을 위한 추상 기본 클래스
class Logger {
public:
    virtual ~Logger() = default; // 가상 소멸자
    virtual void log(const std::string& message) = 0; // 순수 가상 함수
};

// [SEQUENCE: MVP2-10]
// 콘솔에 로그를 출력하는 구상 클래스
class ConsoleLogger : public Logger {
public:
    void log(const std::string& message) override;
};

#endif // LOGGER_H
