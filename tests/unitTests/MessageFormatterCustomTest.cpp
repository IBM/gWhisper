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
#include <libCli/MessageFormatter.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

using namespace ArgParse;

// ---------------------------------------------------------------------------
// Helper: build the ParsedElement tree expected by MessageFormatterCustom.
//
// MessageFormatterCustom::messageToString() does:
//   findFirstSubTree("CustomOutputFormat", found)
//     findFirstSubTree("OutputFormatString", found)
//       iterates children:
//         child.findFirstSubTree("OutputFieldReference", found) -> field name
//         OR child.getMatchedString() -> literal text
//
// We use Grammar::createElement<FixedString>(matchStr, elementName) to create
// named GrammarElement stubs and attach them to ParsedElements.
// ---------------------------------------------------------------------------
struct CustomFormatTree
{
    Grammar grammar;
    ParsedElement root;

    // Named grammar element stubs
    GrammarElement * geRoot      = nullptr;
    GrammarElement * geCustomFmt = nullptr;
    GrammarElement * geFmtStr    = nullptr;

    CustomFormatTree()
    {
        geRoot      = grammar.createElement<FixedString>("", "Root");
        geCustomFmt = grammar.createElement<FixedString>("", "CustomOutputFormat");
        geFmtStr    = grammar.createElement<FixedString>("", "OutputFormatString");

        root.setGrammarElement(geRoot);

        // root -> CustomOutputFormat
        auto customFmt = std::make_shared<ParsedElement>(&root);
        customFmt->setGrammarElement(geCustomFmt);
        root.addChild(customFmt);

        // CustomOutputFormat -> OutputFormatString
        auto fmtStr = std::make_shared<ParsedElement>(customFmt.get());
        fmtStr->setGrammarElement(geFmtStr);
        customFmt->addChild(fmtStr);
    }

    ParsedElement & fmtStrNode()
    {
        return *root.getChildren()[0]->getChildren()[0];
    }

    // Add a literal string output statement (no OutputFieldReference child)
    void addLiteral(const std::string & f_text)
    {
        GrammarElement * ge = grammar.createElement<FixedString>(f_text, "");
        auto child = std::make_shared<ParsedElement>(&fmtStrNode());
        child->setGrammarElement(ge);
        child->setMatchedString(f_text);
        fmtStrNode().addChild(child);
    }

    // Add a field-reference output statement (contains OutputFieldReference child)
    void addFieldRef(const std::string & f_fieldName)
    {
        GrammarElement * geStmt     = grammar.createElement<FixedString>("", "");
        GrammarElement * geFieldRef = grammar.createElement<FixedString>(f_fieldName, "OutputFieldReference");

        auto stmt = std::make_shared<ParsedElement>(&fmtStrNode());
        stmt->setGrammarElement(geStmt);

        auto fieldRef = std::make_shared<ParsedElement>(stmt.get());
        fieldRef->setGrammarElement(geFieldRef);
        fieldRef->setMatchedString(f_fieldName);
        stmt->addChild(fieldRef);

        fmtStrNode().addChild(stmt);
    }
};

// ---------------------------------------------------------------------------
// Fixture: populates a Numbers protobuf message using DynamicMessageFactory
// ---------------------------------------------------------------------------
class MessageFormatterCustomTest : public ::testing::Test
{
protected:
    google::protobuf::DynamicMessageFactory m_factory;
    const google::protobuf::Descriptor * m_numbersDesc = nullptr;
    std::unique_ptr<google::protobuf::Message> m_numbersMsg;

    void SetUp() override
    {
        m_numbersDesc = google::protobuf::DescriptorPool::generated_pool()
                            ->FindMessageTypeByName("examples.Numbers");
        ASSERT_NE(nullptr, m_numbersDesc) << "examples.Numbers descriptor not found. "
            "Ensure the test binary is linked against the examples protobuf library.";

        m_numbersMsg.reset(m_factory.GetPrototype(m_numbersDesc)->New());

        const google::protobuf::FieldDescriptor * f32 = m_numbersDesc->FindFieldByName("m_int32");
        ASSERT_NE(nullptr, f32);
        m_numbersMsg->GetReflection()->SetInt32(m_numbersMsg.get(), f32, 42);

        const google::protobuf::FieldDescriptor * fdbl = m_numbersDesc->FindFieldByName("m_double");
        ASSERT_NE(nullptr, fdbl);
        m_numbersMsg->GetReflection()->SetDouble(m_numbersMsg.get(), fdbl, 3.14);
    }
};

