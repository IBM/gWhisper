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

# This is a very simple completion function. It essentially offloads all
# completion work to the gWhisper tool itself, by issuing a completion request.
# The output of gWhisper is then used to fill the COMPREPLY variable.
_gWhisperCompletion()
{
    # we interpret the first word as the gWhisper command itself:
    COMMANDNAME=${COMP_WORDS[0]}
    if [ $GWHISPER_DEBUG_COMPLETION ]
    then
        echo "## completion for command: ######################################"
        echo "$COMMANDNAME"
    fi

    # index of the first character of command arguments
    ARG_START=$(( ${#COMMANDNAME} + 1 ))
    # length of characters in arguments until (excluding) cursor position
    ARG_LEN=$(( $COMP_POINT - $ARG_START ))

    ARGS=${COMP_LINE:$ARG_START:$ARG_LEN}
    if [ $GWHISPER_DEBUG_COMPLETION ]
    then
        echo "## arguments to complete: #######################################"
        echo "\"$ARGS\""
    fi

    # we retrieve completion choices by just executing gWhisper with the
    # --complete argument in the beginning:
    SUGGESTIONS=$($COMMANDNAME "--complete $ARGS")

    if [ $GWHISPER_DEBUG_COMPLETION ]
    then
        echo "## gWhisper returned the following completion suggestion: ##########"
        echo "\"$SUGGESTIONS\""
    fi

    # We parse the suggestions returned by gWhisper into an array. We only split
    # on newline, wo we have to change $IFS and restore after the conversion:
    OIFS=$IFS;
    IFS=$'\n'; # we only split on newlines
    COMPREPLY=($SUGGESTIONS)
    IFS=$OIFS;


    if [ $GWHISPER_DEBUG_COMPLETION ]
    then
        echo "## parsed suggestion string into array: #########################"
        for i in "${COMPREPLY[@]}"
        do
            echo "entry: '$i'"
        done
    fi

    return 0
}

complete -o nospace -F _gWhisperCompletion gwhisper
