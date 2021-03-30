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

#include <cstdio>
#include <libArgParse/ArgParse.hpp>

#include <libCli/GrammarConstruction.hpp>
#include <libCli/Call.hpp>
#include <libCli/Completion.hpp>
#include <versionDefine.h> // generated during build

using namespace ArgParse;

std::string getArgsAsString(int argc, char **argv)
{
    std::string result;
    bool first = true;
    for(int i = 1; i<argc; i++)
    {
        if(!first)
        {
            result += " ";
        }
        else
        {
            first = false;
        }
        result += argv[i];
    }
    return result;
}

const char* g_helpString =
#include <HelpString.h>
;

int main(int argc, char **argv)
{
    // First we construct the initial Grammar for the CLI tool:
    Grammar grammarPool;
    GrammarElement * grammarRoot = cli::constructGrammar(grammarPool);

    // Now we parse the given arguments using the grammar:
    std::string args = getArgsAsString(argc, argv);
    ParsedElement parseTree;
    ParseRc rc = grammarRoot->parse(args.c_str(), parseTree);

    // TODO: add option to print parse tree after parsing:
    // // Now we act according to the parse tree:
    //std::cout << parseTree.getDebugString() << "\n";

    if(parseTree.findFirstChild("DotExport") != "")
    {
        std::cout << grammarPool.getDotGraph();
        return 0;
    }

    if(parseTree.findFirstChild("Complete") != "")
    {
        bool completeDebug = (parseTree.findFirstChild("CompleteDebug") != "");
        if(parseTree.findFirstChild("fish") != "")
        {
          cli::printFishCompletions(rc.candidates, parseTree, args, completeDebug);
        }
        else
        {
          cli::printBashCompletions(rc.candidates, parseTree, args, completeDebug);
        }
        return 0;
    }

    if(parseTree.findFirstChild("Version") != "")
    {
        std::cout << GWHISPER_BUILD_VERSION << std::endl;
        return 0;
    }

    if(parseTree.findFirstChild("Help") != "")
    {
        printf("%s", g_helpString);
        return 0;
    }

    if(rc.isGood() && (rc.lenParsedSuccessfully == args.length()))
    {
        // std::cout << parseTree.getDebugString() << "\n";
        return cli::call(parseTree);
    }


    if(rc.isBad() && rc.errorType == ParseRc::ErrorType::retrievingGrammarFailed && rc.ErrorMessage.size()!= 0)
    {
        std::cout << rc.ErrorMessage << std::endl;
        std::cout << "Grammar could not be fetched from the server address: "<< parseTree.getMatchedString() << std::endl;
    }
    else
    {
        std::cout << "Parse failed.";
        std::cout << "Parsed until: '" << parseTree.getMatchedString() << "'" << std::endl;
    }

    if( (rc. candidates.size() > 0) && (rc.errorType == ParseRc::ErrorType::missingText))
    {
        std::cout << "Possible Candidates:";
        for(auto candidate : rc.candidates)
        {
            //printf("\nchoice:\n%s", candidate->getDebugString().c_str());
            printf("\n  '%s'", candidate->getMatchedString().c_str());
        }
        std::cout << std::endl;
    }

    return -1;
}
