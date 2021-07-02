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
#include <libArgParse/ArgParse.hpp>
namespace ArgParse
{
    class GrammarFactory
    {
        public:
            explicit GrammarFactory(Grammar & f_grammarPool) :
                m_grammarPool(f_grammarPool)
            {
            }

            /// Creates a Grammar element which is able to parse a List of the following form:
            /// {PREFIX}{ELEMENT}{SEPARATOR}{ELEMENT}{SEPARATOR}...{ELEMENT}{POSTFIX}
            /// With the text elements denoted by curly braces defined by the given
            /// GrammarElements in the function arguments.
            /// NOTE: in the generated parse tree elements are stored on first
            /// and second child level of the returned root grammar element.
            /// I.e. to get a list of parsed list elements a depth-first search
            /// with a max depth of 2 needs to be performed.
            GrammarElement * createList(const std::string & f_rootElementName, GrammarElement * f_element, GrammarElement * f_separator, bool f_emptyListAllowed, GrammarElement * f_prefix = nullptr, GrammarElement * f_postfix = nullptr)
            {
                auto listGrammar = m_grammarPool.createElement<Concatenation>(f_rootElementName);

                auto separatedEntry = m_grammarPool.createElement<Concatenation>();
                separatedEntry->addChild(f_separator);
                separatedEntry->addChild(f_element);

                auto repetition = m_grammarPool.createElement<Repetition>();
                repetition->addChild(separatedEntry);

                if(f_prefix != nullptr)
                {
                    listGrammar->addChild(f_prefix);
                }
                if(f_emptyListAllowed)
                {
                    // need to wrap list body into an optional element
                    auto optionalListBody = m_grammarPool.createElement<Optional>();
                    auto listBody = m_grammarPool.createElement<Concatenation>();
                    listBody->addChild(f_element);
                    listBody->addChild(repetition);
                    optionalListBody->addChild(listBody);
                    listGrammar->addChild(optionalListBody);
                }
                else
                {
                    // directly add list body elements to the concatenation (this gives simple grammar)
                    listGrammar->addChild(f_element);
                    listGrammar->addChild(repetition);
                }
                if(f_postfix != nullptr)
                {
                    listGrammar->addChild(f_postfix);
                }

                return listGrammar;
            }

        private:
            Grammar & m_grammarPool;
    };

}
