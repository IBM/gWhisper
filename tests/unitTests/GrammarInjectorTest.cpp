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

#include <libArgParse/ArgParse.hpp>
using namespace ArgParse;

// -----------------------------------------------------------------------------
//          Mock instances inherited from GrammarInjector
// -----------------------------------------------------------------------------

class GrammarInjectorTest : public GrammarInjector
{
    public:
        explicit GrammarInjectorTest(Grammar & f_grammar) :
            GrammarInjector("Test"),
            m_grammar(f_grammar)
        {
        }

        virtual GrammarElement * getGrammar(ParsedElement * f_parseTree, std::string & f_ErrorMessage) override
        {
            auto result = m_grammar.createElement<Alternation>();
            result->addChild(m_grammar.createElement<FixedString>("inject1"));
            result->addChild(m_grammar.createElement<FixedString>("inject2"));
            return result;
        };

    private:
        Grammar & m_grammar;
};

class GrammarInjectorMockServicesError : public GrammarInjector
{
    public:
        GrammarInjectorMockServicesError(Grammar & f_grammar, const std::string & f_elementName = "") :
            GrammarInjector("Service", f_elementName),
            m_grammar(f_grammar)
        {
        }

        virtual ~GrammarInjectorMockServicesError()
        {
        }

        virtual GrammarElement * getGrammar(ParsedElement * f_parseTree, std::string & f_ErrorMessage) override
        {

            f_ErrorMessage = "Error: Server not found.";
            return nullptr;
        };

    private:
        Grammar & m_grammar;
};

class GrammarInjectorMockServicesSuccess : public GrammarInjector
{
    public:
        GrammarInjectorMockServicesSuccess(Grammar & f_grammar, const std::string & f_elementName = "") :
            GrammarInjector("Service", f_elementName),
            m_grammar(f_grammar)
        {
        }

        virtual ~GrammarInjectorMockServicesSuccess()
        {
        }

        virtual GrammarElement * getGrammar(ParsedElement * f_parseTree, std::string & f_ErrorMessage) override
        {

            f_ErrorMessage = "";
            std::string service = "127.0.0.1:50051";
            auto result = m_grammar.createElement<Alternation>();
            result->addChild(m_grammar.createElement<FixedString>(service));
            return result;
        };

    private:
        Grammar & m_grammar;
};