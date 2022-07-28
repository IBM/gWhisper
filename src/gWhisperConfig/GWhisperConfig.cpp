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
    
        if (f_parseTree.findFirstChild(parameter) != "" && (!checkParameterInConfig(parameter, m_config).empty()))
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

json gWhisperConfig::checkParameterInConfig(const std::string &f_parameter, json &f_startElement)
{
    bool containsParameter = false;
    json setting;
    //json configItems = m_config.items();

    for (const auto& item : f_startElement.items())
    {
        if (item.value().contains(f_parameter))
        {
            //Abbruchkrit
            std::cout << "IF in RECCHECK" << std::endl;
            setting[f_parameter] = item.value().at(f_parameter);
            std::cout << "RECSET: " << setting.dump()<<std::endl;
            std::cout << item.value() << std::endl;
            //containsParameter = true;
            return setting;
            break;
        }
        else
        {
            //json tempElement = json::parse(item.value().items().begin(), item.value().items().end());
            for(auto &innerElement : item.value().items())
            {
                if (innerElement.key() == "")
                {
                    break;
                }
                
                json tempElement;
                tempElement[innerElement.key()] = innerElement.value();
                
                // TODO:  convert innerElement to json object!
                // DO i need to implement custom Data source?
                // Hängt sich gerade am Ssl Parametrr auf, wenn er nach clientCert sucht
                // --> 2. Abbruchbedingung? Wenn unterstes Element? Eig sollte er dann gar nicht in Schleife!
                std::cout  << "RECURSION CALL: "<< tempElement.dump() << std::endl;

                checkParameterInConfig(f_parameter, tempElement); //node als zweiten Parameter
            }
        }
    }
    return setting;

    //std::cout << containsParameter << std::endl;
    //return containsParameter;
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
        //hier: Nur verwenden, wenn SSL gesetzt. Ist Reihenfolge garantiert?? Oder eher in LookupSetting --> BESSER
        
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

/*json gWhisperConfig::accessConfigValueAtKey(const std::string &f_key, json &f_startLayer)
{
    //Doppelte implementierung von checkConfigParameter. Kann man das umgehen?
    json parameterSetting;
  
    for (const auto & item: f_startLayer.items()) /// 1st startLayer= config
    {  
        if (item.value().contains(f_key))
        {
            //Abbruchkrit
            std::cout << "IF in RECACCESS" << std::endl;
            return parameterSetting[item.key()] = item.value();
            break;
        }
        else
        {
            for(auto &innerElement : item.value().items())
            {
                if (innerElement.key() == "")
                {
                    break;
                }
                
                json tempElement;
                tempElement[innerElement.key()] = innerElement.value();
                
                // TODO:  convert innerElement to json object!
                // DO i need to implement custom Data source?
                // Hängt sich gerade am Ssl Parametrr auf, wenn er nach clientCert sucht
                // --> 2. Abbruchbedingung? Wenn unterstes Element? Eig sollte er dann gar nicht in Schleife!
                std::cout  << "RECURSION CALL: "<< tempElement.dump() << std::endl;

                (f_key, tempElement); //node als zweiten Parameter
            }
        }
    }
    // needs to return: item.value().at(f_key)
    return nullptr;
}*/

// f_paramaetr entrpricht label des Grammatik Elements in ParseTree
std::string gWhisperConfig::lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree)
{
    //Asusmption: m_config holds the newes version of config settings, already including the overwrites by user input via cmd
    //TODO: Kommen wir hier vom ParseTree weg?
    bool containsParameter;
    std::cout << "Now in LOOkUP for " << f_parameter << std::endl;
    std::string setting;
    json someJson;
    someJson = checkParameterInConfig(f_parameter, m_config); //copy
    // Achung: Wahrschnl. off by one: checkParameter gitbt null objekt zurück
    //std::cout << "RECLOOKUP: " << someJson.dump()<< std::endl;
    std::cout << "RECLOOKUP: " << someJson.dump()<< std::endl;

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
    else if (someJson.is_null() && f_parseTree.findFirstChild(f_parameter)!="")
    {
        // If parameter is not founf in config file, search in parse tree (services, methods)
        setting = f_parseTree.findFirstChild(f_parameter);
    }
    
    //else:return Error

    std::cout << "Found setting for " << f_parameter << " : " << setting << std::endl;
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