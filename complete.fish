function _gwhisper_getCompletions;
  set -l commandLineUntilCursor (commandline -c)
  set -l commandLineTokenized (commandline -o)
  set -l commandName (commandline -o)[1]

  set -l args (string sub --start (math (string length $commandName) + 1) $commandLineUntilCursor)

  set -l cmd "$commandName --complete=fish \"$args\""
  #echo $cmd > file
  set -l completions (eval $cmd)
  #echo "NEW" >> file
  for completion in $completions
    echo $completion
    #echo $completion >> file
  end
end

complete -c gwhisper -e
complete -f -c gwhisper -a '(_gwhisper_getCompletions)'
