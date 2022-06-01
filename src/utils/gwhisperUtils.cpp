#include "gwhisperUtils.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

namespace gwhisper
{
    namespace util
    {
        std::string readFromFile(const std::string f_path)
        {
            std::ifstream credFile;
            std::ifstream credFile;
            std::cout << "ATTENTION!!!" << std::endl;
            std::cout << std::filesystem::current_path() << std::endl;
            std::cout << f_path << std::endl;

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
