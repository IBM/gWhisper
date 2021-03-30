# Copyright 2019 IBM Corporation
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


set(TARGET_NAME "ArgParse")
set(TARGET_SRC
    ArgParse.cpp
    libArgParse/Alternation.hpp
    libArgParse/ArgParse.hpp
    libArgParse/ArgParseUtils.hpp
    libArgParse/Concatenation.hpp
    libArgParse/EscapedString.hpp
    libArgParse/FixedString.hpp
    libArgParse/GrammarElement.hpp
    libArgParse/GrammarFactory.hpp
    libArgParse/Grammar.hpp
    libArgParse/GrammarInjector.hpp
    libArgParse/Optional.hpp
    libArgParse/ParsedDocument.hpp
    libArgParse/ParsedElement.hpp
    libArgParse/RegEx.hpp
    libArgParse/Repetition.hpp
    libArgParse/WhiteSpace.hpp
    )
add_library(${TARGET_NAME} ${TARGET_SRC})
target_include_directories( ${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME}
    #boost_regex
    )
