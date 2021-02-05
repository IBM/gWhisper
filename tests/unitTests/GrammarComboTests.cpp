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

// ----------------------------------------------------------------------------
// CA Combos
// ----------------------------------------------------------------------------

TEST(CA_ComboTest, CAC_combo) {
    // c1
    //     a1
    //         f1
    //         f2
    //     c2
    //          f3
    //          f4

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;

    Concatenation c1;
    Concatenation c2;

    a1.addChild(&f1);
    a1.addChild(&f2);

    c1.addChild(&a1);
    c1.addChild(&c2);

    c2.addChild(&f3);
    c2.addChild(&f4);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1f3f4", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f2f3f4", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&a1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(0, parsedElement.getChildren()[0]->getChildren().size());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, CAC_combo_uniquePartialMatch) {
    // c1
    //     a1
    //         f1
    //         f2
    //     c2
    //          f3
    //          f4

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;

    Concatenation c1;
    Concatenation c2;

    a1.addChild(&f1);
    a1.addChild(&f2);

    c1.addChild(&a1);
    c1.addChild(&c2);

    c2.addChild(&f3);
    c2.addChild(&f4);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f2", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType) << rc.toString();
    EXPECT_EQ(2, rc.lenParsed);
    EXPECT_EQ(2, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("f2f3f4", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    // It is essential to have the first child here (otherwise grammar injection would not have access to parially parsed tree)
    ASSERT_EQ(2, parsedElement.getChildren().size());
    EXPECT_EQ("f2", parsedElement.getMatchedString());

    EXPECT_EQ(&a1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("f2", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(1, parsedElement.getChildren()[0]->getChildren().size());

    EXPECT_EQ(&c2, parsedElement.getChildren()[1]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[1]->getMatchedString());
    EXPECT_EQ(1, parsedElement.getChildren()[1]->getChildren().size());
    EXPECT_EQ(&f3, parsedElement.getChildren()[1]->getChildren()[0]->getGrammarElement());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, CA_combo_partialMatch) {
    // c1
    //     a1
    //         f1
    //         f2

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    Concatenation c1;

    a1.addChild(&f1);
    a1.addChild(&f2);

    c1.addChild(&a1);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f2", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&a1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());

    EXPECT_EQ(0, parsedElement.getChildren()[0]->getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, CA_combo_partialMatchFollowUpAlternation) {
    // c1
    //     a1
    //         f1
    //         f2
    //     a2
    //          f3
    //          f4

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    Concatenation c1;

    a1.addChild(&f1);
    a1.addChild(&f2);

    a2.addChild(&f3);
    a2.addChild(&f4);

    c1.addChild(&a1);
    c1.addChild(&a2);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f2", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&a1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());

    EXPECT_EQ(0, parsedElement.getChildren()[0]->getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, CCX_combo_noOverParse) {
    // c1
    //     c2
    //         f1
    //         x1
    //     f3

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    RegEx x1("/asdfsadfsadf/");
    Concatenation c1;
    Concatenation c2;

    c1.addChild(&c2);
    c1.addChild(&f3);

    c2.addChild(&f1);
    c2.addChild(&x1);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f1", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(2, rc.lenParsed);
    EXPECT_EQ(2, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&c2, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("f1", parsedElement.getChildren()[0]->getMatchedString());

    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, CCX_combo_noOverParseUnexpectedText) {
    // c1
    //     c2
    //         f1
    //         x1
    //     f3

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    RegEx x1("/asdfsadfsadf/");
    Concatenation c1;
    Concatenation c2;

    c1.addChild(&c2);
    c1.addChild(&f3);

    c2.addChild(&f1);
    c2.addChild(&x1);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f1h", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::unexpectedText, rc.errorType);
    EXPECT_EQ(3, rc.lenParsed);
    EXPECT_EQ(2, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(0, rc.candidates.size());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());

    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, CCX_combo_noOverParsePartial) {
    // c1
    //     c2
    //         f1
    //         x1
    //     f3

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    RegEx x1("/asdfsadfsadf/");
    Concatenation c1;
    Concatenation c2;

    c1.addChild(&c2);
    c1.addChild(&f3);

    c2.addChild(&f1);
    c2.addChild(&x1);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());

    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, CA_combo) {
    // c1
    //     a1
    //         f1
    //         f2
    //     f3

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    Concatenation c1;

    a1.addChild(&f1);
    a1.addChild(&f2);

    c1.addChild(&a1);
    c1.addChild(&f3);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1f3", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f2f3", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&a1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());

    EXPECT_EQ(0, parsedElement.getChildren()[0]->getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, ACACA_combo_NoFork) {
    // a1
    //  c1
    //      f1
    //      a2
    //          f2
    //          f3
    //  c2
    //      f4
    //      a3
    //          f5
    //          f6

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    FixedString f6("f6");
    Alternation a1;
    Alternation a2;
    Alternation a3;
    Concatenation c1;
    Concatenation c2;

    a1.addChild(&c1);
    a1.addChild(&c2);

    c1.addChild(&f1);
    c1.addChild(&a2);

    c2.addChild(&f4);
    c2.addChild(&a3);

    a2.addChild(&f2);
    a2.addChild(&f3);

    a3.addChild(&f5);
    a3.addChild(&f6);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = a1.parse("f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f4", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&a1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, ACA_combo_NoFork) {
    // a1
    //  c1
    //      f1
    //      a2
    //          f2
    //          f3
    //      f4
    //  f5

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    Concatenation c1;

    a1.addChild(&c1);
    a1.addChild(&f5);

    c1.addChild(&f1);
    c1.addChild(&a2);
    c1.addChild(&f4);

    a2.addChild(&f2);
    a2.addChild(&f3);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = a1.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f5", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size()); // This might change to 1, if we enforce the "if it is unique, we add it" paradigm to the limit. I.e. if user has stopped inputting, we just continue parsing
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&a1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, AaCA_combo_PartialMatchFork) {
    // a1
    //  a2
    //    f1
    //  c1
    //    a3
    //      f2
    //      f3

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");

    Alternation a1;
    Alternation a2;
    Alternation a3;
    Concatenation c1;

    a1.addChild(&a2);
    a1.addChild(&c1);

    a2.addChild(&f1);

    c1.addChild(&a3);

    a3.addChild(&f2);
    a3.addChild(&f3);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = a1.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(3, rc.candidates.size());

    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    EXPECT_EQ("f2", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    EXPECT_EQ("f3", rc.candidates[2]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[2]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[2]->getParent());

    // parsedElement
    ASSERT_EQ(0, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&a1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, ACA_combo_PartialMatchFork) {
    // a1
    //  c1
    //      f1
    //      a2
    //          f2
    //          f3
    //      f4
    //  f5

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Alternation a1;
    Alternation a2;
    Concatenation c1;

    a1.addChild(&c1);
    a1.addChild(&f5);

    c1.addChild(&f1);
    c1.addChild(&a2);
    c1.addChild(&f4);

    a2.addChild(&f2);
    a2.addChild(&f3);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = a1.parse("f1f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(3, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1f2f4", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f1f3f4", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&a1, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&a1, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, DoubleConcatWithAlternationFork) {
    FixedString child1("f1");
    FixedString child2("f2");
    FixedString child3("f3");
    FixedString child4("f4");

    Alternation alt;
    alt.addChild(&child3);
    alt.addChild(&child4);

    Concatenation c2;
    c2.addChild(&child2);
    c2.addChild(&alt);

    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&c2);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(2, rc.candidates.size());
    EXPECT_EQ("f1f2f3", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("f1f2f4", rc.candidates[1]->getMatchedString());
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

TEST(CA_ComboTest, RecursiveAlternationFork) {
    FixedString child1("child1");

    Alternation alt;
    FixedString altChild1("ac1");
    FixedString altChild2("ac2");
    alt.addChild(&altChild1);
    alt.addChild(&altChild2);

    Alternation altParent;
    FixedString altChild3("ac3");
    altParent.addChild(&alt);
    altParent.addChild(&altChild3);

    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&altParent);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = myConcatenation.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(3, rc.candidates.size());
    EXPECT_EQ("child1ac1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());
    EXPECT_EQ("child1ac2", rc.candidates[1]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[1]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[1]->getParent());
    EXPECT_EQ("child1ac3", rc.candidates[2]->getMatchedString());
    EXPECT_EQ(&myConcatenation, rc.candidates[2]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[2]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&child1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&myConcatenation, parsedElement.getGrammarElement());
}

TEST(CA_ComboTest, TwoAlternationNoFork) {
    FixedString child1("child1");

    Alternation alt1;
    FixedString altChild1("ac1");
    FixedString altChild2("ac2");
    alt1.addChild(&altChild1);
    alt1.addChild(&altChild2);

    Alternation alt2;
    FixedString altChild3("ac3");
    FixedString altChild4("ac4");
    alt2.addChild(&altChild3);
    alt2.addChild(&altChild4);

    Concatenation myConcatenation;
    myConcatenation.addChild(&child1);
    myConcatenation.addChild(&alt1);
    myConcatenation.addChild(&alt2);

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

// ----------------------------------------------------------------------------
// CR Combos
// ----------------------------------------------------------------------------

TEST(CR_ComboTest, CR_combo_Empty) {
    // c1
    //      r1
    //          f1

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Repetition r1;
    Concatenation c1;

    r1.addChild(&f1);

    c1.addChild(&r1);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(0, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&r1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(1, parsedElement.getChildren()[0]->getChildren().size());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CR_ComboTest, CR_combo_PartialMatchSandwitchVeryComplex) {
    // c1
    //      f1
    //      r1
    //          c2
    //            f2
    //            f4
    //      f3

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3(":");
    FixedString f4("8");
    Repetition r1;
    Concatenation c1;
    Concatenation c2;

    r1.addChild(&c2);

    c1.addChild(&f1);
    c1.addChild(&r1);
    c1.addChild(&f3);

    c2.addChild(&f2);
    c2.addChild(&f4);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f1f2", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(4, rc.lenParsed);
    EXPECT_EQ(4, rc.lenParsedSuccessfully);

    //EXPECT_EQ("f1f2faa2", rc.candidates[0]->getMatchedString());
    //EXPECT_EQ("f1f2faa2", rc.candidates[1]->getMatchedString());
    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("f1f28", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(2, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&f1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("f1", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&r1, parsedElement.getChildren()[1]->getGrammarElement());
    EXPECT_EQ("f2", parsedElement.getChildren()[1]->getMatchedString());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CR_ComboTest, CR_combo_PartialMatchSandwitch) {
    // c1
    //      f1
    //      r1
    //          f2
    //      f3

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3(":");
    Repetition r1;
    Concatenation c1;

    r1.addChild(&f2);

    c1.addChild(&f1);
    c1.addChild(&r1);
    c1.addChild(&f3);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f1f2f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(5, rc.lenParsed);
    EXPECT_EQ(4, rc.lenParsedSuccessfully);

    //EXPECT_EQ("f1f2faa2", rc.candidates[0]->getMatchedString());
    //EXPECT_EQ("f1f2faa2", rc.candidates[1]->getMatchedString());
    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("f1f2f2", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(2, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&f1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("f1", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(&r1, parsedElement.getChildren()[1]->getGrammarElement());
    EXPECT_EQ("f2", parsedElement.getChildren()[1]->getMatchedString());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CR_ComboTest, CR_combo_PartialMatch) {
    // c1
    //      r1
    //          f1

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Repetition r1;
    Concatenation c1;

    r1.addChild(&f1);

    c1.addChild(&r1);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = c1.parse("f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&c1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&r1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(1, parsedElement.getChildren()[0]->getChildren().size());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&c1, parsedElement.getGrammarElement());
}

TEST(CR_ComboTest, CR_combo_RC_partialMatch) {
    // r1
    //      c1
    //          f1

    FixedString f1("f1");
    FixedString f2("f2");
    FixedString f3("f3");
    FixedString f4("f4");
    FixedString f5("f5");
    Repetition r1;
    Concatenation c1;

    r1.addChild(&c1);

    c1.addChild(&f1);

    ParsedElement parent;
    ParsedElement parsedElement(&parent);

    ParseRc rc = r1.parse("f", parsedElement);

    // rc:
    EXPECT_EQ(ParseRc::ErrorType::missingText, rc.errorType);
    EXPECT_EQ(1, rc.lenParsed);
    EXPECT_EQ(0, rc.lenParsedSuccessfully);

    // candidates:
    ASSERT_EQ(1, rc.candidates.size());
    EXPECT_EQ("f1", rc.candidates[0]->getMatchedString());
    EXPECT_EQ(&r1, rc.candidates[0]->getGrammarElement());
    EXPECT_EQ(&parent, rc.candidates[0]->getParent());

    // parsedElement
    ASSERT_EQ(1, parsedElement.getChildren().size());
    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(&c1, parsedElement.getChildren()[0]->getGrammarElement());
    EXPECT_EQ("", parsedElement.getChildren()[0]->getMatchedString());
    EXPECT_EQ(1, parsedElement.getChildren()[0]->getChildren().size());

    EXPECT_EQ(&parent, parsedElement.getParent());
    EXPECT_EQ(false, parsedElement.isStopped());
    EXPECT_EQ(&r1, parsedElement.getGrammarElement());
}

