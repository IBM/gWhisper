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

#pragma once
#include <vector>
#include <memory>

#include <libArgParse/ParsedElement.hpp>

namespace ArgParse
{
    class ParsedElement;

    /* With the help of node coordinates,
     * we can easily find the relative position of a node,
     * which contains documentation.
     */
    typedef struct Coordinate
    {
        uint32_t depth; // depth in the parsed tree
        uint32_t index; // step to right direction from root node of a sub tree.
        uint32_t step;  // step to right direction from root of the parsed tree.

        friend std::ostream & operator<< (std::ostream & out, Coordinate & obj)
        {
            out << "(" << obj.depth << ", "<< obj.index << ", " << obj.step << ")";
            return out;
        }
    } Coordinate;

    /* Manipulate the documents (Description of bash or fish tab completion)
     * which have be parsed from the .proto file written in the custom options,
     * currently we have predefined ServicesOptions, FieldsOptions, MethodOptions.
     */
    class ParsedDocument
    {
        public:
            explicit ParsedDocument():
                m_parseElement(nullptr),
                m_max_step(0)
            {
            }

            explicit ParsedDocument(ParsedElement * f_parseElement):
                m_parseElement(f_parseElement),
                m_max_step(0)
            {
            }

            std::vector<Coordinate> getPath() const
            {
                return m_path;
            }

            void addNodeToPath(Coordinate f_node)
            {
                m_path.push_back(f_node);
            }

            /// update the newest path from the root to the current node with document.
            void updatePath(std::vector<Coordinate> f_path)
            {
                m_path = f_path;
            }

            /// get the maximum step to right direction from root of the parsed tree.
            uint32_t getMaxStep() const
            {
                return m_max_step;
            }

            /// how many steps to right direction from the root.
            void calculateStepFromRoot()
            {
                for(int i = 0; i != m_path.size(); ++i)
                {
                    if(i>0)
                    {
                        for(int j = 0; j<=i; ++j)
                        {
                            m_path[i].step += m_path[j].index;
                        }
                    }
                    // meanwhile update the maximum step.
                    if (m_path[i].step > m_max_step)
                    {
                        m_max_step = m_path[i].step;
                    }
                }
            }

            ParsedElement * getParsedElement() const
            {
                return m_parseElement;
            }

            /// for debugging
            void printPath() const
            {
                std::cout << "(depth, index, step): "; // index calculated from the subtree root, step from the root.
                for(auto node: m_path)
                {
                    std::cout << node << "->";
                }
                std::cout << m_parseElement->getGrammarElement()->getDocument() << std::endl;
            }

            static std::string getOptionString(std::string f_optString);// not used, only for formatting the options().DebugString() of protobuf reflection.

        private:
            std::vector<Coordinate> m_path;
            uint32_t m_max_step;
            ParsedElement * m_parseElement;
    };

    /// abstract the document info tree, stored as coordinates.
    /// @param f_parseElement the root of parsed tree.
    /// @param f_out_documents find all document info and tored in the abstract tree.
    /// @param f_path the path from the root to the current node with document reached, and stored as coordinates.
    /// @param f_depth the depth of the path in the tree reached, will be passed to the next recursive function call.
    /// @param f_index the index of the current node in the subtree reached, will be passed to the next recursive function call.
    void abstractDocTree(ParsedElement * f_parseElement, std::vector<ParsedDocument> & f_out_documents, std::vector<Coordinate> f_path, uint32_t f_depth, uint32_t f_index);
}
