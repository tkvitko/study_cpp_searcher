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
    add_executable(cpp_search_qt_creator main.cpp Crowler.h Crowler.cpp root_certificates.hpp
        DbManager.h DbManager.cpp
        Searcher.h Searcher.cpp
        SafeQueue.h SafeQueue.cpp
        IniParser.h IniParser.cpp
        exceptions.h
        config.ini)
    add_subdirectory(./libpqxx-7.9.0 libpqxx-build)
    target_compile_features(cpp_search_qt_creator PRIVATE cxx_std_17)
    target_link_libraries(cpp_search_qt_creator ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto pqxx)# "${PQXX_LIBRARIES}")
endif()

include(GNUInstallDirs)
install(TARGETS cpp_search_qt_creator
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
