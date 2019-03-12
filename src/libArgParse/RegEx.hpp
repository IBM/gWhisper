// Copyright 2019 IBM Corporation
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
#include <libArgParse/GrammarElement.hpp>

#ifdef BUILD_CONFIG_USE_BOOST_REGEX
    #include <boost/regex.hpp>
#else
    #include <regex>
#endif

namespace ArgParse
{

// we use our own regex namespace here in which we provide regex types.
// Depending on build configuration these are either boost or standard library
// regex implementations
namespace regex
{
    #ifdef BUILD_CONFIG_USE_BOOST_REGEX
        using boost::cmatch;
        using boost::regex_search;
        using boost::regex;
    #else
        using std::cmatch;
        using std::regex_search;
        using std::regex;
    #endif
}

class RegEx : public GrammarElement
{
    public:

        RegEx(const std::string & f_regEx, const std::string & f_elementName = "") :
            GrammarElement("RegEx", f_elementName),
            m_regEx(f_regEx),
            m_regExString(f_regEx)
        {
        }

        virtual std::string toString() override
        {
            std::string result;
            if(m_elementName != "")
            {
                result = "/" + m_elementName + ":" + std::to_string(m_instanceId) + "/";
            }
            else if(m_regExString != "")
            {
                result += "/";
                result += m_regExString;
                result += "/";

            }
            else
            {
                result = m_typeName + "(UnnamedRegex:" + std::to_string(m_instanceId) + ")";
            }
            return result;
        }

        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override
        {
            ParseRc rc;
            ParseRc childRc;
            f_out_ParsedElement.setGrammarElement(this);

            //std::cmatch match;
            regex::cmatch match;
            if(
                    //std::regex_search(f_string, match, m_regEx)
                    regex::regex_search(f_string, match, m_regEx)
                    and
                    (match.position() == 0)
              )
            {
                // match has to be at the beginning
                rc.errorType = ParseRc::ErrorType::success;
                rc.lenParsedSuccessfully = match.length();
                rc.lenParsed = match.length();
                f_out_ParsedElement.setMatchedString(match[0]);
                //printf("regex %u /%s/ did match\n", m_instanceId, m_regExString.c_str());
            }
            else
            {
                //printf("regex %u /%s/ did not match\n", m_instanceId, m_regExString.c_str());
                rc.lenParsedSuccessfully = 0;
                rc.lenParsed = strlen(f_string);
                if(strlen(f_string) == 0)
                {
                    //printf("regex %u /%s/ have missing text\n", m_instanceId, m_regExString.c_str());
                    rc.errorType = ParseRc::ErrorType::missingText;
                }
                else
                {
                    rc.errorType = ParseRc::ErrorType::unexpectedText;
                }
            }

            return rc;
        }
        virtual std::string getDotNode() override
        {
            std::string result = "";
            result += "n" + std::to_string(m_instanceId) + "[label=\"" + std::to_string(m_instanceId) + " " + m_typeName + " " + m_elementName + " " + m_tag + "'" + m_regExString  + "'\"];\n";
            return result;
        }
    private:
        //const std::regex m_regEx;
        const regex::regex m_regEx;
        const std::string m_regExString;
};

}
