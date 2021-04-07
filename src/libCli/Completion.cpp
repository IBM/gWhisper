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

#include <queue>
#include <libCli/Completion.hpp>

using namespace ArgParse;

namespace cli
{
    // This function modifies the candidate string, such that:
    // - Only the arguments after the user input are counted as suggestion
    // - Only one suggestion is completed at a a time
    //   (everything after a whitespace in a suggestion should be anouther suggestion)
    std::string getNextFishSuggestion(const std::string &f_candidateString, const std::string f_userInput, bool &f_out_isTrimmed)
    {
        // In fish, a completion needs to start from last space provided by User Input
        char f_delim = ' ';

        std::string out_suggestion = f_candidateString;
        std::size_t suggStart = f_userInput.find_last_of(f_delim);

        // Used for debugging:
        size_t inputLength = f_userInput.length();
        size_t suggLength = out_suggestion.length();

        // avoid substring error if suggStart (_pos) is undefined
        //if (suggStart >= out_suggestion.length())
        if (suggStart == std::string::npos)
        {
            //{
            //return out_suggestion;
            //    out_suggestion = f_candidateString; //set suggStart to right value (0)
            //}
            suggStart = 0;
        }

        out_suggestion = out_suggestion.substr(suggStart, std::string::npos);

        // Remove whitespace at beginning. Otherwise empty suggestions will be returned.
        if (out_suggestion != " ")
        {
            size_t trim = out_suggestion.find_first_not_of(f_delim);
            out_suggestion = out_suggestion.substr(trim, std::string::npos);
        }

        // Only complete until first whitespace of suggestion
        std::size_t found = out_suggestion.find(f_delim);

        if (found != std::string::npos)
        {
            out_suggestion = out_suggestion.substr(0, found);
            f_out_isTrimmed = true;
        }
        return out_suggestion;
    }

    void printFishCompletions(std::vector<std::shared_ptr<ParsedElement>> &f_candidates, ParsedElement &f_parseTree, const std::string &f_args, bool f_debug)
    {
        // completion requested :)
        if (f_debug)
        {
            std::cerr << "Input string \"" << f_args << std::endl;
        }

        // List with all suggestions for this candidate. Used to eliminate duplicate suggestions
        std::vector<std::string> suggestions;

        // size_t n = parseTree.getMatchedString().size();
        size_t n = f_args.size();
        for (auto candidate : f_candidates)
        {
            std::string candidateStr = candidate->getMatchedStringRaw();

            if (f_debug)
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

            std::string suggestionDoc = candidate.get()->getShortDocument();
            std::string suggestionDocRoot = f_parseTree.getShortDocument();
            // here the document (description of the tab completion) extracted from the original parsed tree and from the candidate tree should be compared,
            // if there is no new document of candidate in the parsed tree, that means the both extracted documents must same,
            // we only find the document at the right most position in the parsed tree (the newest) for tab completion.
            if (suggestionDocRoot == suggestionDoc)
            {
                suggestionDoc = "";
            }

            if (f_debug)
            {
                std::cout << "suggestionDoc: " << suggestionDoc << std::endl;
                printf("pre: '%s'\n", candidateStr.c_str());
                printf("candidateStr[n=%zu] = '%c'\n", n, candidateStr[n]);
            }

            bool isTrimmed = false;
            suggestion = getNextFishSuggestion(candidateStr, f_args, isTrimmed);

            if (std::find(suggestions.begin(), suggestions.end(), suggestion) != suggestions.end())
            {
                // Skip duplicate suggestions, continue with next candidate
                continue;
            }
            else
            {
                suggestions.push_back(suggestion);
            }

            if (f_debug)
            {
                printf("nospace! cand='%s', n=%zu, start=%zu, end = %zu\n", candidateStr.c_str(), n, start, end);
            }

            // Only Add Documentation, if string was not trimmed
            // In most casesa trimmed String means, that we complete with " " or ":". Those symbols do not need an documentation, hence we need to
            // remove the documentation provided by the untrimmed candidate
            if (suggestion != "" && suggestion.back() != ':' && !isTrimmed)
            {
                if (!suggestionDoc.empty())
                {
                    suggestion = suggestion + "\t" + suggestionDoc;
                }
            }

            if (f_debug)
            {
                printf("post: '%s'\n", suggestion.c_str());
            }

            // NOTE: be careful when adding description (tab-delimiter) here, as
            // fish summarizes all options with same description
            printf("%s\n", suggestion.c_str());
        }
    }

