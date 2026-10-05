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
#include <libArgParse/EscapedString.hpp>

using namespace ArgParse;

TEST(EscapedStringTest, SimpleMatch) {
    EscapedString myEscapedString("\"", '\\');
    ParsedElement parsedElement;

    ParseRc rc = myEscapedString.parse("hello", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(5, rc.lenParsedSuccessfully);
    EXPECT_EQ("hello", parsedElement.getMatchedStringRaw());
    EXPECT_EQ("hello", parsedElement.getMatchedString());
}

TEST(EscapedStringTest, EscapedCharacterMatch) {
    EscapedString myEscapedString("\"", '\\');
    ParsedElement parsedElement;

    ParseRc rc = myEscapedString.parse("hello\\\"world", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(12, rc.lenParsedSuccessfully);
    EXPECT_EQ("hello\\\"world", parsedElement.getMatchedStringRaw());
    EXPECT_EQ("hello\"world", parsedElement.getMatchedString());
}

TEST(EscapedStringTest, UnescapedTerminator) {
    EscapedString myEscapedString("\"", '\\');
    ParsedElement parsedElement;

    ParseRc rc = myEscapedString.parse("hello\\\"world\"extra", parsedElement);

    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(12, rc.lenParsedSuccessfully);
    EXPECT_EQ("hello\\\"world", parsedElement.getMatchedStringRaw());
    EXPECT_EQ("hello\"world", parsedElement.getMatchedString());
}

TEST(EscapedStringTest, AutoAddEscapeCharacter) {
    EscapedString myEscapedString("\"", '\\');
    ParsedElement parsedElement;
    ParseRc rc = myEscapedString.parse("hello\\\\world", parsedElement);
    
    EXPECT_EQ(ParseRc::ErrorType::success, rc.errorType);
    EXPECT_EQ(12, rc.lenParsedSuccessfully);
    EXPECT_EQ("hello\\\\world", parsedElement.getMatchedStringRaw());
    EXPECT_EQ("hello\\world", parsedElement.getMatchedString());
}

TEST(EscapedStringTest, ToStringAndDotNode) {
    EscapedString myEscapedString("\"", '\\', "myEscString");
    EXPECT_NE("", myEscapedString.toString());
    EXPECT_NE("", myEscapedString.getDotNode());
}
