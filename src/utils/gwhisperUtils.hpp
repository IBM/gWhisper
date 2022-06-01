#include <string>

namespace gwhisper
{
    namespace util
    {
        /// Utility function for reading contents of a file and returning them as a string
        /// This function terminates the program, if file not found
        /// @param f_path Location of the file to read from as a string
        /// @return File content as simple string. If file cannot be opened, this function returns the status "FAIL" as as string.
        std::string readFromFile(std::string f_path);

    }

}