    struct Suggestion
    {
        std::string completion;
        std::string documentation;
    };

    void printBashCompletions(std::vector<std::shared_ptr<ParsedElement>> &f_candidates, ParsedElement &f_parseTree, const std::string &f_args, bool f_debug)
    {
        // completion requested :)
        if (f_debug)
        {
            std::cerr << "Input string \"" << f_args << "\"\nCandidates:\n"
                      << std::endl;
            for (auto candidate : f_candidates)
            {
                std::string candidateStr = candidate->getMatchedStringRaw();
                printf("pre: '%s'\n", candidateStr.c_str());
            }
        }

        size_t maxSuggestionLen = 0;
        size_t maxSuggestionDocLen = 0;

        // list of suggestions. first=completed string, second=docstring
        std::vector<Suggestion> suggestions;
        size_t n = f_args.size();

        for (auto candidate : f_candidates)
        {
            std::string candidateStr = candidate->getMatchedStringRaw();
            Suggestion suggestion;
            size_t start = n;
            size_t end;
            if (f_debug)
            {
                printf("candidateStr[n=%zu] = '%c'\n", n, candidateStr[n]);
            }
            if (
                (candidateStr[n] != ' ') &&
                (candidateStr[n] != '=') &&
                (candidateStr[n] != ',') &&
                (candidateStr[n] != ':'))
            {
                // bash always expects completion suggestions to start from the last token.
                // Now we need to find out where the last token has started
                // We need to "simulate" bash tokenizer here. tokens are delimited by ' ' '=' or ':'
                start = candidateStr.find_last_of(" =:,", n) + 1;
                if (start == std::string::npos)
                {
                    start = 0;
                }
                end = candidateStr.find_first_of(" ", n) - 1;
                //printf("cand='%s', n=%zu, start=%zu, end = %zu\n",candidateStr.c_str(), n, start, end);
            }
            else
            {
                start = candidateStr.find_last_of(" =:", n - 1) + 1;
                //end = candidateStr.find_first_of(' ', n+1);
                end = n;
            }

            suggestion.completion = candidateStr.substr(start, std::string::npos);
            suggestion.completion.erase(suggestion.completion.find_last_not_of(" ") + 1);

            suggestion.documentation = candidate.get()->getShortDocument();
            std::string suggestionDocRoot = f_parseTree.getShortDocument();
            if (suggestionDocRoot == suggestion.documentation)
            {
                suggestion.documentation = "";
            }
            // find the max length of suggestion and document for right-aligned
            if (!suggestion.documentation.empty())
            {
                if (maxSuggestionLen < suggestion.completion.size())
                {
                    maxSuggestionLen = suggestion.completion.size();
                }
                if (maxSuggestionDocLen < suggestion.documentation.size())
                {
                    maxSuggestionDocLen = suggestion.documentation.size();
                }
            }
            suggestions.push_back(suggestion);
        }

        for (const Suggestion &suggestion : suggestions)
        {
            std::string output = suggestion.completion;
            if ((not suggestion.documentation.empty()) and suggestions.size() > 1)
            {
                // Only print documentation if choice is not unique
                // Bash will select completion automatically if uniqe. Hence
                // we are not allowed to print completions in this case.

                std::string palceholder = "  ";
                // calculate how many whitespaces should be inserted into the suggestion
                for (size_t i = 0; i < maxSuggestionLen - suggestion.completion.size() + maxSuggestionDocLen - suggestion.documentation.size(); ++i)
                {
                    palceholder += " ";
                }
                // final output with documentation(description)
                // unfortunately bash does not seem to support colors in completions
                output += palceholder + "(" + suggestion.documentation + ")";
            }
            if (f_debug)
            {
                printf("post: '%s'\n", output.c_str());
            }
            else
            {
                printf("%s\n", output.c_str());
            }
        }
    }
}
