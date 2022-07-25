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
    std::string line;
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

// Consolidate all cli or config file config parameters in one json_object
// Config Unabhängig vom ParseTree machen
//
void gWhisperConfig::mergeParseTreeInJson(ArgParse::ParsedElement &f_parseTree){
    // Translate Parsetree into JSON
    // (Proxy-like structure: check where to get config infos
    // Calls all other funtions)
    // check, if Parse tree includes config information
    // if so: add information to config file / override config JSON object

    // Other approach: Instead of looping through json, define list of valid config parameters (= keys in JSON)
    // Check, if parameter is set via cli. If so, use value of user input (= from parse tree)
    // If not: check, if parameter is set via config file. Use this value if it is set
    // Config parameters (as labeled in GrammarConstruction):
    // --ssl, --clientCert, --serverCert, --clientKey, --disableCache, -rpcTimeoutMilliseconds=, --connectTimeoutMilliseconds=
    //std::vector<std::string> configParameters = {"ssl", "OptionClientCert", "OptionClientKey", "OptionServerCert"};
    for (std::string parameter : m_configParameters)
    {
        //new method is in config
    
        if (f_parseTree.findFirstChild(parameter) != "" && checkParameterInConfig(parameter))
        {
            std::cout << "IF inMergeParseTree" << std::endl;
            updateConfig(parameter, f_parseTree);
            // TODO: What if parameter is not in config? --> add entry to json?
        }       
            //find string in configFile (idea: every parameter is listed in json, but maybe not set)
            //update value of parameter to value from parseTree
            //if key not in config: add new jsaon element

    }
    // "return" updated config
}

bool gWhisperConfig::checkParameterInConfig(const std::string &f_parameter)
{
    bool containsParameter = false;

    for (const auto& item : m_config.items())
    {
        std::cout<< "CHECK PARAMETER:" << f_parameter<<std::endl;
        if (item.value().contains(f_parameter))
        {
            std::cout << "IF in CHECK" << std::endl;
            containsParameter = true;
            break;

        }

        // If parameter is not found in 1st layer, search in 2nd layer of config file
        for (const auto& innerElement : item.value().items())
        {
            std::cout<< "CHECK PARAMETER:" << f_parameter<<std::endl;
            if(innerElement.value().contains(f_parameter)){
                containsParameter = true;
                break;
            }
            else{
                std::cerr << "Error while setting " << f_parameter << " :No such parameter in config!" << std::endl;
            }
        }
    }
    std::cout << containsParameter << std::endl;
    return containsParameter;
}


void gWhisperConfig::updateConfig(std::string &f_parameter, ArgParse::ParsedElement &f_parseTree){
    if (f_parameter == "Ssl" || f_parameter == "DisableCache" )
    {
         for (const auto& item : m_config.items())
         {
             item.value().at(f_parameter) = "YES";
         }
        //TODO: brauchen  wir das Setting?
        // TODO: Richtiges überschreiben mir richtigen ebenen!!
        //m_config.at(f_parameter) = "Yes";
        //m_config.value().items().a
        std::cout << "OVERRIDE CONFIG" << std::endl;
    } 
    else if (f_parameter == "ClientCertFile")
    {
        // TODO: What when default File is used (no parameter given by user)
        std::string newSetting = f_parseTree.findFirstChild(f_parameter);
        m_config.at(f_parameter) = newSetting;

    }
    else if (f_parameter == "ClientKeyFile")
    {
        
    }
     else if (f_parameter == "ServerCertFile")
    {

    }
    else if (f_parameter == "DisableCache")
    {
        
    }
     else if (f_parameter == "RpcTimeout")
    {

    }
    else if (f_parameter == "connectTimeout")
    {
        
    }
}

std::unique_ptr<std::string> gWhisperConfig::accessConfigValueAtKey(const std::string &f_key)
{
    //needs value as return type. Macht das Sinn?
    bool containsParameter = checkParameterInConfig(f_key);

    if(containsParameter)
    {
        for (const auto& item : m_config.items())
            {
                if (item.value().at(f_key).is_null())
                {
                    std::cout << "ENTERED NULL-IF"<< std::endl;
                    //setting = "";
                    break;
                } 
                else 
                {
                    // TODO is there a simple conversion to string for everything?
                    //setting = item.value().at(f_parameter);
                    break;
                }
                break;  
            }

    }
    // needs to return: item.value().at(f_key)
    return nullptr;
}

// f_paramaetr entrpricht label des Grammatik Elements in ParseTree
std::string gWhisperConfig::lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree)
{
    //Asusmption: m_config holds the newes version of config settings, already including the overwrites by user input via cmd
    //TODO: Kommen wir hier vom ParseTree weg?
    std::string setting;

    // first layer of config file
    for (const auto& item : m_config.items())
    {
        std::cout<< "PARAMETER:" << f_parameter<<std::endl;
        std::cout << "key: " << item.key() << ", value: " << item.value() << '\n';

        bool containsParameter = checkParameterInConfig(f_parameter);


        if(containsParameter)
        {
            for (const auto& item : m_config.items())
            {
                if (item.value().at(f_parameter).is_null())
                {
                    std::cout << "ENTERED NULL-IF"<< std::endl;
                    setting = "";
                    break;
                } 
                else 
                {
                    // TODO is there a simple conversion to string for everything?
                    setting = item.value().at(f_parameter);
                    break;
                }
                break;  
            }
        }

        // If parameter is not found in 1st layer, search in 2nd layer of config file
        for (const auto& innerElement : item.value().items())
            {
            std::cout<< "PARAMETER:" << f_parameter<<std::endl;
            if(innerElement.value().contains(f_parameter)){
                std::cout << "SETTING IN 2nd LOOP " << std::endl;
                setting = innerElement.value().at(f_parameter);
                break;
            }
            std::cout << "2nd Loop:" << std::endl;
            std::cout << "key: " << innerElement.key() << ", value: " << innerElement.value() << '\n';
     
            
            std::cout << "SETTING: " << std::endl;
            std::cout << setting << std::endl;
            }



        // If parameter is not founf in config file, search in parse tree
        if (f_parseTree.findFirstChild(f_parameter)!= "")
        //--> eigentlich brauche ich das hier doch gar nicht mehr, oder? Weg von Proxyidee
        {
            setting = f_parseTree.findFirstChild(f_parameter);
        }
        //else:return Error
    }
    return setting;
}

/*void convertParseTree(){
    
}*/

// Ersetze ParseTree beim Suchen -> f_parseTree.findFirstChild
// Für Parameter, die nicht im config file stehen / keine optionen sind z.B. services
gWhisperConfig::gWhisperConfig(ArgParse::ParsedElement &f_parseTree){ //ParameterKey
    if(m_config.is_null())
    {
        parseConfigFile();
    }
    std::cout << "Constructor" << std::endl;
    mergeParseTreeInJson(f_parseTree); // Check, if we always work with the right parse tree. Maybe Chcek, if merge is neccessay
    //findParameter() -->either ParseTree or Config
}

gWhisperConfig::~gWhisperConfig(){

}