#!/bin/bash
#
for file in $*
do
    if test -r "$file"
    then
        base="${file##*/}"
        dir="${base%.png}"
        mkdir -p "out/$dir"
        out="out/$dir/$base"
        log="out/$dir/$dir"
        frame=-1
        while ./test-core "$file" "$frame" "$out" >"${log}.${frame}.log"
        do
            : echo "INFO: $file[$frame]: OK"
            frame=$((frame + 1))
        done
        if test $frame -ge 1
        then
            : echo "INFO: ${file}: $frame FRAMES"
        elif test $frame -ge 0
        then
            echo "INFO: ${file}[0]: NO fcTL"
        else
            echo "ERROR: ${file}[$frame] NOT FOUND"
            egrep 'ERROR' "${log}.${frame}.log"
        fi
    else
        echo "ERROR: $file: not found"
    fi
done
