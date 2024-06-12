// Copyright 2022 IBM Corporation
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

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "GwhisperSettings.hpp"
#include "libArgParse/ArgParse.hpp"
#include "../utils/gwhisperUtils.hpp"

using json = nlohmann::json;

void gWhisperSettings::parseConfigFile(const std::string &f_inputFile){
    // Parse JSON File into member SON object
    std::ifstream ifs(f_inputFile.c_str());
    std::string configString = gwhisper::util::readFromFile(f_inputFile);


    /*if (!ifs.is_open())
    {
        std::cerr << "Error while opening config file at: " << f_inputFile << std::endl;
        exit(EXIT_FAILURE);
    }
    // TODO: check for empty value with  nlohmann::isempty()*/
    try
    {
       m_config = json::parse(configString);
    }
    catch(json::parse_error)
    {
        std::cerr << "Error while parsing config file at " << f_inputFile << std::endl;
        std::cerr << "Try checking for JSON Syntax errors or empty JSON file." << std::endl;
        exit(EXIT_FAILURE);
    }

}

std::string gWhisperSettings::findParameterSettingInConfig(const std::string &f_parameter,const  json &f_startElement)
{
        for (const auto &item : f_startElement.items())
        {
            if(item.key() == f_parameter)
            {  
               if(item.value().is_null())
               {
                   return "";
               }
               else
               {
                   return item.value();
               }
               
            }

            // Only Call recursion, if current element contains further elements
            if (item.value().is_object())
            {
                return findParameterSettingInConfig(f_parameter, item.value());
            }
        }
       return "";
}

std::string gWhisperSettings::lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree)
{
    bool containsParameter;
    std::string setting;

    if (f_parseTree.findFirstChild(f_parameter) != "")
    {
        return f_parseTree.findFirstChild(f_parameter);
    }
    else
    {
        setting = findParameterSettingInConfig(f_parameter, m_config);
        return setting;
    }
}

gWhisperSettings::gWhisperSettings(ArgParse::ParsedElement &f_parseTree){ //ParameterKey
    if(m_config.is_null())
    {
        bool useCustomPath = f_parseTree.findFirstChild("ConfigFile")!= "";
        const char* home = std::getenv("HOME");
        std::string defaultPath = "/.config/gWhisperConfig.json";
        std::string inputFile = home + defaultPath;

        if(useCustomPath)
        {
            inputFile = f_parseTree.findFirstChild("ConfigFilePath");
        }

        parseConfigFile(inputFile);
    }
}

gWhisperSettings::~gWhisperSettings(){}