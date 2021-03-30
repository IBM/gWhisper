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

namespace ArgParse
{
class FixedString : public GrammarElement
{
    public:
        FixedString(const std::string & f_string, const std::string & f_elementName = "") :
            GrammarElement("FixedString", f_elementName),
            m_string(f_string)
        {
        }

        virtual std::string toString() override
        {
            return m_string;
        }

        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override
        {
            ParseRc rc;
            ParseRc childRc;
            f_out_ParsedElement.setGrammarElement(this);
            //std::cout << " FixedString"<< std::to_string(m_instanceId) <<  "parsing '" << std::string(f_string) << "'" << std::endl;
            //printf("comparing: '%s' == '%s'\n", f_string, m_string.c_str());
            if(strncmp(m_string.c_str(), f_string, m_string.size()) == 0)
            {
                //printf(" -> same\n");
                rc.errorType = ParseRc::ErrorType::success;
                rc.lenParsedSuccessfully = m_string.size();
                rc.lenParsed = m_string.size();
                f_out_ParsedElement.setMatchedString(m_string);
            }
            else
            {
                rc.lenParsedSuccessfully = 0;
                if(strncmp(m_string.c_str(), f_string, strlen(f_string)) == 0)
                {
                    rc.lenParsed = strlen(f_string);
                    // have a candidate for completion :)
                    //printf(" -> completion possible\n");
                    // create a candidate:
                    auto candidate = std::make_shared<ParsedElement>(&f_out_ParsedElement);
                    candidate->setGrammarElement(this);
                    candidate->setMatchedString(m_string);
                    rc.candidates.push_back(candidate);

                    // set rc
                    rc.errorType = ParseRc::ErrorType::missingText;
                }
                else
                {
                    //printf(" -> totally different\n");
                    rc.errorType = ParseRc::ErrorType::unexpectedText;
                }
            }
            //std::cout << " FixedString"<< std::to_string(m_instanceId) <<  " rc=" << rc.toString() << std::endl;
            return rc;
        }
        virtual std::string getDotNode() override
        {
            std::string result = "";
            result += "n" + std::to_string(m_instanceId) + "[label=\"" + std::to_string(m_instanceId) + " " + m_typeName + " " + m_elementName + " " + m_tag + "'" + m_string  + "'"  + " doc: \\\""+ m_document + "\\\"" + "\"];\n";
            return result;
        }
    private:
        const std::string m_string;
};

}
