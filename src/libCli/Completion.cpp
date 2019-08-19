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
        std::string suggestionDoc = candidate->getMatchedStringDoc();
        std::string suggestion;
        size_t start = n;
        size_t end;

        if(f_debug)
        {
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
        suggestion = suggestion + "\t" + suggestionDoc + "'\n";
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
