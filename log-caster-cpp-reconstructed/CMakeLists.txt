# [SEQUENCE: CPP-MVP1-1]
# CMake 최소 요구 버전 설정
cmake_minimum_required(VERSION 3.10)

# [SEQUENCE: CPP-MVP1-2]
# 프로젝트 이름 및 버전 설정
project(LogCaster-CPP VERSION 1.0)

# [SEQUENCE: CPP-MVP1-3]
# C++ 표준 설정 (C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# [SEQUENCE: CPP-MVP2-2]
# MVP3에 필요한 소스 파일 목록 (QueryParser.cpp 추가)
set(SOURCES
    src/main.cpp
    src/LogServer.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/LogBuffer.cpp
    src/QueryHandler.cpp
    src/QueryParser.cpp
    src/Persistence.cpp
    # [SEQUENCE: CPP-MVP6-1]
    src/IRCServer.cpp
    src/IRCClient.cpp
    src/IRCChannel.cpp
    src/IRCChannelManager.cpp
    src/IRCClientManager.cpp
    src/IRCCommandParser.cpp
    src/IRCCommandHandler.cpp
)

# [SEQUENCE: CPP-MVP1-4]
# 실행 파일 생성
add_executable(logcaster-cpp ${SOURCES})


# [SEQUENCE: CPP-MVP1-5]
# 헤더 파일 경로 추가
target_include_directories(logcaster-cpp PUBLIC include)

# [SEQUENCE: CPP-MVP1-6]
# [SEQUENCE: CPP-MVP2-3]
# 스레드 라이브러리 링크
find_package(Threads REQUIRED)
target_link_libraries(logcaster-cpp PRIVATE Threads::Threads)

# [SEQUENCE: CPP-MVP4-3]
# 구형 GCC/G++ 컴파일러를 위한 stdc++fs 라이브러리 링크
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(logcaster-cpp PRIVATE stdc++fs)
endif()

# [SEQUENCE: CPP-MVP1-7]
# 설치 경로 설정 (선택 사항)
install(TARGETS logcaster-cpp DESTINATION bin)
