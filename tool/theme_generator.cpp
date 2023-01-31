#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <locale>
#include <vector>
#include <sstream>

#include <dirent.h>

std::string basename(std::string& filename) {
    std::size_t dot = filename.rfind('.');

    if(dot == std::string::npos)
        return filename;

    return filename.substr(0, filename.size() - dot);
}

// trim from start
inline std::string ltrim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
 inline std::string rtrim(std::string s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
inline std::string trim(std::string s) {
    return ltrim(rtrim(s));
}

std::pair<std::string, std::string> from_yaml(std::string& data) {
    std::size_t semicolon = data.find(':');

    if(semicolon == std::string::npos)
        throw std::runtime_error(std::string()
            + "Could not parse data "
            + data
        );

    const auto name = data.substr(0, semicolon);
    std::string value = trim(data.substr(semicolon + 1));

    return {
        name, value.substr(1, value.size() - 2)
    };
}

std::string slugify(std::string s) {
    std::replace(s.begin(), s.end(), '-', '_');
    std::replace(s.begin(), s.end(), ' ', '_');
    std::transform(
        s.begin(), s.end(), s.begin(),
        [](unsigned char c){ return std::tolower(c); }
    );
    return s;
}

unsigned from_hex(std::string s) {
    std::stringstream ss;
    ss << std::hex << s;

    unsigned x;
    ss >> x;
    return x;
}

int main(int argc, char** argv) {
    if(argc < 3) {
        throw std::runtime_error(std::string()
            + argv[0]
            + ": provide source directory as a first argument"
            + " and target directory as a second argument."
        );
    }

    const std::string source(argv[1]);
    const std::string target(argv[2]);


    std::ofstream target_file(target);
    if(!target_file) {
        throw std::runtime_error(std::string()
            + "Could not open file "
            + target
        );
    }

    target_file << "#pragma once\n\n";

    DIR* dir = opendir(source.c_str());
    if(!dir) {
        throw std::runtime_error(std::string()
            + "Could not open directory "
            + source
        );
    }

    std::vector<std::string> themes;

    struct dirent *d;

    while (d = readdir(dir)) {
        std::string source_name(d->d_name);

        if(source_name[0] == '.')
            continue;

        std::string source_path = source + "/" + source_name;

        std::ifstream source_file(source_path);

        if(!source_file) {
            throw std::runtime_error(std::string()
                + "Could not open file "
                + source_path
            );
        }

        std::string author;
        std::string scheme;
        std::vector<std::string> colors;

        std::string line;
        while(getline(source_file, line)){
            const auto p = from_yaml(line);
            const auto name = p.first;
            const auto value = p.second;

            if(name == "scheme") {
                scheme = value;
            }
            else if(name == "author") {
                author = value;
            }
            else if(name.substr(0, 4) == "base") {
                colors.push_back(value);
            }
        }

        target_file
            << "// Scheme: " << scheme << "\n"
            << "// Author: " << author << "\n"
            << "const uint32_t " << slugify(scheme) << "_colors[16] = {\n\t";

        for(const auto color : colors)
            target_file << std::string("0x") << color << ", ";

        target_file
            << "\n};\n";

        target_file
            << "const uint8_t " << slugify(scheme) << "_colors_flat[16*3] = {\n";

        for(const auto color : colors) {
            const uint32_t v = from_hex(color);

            uint8_t r = ((v >> 16) & 0xFF);
            uint8_t g = ((v >> 8) & 0xFF);
            uint8_t b = (v & 0xFF);

            target_file << "\t";
            target_file << std::to_string(r) << ", ";
            target_file << std::to_string(g) << ", ";
            target_file << std::to_string(b) << ",\n";
        }

        target_file
            << "\n};\n\n\n";

        themes.push_back(slugify(scheme));
    }

    target_file
        << "char* get_themes() {\n\treturn \"classic; ";

    for(const auto theme : themes)
        target_file
            << theme << "; ";

    target_file
        << "\";\n}\n\n";

    target_file
        << "uint32_t* get_colors(const char* name) {\n";

    for(const auto theme : themes)
        target_file
            << "\tif(strcmp(\"" << theme << "\", name) == 0) return " << theme << "_colors;\n";

    target_file
        << "\treturn classic_colors;\n}\n\n";

    target_file
        << "uint8_t* get_colors_flat(const char* name) {\n";

    for(const auto theme : themes)
        target_file
            << "\tif(strcmp(\"" << theme << "\", name) == 0) return " << theme << "_colors_flat;\n";

    target_file
        << "\treturn classic_colors_flat;\n}\n\n";


    closedir (dir);
}