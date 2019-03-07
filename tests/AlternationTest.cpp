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
//          Alternation
// -----------------------------------------------------------------------------

TEST(AlternationTest, NoChildEmptyString) {
    EXPECT_EQ(true, true);

    Alternation myAlternation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, NoChildNonEmptyString) {
    Alternation myAlternation;
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("test", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, OneChildEmptyString) {
    FixedString child1("child1");
    Alternation myAlternation;
    myAlternation.addChild(&child1);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myAlternation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, TwoChildsSimilarStart) {
    FixedString child1("child1");
    FixedString child2("child12345");
    Alternation myAlternation;
    myAlternation.addChild(&child1);
    myAlternation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("child12345", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(strlen("child12345"), rc.lenParsed);
    EXPECT_EQ(strlen("child12345"), rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, TwoChildEmptyString) {
    FixedString child1("child1");
    FixedString child2("child2");
    Alternation myAlternation;
    myAlternation.addChild(&child1);
    myAlternation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myAlternation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("child2", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&myAlternation, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, TwoChildCorrectStartString) {
    FixedString child1("child1");
    FixedString child2("child2");
    Alternation myAlternation;
    myAlternation.addChild(&child1);
    myAlternation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("child", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(strlen("child"), rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myAlternation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("child2", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&myAlternation, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, TwoChildUniquePartialMatch) {
    FixedString child1("child1asd");
    FixedString child2("child2fgh");
    Alternation myAlternation;
    myAlternation.addChild(&child1);
    myAlternation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("child2", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(strlen("child2"), rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("child2fgh", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myAlternation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
    EXPECT_EQ(&child2, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, TwoChildWrongString) {
    FixedString child1("child1");
    FixedString child2("child2");
    Alternation myAlternation;
    myAlternation.addChild(&child1);
    myAlternation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("asd", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

TEST(AlternationTest, TwoChildCorrectStringForBoth) {
    FixedString child1("child1");
    FixedString child2("child123");
    Alternation myAlternation;
    myAlternation.addChild(&child1);
    myAlternation.addChild(&child2);
    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myAlternation.parse("child1", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(6, rc.lenParsed);
    EXPECT_EQ(6, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());

    // parsedElement
    // hmmm what to expect here: one or two? i.e. is the matched one also a candidate (maybe relevant for further completion...)
    //  -> hm maybe not: maybe we should in general continue parsing
    //  -> hmmm we actually do this already!!! why is it not working?
    ASSERT_EQ(1, parsedElement.getChildren().size()); 
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myAlternation, parsedElement.getGrammarElement());
}

// Not yet supported
//TEST(AlternationTest, OptionalChild) {
//    FixedString child1("child1");
//    Optional child2;
//    FixedString childOpt("child2");
//    child2.addChild(&childOpt);
//
//    Alternation myAlternation;
//    myAlternation.addChild(&child1);
//    myAlternation.addChild(&child2);
//
//    ParsedElement parent;
//    ParsedElement parsedElement(&parent);
//
//    ParseRc rc = myAlternation.parse("", parsedElement);
//
//    // rc:
//    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
//    EXPECT_EQ(0, rc.lenParsedSuccessfully);
//
//    // candidates:
//    ASSERT_EQ(2, rc.candidates.size());
//    EXPECT_EQ("child1", rc.candidates[0]->getMatchedString());
//    EXPECT_EQ(&myAlternation, rc.candidates[0]->getGrammarElement());
//    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
//
//    EXPECT_EQ("child2", rc.candidates[1]->getMatchedString());
//    EXPECT_EQ(&myAlternation, rc.candidates[1]->getGrammarElement());
//    EXPECT_EQ(&parent, rc.candidates[1]->getParent());
//
//    // parsedElement
//    ASSERT_EQ(0, parsedElement.getChildren().size());
//    EXPECT_EQ(&parent, parsedElement.getParent());
//    EXPECT_EQ(false, parsedElement.isStopped());
//    EXPECT_EQ(nullptr, parsedElement.getGrammarElement());
//}
