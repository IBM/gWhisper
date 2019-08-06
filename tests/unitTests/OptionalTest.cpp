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
//          Optional
// -----------------------------------------------------------------------------

TEST(OptionalTest, OneChildEmptyString) {
    EXPECT_EQ(true, true);

    Optional myOptional;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);
    
    FixedString child1("child1");
    myOptional.addChild(&child1);

    ParseRc rc = myOptional.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myOptional, parsedElement.getGrammarElement());
}

TEST(OptionalTest, OneChildNonEmptyString) {
    EXPECT_EQ(true, true);

    Optional myOptional;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);
    
    FixedString child1("child1");
    myOptional.addChild(&child1);

    ParseRc rc = myOptional.parse("child1", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(6, rc.lenParsed);
    EXPECT_EQ(6, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myOptional, parsedElement.getGrammarElement());
}

TEST(OptionalTest, TwoChildsSimilarStart) {
    FixedString child1("child1");
    FixedString child2("child12345");
    Optional myOptional;
    myOptional.addChild(&child1);
    myOptional.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myOptional.parse("child12345", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(strlen("child1"), rc.lenParsed);
    EXPECT_EQ(strlen("child1"), rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myOptional, parsedElement.getGrammarElement());
}

TEST(OptionalTest, TwoChildEmptyString) {
    FixedString child1("child1");
    FixedString child2("child2");
    Optional myOptional;
    myOptional.addChild(&child1);
    myOptional.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myOptional.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myOptional, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myOptional, parsedElement.getGrammarElement());
}