// ---------------------------------------------------------------------------
// Test: no CustomOutputFormat sub-tree returns a warning string
// ---------------------------------------------------------------------------
TEST_F(MessageFormatterCustomTest, NoCustomOutputFormatReturnsWarning)
{
    Grammar g;
    GrammarElement * ge = g.createElement<FixedString>("", "Root");
    ParsedElement emptyTree;
    emptyTree.setGrammarElement(ge);

    cli::MessageFormatterCustom fmt(emptyTree);
    std::string result = fmt.messageToString(*m_numbersMsg, m_numbersDesc);
    EXPECT_NE(std::string::npos, result.find("Warning"))
        << "Expected a warning when no format is given, got: " << result;
}

// ---------------------------------------------------------------------------
// Test: literal-only format string is passed through unchanged
// ---------------------------------------------------------------------------
TEST_F(MessageFormatterCustomTest, LiteralOnlyOutputFormat)
{
    CustomFormatTree tree;
    tree.addLiteral("hello_world");

    cli::MessageFormatterCustom fmt(tree.root);
    std::string result = fmt.messageToString(*m_numbersMsg, m_numbersDesc);
    EXPECT_EQ("hello_world", result);
}

// ---------------------------------------------------------------------------
// Test: field reference emits the field's value
// ---------------------------------------------------------------------------
TEST_F(MessageFormatterCustomTest, FieldReferenceOutputsValue)
{
    CustomFormatTree tree;
    tree.addFieldRef("m_int32");

    cli::MessageFormatterCustom fmt(tree.root);
    std::string result = fmt.messageToString(*m_numbersMsg, m_numbersDesc);
    // The integer value 42 must appear in the output
    EXPECT_NE(std::string::npos, result.find("42"))
        << "Expected '42' in output, got: " << result;
}

// ---------------------------------------------------------------------------
// Test: unknown field reference produces "???"
// ---------------------------------------------------------------------------
TEST_F(MessageFormatterCustomTest, UnknownFieldReferenceOutputsQuestionMarks)
{
    CustomFormatTree tree;
    tree.addFieldRef("no_such_field");

    cli::MessageFormatterCustom fmt(tree.root);
    std::string result = fmt.messageToString(*m_numbersMsg, m_numbersDesc);
    EXPECT_NE(std::string::npos, result.find("???"))
        << "Expected '???' for unknown field reference, got: " << result;
}

// ---------------------------------------------------------------------------
// Test: prefix + field + postfix are all present in output
// ---------------------------------------------------------------------------
TEST_F(MessageFormatterCustomTest, PrefixFieldAndPostfixAreCombined)
{
    CustomFormatTree tree;
    tree.addLiteral("prefix_");
    tree.addFieldRef("m_int32");
    tree.addLiteral("_postfix");

    cli::MessageFormatterCustom fmt(tree.root);
    std::string result = fmt.messageToString(*m_numbersMsg, m_numbersDesc);
    EXPECT_NE(std::string::npos, result.find("prefix_"))  << result;
    EXPECT_NE(std::string::npos, result.find("42"))       << result;
    EXPECT_NE(std::string::npos, result.find("_postfix")) << result;
}

// ---------------------------------------------------------------------------
// Test: multiple field references in same format string
// ---------------------------------------------------------------------------
TEST_F(MessageFormatterCustomTest, MultipleFieldReferences)
{
    CustomFormatTree tree;
    tree.addFieldRef("m_int32");
    tree.addLiteral(" ");
    tree.addFieldRef("m_double");

    cli::MessageFormatterCustom fmt(tree.root);
    std::string result = fmt.messageToString(*m_numbersMsg, m_numbersDesc);
    EXPECT_NE(std::string::npos, result.find("42"))   << result;
    EXPECT_NE(std::string::npos, result.find("3.14")) << result;
}
