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

#include "GWhisperConfig.hpp"
#include "libArgParse/ArgParse.hpp"

using json = nlohmann::json;

void gWhisperConfig::parseConfigFile(){
    // Parse JSON File into JSON object
    // JSON Object as member
    std::ifstream ifs("/home/anna/.cache/gwhisper/config.json");
    //std::string line;
    if (!ifs.is_open())
    {
        std::cout << "Error while opening file" << std::endl;
    }
    // TODO: check for empty value with  nlohmann::isempty()
    // m_config = JSON onbject
    //m_config = json::parse(ifs); //as member? If so: is json the right 
    ifs >> m_config;
    ifs.close();

}

void gWhisperConfig::retrieveConfigParameters(json &f_startElement){
    //recursively go over config file and retrieve all Parameterrs = Keys
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

json gWhisperConfig::findParameterSettingInConfig(const std::string &f_parameter,const  json &f_startElement)
{
    bool containsParameter = false;
    //std::cout << "REC WITH: " << f_startElement.dump()<< std::endl;
    json setting;

    for (const auto& item : f_startElement.items())
    {
        std::cout << "OUTER REC WITH: " << item.key() << " : " << item.value()<< std::endl;
        //if (f_startElement.contains(f_parameter))
        if (item.value().contains(f_parameter))
        {
            //Abbruchkrit
            std::cout << "REC WITH: " << item.key() << " : " << item.value()<< std::endl;
            std::cout << "IF in RECCHECK" << std::endl;
            setting[f_parameter] = item.value().at(f_parameter);
            std::cout << "RECSET: " << setting.dump()<<std::endl;
            std::cout << item.value() << std::endl;
            break;
        }
        /*else --> for now neverending loop
        {
            if (item.key() == "")
            {
                break; 
            }

            if (item.value().is_structured())
            {
                std::cout << "RECURSION_CALL: " << item.key() << " ; " << item.value() << std::endl;
                json innerElement;
                innerElement[item.key()] = item.value();
                return findParameterSettingInConfig(f_parameter, innerElement);
            } 
            else
            {
                continue;
            }

        }*/
        for(auto &innerElement : item.value().items())
        //for (auto innerElement = f_startElement.begin(); innerElement != f_startElement.end(); ++innerElement)
        {   
            // maybe something like this to avoid copy later: if (*innerElement->contains(f_parameter)){}
            if (innerElement.key() == "")
            {
                break; 
            }

            // Call recursion only, if current element contains further elements
            if(innerElement.value().is_structured())
            {
                // Copy is expensive! Workaround?
                // Warum hier nicht auf akuteller m_config?
                json tempElement;
                tempElement[innerElement.key()] = innerElement.value();
                std::cout << "RECURSION_CALL: " << innerElement.key() << " ; " << innerElement.value() << std::endl;
                return findParameterSettingInConfig(f_parameter, tempElement); //node als zweiten Parameter
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
std::string gWhisperConfig::lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree)
{
    // live wie in Proxy
    //Asusmption: m_config holds the newest version of config settings, already including the overwrites by user input via cmd
    //TODO: Kommen wir hier vom ParseTree weg?
    bool containsParameter;
    std::cout << "Now in LOOkUP for " << f_parameter << std::endl;
    std::string setting;
    json someJson;
    std::cout<< "CURRENT CONFIG: " << m_config.dump() << std::endl;

    if (f_parseTree.findFirstChild(f_parameter) != "")
    {
        setting = f_parseTree.findFirstChild(f_parameter);
    }
    else
    {
        someJson = findParameterSettingInConfig(f_parameter, m_config); //copy
        // Achung:checkParameter kann null objekt zurück geben! Warum? TODO: In checkParameter beheben
        // std::cout << "RECLOOKUP: " << someJson.dump()<< std::endl;

        if (!someJson.is_null())
        {    
            if (!someJson.at(f_parameter).is_null())
            {
                std::cout << "SUCCESS" << std::endl;
                containsParameter = true;
                setting = someJson.at(f_parameter);
            }

            if(someJson.at(f_parameter).is_null())
            {
                std::cout << "ENTERED NULL-IF"<< std::endl;
                setting = "";
            }
        }
    }
    //else:return Error
    std::cout << "Found setting for " << f_parameter << " : " << setting << std::endl;
    return setting;
}

// Ersetze ParseTree beim Suchen -> f_parseTree.findFirstChild
// Für Parameter, die nicht im config file stehen / keine optionen sind z.B. services
gWhisperConfig::gWhisperConfig(ArgParse::ParsedElement &f_parseTree){ //ParameterKey
    if(m_config.is_null())
    {
        parseConfigFile();
    }
    std::cout << "Constructor" << std::endl;
    retrieveConfigParameters(m_config);
    // Check, if we always work with the right parse tree. Maybe Chcek, if merge is neccessay
    //findParameter() -->either ParseTree or Config
    std::cout << "PARAMETER LIST:" << std::endl;
    for (std::string param : m_configParameters)
    {
        std::cout << param << std::endl;
    }
}

gWhisperConfig::~gWhisperConfig(){

}