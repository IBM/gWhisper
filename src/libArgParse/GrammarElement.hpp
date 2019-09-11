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
#include <libArgParse/ArgParse.hpp>
#include <vector>
#include <string>
#include <memory>
#include <libArgParse/ArgParseUtils.hpp>

namespace ArgParse
{
// a Graph
class GrammarElement;

class GrammarElement
{
    public:
        GrammarElement(const std::string & f_typeName, const std::string & f_elementName = "") :
            m_parent(this),
            m_typeName(f_typeName),
            m_elementName(f_elementName),
            m_document(""),
            m_instanceId(getAndIncrementInstanceCounter())
        {
        }

        /* Parses a given string into a parse tree.
         * @param f_string string to be parsed (null terminated cstring)
         * @param f_out_ParsedElement Reference to a ParsedElement, which will
         *  be filled with the parse result (parse tree).
         *  i.e. this Grammar element will be parsed into the given ParsedElement f_out_ParsedElement.
         *  The f_out_ParsedElement will always reference this GrammarElement
         *  instance after it is passed to this method.
         *  If the parse method could uniquely identify or parse a child, the
         *  f_out_ParsedElement object will also have a child referencing to the
         *  child-GrammarElement.
         * @param candidateDepth This parameter controls the maximum recursion/fork
         *  depth when analyzing candidates.
         *  This means, that when a Element could continue parsing with X1 > 1 candidates,
         *  Parsing is tried for each of these candidates, but candidateDepth is reduced by 1.
         *  If those candidates return X2 sub-candidates the following will happen:
         *  If X2 > 1 and the candidateDepth is zero, parsing will stop and the
         *      returned candidates are ignored.
         *  If X2 > 1 and the candidateDepth is > 0, parsing is done for all
         *      those children with a candidateDepth reduced by 1 again.
         *  If X2 == 1 parsing will continue
         *  NOTE: This description describes the behavior of Concatenations as an example
         *      Behavior for other Elements is similar, but might differ slightly.
         *      For example Alternations always continues parsing
         *      (think: An Alternation with an alternation as a child is
         *      semantically the same as a single Alternation with merged children)
         *  Currently only a candidateDepth of 1 is tested.
         * @param startChild The child number to start parsing from. Can be
         *  used to start parsing from a specific grammar element.
         *  Currently this is mostly used internally.
         *  TODO: Think about making this protected
         * @returns ParseRc with the following attributes:
         *          candidate list containing a list of ParsedElements which
         *          could be used in place of f_out_ParsedElement.
         */
        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) = 0;

        virtual std::string toString()
        {
            std::string result;
            result = m_typeName + "(" + m_elementName + ":" + std::to_string(m_instanceId) + ")" + "{";
            for(auto child: m_children)
            {
                result += child->toString();
                result += ", ";
            }
            result += "}";
            return result;
        }

        // TODO: maybe we should return a pointer to the newly added child here.
        // this would make grammar construction easier
        GrammarElement * addChild(GrammarElement * f_child)
        {
            f_child->setParent(this);
            m_children.push_back(f_child); 
            return this;
        }

        void setParent(GrammarElement * f_parent)
        {
            m_parent = f_parent;
        }

        // TODO: what is the difference between tag and ElementName??
        //  tag does not seem to be used
        std::string getTag() const
        {
            return m_tag;
        }

        std::string getTypeName() const
        {
            return m_typeName;
        }

        std::string getElementName() const
        {
            return m_elementName;
        }

        std::string getDocument() const
        {
            return m_document;
        }

        void setDocument(const std::string & f_document)
        {
            m_document = f_document;
        }

        virtual ~GrammarElement()
        {

        }

        virtual std::string getDotNode()
        {
            std::string result = "";
            result += "n" + std::to_string(m_instanceId) + "[label=\"" + std::to_string(m_instanceId) + " " + m_typeName + " " + m_elementName + " " + m_tag + " doc: \\\""+ m_document + "\\\"" + "\"];\n";
            for(auto child : m_children)
            {
                result += " n" + std::to_string(m_instanceId) + " -> n" + std::to_string(child->m_instanceId) + ";\n";
            }
            //std::cout << " got dot string: " << result;
            return result;
        }
    protected:
        GrammarElement * m_parent;
        std::vector< GrammarElement * > m_children;

        std::string m_tag;
        const std::string m_typeName;
        const std::string m_elementName;
        const uint32_t m_instanceId;
        std::string m_document;
    private:
        static uint32_t getAndIncrementInstanceCounter()
        {
            static uint32_t instanceCounter = 0;
            return instanceCounter++;
        }
};


}
