#include "gwhisperUtils.hpp"
#include <fstream>
namespace gwhisper
{
    namespace util
    {
        std::string readFromFile(const std::string f_path)
        {
            std::ifstream credFile;
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
