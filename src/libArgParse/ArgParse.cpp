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
    result += f_prefix + m_grammarElement->getTypeName() +
        "(" + m_grammarElement->getElementName() + "/"  + m_grammarElement->getTag()+ "): "+
        " matched string: \"" + getMatchedString() + "\" " +
        " document: \"" + m_grammarElement->getDocument() + "\" " +
        "(" + std::string(m_stops ? "stopped" : "alive") + ")\n";
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

ArgParse::ParsedElement * ArgParse::ParsedElement::findRightMostElement()
{
    ParsedElement * cursor = this;
    while(cursor->getChildren().size()!=0)
    {
        cursor = cursor->getChildren().back().get();
    };
    return cursor;
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
        child->findAllSubTrees(f_elementName, f_out_result, f_doNotSearchChildsOfMatchingElements, f_depth - 1);
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

std::string ArgParse::ParsedDocument::getOptionString(std::string f_optString)
{
    std::string::size_type pos_eq = f_optString.find_first_of(':');
    std::string output;
    if (pos_eq != std::string::npos)
    {
        output = f_optString.substr(pos_eq+1);
    }
    output.erase(remove(output.begin(), output.end(), '"' ), output.end());
    for(size_t i =0; i<output.length(); i++)
    {
        if(output[i] == '\n')
        {
            output[i] = ' ';
        }
    }
    std::string delims = "\r\n\t ";
    output.erase(0, output.find_first_not_of(delims));
    output.erase(output.find_last_not_of(delims) + 1);

    return output;
}

///ParsedDocument
void ArgParse::abstractDocTree(ParsedElement * f_parseElement, std::vector<ParsedDocument> & f_out_documents, std::vector<Coordinate> f_path, uint32_t f_depth, uint32_t f_index)
{
    if(f_parseElement == nullptr) return;
    ArgParse::ParsedDocument document_info(f_parseElement);
    ArgParse::Coordinate node = {f_depth, 0, f_index}; //no column info
    f_path.push_back(node);

    if(!f_parseElement->getGrammarElement()->getDocument().empty())
    {
        document_info.updatePath(f_path);
        document_info.calculateStepFromRoot();
        f_out_documents.push_back(document_info);
    }
    auto children = f_parseElement->getChildren();
    for(int i = 0; i < children.size(); ++i)
    {
        abstractDocTree(children[i].get(), f_out_documents, f_path, f_depth+1, i);
    }
}

std::string ArgParse::searchDocument(ParsedElement * f_parseElement, bool f_debug)
{
    std::vector<ParsedDocument> documents;
    std::vector<Coordinate> paths;
    ArgParse::abstractDocTree(f_parseElement, documents, paths, 0, 0);
    if(f_debug)
    {
        for(auto & document_info : documents)
        {
            document_info.printPath();
        }
    }

    if(documents.size() != 0)
    {
        auto rightmost_info = documents.back();
        for(auto& document_info : documents)
        {
            std::vector<Coordinate>::iterator it;
            std::vector<Coordinate>::iterator rightmost_it = rightmost_info.getPath().begin();

            for(it = document_info.getPath().begin();it != document_info.getPath().end();)
            {
                if(it->depth == rightmost_it->depth and it->index > rightmost_it->index)
                {
                    rightmost_info = document_info;
                    break;
                }
                ++it;
                ++rightmost_it;
            }
        }
        return rightmost_info.getParsedElement()->getGrammarElement()->getDocument();
    }
    else
    {
        return "";
    }
}
