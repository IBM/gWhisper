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

class Optional : public GrammarElement
{
    public:
        Optional(const std::string & f_elementName = "") :
            GrammarElement("Optional", f_elementName)
        {
        }

        virtual std::string toString() override
        {
            std::string result;
            result += "[";
            for(auto child: m_children)
            {
                result += child->toString();
            }
            result += "]";
            return result;
        }

        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override
        {
            ParseRc rc;
            ParseRc childRc;
            f_out_ParsedElement.setGrammarElement(this);

            GrammarElement * candidate = nullptr;
            ParseRc candidateRc;

            auto child = m_children[0]; // FIXME: range check
            auto newParsedElement = std::make_shared<ParsedElement>(&f_out_ParsedElement);
            //printf("Optional start parse\n");
            childRc = child->parse(&f_string[rc.lenParsedSuccessfully], *newParsedElement);
            //printf("Optional parse RC: ");
            //childRc.print();
            //printf("\n");
            if(childRc.isGood())
            {
                rc.lenParsedSuccessfully += childRc.lenParsedSuccessfully;
                rc.lenParsed += childRc.lenParsed;
                f_out_ParsedElement.addChild(newParsedElement);
            }
            if((not childRc.isGood()) && (childRc.errorType != ParseRc::ErrorType::unexpectedText))
            {
                // add all candidates resulting from the child:
                for(auto candidate : childRc.candidates)
                {
                //printf("add optional candidate : '%s'\n", candidate->getMatchedString().c_str());
                    auto realCandidate = std::make_shared<ParsedElement>(f_out_ParsedElement.getParent());
                    realCandidate->setGrammarElement(this);
                    realCandidate->setStops();
                    realCandidate->addChild(candidate);
                    rc.candidates.push_back(realCandidate);
                }
            }

            rc.errorType = ParseRc::ErrorType::success;

            return rc;
        }
};

}
