/*
    Utils.cc nachocpol@gmail.com
*/

#include "Utils.h"
#include <iostream>
#include <fstream>
#include <string>

std::string util::LoadTextFromFile(const char * path)
{
    std::string out;
    std::string cur;

    std::ifstream file;
    file.open(path, std::ifstream::in);

    if (!file)
    {
        printf(" ERROR: Failed to open:%s.\n", path);
        return out;
    }

    while (!file.eof())
    {
        // Read next line and concatenate
        std::getline(file, cur);
        out = out + "\n" + cur;
    }

    file.close();
    return out;
}
