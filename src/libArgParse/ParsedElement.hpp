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
#include <libArgParse/GrammarElement.hpp>
#include <vector>
#include <string>
#include <memory>
#include <limits>

namespace ArgParse
{
class GrammarElement;
// a Tree
class ParsedElement
{
    public:
        ParsedElement() :
            m_grammarElement(nullptr),
            m_parent(this)
        {
        }

        explicit ParsedElement(ParsedElement * f_parent) :
            m_grammarElement(nullptr),
            m_parent(f_parent)
        {
        }

        explicit ParsedElement(GrammarElement * f_grammarElement) :
            m_grammarElement(f_grammarElement),
            m_parent(this)
        {
        }

        GrammarElement * getGrammarElement()
        {
            return m_grammarElement;
        }

        void setGrammarElement(GrammarElement * f_grammarElement)
        {
            m_grammarElement = f_grammarElement;
        }

        ParsedElement & addChild(std::shared_ptr<ParsedElement> f_element)
        {
            f_element->setParent(this);
            m_children.push_back(f_element);
            return *(m_children.back());
        }

        void setMatchedStringUnescaped(const std::string & f_string)
        {
            m_matchedStringUnEscaped = f_string;
        }

        void setMatchedString(const std::string & f_string)
        {
            m_matchedStringRaw = f_string;
        }

        /// prints the "flattened parse tree" i.e. the complete matched string.
        /// Escape characters are still contained here (useful for completion)
        std::string getMatchedStringRaw() const
        {
            std::string result = m_matchedStringRaw;
            for(auto child : m_children)
            {
                result += child->getMatchedStringRaw();
            }
            return result;
        }

        /// prints the "flattened parse tree" i.e. the complete matched string.
        /// Escaped characters are already processed here
        std::string getMatchedString() const
        {
            std::string result = m_matchedStringUnEscaped;
            if(result=="")
            {
                // we do not have escaped characters -> can use raw string
                result = m_matchedStringRaw;
            }
            for(auto child : m_children)
            {
                result += child->getMatchedString();
            }
            return result;
        }

        /// prints out the complete parse tree.
        std::string getDebugString(const std::string & f_prefix = "");

        /// Returns a list of children.
        std::vector<std::shared_ptr<ParsedElement> > & getChildren()
        {
            return m_children;
        }

        /// depth first search for a single element, directly returning the matched string.
        /// @param f_elementName element name to search for (inherited from grammar element)
        /// @param f_depth The maximum depth which should still be searched.
        ///  f_depth=0 will only compare f_elementName with the element name of
        ///  the ParsedElement for which this method is called.
        ///  f_depth=1 will check all children
        ///  f_depth=2 will check all children and their children
        ///  and so on.
        std::string findFirstChild(const std::string & f_elementName, uint32_t f_depth = std::numeric_limits<uint32_t>::max());

        /// depth first search for a single element.
        /// @param f_elementName element name to search for (inherited from grammar element)
        /// @param f_out_found out parameter. true is written if a match was found
        ///  false is written otherwise.
        /// @param f_depth The maximum depth which should still be searched.
        ///  f_depth=0 will only compare f_elementName with the element name of
        ///  the ParsedElement for which this method is called.
        ///  f_depth=1 will check all children
        ///  f_depth=2 will check all children and their children
        ///  and so on.
        ParsedElement & findFirstSubTree(const std::string & f_elementName, bool & f_out_found, uint32_t f_depth = std::numeric_limits<uint32_t>::max());

        /// depth first search for elements.
        /// @param f_elementName element name to search for (inherited from grammar element)
        /// @param f_out_result vector to which found elements are written to
        /// @param f_doNotSearchChildsOfMatchingElements if true, the search will
        ///  not traverse deeper in a branch after finding the first match.
        ///  example: <NodeId>.<ElementName>
        ///  1.A -> 2.B -> 3.B
        ///      -> 4.C -> 5.B
        ///             -> 6.D
        ///      -> 7.B
        ///  search for B with f_doNotSearchChildsOfMatchingElements == true:
        ///      2, 5, 7
        ///  search for B with f_doNotSearchChildsOfMatchingElements == false:
        ///      2, 3, 5, 7
        /// @param f_depth The maximum depth which should still be searched.
        ///  f_depth=0 will only compare f_elementName with the element name of
        ///  the ParsedElement for which this method is called.
        ///  f_depth=1 will check all children
        ///  f_depth=2 will check all children and their children
        ///  and so on.
        void findAllSubTrees(const std::string & f_elementName, std::vector<ArgParse::ParsedElement *> & f_out_result, bool f_doNotSearchChildsOfMatchingElements = false, uint32_t f_depth = std::numeric_limits<uint32_t>::max());

        /// get docString of right most node in the subtree
        std::string getShortDocument() const;

        void setParent(ParsedElement * f_parent)
        {
            m_parent = f_parent;
        }

        ParsedElement * getParent() const
        {
            return m_parent;
        }

        ParsedElement * getRoot() const
        {
            ParsedElement * result = getParent();
            while(result->getParent() != result)
            {
                result = result->getParent();
            }

            return result;
        }
        void setStops()
        {
            if(m_children.size() == 0)
            {
                m_stops = true;
            }
            else
            {

                m_children.back()->setStops();
            }
        }
        bool isStopped() const
        {
            if(m_stops)
            {
                return true;
            }
            for(auto child :m_children)
            {
                bool stopped = child->isStopped();
                if(stopped)
                {
                    return true;
                }
            }
            return false;
        }

        void setIncompleteParse()
        {
            m_incompleteParse = true;
        }

        bool isCompletelyParsed() const
        {
            return (not m_incompleteParse);
        }

    private:
        GrammarElement * m_grammarElement;
        ParsedElement * m_parent;
        std::vector< std::shared_ptr<ParsedElement> > m_children;
        bool m_stops = false;
        std::string m_matchedStringRaw;
        std::string m_matchedStringUnEscaped;
        bool m_incompleteParse = false;
};

}
