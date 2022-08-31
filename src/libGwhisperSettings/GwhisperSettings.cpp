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

using json = nlohmann::json;

void gWhisperSettings::parseConfigFile(const std::string &f_inputFile){
    // Parse JSON File into member SON object
    std::ifstream ifs(f_inputFile.c_str());

    if (!ifs.is_open())
    {
        std::cerr << "Error while opening config file at: " << f_inputFile << std::endl;
        exit(EXIT_FAILURE);
    }
    // TODO: check for empty value with  nlohmann::isempty()
    try
    {
        ifs >> m_config;
    }
    catch(json::parse_error)
    {
        std::cerr << "Error while parsing config file at " << f_inputFile << std::endl;
        std::cerr << "Try checking for JSON Syntax errors or empty JSON file." << std::endl;
        exit(EXIT_FAILURE);
    }
    
    ifs.close();

}

void gWhisperSettings::retrieveConfigParameters(json &f_startElement){
    if(f_startElement.is_structured())
    // Abbruch: When keine weiteren objecte
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

std::string gWhisperSettings::findParameterSettingInConfig(const std::string &f_parameter,const  json &f_startElement)
{
   /* if(f_startElement.contains(f_parameter))
    {   
        return f_startElement.at(f_parameter);
    }*/
   // std::cout << "RECURSION: " << f_startElement << std::endl;
    //if(f_startElement.is_structured())
    //{
        for (const auto &item : f_startElement.items())
        {
           // std::cout << item.key() << std::endl;
            //std::cout << item.value() << std::endl;
            // if item_key == f_parameter
            if(item.key() == f_parameter)
            //if (item.value().contains(f_parameter))
            {  
                //std::cout << "--"<< item.key() <<"--"<< std::endl;
                //std::cout << "VALUE 2: " << item.value() << std::endl;
              //  std::cout << "VALUE at param: " << item.value() << std::endl;
               // return item.value().at(f_parameter);
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
   // }
}

/*json gWhisperSettings::findParameterSettingInConfig(const std::string &f_parameter,const  json &f_startElement)
{
// TODO: If enough time, optimmize for
    bool containsParameter = false;
    json setting;

    for (const auto& item : f_startElement.items())
    {
        if (item.value().contains(f_parameter))
        {
            //Abbruchkrit
            //json setting;
            return setting[f_parameter] = item.value().at(f_parameter);
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
}*/

// f_paramaetr entrpricht label des Grammatik Elements in ParseTree
std::string gWhisperSettings::lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree)
{
    bool containsParameter;
    std::string setting;
   // json someJson;
   // std::cout << "SEARCH NOw" << f_parameter << std::endl;

    if (f_parseTree.findFirstChild(f_parameter) != "")
    {
        return f_parseTree.findFirstChild(f_parameter);
    }
    else
    {
        setting = findParameterSettingInConfig(f_parameter, m_config);
      //  std::cout << "Found " << setting << " for: " << f_parameter << std::endl;
        return setting;
        /*if(!(setting == "null"))
        {
            //std::cout << "JSON " << someJson << std::endl;
            return setting;
        }
        else
        {
            std::cout << "SETTING IS NULL" << std::endl;
            return "";
        }*/
        /*someJson = findParameterSettingInConfig(f_parameter, m_config); 
        if (!someJson.is_null())
        {    
            if (!someJson.at(f_parameter).is_null())
            {
                containsParameter = true;
                return someJson.at(f_parameter);
            }
            else
            {
                // if(someJson.at(f_parameter).is_null())
                return ""; // TODO: Is this the right semantic for Timeout?
            }
        }
        else
        {
            std::cerr << "" << std::endl;
            exit(EXIT_FAILURE);
        }*/
    }
}

// Ersetze ParseTree beim Suchen -> f_parseTree.findFirstChild
// Für Parameter, die nicht im config file stehen / keine optionen sind z.B. services
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
    retrieveConfigParameters(m_config);
}

gWhisperSettings::~gWhisperSettings(){}