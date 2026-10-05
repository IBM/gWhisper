// Copyright 2026 IBM Corporation
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

#include <gtest/gtest.h>
#include <libArgParse/ArgParse.hpp>
#include <libArgParse/GrammarFactory.hpp>

using namespace ArgParse;

TEST(GrammarFactoryTest, CreateListNotEmptyAllowed) {
    Grammar pool;
    GrammarFactory factory(pool);

    GrammarElement* element = pool.createElement<FixedString>("a");
    GrammarElement* separator = pool.createElement<FixedString>(",");
    GrammarElement* listGrammar = factory.createList("a_list", element, separator, false);

    ParsedElement parsedElement;
    
    ParseRc rc = listGrammar->parse("a,a,a", parsedElement);
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(5, rc.lenParsedSuccessfully);

    ParsedElement parsedElementEmpty;
    rc = listGrammar->parse("", parsedElementEmpty);
    EXPECT_NE(ParseRc::ErrorType::success, rc.errorType);
}

TEST(GrammarFactoryTest, CreateListEmptyAllowed) {
    Grammar pool;
    GrammarFactory factory(pool);

    GrammarElement* element = pool.createElement<FixedString>("a");
    GrammarElement* separator = pool.createElement<FixedString>(",");
    GrammarElement* listGrammar = factory.createList("a_list", element, separator, true);

    ParsedElement parsedElement;
    
    ParseRc rc = listGrammar->parse("", parsedElement);
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    ParsedElement parsedElementNonEmpty;
    rc = listGrammar->parse("a,a", parsedElementNonEmpty);
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(3, rc.lenParsedSuccessfully);
}

TEST(GrammarFactoryTest, CreateListWithPrefixPostfix) {
    Grammar pool;
    GrammarFactory factory(pool);

    GrammarElement* element = pool.createElement<FixedString>("a");
    GrammarElement* separator = pool.createElement<FixedString>(",");
    GrammarElement* prefix = pool.createElement<FixedString>("[");
    GrammarElement* postfix = pool.createElement<FixedString>("]");
    GrammarElement* listGrammar = factory.createList("a_list", element, separator, false, prefix, postfix);

    ParsedElement parsedElement;
    
    ParseRc rc = listGrammar->parse("[a,a]", parsedElement);
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(5, rc.lenParsedSuccessfully);
}
