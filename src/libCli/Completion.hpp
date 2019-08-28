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
#include <libArgParse/ArgParse.hpp>

namespace cli
{
    typedef struct coordinate
    {
        uint32_t level;
        uint32_t order;
    } coordinate;

    typedef struct document_info
    {
        std::vector<coordinate> path;
        std::string document;
    } document_info;

    template<typename... Args>
    void searchChilden(ArgParse::ParsedElement * f_parseElement, std::string & f_out_document, Args...f_compared_string)
    {
        std::string childDoc = f_parseElement->getGrammarElement()->getDocument();
        if(!childDoc.empty())
        {
            std::string delims = "\r\n\t";
            childDoc.erase(0, childDoc.find_first_not_of(delims));
            childDoc.erase(childDoc.find_last_not_of(delims) + 1);

            std::vector<std::string> list = {f_compared_string...};
            for(auto& compared : list)
            {
                if(compared.find(childDoc) != std::string::npos)
                {
                    return;
                }
                else
                {
                    f_out_document = childDoc;
                }
            }
        }
        else
        {
            auto& childen = f_parseElement->getChildren();
            if(childen.size() > 0)
            {
                for(auto& child: childen)
                {
                    searchChilden(child.get(), f_out_document, std::forward<Args>(f_compared_string)...);
                }
            }
        }
    }

    void searchParent(ArgParse::ParsedElement * f_parseElement, std::string & f_out_document);

    ArgParse::ParsedElement * findRightMost(ArgParse::ParsedElement * f_parseElement, uint32_t & f_depth);

    std::string searchDocument(ArgParse::ParsedElement * f_parseElement);

    void abstractDocTree(ArgParse::ParsedElement * f_parseElement, std::vector<document_info> & f_out_documents, std::vector<coordinate> f_path, uint32_t f_depth, uint32_t f_numberOfNode);


    /// Function which prints bash completions to stdout for given list of parseTrees.
    /// NOTE: this is not calculationg completions, it merely formats existing completion results
    ///       in a way, so that bash can handle them.
    /// @param f_candidates vector of parse trees, each representing a completion candidate
    /// @param f_parseTree the parstree which contains everything which could already be matched.
    /// @param f_args the string given by the user which awaits completion
    /// @param f_debug enables debug output if true
    void printBashCompletions(
            std::vector<std::shared_ptr<ArgParse::ParsedElement> > & f_candidates,
            ArgParse::ParsedElement & f_parseTree,
            const std::string & f_args,
            bool f_debug
            );

    // TODO: only one function with enum dialect argument
    void printFishCompletions(
            std::vector<std::shared_ptr<ArgParse::ParsedElement> > & f_candidates,
            ArgParse::ParsedElement & f_parseTree,
            const std::string & f_args,
            bool f_debug
            );
}
