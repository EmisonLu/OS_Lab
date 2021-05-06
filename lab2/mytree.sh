count=""  # Indented spaces for output
re_tree() {
    if test -d $1; then  # If $1 is a directory
        (
            count="$count    "  # Indent the output plus four spaces

            cd $1
            if [ $? -eq 1 ]; then  # If this directory cannot be entered
                return
            fi

            for j in *; do
                if [ -f $j -o -d $j ]; then  # If $j is a file or directory
                    echo "${count}$j"
                fi
                re_tree $j  # Call itself recursively
            done
        )
    fi
}

# Handle the current directory
for i in *; do
    if test -d $i; then  # If $i is a directory, print and call the function
        echo $i
        re_tree $i
    else  # If $i is a file, print
        echo $i
    fi
done
