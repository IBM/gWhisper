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

#include <libArgParse/ArgParse.hpp>

//uint32_t ArgParse::GrammarElement::m_instanceCounter = 0;

std::string ArgParse::ParsedElement::getDebugString(const std::string & f_prefix)
{
    std::string result;
    if(m_grammarElement == nullptr)
    {
        return "!!Uninitialized Element!!";
    }
    result += f_prefix + m_grammarElement->getTypeName() + "(" + m_grammarElement->getElementName() + "): \"" + getMatchedString() + "\" (" + std::string(m_stops ? "stopped" : "alive") + ")\n";
    for(auto child : m_children)
    {
        result += child->getDebugString(f_prefix + "  ");
    }
    return result;
}

std::string ArgParse::ParsedElement::findFirstChild(const std::string & f_elementName, uint32_t f_depth)
{
    bool found = false;
    ParsedElement & result = findFirstSubTree(f_elementName, found, f_depth);
    if(found)
    {
        return result.getMatchedString();
    }
    else
    {
        return "";
    }
}

void ArgParse::ParsedElement::findAllSubTrees(const std::string & f_elementName, std::vector<ArgParse::ParsedElement *> & f_out_result, bool f_doNotSearchChildsOfMatchingElements, uint32_t f_depth)
{
    if(m_grammarElement->getElementName() == f_elementName)
    {
        f_out_result.push_back(this);
        if(f_doNotSearchChildsOfMatchingElements)
        {
            return;
        }
    }

    if(f_depth == 0)
    {
        return;
    }

    for(auto child : m_children)
    {
        child->findAllSubTrees(f_elementName, f_out_result, f_depth - 1);
    }
}

ArgParse::ParsedElement & ArgParse::ParsedElement::findFirstSubTree(const std::string & f_elementName, bool & f_out_found, uint32_t f_depth)
{
    //std::cout << "searching for " << f_elementName << " going through " << m_grammarElement->getTypeName() << " " << m_grammarElement->getElementName() << std::endl;
    if(m_grammarElement->getElementName() == f_elementName)
    {
        f_out_found = true;
        //std::cout << "searched for " << f_elementName << " returning true\n";
        return *this;
    }
    else if(f_depth == 0)
    {
        f_out_found = false;
        return *this;
    }
    else
    {
        for(auto child : m_children)
        {
            bool found = false;
            ParsedElement & result = child->findFirstSubTree(f_elementName, found, f_depth - 1);
            if(found)
            {
                f_out_found = true;
                return result;
            }
        }
    }

    f_out_found = false;
    return *this;
}

std::string ArgParse::Grammar::getDotGraph()
{
    //std::cout << "GENERATING DOT GRAPH\n";
    std::string result = "digraph {\n";
    result += "ordering=out;\n";
    for(auto & node : m_nodes)
    {
        //printf("getting info for node %p\n", node.get());
        result += node->getDotNode();
    }
    result += "}\n";
    return result;
}
