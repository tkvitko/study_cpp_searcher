#include "IniParser.h"
#include "exceptions.h"
#include <iostream>
#include <fstream>
#include <algorithm>


IniParser::IniParser(std::string filename){
    // конструктор, принимающий имя файла

    filename_ = filename;
    std::ifstream source_file = std::ifstream(this->filename_);
    std::string current_section;
    unsigned int line_number = 0;

    if (source_file) {
        while (source_file) {
            std::string line;
            std::getline(source_file, line);
            line_number++;

            // заполнение data
            if (this->get_line_type(line) == LineType::comment) {
                // если это комментарий, пропускаем строку

            } else if (this->get_line_type(line) == LineType::section) {
                // если это описание секции:
                this->process_section_line(line, current_section);

            } else if (this->get_line_type(line) == LineType::field) {
                // если это описание значения:
                this->process_field_line(line, current_section, line_number);

            } else if (this->get_line_type(line) == LineType::unknown) {
                std::cout << "Warning: bad syntax in line " << line_number << std::endl;
            }
        }
    } else {
        throw NoSuchFile();
    }
};

// деструктр
IniParser::~IniParser() {};

std::string IniParser::_get_value(const std::string& value_path) {
    auto section_field = this->split_line(value_path, '.');
    auto section = section_field[0];
    auto field = section_field[1];
    if (this->data.find(section) != this->data.end()) {
        if (this->data[section].find(field) != this->data[section].end()) {
            std::string value = this->data[section][field];
            return value;
        } else {
            std::cout << "No field " << field << " in section " << section << std::endl;
            std::cout << "You probably meant: " << std::endl;
            for (auto el : data[section]) {
                std::cout << el.first << std::endl;
            };
            throw NoSuchFieldInSection();
        }
    } else {
        throw NoSuchSection();
    }
}
void IniParser::process_section_line(std::string& line, std::string& current_section) {
    /*
     метод процессинга строки с секцией:
     - line - строка,
     - current_section - текущая секция в цикле обработки файла.
     */

    line.erase(std::remove(line.begin(), line.end(), '['), line.end());
    line.erase(std::remove(line.begin(), line.end(), ']'), line.end());
    line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
    current_section = line;

    if (this->data.find(current_section) == this->data.end()) {
        // если такой секции ещё нет
        std::map<std::string, std::string> empty;
        this->data.insert({line, empty});
    }
}

void IniParser::process_field_line(std::string& line, const std::string& current_section, const int& line_number) {
    /*
     метод процессинга строки с полем (парой ключ-значение):
     - line - строка,
     - current_section - текущая секция в цикле обработки файла,
     - line_number - номер строки в файле.
     */

    auto key_value = this->split_line(line, '=');

    // подготовка ключа (удаление всех символов пробела)
    auto key = key_value[0];
    key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
    // подготовка значения (удаление символов пробела, идущих до первого символа)
    if (key_value.size() > 1) {
        auto value = key_value[1];
        auto value_start = value.find_first_not_of(' ');
        value = value.substr(value_start != std::string::npos ? value_start : 0);
        // удаление комментария из строки значения
        auto comment_start_pos = value.find(";");
        value = value.substr(0, comment_start_pos);
        this->data[current_section][key] = value;
    } else {
        // если значения нет, ничего не добавляем
    }

}

// метод для получения значения конкретного поля файла
template <typename VALUE_TYPE>
VALUE_TYPE IniParser::get_value(std::string value_path) {
    static_assert(sizeof(VALUE_TYPE) == -1, "not implemented type for get_value");
};

template <>
std::string IniParser::get_value(std::string value_path) {
    std::string value = this->_get_value(value_path);
    return value;
};

template <>
int IniParser::get_value(std::string value_path) {
    std::string value = this->_get_value(value_path);
    return std::stoi(value);
};

template <>
unsigned short IniParser::get_value(std::string value_path) {
    std::string value = this->_get_value(value_path);
    return std::stoul(value);
};

template <>
float IniParser::get_value(std::string value_path) {
    std::string value = this->_get_value(value_path);
    return std::stof(value);
};
