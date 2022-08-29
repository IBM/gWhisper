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

#include "GWhisperSetting.hpp"
#include "libArgParse/ArgParse.hpp"

using json = nlohmann::json;

void gWhisperSetting::parseConfigFile(const std::string &f_inputFile){
    // Parse JSON File into member SON object
    std::ifstream ifs(f_inputFile.c_str());

    if (!ifs.is_open())
    {
        std::cout << "Error while opening config file at: " << f_inputFile << std::endl;
        exit(EXIT_FAILURE);
    }
    // TODO: check for empty value with  nlohmann::isempty()
    ifs >> m_config;
    ifs.close();

}

void gWhisperSetting::retrieveConfigParameters(json &f_startElement){
    //recursively go over config file and retrieve all parameter keys
    if(f_startElement.is_structured())
    {
         for (const auto &item : f_startElement.items())
        {
            m_configParameters.push_back(item.key());

            if (item.value().is_object())
            {
                retrieveConfigParameters(item.value());
            }
        }
    }
}

json gWhisperSetting::findParameterSettingInConfig(const std::string &f_parameter,const  json &f_startElement)
{
    // TODO: If enough time, optimmize for
    bool containsParameter = false;
    json setting;

    for (const auto& item : f_startElement.items())
    {
        if (item.value().contains(f_parameter))
        {
            //Abbruchkrit

            setting[f_parameter] = item.value().at(f_parameter);
            break;
        }

        for(auto &innerElement : item.value().items())
        {   
            if (innerElement.key() == "")
            {
                break; 
            }

            // Only Call recursion, if current element contains further elements
            if(innerElement.value().is_structured())
            {
                json tempElement;
                tempElement[innerElement.key()] = innerElement.value();
                return findParameterSettingInConfig(f_parameter, tempElement);
            }
            else
            {
                continue;
            }
        }
    }
    return setting;
}

// f_paramaetr entrpricht label des Grammatik Elements in ParseTree
std::string gWhisperSetting::lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree)
{
    bool containsParameter;
    std::string setting;
    json someJson;

    if (f_parseTree.findFirstChild(f_parameter) != "")
    {
        setting = f_parseTree.findFirstChild(f_parameter);
    }
    else
    {
        someJson = findParameterSettingInConfig(f_parameter, m_config); //copy
        if (!someJson.is_null())
        {    
            if (!someJson.at(f_parameter).is_null())
            {
                containsParameter = true;
                setting = someJson.at(f_parameter);
            }

            if(someJson.at(f_parameter).is_null())
            {
                setting = ""; // TODO: Is this the right semantic for Timeout?
            }
        }
    }
    //std::cout << "Found setting for " << f_parameter << " : " << setting << std::endl;
    return setting;
}

// Ersetze ParseTree beim Suchen -> f_parseTree.findFirstChild
// Für Parameter, die nicht im config file stehen / keine optionen sind z.B. services
gWhisperSetting::gWhisperSetting(ArgParse::ParsedElement &f_parseTree){ //ParameterKey
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
    retrieveConfigParameters(m_config);
}

gWhisperSetting::~gWhisperSetting(){}