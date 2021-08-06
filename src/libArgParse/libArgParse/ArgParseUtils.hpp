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
#include <string>
#include <vector>
#include <memory>
#include <libArgParse/ParsedElement.hpp>

namespace ArgParse
{
struct ParseRc
{
    enum class ErrorType
    {
        success,
        missingText,
        unexpectedText,
        retrievingGrammarFailed,
    };

    ErrorType errorType = ErrorType::success;
    std::string ErrorMessage = "";

    // TODO: define what those two lengths actually mean
    size_t lenParsedSuccessfully = 0;
    size_t lenParsed = 0;

    std::string toString() const
    {
        switch(errorType)
        {
            case ErrorType::success:
                return "success";
                break;
            case ErrorType::missingText:
                return "missingText";
                break;
            case ErrorType::unexpectedText:
                return "unexpectedText";
                break;
            case ErrorType::retrievingGrammarFailed:
                return "retrievingGrammarFailed";
                break;
            default:
                return "???";
            break;
        }
    }

    bool isGood() const
    {
        return errorType == ErrorType::success;
    }

    bool isBad() const
    {
        return not isGood();
    }

    std::vector<std::shared_ptr<ParsedElement> > candidates;
};
}
