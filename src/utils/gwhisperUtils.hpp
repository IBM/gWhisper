#include <string>

namespace gwhisper::util
{
    /// Utility-Function for reading contents of a file and returning them as a string
    /// This function terminates the program, if file not found
    /// @param f_path Location of the file to read from as a string
    /// @return File content as simple string
    std::string readFromFile(const std::string f_path);

}
