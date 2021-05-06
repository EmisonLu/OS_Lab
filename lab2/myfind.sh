# Define a function, recursively find each subdirectory
deep_find() {
    if test -d $1; then  # If $1 is a directory
        (
            cd $1
            if [ $? -eq 1 ]; then  # If this directory cannot be entered
                return
            fi

            for j in *; do  # Traverse every file in this directory
                deep_find $j $2  # Call itself recursively
            done
        )
    else  # If $1 is not a directory
        case $1 in     
            *.c | *.h)  # If $1 is a file ending in .c or .h
                res=`grep -wn $2 $1`
                if [ $? -eq 0 ]; then  # If this file exists pattern
                    echo "`pwd`/$1:"  # Print the absolute path of the file
                    grep -wn $2 $1  # Search in the file and return results
                    echo
                fi
                ;;
        esac
    fi
}

if [ $# -eq 0 ]; then  # If no parameters are passed in
    echo "Please enter parameter!"
    exit
elif [ $# -eq 1 ]; then  # If only one parameter is passed in
    path=.
    pattern=$1
else
    path=$1
    pattern=$2
fi

cd $path  # Enter the path directory
if [ $? -eq 1 ]; then  # If the path entered is wrong
    echo "Path wrong!"
    exit
fi

deep_find . $pattern  # Call the function, perform recursive query
