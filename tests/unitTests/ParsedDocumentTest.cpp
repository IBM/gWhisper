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

TEST(ParsedDocumentTest, CoordinateMembers) {
    EXPECT_EQ(true, true);

    Coordinate node = {4, 1, 6};
    ASSERT_EQ(node.depth, 4);
    ASSERT_EQ(node.index, 1);
    ASSERT_EQ(node.step, 6);
}

TEST(ParsedDocumentTest, CoordinateOutputOperatorOverload) {

    Coordinate node1 = {0, 1, 0};
    Coordinate node2 = {1, 4, 0};
    std::stringstream buffer;
    buffer << node1;
    // rc:
    ASSERT_EQ(buffer.str(), "(0, 1, 0)");
    buffer << "->" << node2;
    ASSERT_EQ(buffer.str(), "(0, 1, 0)->(1, 4, 0)");
}

TEST(ParsedDocumentTest, GetDocumentFromGrammarElement) {

    std::string document = "test document";
    ParsedElement parsedElement;
    FixedString fixString1("fixString1");
    fixString1.setDocument(document);
    parsedElement.setGrammarElement(&fixString1);
    ParsedDocument doc(&parsedElement);
    ASSERT_EQ(doc.getParsedElement()->getGrammarElement()->getDocument(), document);
}

TEST(ParsedDocumentTest, GetMaxStep) {

    std::vector<Coordinate> f_path;
    ParsedElement parsedElement;
    Coordinate root = {0, 0, 0};
    Coordinate node1 = {1, 1, 0};
    Coordinate node2 = {2, 3, 0};
    ParsedDocument doc(&parsedElement);
    doc.addNodeToPath(root);
    doc.addNodeToPath(node1);
    doc.addNodeToPath(node2);
    doc.calculateStepFromRoot();
    ASSERT_EQ(doc.getMaxStep(), 4);
}

TEST(ParsedDocumentTest, UpdatePath) {

    ParsedDocument doc;

    Coordinate root = {0, 0, 0};
    Coordinate node1 = {1, 1, 0};
    Coordinate node2 = {2, 3, 0};
    doc.addNodeToPath(root);
    doc.addNodeToPath(node1);
    doc.addNodeToPath(node2);
    ASSERT_EQ(doc.getPath().size(), 3);

    std::vector<Coordinate> new_path = doc.getPath();
    new_path.push_back({3, 0, 0});
    doc.updatePath(new_path);
    ASSERT_EQ(doc.getPath().size(), 4);
    auto path = doc.getPath();
    ASSERT_EQ(path[3].index, 0);
    ASSERT_EQ(path[3].step, 0);
    ASSERT_EQ(path[3].depth, 3);
}

TEST(ParsedDocumentTest, CalculateStepFromRoot) {

    ParsedDocument doc;
    Coordinate root = {0, 0, 0};
    Coordinate node1 = {1, 0, 0};
    Coordinate node2 = {2, 1, 0};
    Coordinate node3 = {3, 2, 0};

    doc.addNodeToPath(root);
    doc.addNodeToPath(node1);

    ASSERT_EQ(doc.getPath().size(), 2);
    ASSERT_EQ(doc.getPath()[1].depth, 1);
    ASSERT_EQ(doc.getPath()[1].index, 0);
    ASSERT_EQ(doc.getPath()[1].step, 0);
    ASSERT_EQ(doc.getMaxStep(), 0);

    doc.addNodeToPath(node2);
    doc.calculateStepFromRoot();
    ASSERT_EQ(doc.getPath().size(), 3);
    ASSERT_EQ(doc.getPath()[2].index, 1);
    ASSERT_EQ(doc.getMaxStep(), 1);

    doc.addNodeToPath(node3);
    ASSERT_EQ(doc.getPath().size(), 4);

    doc.calculateStepFromRoot();
    ASSERT_EQ(doc.getPath()[3].step, 3);
    ASSERT_EQ(doc.getMaxStep(), 3);
}
