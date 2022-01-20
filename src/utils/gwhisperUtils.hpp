// Copyright 2019 IBM Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <string>

namespace gwhisper
{
    namespace util
    {
        /// Utility function for reading contents of a file and returning them as a string
        /// This function terminates the program, if file not found
        /// @param f_path Location of the file to read from as a string
        /// @return File content as simple string. If file cannot be opened, this function returns the status "FAIL" as as string.
        std::string readFromFile(const std::string f_path);

        /// Utility function for creating a folder at given location under Linux
        /// This function skips creating the folder, if it already exists.
        /// @param f_dirLocation Location, where folder should be created
        /// @param f_dirName Name for new folder
        /// @return 
        void createFolder(const std::string f_dirPath, const std::string f_dirName);

        void createFile(const std::string f_filePath, const std::string f_fileName);

    }

}
