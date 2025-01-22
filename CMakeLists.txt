cmake_minimum_required(VERSION 3.20)

project(cpp_search_qt_creator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)
find_package(OpenSSL)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(crowler
        main.cpp
        Crowler.h Crowler.cpp
        root_certificates.hpp
        DbManager.h DbManager.cpp
        Searcher.h Searcher.cpp
        SafeQueue.h SafeQueue.cpp
        IniParser.h IniParser.cpp
        exceptions.h
        constants.h
    )
    add_executable(http_server
        http_server_small.cpp
        DbManager.h DbManager.cpp
        Searcher.h Searcher.cpp
        IniParser.h IniParser.cpp
        constants.h
    )
    add_subdirectory(./libpqxx-7.9.0 libpqxx-build)
    target_compile_features(crowler PRIVATE cxx_std_17)
    target_compile_features(http_server PRIVATE cxx_std_17)
    target_link_libraries(crowler ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto pqxx)# "${PQXX_LIBRARIES}")
    target_link_libraries(http_server ${Boost_LIBRARIES} pqxx)
endif()

include(GNUInstallDirs)
install(TARGETS crowler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
