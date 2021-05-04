#!/bin/bash

unamestr=$(uname)
if [[ "$unamestr" == 'Darwin' ]]; then
	format_cmd="clang-format -i -style=file '{}'"
elif [[ "$unamestr" == 'Linux' ]]; then
	format_cmd="clang-format -i -style=file '{}'"
fi


if [ "${1}" = "all" ]; then
    find src -iname "*.cpp" -o -iname "*.hpp" | xargs -I{} sh -c "${format_cmd}"
elif [ "$1" = "modified" ]; then
    # Run on all changed as well as untracked cpp/hpp files, as compared to the current HEAD. Skip deleted files.
    { git diff --diff-filter=d --name-only & git ls-files --others --exclude-standard; } | grep -E "^src.*\.[ch]pp$" | xargs -I{} sh -c "${format_cmd}"
elif [ "$1" = "staged" ]; then
    # Run on all files that are staged to be committed.
    git diff --diff-filter=d --cached --name-only | grep -E "^src.*\.[chi]pp$" | xargs -I{} sh -c "${format_cmd}"
else
    # Run on all changed as well as untracked cpp/hpp files, as compared to the current main. Skip deleted files.
    { git diff --diff-filter=d --name-only main & git ls-files --others --exclude-standard; } | grep -E "^src.*\.[ch]pp$" | xargs -I{} sh -c "${format_cmd}"
fi
