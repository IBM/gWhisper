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

class gWhisperSettings{
    public:
    /// Retrieves inital config object.
    // Uses the parse tree only to retrieve the location of the config gile.
    /// @param f_parseTree 
    gWhisperSettings(ArgParse::ParsedElement &f_parseTree);
    ~gWhisperSettings();

    // /Searches setting for a parameter in the parse tree.
    /// Searches for in the config file, if parameter is not found in the parse tree.
    /// @param f_parameter
    /// @param f_parseTree
    /// @return Returns value of the parameter as string. Returns an empty string
    /// if the parameter is not set.
    std::string lookUpSetting(const std::string &f_parameter, ArgParse::ParsedElement &f_parseTree);
    

    private:
    /// Parses a valid json file into a json object
    /// @param f_inputFile
    void parseConfigFile(const std::string &f_inputFile);

    /// Recursively retrieves all keys from a (nested) json file
    /// @param f_startElement
    void retrieveConfigParameters(json &f_startElement);

    /// Recursively searches the value of a key in a (nested) json object
    /// @param f_parameter
    /// @param f_startLayer
    /// @return returns setting for parameter, if setting is found else returns null Json object (at the moment)
    std::string findParameterSettingInConfig(const std::string &f_parameter, const json &f_startLayer);

    json m_config;
    std::vector<std::string> m_configParameters; 
};