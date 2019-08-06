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

#include <gtest/gtest.h>
#include <libArgParse/ArgParse.hpp>
using namespace ArgParse;

// -----------------------------------------------------------------------------
//          FixedString
// -----------------------------------------------------------------------------

TEST(FixedStringTest, EmptyFixedStringEmptyString) {
    FixedString myFixedString("");
    ParsedElement parsedElement;

    ParseRc rc = myFixedString.parse("", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.candidates.size());
    EXPECT_EQ(0, rc.lenParsedSuccessfully);
}

TEST(FixedStringTest, EmptyFixedStringNonEmptyString) {
    FixedString myFixedString("");
    ParsedElement parsedElement;

    ParseRc rc = myFixedString.parse("test", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.candidates.size());
    EXPECT_EQ(0, rc.lenParsedSuccessfully);
}

TEST(FixedStringTest, EmptyString) {
    FixedString myFixedString("asd");
    ParsedElement parsedElement;

    ParseRc rc = myFixedString.parse("", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.candidates.size());
    EXPECT_EQ("asd", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(0, rc.lenParsedSuccessfully);
    EXPECT_EQ(&myFixedString, rc.candidates[0]->getGrammarElement());
}

TEST(FixedStringTest, MatchingString) {
    FixedString myFixedString("asd");
    ParsedElement parsedElement;

    ParseRc rc = myFixedString.parse("asd", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(3, rc.lenParsedSuccessfully);
    EXPECT_EQ(0, rc.candidates.size());
}

TEST(FixedStringTest, MatchingStringTooMuch) {
    FixedString myFixedString("asd");
    ParsedElement parsedElement;

    ParseRc rc = myFixedString.parse("asdfgh", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(3, rc.lenParsedSuccessfully);
    EXPECT_EQ(0, rc.candidates.size());
}

TEST(FixedStringTest, HalfMatchingString) {
    FixedString myFixedString("asd");
    ParsedElement parsedElement;

    ParseRc rc = myFixedString.parse("a", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.candidates.size());
    EXPECT_EQ(0, rc.lenParsedSuccessfully);
    EXPECT_EQ("asd", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myFixedString, rc.candidates[0]->getGrammarElement());
}

TEST(FixedStringTest, NonMatchingString) {
    FixedString myFixedString("asd");
    ParsedElement parsedElement;

    ParseRc rc = myFixedString.parse("b", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);
    EXPECT_EQ(0, rc.candidates.size());
}

// Not yet support
// TEST(FixedStringTest, GrammarInjectorWrongServer) {
//     FixedString myFixedString("127.0.0.1:");
//     ParsedElement parent;
//     ParsedElement parsedElement(&parent);

//     Grammar grammarPool;
//     GrammarInjectorMockServicesError inject1(grammarPool);
//     FixedString.addChild(&inject1);
//     ParseRc rc = myAlternation.parse("129.0.0.1 examples", parsedElement);

//     // rc:
//     ASSERT_NE(0, rc.ErrorMessage.size());
//     EXPECT_EQ(ParseRc::ErrorType::retrievingGrammarFailed, rc.errorType);
//     EXPECT_EQ(0, rc.lenParsedSuccessfully);

//     // candidates:
//     ASSERT_EQ(0, rc.candidates.size());

//     // parsedElement
//     ASSERT_EQ(0, parsedElement.getChildren().size());
//     EXPECT_EQ(&parent, parsedElement.getParent());
//     EXPECT_EQ(false, parsedElement.isStopped());
//     EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
// }