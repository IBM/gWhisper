#include "gwhisperUtils.hpp"
#include <fstream>
namespace gwhisper
{
    namespace util
    {

        /// Utility-Function for reading contents of a file and returning them as a string
        /// This function terminates the program, if file not found
        /// @param f_path Location of the file to read from as a string
        /// @return File content as simple string
        std::string readFromFile(const std::string f_path)
        {
            std::ifstream credFile(f_path);
            const char *file = f_path.c_str();
            if (file)
            {

                std::string str{std::istreambuf_iterator<char>(credFile),
                                std::istreambuf_iterator<char>()};
                return str;
            }
            else
            {
                //std::cerr << "File not found at: " << f_path << std::endl;
                return ("FAIL");
            }
        }
    }

}
