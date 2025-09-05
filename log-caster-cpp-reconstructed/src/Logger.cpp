// [SEQUENCE: MVP2-11]
#include "Logger.h"

void ConsoleLogger::log(const std::string& message) {
    // [SEQUENCE: MVP2-12]
    // 현재 시간 타임스탬프 생성
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // [SEQUENCE: MVP2-13]
    // 타임스탬프와 메시지를 콘솔에 출력
    std::cout << "[" << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S") << "] "
              << message << std::endl;
}
