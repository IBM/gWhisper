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
class Repetition : public GrammarElement
{
    public:
        explicit Repetition(const std::string & f_elementName = ""):
            GrammarElement("Repetition", f_elementName)
        {
        }

        virtual std::string toString() override
        {
            std::string result;
            result += "(";
            result += m_children[0]->toString();
            result += ")*";
            return result;
        }

        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override
        {
            //printf("rep parse\n");
            ParseRc rc;
            ParseRc childRc;
            f_out_ParsedElement.setGrammarElement(this);

            GrammarElement * child = nullptr;
            if(m_children.size() > 0)
            {
                child = m_children[0];
            }
            std::vector<std::shared_ptr<ParsedElement>> successfullyParsedChilds;
            bool overParsed = false;
            while(childRc.isGood() && (child != nullptr) )
            {
                if(childRc.isGood() && (f_string[rc.lenParsedSuccessfully] == '\0'))
                {
                    // we parsed successfully and exactly aligned with the end :)
                    // we continue parsing once more, to get possible further candidates
                    // then we end
                    // we set this flag here, to remember to switch the RC to success
                    overParsed = true;
                }
                auto newParsedElement = std::make_shared<ParsedElement>(&f_out_ParsedElement);
                //printf("Optional start parse\n");
                childRc = child->parse(&f_string[rc.lenParsedSuccessfully], *newParsedElement);
                //std::cout << " Rep "<< std::to_string(m_instanceId) <<  " parsed child. rc=" << childRc.toString() << std::endl;
                //printf("Optional parse RC: ");
                //childRc.print();
                //printf("\n");
                if(childRc.isGood() || childRc.errorType == ParseRc::ErrorType::missingText)
                {
                    rc.lenParsedSuccessfully += childRc.lenParsedSuccessfully;
                    rc.lenParsed += childRc.lenParsed;
                    f_out_ParsedElement.addChild(newParsedElement);
                }
                if(childRc.isGood())
                {
                    // TODO: make unittest where those lists are different (partial parse should only complete partial result, but not append to successful list completions)
                    successfullyParsedChilds.push_back(newParsedElement);
                }
                if(childRc.isBad())
                {
                    rc.ErrorMessage = childRc.ErrorMessage;
                    if (childRc.errorType == ParseRc::ErrorType::retrievingGrammarFailed)
                    {
                        rc.errorType = ParseRc::ErrorType::retrievingGrammarFailed;
                    }
                }
            }
            // FIXME: maybe we have to move this in the loop and also process good rcs here with candidates (from optional/repetition/etc.) similarly to concat
            if (childRc.isBad() && (childRc.errorType != ParseRc::ErrorType::unexpectedText))
            {
                // add all candidates resulting from the child:
                for (auto candidate : childRc.candidates)
                {
                    //std::cout << "Rep " << std::to_string(m_instanceId) << " handling candidate '" << candidate->getMatchedString() << "'" << std::endl;
                    //printf("add optional candidate : '%s'\n", candidate->getMatchedString().c_str());
                    auto realCandidate = std::make_shared<ParsedElement>(f_out_ParsedElement.getParent());
                    realCandidate->setGrammarElement(this);
                    realCandidate->setStops(); // think about this is this required for repetition?
                    // add all previous childs (similar to concatenation):
                    for (auto previousChild : successfullyParsedChilds)
                    {
                        realCandidate->addChild(previousChild);
                    }
                    realCandidate->addChild(candidate);
                    //std::cout << " Rep "<< std::to_string(m_instanceId) <<  " add candidate '" << realCandidate->getMatchedString() << "'" << std::endl;
                    rc.candidates.push_back(realCandidate);
                }
                if(overParsed)
                {
                    // if we are at the end of the string we return no error
                    // -> why??
                    rc.errorType = ParseRc::ErrorType::success;
                }
                else
                {
                    if (childRc.errorType == ParseRc::ErrorType::retrievingGrammarFailed)
                    {
                        rc.ErrorMessage = childRc.ErrorMessage;
                        rc.errorType = ParseRc::ErrorType::retrievingGrammarFailed;
                    }
                    else
                    {
                        // otherwise we still have missing text
                        // FIXME: this makes repeated element not optional in some cases
                        rc.errorType = ParseRc::ErrorType::missingText;
                        //rc.errorType = ParseRc::ErrorType::success;
                    }
                }
            }
            else
            {
                // if we saw something completely wrong, we declare us as success (we also allow zero matches)
                rc.errorType = ParseRc::ErrorType::success;
            }

            // FIXME empty string is also a candidate!!
            return rc;
        }
};

}
