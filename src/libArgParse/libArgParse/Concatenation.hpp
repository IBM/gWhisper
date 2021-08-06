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
#include <libArgParse/GrammarElement.hpp>
namespace ArgParse
{
class Concatenation : public GrammarElement
{
    public:
        explicit Concatenation(const std::string & f_elementName = "") :
            GrammarElement("Concatenation", f_elementName)
        {
        }

        virtual std::string toString() override
        {
            std::string result;
            if(m_children.size()>1)
            {
                result += "(";
            }
            //bool first = true;
            for(auto child: m_children)
            {
                //if(!first)
                //{
                //    result += " ";
                //}
                //else
                //{
                //    first = false;
                //}
                result += child->toString();
            }
            if(m_children.size()>1)
            {
                result += ")";
            }
            return result;
        }

        virtual ParseRc parse(const char * f_string, ParsedElement & f_out_ParsedElement, size_t candidateDepth = 1, size_t startChild = 0) override
        {
            //std::cout << "Concat " << std::to_string(m_instanceId) << " parsing '" << std::string(f_string) << "' cd=" << std::to_string(candidateDepth) << std::endl; 
            ParseRc rc;
            ParseRc childRc;
            f_out_ParsedElement.setGrammarElement(this);

            if(m_children.size() == 0)
            {
                rc.ErrorMessage = "Concatenation Element has no child. Grammar incomplete.";
                rc.errorType = ParseRc::ErrorType::retrievingGrammarFailed;
                return rc;
            }

            //std::cout << "Concat "<< std::to_string(m_instanceId) <<  " parsing '" << std::string(f_string) << "' starting with child "  << std::to_string(startChild) << "/" << std::to_string(m_children.size()-1)<< "' cd=" << std::to_string(candidateDepth) <<  std::endl;
            for(size_t i = startChild; (i<m_children.size()) && rc.isGood(); i++)
            {
                //printf(" parsing child %zu\n", i);
                GrammarElement* child = m_children[i];

                auto newParsedElement = std::make_shared<ParsedElement>(&f_out_ParsedElement);
                childRc = child->parse(&f_string[rc.lenParsed], *newParsedElement);
                //std::cout << " Concat "<< std::to_string(m_instanceId) <<  " parsed child" << std::to_string(i) << " rc=" << childRc.toString() << " #candidates: " << std::to_string(childRc.candidates.size()) << " cd=" << candidateDepth<< std::endl;
                rc.lenParsed += childRc.lenParsed;
                rc.lenParsedSuccessfully += childRc.lenParsedSuccessfully;
                if(childRc.isBad() && childRc.errorType == ParseRc::ErrorType::retrievingGrammarFailed)
                {
                    rc.ErrorMessage = childRc.ErrorMessage;
                }

                // merge RCs
                rc.errorType = childRc.errorType;

                if(childRc.candidates.size() > 0)
                {
                    for(auto candidate : childRc.candidates)
                    {
                        //std::cout << "Concat " << std::to_string(m_instanceId) << GrammarElement::toString() << " handling candidate from child " << std::to_string(i)<< " '" << candidate->getMatchedString() << "' cd=" << std::to_string(candidateDepth) << std::endl; 
                        // we create a new candidate (same tree level as f_out_ParsedElement)
                        auto candidateRoot = std::make_shared<ParsedElement>(f_out_ParsedElement.getParent());
                        candidateRoot->setGrammarElement(this);

                        // first add all previous childs to the new root (from concatenation before the failing element):
                        for(auto oldChild : f_out_ParsedElement.getChildren())
                        {
                            candidateRoot->getChildren().push_back(oldChild);
                        }

                        // add the candidate to the new root:
                        candidateRoot->addChild(candidate);

                        // decide on future recursion depth:
                        size_t newCandidateDepth = candidateDepth;
                        if(childRc.candidates.size() > 1)
                        {
                            // if we have multiple candidates, we reduce the candidateDepth by 1
                            newCandidateDepth--;
                        }

                        // we only go on parsing the next children, if we have only one candidate (unique follow up possible)
                        // or we still are allowed to fork
                        // Otherwise we just do not return ANY candidates FIXME!!! i just added else path to add candidate in any case
                        if(candidateDepth > 0 || childRc.candidates.size() == 1)
                        {
                            // Now continue parsing the other children, using this candidate as a base:
                            ParseRc candidateRc;
                            if(!candidateRoot->isStopped())
                            {
                                //std::cout << "  -> forking for this candidate" << std::endl;
                                candidateRc = parse("", *candidateRoot, newCandidateDepth, i+1);
                            }
                            if(candidateRc.candidates.size() == 0)
                            {
                                // if we could not find any more candidates, we add this candidate:
                                candidateRoot->setStops(); //If we could not find a candidate we need to stop compeleting TODO: write unit test to ensure this
                                rc.candidates.push_back(candidateRoot);
                                //std::cout << "Concat " << std::to_string(m_instanceId) << " 0push candidate '" << candidateRoot->getMatchedString() << "'" << std::endl; 
                            }
                            else if(candidateRc.candidates.size() == 1)
                            {
                                rc.candidates.push_back(candidateRc.candidates[0]);
                                //std::cout << "Concat " << std::to_string(m_instanceId) << " 1push candidate '" << candidateRc.candidates[0]->getMatchedString() << "'" << std::endl; 
                            }
                            else
                            {
                                // if we could find more candidates in the recursion, we add them instead :)
                                // but only if we are still allowed to
                                for(auto cnd : candidateRc.candidates)
                                {
                                    rc.candidates.push_back(cnd);
                                    //std::cout << "Concat " << std::to_string(m_instanceId) << " Mpush candidate '" << candidateRc.candidates[0]->getMatchedString() << "'" << std::endl;
                                }
                            }
                        }
                        else if(i == 0)
                        {
                            // no forks are allowed, but we sill can add the candidates we already found
                            // in case we have nothing else already added (Concat with only alternation as child)
                            // TODO: think this through, I do not 100% understand how this works and if it does the rigth thing
                            rc.candidates.push_back(candidateRoot);
                            //std::cout << "Concat " << std::to_string(m_instanceId) << " i0push candidate '" << candidateRoot->getMatchedString() << "'" << std::endl; 
                        }
                    }

                }

                if(childRc.isGood() || childRc.errorType == ParseRc::ErrorType::missingText)
                {
                    //std::cout << "Concat "<< std::to_string(m_instanceId) <<  " have good child "  << std::to_string(i) << " matched string: " << newParsedElement->getMatchedString() << std::endl;
                    // need to do this AFTER candidate evaluation, as some parse
                    // methods will return good rc but still have candidates (e.g. repetition)
                    // in this case we would add their results to their candidates again
                    f_out_ParsedElement.addChild(newParsedElement);
                    if(not childRc.isGood())
                    {
                        // TODO: add this for other GrammarElements too
                        // TODO: write unit test for this!
                        f_out_ParsedElement.setIncompleteParse();
                    }
                }
            }

            //std::cout << "Concat "<< std::to_string(m_instanceId) <<  " returning rc=" << rc.toString() << " with " << std::to_string(rc.candidates.size()) << " candidates" << std::endl;

            return rc;
        }
};

}
