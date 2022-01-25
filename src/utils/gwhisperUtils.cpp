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

#pragma once

#include "gwhisperUtils.hpp"
#include <fstream>
#include <filesystem>
#include <experimental/filesystem>

//namespace fs = std::experimental::filesystem;

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

        void createFile(const std::string f_filePath){ //Lieber nur filePath? (inkl FileName)
            

            if(std::filesystem::exists(f_filePath)){
                // find last . // What if no dot?
                // Insert "_copy" before dot
                // filepath = modified name

            }
            else{
                std::ofstream newFile(f_filePath);
                newFile.close();
            }

            //Todo: What if creating file fails?
        }

        void createFolder(const std::string f_dirLocation, const std::string f_dirName) //evtl. use filesystem path instead of string
        {
            //Check if  folder exits
            const std::filesystem::path path = f_dirLocation + f_dirName;
            if (!(std::filesystem::is_directory(path))){
                //Create path for folder (from root)
                std::filesystem::create_directories(path); //evtl. mi
            }

            

            //TODO: Check, if .cache exists
            //If not: create .cache
            //Create folder "foldername"
            //const std::string command = "./createFolder.sh " + f_dirLocation + " " + f_dirName ; // eachte spaceafter scriptname
            //std::system(command.c_str());
            
        }
    }
}