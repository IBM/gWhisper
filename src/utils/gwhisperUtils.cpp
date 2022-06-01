#include "gwhisperUtils.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

namespace gwhisper
{
    namespace util
    {
        std::string readFromFile(std::string f_path)
        {
            std::ifstream credFile;

            std::size_t foundTilde = f_path.find_first_of("~");
            if(foundTilde != std::string::npos)
            {
                const char* home = std::getenv("HOME");
                if (!home)
                {
                    std::cerr << "Error while fetching home envoronment. Try checking your home environment variable." << std::endl;
                    exit(EXIT_FAILURE);
                }
                std::string subPath = f_path.substr(foundTilde+1);
                f_path = home + subPath;
            }

            credFile.open(f_path);
            if (!credFile)
            {
                return ("FAIL");
            }

            else
            {

                std::string str{std::istreambuf_iterator<char>(credFile),
                                std::istreambuf_iterator<char>()};
                return str;

                //std::cerr << "File not found at: " << f_path << std::endl;
            }
        }
    }
}
