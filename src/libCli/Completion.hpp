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
