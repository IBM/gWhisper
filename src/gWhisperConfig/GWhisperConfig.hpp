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

#pragma once
#include <nlohmann/json.hpp>
#include "libArgParse/ArgParse.hpp"

using json = nlohmann::json;

class gWhisperConfig{
    public:
    gWhisperConfig(ArgParse::ParsedElement &f_parseTree);
    ~gWhisperConfig();

    std::string lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree); //Parse Tree als Member?
    

    private:
    void parseConfigFile();
    void mergeParseTreeInJson(ArgParse::ParsedElement &f_parseTree);
    void updateConfig(std::string &f_parameter, ArgParse::ParsedElement &f_parseTree);

    json m_config;
    std::vector<std::string> m_configParameters = {"Ssl", "ClientCertFile", "ClientKeyFile", "ServerCertFile"};

    // Idee: Diese Klasse als langlebiges Object --> z.B. direkt in main parse tree ersetzen und newParseTree weitergeben
    // NewParseTree: 
    // Alle Lookups auf Parse Tree ersetzen mit LookUp 

};