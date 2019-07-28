function getCompletions;
  set commandLineUntilCursor (commandline -c)
  set commandLineTokenized (commandline -o)
  set commandName (commandline -o)[1]

  set args (string sub --start (math (string length $commandName) + 1) $commandLineUntilCursor)

  set completions ($commandName --complete=fish $args)
  #set completions (cat test)
  echo "NEW" > file
  for completion in $completions
    echo $completion
    echo $completion >> file
  end
end

complete -c gwhisper -e
complete -f -c gwhisper -a '(getCompletions)'
