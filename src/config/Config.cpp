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

#include "Config.hpp"
#include "libArgParse/ArgParse.hpp"

using json = nlohmann::json;

void Config::parseConfigFile(){
    // Parse JSON File into JSON object
    // JSON Object as member
       std::ifstream ifs("/home/anna/.cache/gwhisper/config.json");
        std::string line;
        if (!ifs.is_open()){
            std::cout << "Error while opening file" << std::endl;
        }
        m_config = json::parse(ifs); //as member? If so: is json the right 

}

void Config::mergeParseTreeInJson(ArgParse::ParsedElement &f_parseTree){
    // Translate Parsetree into JSON
    // (Proxy-like structure: check where to get config infos
    // Calls all other funtions)
    // check, if Parse tree includes config information
    // if so: add information to config file / override config JSON object
    for (auto& element : m_config){
        // TODO TYPING!!
        f_parseTree.findFirstChild(element);
        std::cout << element << std::endl;
    }
}

void convertParseTree(){
    
}

void fetchConfig(){
    //fetch config relevant options 
    // return config parameters to use --> set as new grammar root
    // todo: How to convert config  object in Grammar object?
}

// Konstruktor: calls choose Inout on every call?
// Problem: Parses config everytime. TODO: possibility toavoid this?