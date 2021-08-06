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
class EscapedString : public GrammarElement
{
    public:
        EscapedString(const std::string & f_escapedCharacters, char f_escapeCharacter, const std::string & f_elementName = "") :
            GrammarElement("EscapedString", f_elementName),
            m_escapedCharacters(f_escapedCharacters),
            m_escapeCharacter(f_escapeCharacter)
        {
            if (m_escapedCharacters.find(m_escapeCharacter) == std::string::npos)
            {
                // escaped character als needs to be escaped
                // if user did not add it to list of escaped chars, do it now
                m_escapedCharacters.append(1, m_escapeCharacter);
            }
        }

        virtual std::string toString() override
        {
            std::string result;
            if(m_elementName != "")
            {
                result = m_elementName + ":" + std::to_string(m_instanceId) + " ";
            }
            result += "/[^" + m_escapedCharacters + "]* " + " escaped by " + m_escapeCharacter;
            return result;
        }

        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override
        {
            ParseRc rc;
            ParseRc childRc;
            f_out_ParsedElement.setGrammarElement(this);
            //std::cout << " FixedString"<< std::to_string(m_instanceId) <<  "parsing '" << std::string(f_string) << "'" << std::endl;
            
            const char* parsePos = f_string;
            const char * interestingPosition;
            std::string matchedStringUnEscaped;
            while(1)
            {
                interestingPosition = strpbrk(parsePos, m_escapedCharacters.c_str());
                if(interestingPosition == nullptr)
                {
                    // not found, whole string matches :)
                    matchedStringUnEscaped += std::string(parsePos);
                    parsePos += strlen(parsePos);
                    break;
                }
                if(*(interestingPosition) == m_escapeCharacter and strpbrk(interestingPosition+1, m_escapedCharacters.c_str()) == interestingPosition+1)
                {
                    // found escaped character -> continue parse
                    matchedStringUnEscaped += std::string(parsePos, interestingPosition - parsePos);

                    matchedStringUnEscaped += *(interestingPosition+1);

                    parsePos = interestingPosition+2;
                }
                else
                {
                    if(parsePos != interestingPosition)
                    {
                        matchedStringUnEscaped += std::string(parsePos, interestingPosition - parsePos);
                    }
                    parsePos = interestingPosition;
                    // found end of string
                    break;
                }
            }
            // Now parsePos points to one character behind last matched.

            rc.lenParsedSuccessfully = parsePos - f_string;
            rc.lenParsed = parsePos - f_string;
            f_out_ParsedElement.setMatchedString(std::string(f_string, rc.lenParsedSuccessfully));
            f_out_ParsedElement.setMatchedStringUnescaped(matchedStringUnEscaped);

            return rc;
        }
        virtual std::string getDotNode() override
        {
            std::string result = "";
            result += "n" + std::to_string(m_instanceId) + "[label=\"" + std::to_string(m_instanceId) + " " + m_typeName + " " + m_elementName + " " + m_tag + "'[^" + m_escapedCharacters  + "]* esc: " + m_escapeCharacter + "'"  + " doc: \\\""+ m_document + "\\\"" + "\"];\n";
            return result;
        }
    private:
        std::string m_escapedCharacters;
        char m_escapeCharacter;
};

}
