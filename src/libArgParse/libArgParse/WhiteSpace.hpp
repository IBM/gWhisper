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
class WhiteSpace : public GrammarElement
{
    public:
        WhiteSpace() :
            GrammarElement("WhiteSpace")
        {
        }

        virtual std::string toString() override
        {
            return " ";
        }

        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override
        {
            ParseRc rc;
            ParseRc childRc;
            f_out_ParsedElement.setGrammarElement(this);

            std::string matchedString = "";
            size_t i;
            for(i = 0; i< strlen(f_string); i++)
            {
                if(f_string[i] == ' ')
                {
                    matchedString += f_string[i];
                }
                else
                {
                    break;
                }
            }
            //printf("comparing: '%s' == '%s'\n", f_string, m_string.c_str());
            if(matchedString != "")
            {
                //printf(" -> same\n");
                rc.errorType = ParseRc::ErrorType::success;
                rc.lenParsedSuccessfully = i;
                rc.lenParsed = i;
                f_out_ParsedElement.setMatchedString(matchedString);
            }
            else
            {
                rc.lenParsedSuccessfully = 0;
                if(i == strlen(f_string))
                {
                    rc.lenParsed = strlen(f_string);
                    // have a candidate for completion :)
                    //printf(" -> completion possible\n");
                    // create a candidate:
                    auto candidate = std::make_shared<ParsedElement>(&f_out_ParsedElement);
                    candidate->setGrammarElement(this);
                    candidate->setMatchedString(" ");
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

            return rc;
        }
};

}
