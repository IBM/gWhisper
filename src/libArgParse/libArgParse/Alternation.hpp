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
class Alternation : public GrammarElement
{
    public:
        explicit Alternation(const std::string & f_elementName = ""):
            GrammarElement("Alternation", f_elementName)
        {
        }

        virtual std::string toString() override
        {
            std::string result;
            if(m_children.size()>1)
            {
                result += "(";
            }
            bool first = true;
            for(auto child: m_children)
            {
                if(!first)
                {
                    result += "||";
                }
                else
                {
                    first = false;
                }
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
            ParseRc rc;
            f_out_ParsedElement.setGrammarElement(this);

            //std::cout << "Alternation " << std::to_string(m_instanceId) << ": parsing '" << f_string << " cd=" << candidateDepth << "'\n";

            std::vector<std::shared_ptr<ParsedElement> > candidateList;

            std::shared_ptr<ParsedElement> winner;
            std::shared_ptr<ParsedElement> maybeWinner;
            GrammarElement * maybeWinnerGE;
            GrammarElement * winnerGE;
            size_t maybeCount = 0;
            size_t newCandidateDepth = candidateDepth;
            if(candidateDepth > 0)
            {
                // New idea: never allow forks by default.
                // if we realize in the end, that we are unique, we parse
                // again, this time allowing forks
                newCandidateDepth--;
            }
            std::vector<GrammarElement*> maybeList;
            for(auto child : m_children)
            {
                auto newParsedElement = std::make_shared<ParsedElement>(&f_out_ParsedElement);
                ParseRc childRc = child->parse(f_string, *newParsedElement, newCandidateDepth);
                //std::cout << " Alternation pass1 "<< std::to_string(m_instanceId) <<  " parsed child ? rc=" << childRc.toString() << " #candidates: " << std::to_string(childRc.candidates.size()) << std::endl;
                if(childRc.isGood())
                {
                    if(rc.lenParsedSuccessfully <= childRc.lenParsedSuccessfully)
                    {
                        // FIXME: what about optional childs? They always succeed but have candidates
                        //  -> this is low prio, as optional childs in alternations do not really make sense, but should be supported in the end anyways
                        rc.lenParsedSuccessfully = childRc.lenParsedSuccessfully;
                        rc.lenParsed = childRc.lenParsed;
                        winner = newParsedElement;
                        winnerGE = child;
                    }
                    maybeList.push_back(child);
                }
                if(childRc.isBad() && (childRc.errorType == ParseRc::ErrorType::missingText))
                {
                    maybeWinner = newParsedElement;
                    maybeWinnerGE = child;
                    maybeCount++;
                    maybeList.push_back(child);
                }
                else if(childRc.isBad() && (childRc.errorType == ParseRc::ErrorType::retrievingGrammarFailed))
                {
                    rc.ErrorMessage = rc.ErrorMessage + childRc.ErrorMessage + " ";
                    rc.errorType = ParseRc::ErrorType::retrievingGrammarFailed;
                }

                // Add all candidates regardless of RC
                for(auto candidate : childRc.candidates)
                {
                    //std::cout << "  Alternation"<< std::to_string(m_instanceId) << ": have possible candidate: '" << candidate->getMatchedString() << "'" << std::endl;
                    candidateList.push_back(candidate);
                }
            }

            if(winner != nullptr)
            {
                // have winner
                f_out_ParsedElement.addChild(winner);

                // parse again allowing forks this time. to get possible candidates in optional paths
                ParsedElement unused;
                ParseRc childRc = winnerGE->parse(f_string, unused, candidateDepth);
                candidateList = childRc.candidates;

                // unfortunately we now lost the previous candidates.
                // need to parse again all children except winner to get those again.
                // TODO: this is ugly. fix it better
                for(auto child : maybeList)
                {
                    if(child == winnerGE)
                    {
                        continue;
                    }
                    auto newParsedElement = std::make_shared<ParsedElement>(&f_out_ParsedElement);
                    ParseRc childRc = child->parse(f_string, *newParsedElement, newCandidateDepth);
                    if(childRc.lenParsed < rc.lenParsed)
                    {
                        // we skip this succestion if the length is less tah our winner
                        // (only keep additional options if they branch after the winner)
                        continue;
                    }
                    for(auto candidate : childRc.candidates)
                    {
                        //std::cout << "  Alternation"<< std::to_string(m_instanceId) << ": have possible candidate: '" << candidate->getMatchedString() << "'" << std::endl;
                        candidateList.push_back(candidate);
                    }
                }
            }
            else
            {
                if(maybeCount == 1)
                {
                    // we have had enough text to uniquely select one of the alternates
                    // => we can add it to the f_out_ParsedElement:
                    f_out_ParsedElement.addChild(maybeWinner);
                    // in this case we could uniquely identify a candidate :)
                    // so we need to parse again for candidates, this time allowing for forks
                    candidateList.clear();
                    ParsedElement unused;
                    ParseRc childRc = maybeWinnerGE->parse(f_string, unused, candidateDepth);
                    candidateList = childRc.candidates;
                    //std::cout << " Alternation pass2 "<< std::to_string(m_instanceId) <<  " parsed child ? rc=" << childRc.toString() << " #candidates: " << std::to_string(childRc.candidates.size()) << std::endl;
                }

                if(rc.errorType != ParseRc::ErrorType::retrievingGrammarFailed)
                {
                    // merge RCs
                    if(m_children.size() == 0)
                    {
                        rc.errorType = ParseRc::ErrorType::success;
                    }
                    else if(candidateList.size() == 0)
                    {
                        rc.errorType = ParseRc::ErrorType::unexpectedText;
                    }
                    else
                    {
                        rc.errorType = ParseRc::ErrorType::missingText;
                        rc.lenParsed = strlen(f_string);
                    }
                }
            }

            // add all candidates to the candidate list
            for(auto candidate : candidateList)
            {
                //std::cout << "Alt " << std::to_string(m_instanceId) << " handling candidate '" << candidate->getMatchedString() << "'" << std::endl; 
                auto candidateRoot = std::make_shared<ParsedElement>(f_out_ParsedElement.getParent());
                candidateRoot->setGrammarElement(this);
                candidateRoot->addChild(candidate);
                rc.candidates.push_back(candidateRoot);
            }

            return rc;
        }
};

}
