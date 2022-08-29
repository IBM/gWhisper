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

class gWhisperSetting{
    public:
    gWhisperSetting(ArgParse::ParsedElement &f_parseTree);
    ~gWhisperSetting();

    std::string lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree);
    

    private:
    void parseConfigFile(const std::string &f_inputFile);
    void retrieveConfigParameters(json &f_startElement);

    /// Searches in all layers of config file, if config contains parameter
    /// in
    /// out: returns setting for parameter, if setting is found else returns null Json object (at the moment)
    json findParameterSettingInConfig(const std::string &f_parameter, const json &f_startLayer);

    json m_config;
    std::vector<std::string> m_configParameters; 
};