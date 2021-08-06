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
#include "GrammarInjectorTest.cpp"

using namespace ArgParse;

TEST(RepetitionTest, NoChildEmptyString) {
    Repetition r1;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = r1.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    EXPECT_EQ("", parsedElement.getMatchedString());
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}

TEST(RepetitionTest, NoChildFilledString) {
    Repetition r1;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = r1.parse("asd", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    EXPECT_EQ("", parsedElement.getMatchedString());
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}


TEST(RepetitionTest, ExactMatch) {
    FixedString c1("child1");
    Repetition r1;
    r1.addChild(&c1);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = r1.parse("child1", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(strlen("child1"), rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&r1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    EXPECT_EQ("child1", parsedElement.getMatchedString());
    ASSERT_EQ(2, parsedElement.getChildren().size());
    EXPECT_EQ(&c1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("child1", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&c1, parsedElement.getChildren()[1]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[1]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}


TEST(RepetitionTest, MultipleMatch) {
    FixedString c1("child1");
    Repetition r1;
    r1.addChild(&c1);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = r1.parse("child1child1child1", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(strlen("child1child1child1"), rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1child1child1child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&r1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    EXPECT_EQ("child1child1child1", parsedElement.getMatchedString());
    ASSERT_EQ(4, parsedElement.getChildren().size());
    EXPECT_EQ(&c1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("child1", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&c1, parsedElement.getChildren()[3]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[3]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}

TEST(RepetitionTest, NoMatch) {
    FixedString c1("child1");
    Repetition r1;
    r1.addChild(&c1);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = r1.parse("asd", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    // We do not expect any candidates, as we already have non matching text
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    EXPECT_EQ("", parsedElement.getMatchedString());
    // also do not expect any parsed elements, as we could not uniquely identify
    // any (partially) matching childs
    ASSERT_EQ(0, parsedElement.getChildren().size());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}

TEST(RepetitionTest, PartialMatch) {
    FixedString c1("child1");
    Repetition r1;
    r1.addChild(&c1);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = r1.parse("ch", parsedElement);

    // rc:
    // FIXME: what do want to have as RC here ??? we are not really expecting any more text, as a match of 0 would be fine... 
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&r1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    EXPECT_EQ("", parsedElement.getMatchedString());
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&c1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}

TEST(RepetitionTest, GrammarInjectorWrongServer) {
    FixedString c1("127.0.0.1");
    Repetition r1;

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    Grammar grammarPool;
    GrammarInjectorMockServicesError inject1(grammarPool);
    r1.addChild(&inject1);
    r1.addChild(&c1);
    ParseRc rc = r1.parse("129.0.0.1 examples", parsedElement);

    // rc:
    ASSERT_NE(0, rc.ErrorMessage.size());
    EXPECT_EQ(ParseRc::ErrorType::retrievingGrammarFailed, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}
