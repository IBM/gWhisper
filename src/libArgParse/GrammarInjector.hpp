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
#include <libArgParse/Grammar.hpp>

namespace ArgParse
{

class GrammarInjector : public GrammarElement
{
    public:
        explicit GrammarInjector(const std::string & f_typeName, const std::string & f_elementName = "") :
            GrammarElement("GrammarInjector::" + f_typeName, f_elementName)
        {
        }
        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override final
        {
            if(m_children.size() == 0)
            {
                ParseRc rc;
                // we first need to inject new grammar:
                GrammarElement * newGrammar = getGrammar(f_out_ParsedElement.getRoot(), rc.ErrorMessage);

                if(newGrammar != nullptr)
                {
                    // retrieving grammar succeeded :-)
                    addChild(newGrammar);
                }
                else
                {
                    // retrieving grammar failed :-(
                    // -> we need to cause parse to fail due to missing grammar.
                    if(rc.ErrorMessage.size() != 0)
                    {
                        rc.errorType = ParseRc::ErrorType::retrievingGrammarFailed;
                    }
                    else
                    {
                        rc.errorType = ParseRc::ErrorType::unexpectedText;
                    }
                    return rc;
                }
            }

            f_out_ParsedElement.setGrammarElement(this);
            auto child = std::make_shared<ParsedElement>(&f_out_ParsedElement);
            // we transparently skip to parsing the new child
            //return m_children[0]->parse(f_string, f_out_ParsedElement, candidateDepth);
            ParseRc childRc = m_children[0]->parse(f_string, *child, candidateDepth);

            f_out_ParsedElement.addChild(child);

            return childRc;
        }

        virtual GrammarElement * getGrammar(ParsedElement * f_parseTree, std::string & f_ErrorMessage) = 0;
};
}
