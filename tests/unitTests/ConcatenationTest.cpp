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

// -----------------------------------------------------------------------------
//          Concatenation
// -----------------------------------------------------------------------------

TEST(ConcatenationTest, NoChildEmptyString) {
    EXPECT_EQ(true, true);

    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::retrievingGrammarFailed, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, NoChildNonEmptyString) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("test", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::retrievingGrammarFailed, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, OneChildEmptyString) {
    FixedString child1("child1");
    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    ASSERT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    ASSERT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, TwoChildsCompleteMatch) {
    FixedString child1("child1");
    FixedString child2("child2");
    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("child1child2asd", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(strlen("child1child2"), rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(2, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, TwoChildsSecondIsPartialMatch) {
    FixedString child1("child1");
    FixedString child2("child2");
    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("child1c", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(strlen("child1c"), rc.lenParsed);
    EXPECT_EQ(strlen("child1"), rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1child2", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(2, parsedElement.getChildren().size());
    EXPECT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("child1", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&child2, parsedElement.getChildren()[1]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[1]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, TwoChildEmptyString) {
    FixedString child1("child1");
    FixedString child2("child2");
    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1child2", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, TwoChildCorrectStartString) {
    FixedString child1("child1");
    FixedString child2("child2");
    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("child", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1child2", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, TwoChildWrongString) {
    FixedString child1("child1");
    FixedString child2("child2");
    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("asd", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, SingleAlternationFork) {
    FixedString child1("child1");
    Alternation alt;
    FixedString altChild1("ac1");
    FixedString altChild2("ac2");
    alt.addChild(&altChild1);
    alt.addChild(&altChild2);

    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&alt);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);
    
    ParseRc rc = myConcatenation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("child1ac1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("child1ac2", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, SingleAlternationNoFork) {
    FixedString child1("child1");
    Alternation alt;
    FixedString altChild1("ac1");
    FixedString altChild2("ac2");
    alt.addChild(&altChild1);
    alt.addChild(&altChild2);

    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&alt);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("", parsedElement, 0);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, GrammarInjectorWrongServer) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    Grammar grammarPool;
    GrammarInjectorMockServicesError inject1(grammarPool);
    myConcatenation.addChild(&inject1);
    ParseRc rc = myConcatenation.parse("129.0.0.1 examples", parsedElement);

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
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, GrammarInjectorBeforeFixStringWrongServer) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    FixedString child1("child1");

    Grammar grammarPool;
    GrammarInjectorMockServicesError inject1(grammarPool);

    myConcatenation.addChild(&inject1);
    myConcatenation.addChild(&child1);

    ParseRc rc = myConcatenation.parse("129.0.0.1 examples", parsedElement);

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
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, GrammarInjectorAfterFixStringWrongServer) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    FixedString child1("child1");

    Grammar grammarPool;
    GrammarInjectorMockServicesError inject1(grammarPool);

    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&inject1);

    ParseRc rc = myConcatenation.parse("129.0.0.1 examples", parsedElement);

    // rc:
    EXPECT_EQ(0, rc.ErrorMessage.size());
    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, GrammarInjectorAfterAlternationWrongServer) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    Alternation acl1("acl1");

    Grammar grammarPool;
    GrammarInjectorMockServicesError inject1(grammarPool);

    myConcatenation.addChild(&acl1);
    myConcatenation.addChild(&inject1);

    ParseRc rc = myConcatenation.parse("129.0.0.1 examples", parsedElement);

    // rc:
    EXPECT_NE(0, rc.ErrorMessage.size());
    EXPECT_EQ(ParseRc::ErrorType::retrievingGrammarFailed, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, GrammarInjectorWithTwoFixStringWrongServer) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    FixedString child1("child1");
    FixedString child2("child2");

    Grammar grammarPool;
    GrammarInjectorMockServicesError inject1(grammarPool);

    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&inject1);
    myConcatenation.addChild(&child2);

    ParseRc rc = myConcatenation.parse("129.0.0.1 examples", parsedElement);

    // rc:
    EXPECT_EQ(0, rc.ErrorMessage.size());
    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, GrammarInjectorBeforeFixStringRightServer) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    FixedString child1("child1");

    Grammar grammarPool;
    GrammarInjectorMockServicesSuccess inject1(grammarPool);
    myConcatenation.addChild(&inject1);
    myConcatenation.addChild(&child1);

    ParseRc rc = myConcatenation.parse("127.0.0.1 examples", parsedElement);

    // rc:
    ASSERT_EQ(0, rc.ErrorMessage.size());
    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, GrammarInjectorRightServer) {
    Concatenation myConcatenation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    Grammar grammarPool;
    GrammarInjectorMockServicesSuccess inject1(grammarPool);
    myConcatenation.addChild(&inject1);
    ParseRc rc = myConcatenation.parse("127.0.0.1 examples", parsedElement);

    // rc:
    ASSERT_EQ(0, rc.ErrorMessage.size());
    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(ConcatenationTest, OptionalChild) {
   FixedString child1("child1");
   Optional childOpt("childOpt");
   child1.addChild(&childOpt);

   Concatenation myConcatenation;
   myConcatenation.addChild(&child1);

   ParsedElement parent;
   ParsedElement parsedElement(&parent);

   ParseRc rc = myConcatenation.parse("", parsedElement);

   // rc:
   EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
   EXPECT_EQ(0, rc.lenParsedSuccessfully);

   // candidates:
   ASSERT_EQ(1, rc.candidates.size());
   EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
   EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
   EXPECT_EQ(&parent, rc.candidates[0]->getParent());

   // parsedElement
   ASSERT_EQ(1, parsedElement.getChildren().size());
   EXPECT_EQ(&parent, parsedElement.getParent());
   EXPECT_EQ(false, parsedElement.isStopped());
   EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}
