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


/// This is a summary header file for a group of classes which compose an argument parsing system.
/// The high-level concept of this framework is as follows:
/// - GrammarElement instances are created from a memory pool and compose a graph representing the grammar to parse.
/// - Multiple derivations of GrammarElement implement different grammar features (Alternation, Concatenation, FixedString, Optional, RegEx, ...)
///   Those elements provide the building blocks for the grammar to be implemented by the user.
/// - GrammarElements may be combined by calling the addChild() methods.
/// - GrammarElements may be associated with a string tag, which will be assigned to all elements which get parsed by this element. (similar to backreferences in regex)
/// - Once the Grammar is constructed, the user may call parse() with a given string on one of the GramarElements (typically the root/start element)
/// - The parse() function will generate
///   - a parse tree containing all parsed values, composed of ParsedElement instances
///   - a list of possible parse trees results if parse was incomplete. Those may be used for completion.
///   - additional meta information about the parse (return code, parsed length, etc.)
/// - The parse tree provides utility functions to access parsed data:
///   - search of tagged elements
///   - iteration/inspection of the tree
/// - Each ParsedElement contains a reference to its associated GrammarElement.

// TODO: individual classes and their member functions of this framework are not yet documented well enough

#pragma once
#include <inttypes.h>
#include <memory>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>
#include <functional>
#include <stdio.h>
#include <iostream>

#include <libArgParse/Alternation.hpp>
#include <libArgParse/Concatenation.hpp>
#include <libArgParse/FixedString.hpp>
#include <libArgParse/Optional.hpp>
#include <libArgParse/ParsedElement.hpp>
#include <libArgParse/ParsedDocument.hpp>
#include <libArgParse/RegEx.hpp>
#include <libArgParse/Repetition.hpp>
#include <libArgParse/WhiteSpace.hpp>
#include <libArgParse/GrammarElement.hpp>
#include <libArgParse/ArgParseUtils.hpp>
#include <libArgParse/GrammarInjector.hpp>
#include <libArgParse/GrammarFactory.hpp>
#include <libArgParse/EscapedString.hpp>

