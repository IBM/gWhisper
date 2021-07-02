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

#include <libArgParse/GrammarElement.hpp>
#include <memory>
namespace ArgParse
{

    class Grammar
    {
        public:
            Grammar() :
                m_rootElement(nullptr)
            {
            }
            template<class T, class... Args >
                T * createElement( Args&&... f_args )
                {
                    T * newNodePtr = new T(std::forward<Args>(f_args)...);
                    std::unique_ptr<T> newNode(newNodePtr);
                    m_nodes.push_back(std::move(newNode));
                    return static_cast<T*>(m_nodes.back().get());
                }

            void setRoot( GrammarElement * f_rootElement)
            {
                m_rootElement = f_rootElement;
            }

            std::string getDotGraph();

            virtual ~Grammar()
            {
            }

        private:
            std::vector< std::unique_ptr<GrammarElement> > m_nodes;
            GrammarElement * m_rootElement;
    };

}

