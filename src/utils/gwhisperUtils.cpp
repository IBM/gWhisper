// Copyright 2021 IBM Corporation
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

#include "gwhisperUtils.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include<string>

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
                std::cerr << "File not found at: " << f_path << std::endl; //error message here?
                return ("FAIL");
            }

            else
            {

                std::string str{std::istreambuf_iterator<char>(credFile),
                                std::istreambuf_iterator<char>()};
                return str;
            }
        }

        std::string createFile(std::string f_filePath) //TODO What if folder fails?
        {   
            if(std::filesystem::exists(f_filePath))
            {
                //TODO: TEST!!
                size_t pos = f_filePath.find_last_of('.');
                std::string substr1 = f_filePath.substr(0, pos);
                std::string substr2 = f_filePath.substr(pos+1);

                substr1.append("_copy");
                f_filePath = substr1 + substr2;
            }

            std::ofstream newFile(f_filePath);
            if(!newFile)
            {
                std::cerr << "Error while creating new file "<<f_filePath<< std::endl; //Error message here?
                return "FAIL";
            }
            newFile.close();
            return "OK";
        }

        std::string createFolder(const std::string f_pathToNewFolder)
        {
            if (!(std::filesystem::is_directory(f_pathToNewFolder)))
            {
                try
                {
                    std::filesystem::create_directories(f_pathToNewFolder); //creates all missing directories
                }
                catch(std::filesystem::filesystem_error e)
                {
                    std::cerr<<e.what()<<std::endl;
                    return "FAIL";
                }
            }
            return "OK"; //or better path to new Folder?
        }
    }
}