#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

class NoValueInLine : public std::exception {
public:
    const char* what() const noexcept override {
        return "No value";
    };
};

class BadValueLine : public std::exception {
public:
    const char* what() const noexcept override {
        return "Bad line with field";
    };
};

class NoSuchSection : public std::exception {
public:
    const char* what() const noexcept override {
        return "No such section";
    };
};

class NoSuchFieldInSection : public std::exception {
public:
    const char* what() const noexcept override {
        return "No such field in section";
    };
};

class NoSuchFile : public std::exception {
public:
    const char* what() const noexcept override {
        return "No such file";
    };
};

class NotIntValue : public std::exception {
public:
    const char* what() const noexcept override {
        return "Value cant be represent of int";
    };
};

class NotFloatValue : public std::exception {
public:
    const char* what() const noexcept override {
        return "Value cant be represent of int";
    };
};

#endif // EXCEPTIONS_H
