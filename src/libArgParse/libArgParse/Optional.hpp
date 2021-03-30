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
        explicit Optional(const std::string & f_elementName = "") :
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

            ParseRc candidateRc;
            if(m_children.size() == 0)
            {
                rc.ErrorMessage = "Optional Element has no child. Grammar incomplete.";
                rc.errorType = ParseRc::ErrorType::retrievingGrammarFailed;
                return rc;
            }

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
            //if((not childRc.isGood()) && (childRc.errorType != ParseRc::ErrorType::unexpectedText))
            if(childRc.errorType != ParseRc::ErrorType::unexpectedText)
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
                if(childRc.lenParsedSuccessfully == 0)
                {
                    // nothing parsed -> end of string
                    // we add also the not taken option (empty candidate):
                    auto realCandidate = std::make_shared<ParsedElement>(f_out_ParsedElement.getParent());
                    realCandidate->setGrammarElement(this);
                    realCandidate->setStops();
                    rc.candidates.push_back(realCandidate);
                }
            }

            // create rc code:
            if((childRc.errorType == ParseRc::ErrorType::missingText) && (childRc.lenParsed >= 1))
            {
                // In this case we reached the end of the text.
                // but we found out, that the option is actually selected as we matched at least one char
                rc.lenParsed += childRc.lenParsed;
                rc.errorType = ParseRc::ErrorType::missingText;
            }
            else if(childRc.isBad() && childRc.errorType == ParseRc::ErrorType::retrievingGrammarFailed)
            {
                rc.ErrorMessage = childRc.ErrorMessage;
                rc.errorType = ParseRc::ErrorType::retrievingGrammarFailed;
            }
            else
            {
                rc.errorType = ParseRc::ErrorType::success;
            }

            //std::cout << "Optional "<< std::to_string(m_instanceId) <<  " returning rc=" << rc.toString() << " with " << std::to_string(rc.candidates.size()) << " candidates" << std::endl;
            return rc;
        }
};

}
