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
        m_config = json::parse(ifs); //as member? If so: is json the right 

}

void gWhisperConfig::mergeParseTreeInJson(ArgParse::ParsedElement &f_parseTree){
    // Translate Parsetree into JSON
    // (Proxy-like structure: check where to get config infos
    // Calls all other funtions)
    // check, if Parse tree includes config information
    // if so: add information to config file / override config JSON object

    /*std::cout<<"Its nothing"<<std::endl;
    std::string tmp_element;
    // std::string tmp_config = m_config.dump();
    //for (json::iterator it = m_config.begin();it != m_config.end(); it++){
    for (auto &element: m_config.items()){
        // TODO TYPING!!
        for(auto &val: element.value().items()){
            std::cout << tmp_element << std::endl;
            tmp_element = val.value().dump();
            std::string paramKey = f_parseTree.findFirstChild(tmp_element);
            //paramKey. -> can I somehow get value behind Grammar lable, without doing a switch case or simimlar?
        }       

        std::cout << element << std::endl;
    }

     std::cout<<"Its something"<<std::endl;*/

    // Other approach: Instead of looping through json, define list of valid config parameters (= keys in JSON)
    // Check, if parameter is set via cli. If so, use value of user input (= from parse tree)
    // If not: check, if parameter is set via config file. Use this value if it is set
    // Config parameters (as labeled in GrammarConstruction):
    // --ssl, --clientCert, --serverCert, --clientKey, --disableCache, -rpcTimeoutMilliseconds=, --connectTimeoutMilliseconds=
    //std::vector<std::string> configParameters = {"ssl", "OptionClientCert", "OptionClientKey", "OptionServerCert"};
    for (std::string parameter : m_configParameters)
    {
        if (f_parseTree.findFirstChild(parameter) != "" && m_config.contains(parameter))
        {
            updateConfig(parameter, f_parseTree);
            // TODO: What if parameter is not in config? --> add entry to json?
        }       
            //find string in configFile (idea: every parameter is listed in json, but maybe not set)
            //update value of parameter to value from parseTree
            //if key not in config: add new jsaon element
    }

    //}
    // "return" updated config

}

void gWhisperConfig::updateConfig(std::string &f_parameter, ArgParse::ParsedElement &f_parseTree){
    if (f_parameter == "Ssl" || f_parameter == "DisableCache" )
    {
        //TODO: brauchen  wir das Setting?
        m_config.at(f_parameter) = "True";
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

std::string gWhisperConfig::lookUpSetting(std::string &f_parameter, ArgParse::ParsedElement &f_parseTree)
{
    //Asusmption: m_config holds the newes version of config settings, already including the overwrites by user input via cmd
    //TODO: Kommen wir hier vom ParseTree weg?
    std::string setting;
    if (m_config.contains(f_parameter))
    {
        //get value setting = m_config.at(key)
    }
    else if (f_parseTree.findFirstChild(f_parameter)!= "")
    {
        setting = f_parseTree.findFirstChild(f_parameter);
        
    }
    //else: Error
    return setting;
}

/*void convertParseTree(){
    
}*/

// Ersetze ParseTree beim Suchen -> f_parseTree.findFirstChild
gWhisperConfig::gWhisperConfig(ArgParse::ParsedElement &f_parseTree){ //ParameterKey
    if(not m_config)
    {
        parseConfigFile();
    }
    mergeParseTreeInJson(f_parseTree); // Check, if we always work with the right parse tree. Maybe Chcek, if merge is neccessay
    //findParameter() -->either ParseTree or Config
}

gWhisperConfig::~gWhisperConfig(){

}