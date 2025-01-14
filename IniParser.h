#ifndef INIPARSER_H
#define INIPARSER_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include "exceptions.h"



class IniParser {
private:
    std::string filename_;
    std::map<std::string, std::map<std::string, std::string>>  data;

    enum class LineType {
        // список типов строк
        section,
        field,
        comment,
        empty,
        unknown
    };

    static LineType get_line_type (const std::string& line) {
        // метод для определения типа строки
        if (line.rfind(';') == 0) {
            return LineType::comment;

        } else if (line.size() == 0) {
            return LineType::empty;

        } else if (line.find('[') != std::string::npos
                   && line.find(']') != std::string::npos
                   && line.find('=') == std::string::npos) {
            return LineType::section;

        } else if (line.find('=') != std::string::npos
                   && line.find('[') == std::string::npos
                   && line.find(']') == std::string::npos) {
            return LineType::field;

        } else {
            return LineType::unknown;
        }
    };

    static std::vector<std::string> split_line(const std::string& line, const char delimiter) {
        // метод для разбиения строки поля на ключ и значение
        std::stringstream line_stream(line);
        std::string segment;
        std::vector<std::string> segments;

        while(std::getline(line_stream, segment, delimiter))
        {
            segments.push_back(segment);
        }
        if (segments.size() == 2) {
            return segments;
        } else if (segments.size() == 1){
            throw NoValueInLine();
        } else {
            throw BadValueLine();
        }
    }

    std::string _get_value(const std::string& value_path);

    void process_section_line(std::string& line, std::string& current_section);

    void process_field_line(std::string& line, const std::string& current_section, const int& line_number);

public:

    IniParser(std::string filename);

    // деструктр
    ~IniParser();

    // метод для получения значения конкретного поля файла
    template <typename VALUE_TYPE>
    VALUE_TYPE get_value(std::string value_path);

    template <>
    std::string get_value(std::string value_path);

    template <>
    int get_value(std::string value_path);

    template <>
    float get_value(std::string value_path);

};

#endif // INIPARSER_H
