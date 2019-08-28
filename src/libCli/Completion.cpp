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

#include <libCli/Completion.hpp>

using namespace ArgParse;

namespace cli
{

void abstractDocTree(ArgParse::ParsedElement * f_parseElement, std::vector<document_info> & f_out_documents, std::vector<coordinate> f_path, uint32_t f_depth, uint32_t f_numberOfNode)
{
    if(f_parseElement == nullptr) return;
    ArgParse::ParsedElement * cursor = f_parseElement;
    document_info docInfo;
    coordinate node = {f_depth, f_numberOfNode};
    f_path.push_back(node);
    docInfo.document = cursor->getGrammarElement()->getDocument();

    if(!docInfo.document.empty())
    {
        docInfo.path = f_path;
        f_out_documents.push_back(docInfo);
    }

    auto children = cursor->getChildren();

    for(int i = 0; i < children.size(); ++i)
    {
        abstractDocTree(children[i].get(), f_out_documents, f_path, f_depth+1, i);
    }
}

//not uesd
ArgParse::ParsedElement * findRightMost(ArgParse::ParsedElement * f_parseElement, uint32_t &  f_depth)
{
    ParsedElement * rightMost = f_parseElement;
    while(rightMost->getChildren().size()!=0)
    {
        f_depth++;
        rightMost = rightMost->getChildren().back().get();
    };

    return rightMost;
}

// not used
void searchParent(ArgParse::ParsedElement * f_parseElement, std::string & f_out_document)
{
    uint32_t depth = 0;
    ParsedElement * rightMost = findRightMost(f_parseElement, depth);
    f_out_document = rightMost->getGrammarElement()->getDocument();
    ParsedElement * parent = rightMost->getParent();
    while(f_out_document == "")
    {
        if(parent->getGrammarElement()->getDocument() != "")
        {
            f_out_document = parent->getGrammarElement()->getDocument();
        }
        else
        {
            auto children = parent->getChildren();
            if(children.size() > 0)
            {
                auto child_iter = children.end()-1;
                while (child_iter != children.begin())
                {
                    if((*child_iter)->getGrammarElement()->getDocument() != "")
                    {
                        f_out_document = (*child_iter)->getGrammarElement()->getDocument();
                        break;
                    }
                    --child_iter;
                }
            }
        }
        if(parent != parent->getParent())
        {
            parent = parent->getParent();
        }
        else
        {
            break;
        }
    }
}

std::string searchDocument(ArgParse::ParsedElement * f_parseElement)
{
    std::string service_document;
    std::string method_document;
    std::string field_document;

    // bool service_found;
    // bool method_found;
    // bool message_found;

    // auto service_tree = f_parseElement->findFirstSubTree("Service", service_found);
    // auto method_tree = f_parseElement->findFirstSubTree("Method", method_found);
    // auto message_tree = f_parseElement->findFirstSubTree("Message", message_found);

    // std::string service_document = f_parseElement->findDocumentIncomplete("Service");
    // std::string method_document = f_parseElement->findDocumentIncomplete("Method");
    // std::string message_document = f_parseElement->findDocumentIncomplete("Message");

    // std::map<std::pair<uint32_t, uint32_t>, std::string> documents = {{std::make_pair(0, 0), ""}};
    std::vector<document_info> documents;
    std::vector<coordinate> paths;

    abstractDocTree(f_parseElement, documents, paths, 0, 0);
    for(auto& document_info : documents)
    {
        for(auto& node: document_info.path)
        {
            std::cout << "(" << node.level << ", " << node.order << ")->";
        }
        std::cout << document_info.document << std::endl;
    }

    // if(&service_tree and &method_tree and &message_tree)
    // {
    //     searchChilden(&service_tree, service_document);
    //     searchChilden(&method_tree, method_document, service_document);
    //     searchChilden(&message_tree, field_document, service_document, method_document);
    //     // std::cout << "service_document: " << service_document << std::endl;
    //     // std::cout << "method_document: " << method_document << std::endl;
    //     // std::cout << "field_document: " << field_document << std::endl;

    //     if(!field_document.empty()){
    //         return field_document;
    //     }
    //     else if(!method_document.empty())
    //     {
    //         return method_document;
    //     }
    //     return service_document;
    // }

    std::string document = "well";
    return document;
}


void printFishCompletions( std::vector<std::shared_ptr<ParsedElement> > & f_candidates, ParsedElement & f_parseTree, const std::string & f_args, bool f_debug)
{
    // completion requested :)
    if(f_debug)
    {
        std::cerr << "Input string \"" << f_args << std::endl;
    }

    //size_t n = parseTree.getMatchedString().size();
    size_t n = f_args.size();
    for(auto candidate : f_candidates)
    {
        std::string candidateStr = candidate->getMatchedString();

        if(f_debug)
        {
            std::cout << "------------------------------------------------" << std::endl;
            std::cout << "candiate string: " << candidateStr << std::endl;
            std::cout << "candiate's debug string: " << std::endl;
            std::cout << candidate->getDebugString() << std::endl;
            std::cout << "------------------------------------------------" << std::endl;
        }

        std::string suggestion;
        size_t start = n;
        size_t end;

        std::string suggestionDoc = searchDocument(candidate.get());

        if(f_debug)
        {
            std::cout << "suggestionDoc: " << suggestionDoc << std::endl;
            printf("pre: '%s'\n", candidateStr.c_str());
            printf("candidateStr[n=%zu] = '%c'\n", n, candidateStr[n]);
        }

        start = candidateStr.find_last_of(" ", n)+1;
        if(start == std::string::npos)
        {
          start = 0;
        }
        end = candidateStr.find_first_of(" ", n)-1;

        if(f_debug)
        {
          printf("nospace! cand='%s', n=%zu, start=%zu, end = %zu\n",candidateStr.c_str(), n, start, end);
        }

        suggestion = candidateStr.substr(start, std::string::npos);
        size_t trimEnd = suggestion.find_last_not_of(' ');
        suggestion = suggestion.substr(0, trimEnd+1);
        //suggestion = candidateStr.substr(start, end-start+1);

        if(suggestionDoc != "")
        {
            suggestion = suggestion + "\t" + suggestionDoc;
        }

        if(f_debug)
        {
            printf("post: '%s'\n", suggestion.c_str());
        }
        // NOTE: be careful when adding description (tab-delimiter) here, as
        // fish summarizes all options with same description
        printf("%s\n", suggestion.c_str());
    }
}

void printBashCompletions( std::vector<std::shared_ptr<ParsedElement> > & f_candidates, ParsedElement & f_parseTree, const std::string & f_args, bool f_debug)
{
    // completion requested :)
    if(f_debug)
    {
        std::cerr << "Input string \"" << f_args << "\"\nCandidates:\n" << std::endl;
        for(auto candidate : f_candidates)
        {
            std::string candidateStr =candidate->getMatchedString();
            printf("pre: '%s'\n", candidateStr.c_str());
        }
    }

    //size_t n = parseTree.getMatchedString().size();
    size_t n = f_args.size();
    for(auto candidate : f_candidates)
    {
        std::string candidateStr =candidate->getMatchedString();
        std::string suggestion;
        size_t start = n;
        size_t end;
        if(f_debug)
        {
            printf("candidateStr[n=%zu] = '%c'\n", n, candidateStr[n]);
        }
        if(
                (candidateStr[n] != ' ')
                &&
                (candidateStr[n] != '=')
                &&
                (candidateStr[n] != ',')
                &&
                (candidateStr[n] != ':')
        )
        {
            // bash always expects completion suggestions to start from the last token.
            // Now we need to find out where the last token has started
            // We need to "simulate" bash tokenizer here. tokens are delimited by ' ' '=' or ':'
            start = candidateStr.find_last_of(" =:,", n)+1;
            if(start == std::string::npos)
            {
                start = 0;
            }
            end = candidateStr.find_first_of(" ", n)-1;
            //printf("cand='%s', n=%zu, start=%zu, end = %zu\n",candidateStr.c_str(), n, start, end);
        }
        else
        {
            start = candidateStr.find_last_of(" =:", n-1)+1;
            //end = candidateStr.find_first_of(' ', n+1);
            end = n;
        }
        //printf("start=%zu, end=%zu\n", start, end);
        //suggestion = candidateStr.substr(n, std::string::npos);
        suggestion = candidateStr.substr(start, std::string::npos);
        //suggestion = candidateStr.substr(start, end-start+1);
        if(f_debug)
        {

            printf("post: '%s'\n", suggestion.c_str());
        }
        else
        {
            printf("%s\n", suggestion.c_str());
        }
    }
}

}
