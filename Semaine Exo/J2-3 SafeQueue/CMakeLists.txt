cmake_minimum_required(VERSION 3.10)
project(TestSafeQueue)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
set(SOURCES
    main.cpp
)

# Headers
set(HEADERS
    SafeQueue.h
    TestItem.h
)

# Executable
add_executable(TestSafeQueue ${SOURCES} ${HEADERS})

# Windows specific
if(WIN32)
    target_compile_definitions(TestSafeQueue PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Output message
message(STATUS "Configuration TestSafeQueue")
message(STATUS "  - Sources: ${SOURCES}")
message(STATUS "  - Headers: ${HEADERS}")
