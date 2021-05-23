#!/bin/bash

#Made by Ron Even

VERSION=1.1

TMP_FILE="./.tmpfile"

DEBUG=0
if [ "$1" == "-d" ]; then
        DEBUG=1
fi

COUNTER=0
FAILED_TESTS=""

print_single_invalid_output()
{
    echo -e "$1\n"
    if [ $DEBUG -eq 1 ]; then
            echo -e "Expected as hex:"
            echo -e "$1" | hexdump -C
            printf "\n"
    fi
}

print_actual_output()
{
    echo -e "Got:\n$1\n"
    if [ $DEBUG -eq 1 ]; then
            echo -e "Got as hex:"
            echo -e "$1" | hexdump -C
            echo -e "\n"
    fi
}

print_error_header_for_command()
{
    echo -e "\t\t❌ FAILED"
    echo -e "$1\n"
}

print_success_header()
{
    echo -e "\t\t✅ PASSED"
}

test_prolog() {
    COUNTER=$((COUNTER + 1))
    echo "*****************************************"
    printf "$COUNTER"
}

mark_current_test_as_failed() {
    FAILED_TESTS+="$COUNTER,"
}

validate_ex31() {
        command_file="$1"
        text1="$2"
        text2="$3"

        test_prolog

        files_dir=tests_ex31/"$COUNTER"
        mkdir -p "$files_dir"
        printf "$text1" > "$files_dir"/1.txt
        printf "$text2" > "$files_dir"/2.txt

        command="$command_file $files_dir/1.txt $files_dir/2.txt"

        eval "$command" > $TMP_FILE 2>&1
        return_code=$?
        result=$(cat $TMP_FILE; echo $return_code)
        rm -f $TMP_FILE

        # Ignore first arguments
        shift
        shift
        shift

        matched=0
        general_error=0

        for arg in "$@"
        do
            expected=`printf "$arg"`

            if [ "$result" == "$expected" ]
            then
                    print_success_header
                    matched=1
            fi
        done

        # If we reached here and matched=0 it means no matches were found. Print details
        if [ $matched -eq 0 -a $general_error -ne 1 ]
        then
            print_error_header_for_command "$command"
            print_actual_output "$result"

            if [ $# -gt 1 ]
            then
                echo -e "Expected (one of the $# following outputs):\n"
                for arg in "$@"
                do
                    expected=`printf "%s" "$arg"`

                    print_single_invalid_output "$expected"

                    if [ "$arg" != "${@: -1}" ]
                    then
                        echo -e "----- OR -----\n"
                    fi
                done
            else
                echo -e "Expected:\n"
                print_single_invalid_output "$expected"
            fi

            mark_current_test_as_failed
        fi
}

compare_single_ex32_output() {
    output_name=$1
    should_sort=$2
    actual_results="$(cat < "$output_name" | sed 's/\xe2\x80\x98/\x27/g' | sed 's/\xe2\x80\x99/\x27/g')"
    expected_results="$(cat < expected/"$output_name")"

    if [ "$should_sort" -eq 1 ]
    then
        actual_results="$(printf "$actual_results" | sort)"
        expected_results="$(printf "$expected_results" | sort)"
    fi

    if [ "$actual_results" == "$expected_results" ]
    then
        return 0
    else
        print_error_header_for_command
        echo "Invalid result for $output_name"
        print_actual_output "$actual_results"
        echo -e "Expected:\n"
        print_single_invalid_output "$expected_results"
        return 1
    fi

}

validate_ex32() {
    test_dir=$1
    test_config=$2
    expected_return_code=0

    cwd="$(pwd)"

    ln -sf "$cwd"/a.out "$cwd/$test_dir/a.out"
    ln -sf "$cwd"/comp.out "$cwd/$test_dir/comp.out"

    test_prolog
    cd "$test_dir"

    rm -f errors.txt results.csv

    ./a.out "$test_config"
    return_code=$?
    if [ "$return_code" -ne 0 ]
    then
        print_error_header_for_command
        echo "ex32 returned $return_code instead of $expected_return_code!"
        echo "Maybe it crashed?"
        mark_current_test_as_failed
    fi

    # Start comparing the results
    failed_count=0
    compare_single_ex32_output results.csv 1
    failed_count=$((failed_count+$?))
    compare_single_ex32_output errors.txt 0
    failed_count=$((failed_count+$?))

    if [ "$failed_count" -eq 0 ]
    then
        print_success_header
    else
        mark_current_test_as_failed
    fi

    cd "$cwd"
}

validate_test_files() {
    REQUIRED_FILES="tests.sh comp.out a.out"
    NOT_FOUND_REQUIRED_FILES=0
    for required in $REQUIRED_FILES
    do
        found=$(ls | grep -wc "$required")
        if [ $found -eq 0 ]
        then
            echo "❌ couldn't find $required"
            NOT_FOUND_REQUIRED_FILES=$[NOT_FOUND_REQUIRED_FILES + 1]
        fi
    done

    if [ $NOT_FOUND_REQUIRED_FILES -ne 0 ]
    then
        echo "$NOT_FOUND_REQUIRED_FILES files/directories couldn't be found in the tests directory (`pwd`)"
        echo "Please make sure they exist and run tests.sh again"
        exit 1
    fi
}

declare_step() {
    echo "Testing $1..."
}

declare_stage() {
    echo "           Testing $1             "
}

echo "           Compiling files             "
echo "*****************************************"
echo ""
rm -f a.out comp.out
gcc ex31.c -o comp.out
gcc ex32.c

validate_test_files

SCRIPT="./comp.out"
TEST_DIR="tests_ex31"
rm -rf "$TEST_DIR"
declare_stage "$SCRIPT"
if [ -f "./$SCRIPT" ]
then
        declare_step "Trivial identical"
        validate_ex31 "$SCRIPT" "12ab23" "12ab23" "1"
        validate_ex31 "$SCRIPT" "12AB23" "12AB23" "1"

        declare_step "Identical with whitespaces similarities"
        validate_ex31 "$SCRIPT" "12     ab23" "12   ab23" "3"
        validate_ex31 "$SCRIPT" "12   ab23" "12     ab23" "3"
        validate_ex31 "$SCRIPT" "12ab23" "12     ab23" "3"
        validate_ex31 "$SCRIPT" "12     ab23" "12ab23" "3"
        validate_ex31 "$SCRIPT" "12  ab  23" "12  ab23  " "3"
        validate_ex31 "$SCRIPT" "12  ab  23       " "12  ab23  " "3"
        validate_ex31 "$SCRIPT" "12  ab23  " "12  ab  23       " "3"

        declare_step "Identical with newline similarities"
        validate_ex31 "$SCRIPT" "12\n\n\n\n\nab23" "12\n\n\nab23" "3"
        validate_ex31 "$SCRIPT" "12\n\n\nab23" "12\n\n\n\n\nab23" "3"
        validate_ex31 "$SCRIPT" "12ab23" "12\n\n\n\n\nab23" "3"
        validate_ex31 "$SCRIPT" "12\n\n\n\n\nab23" "12ab23" "3"
        validate_ex31 "$SCRIPT" "12\n\nab\n\n23" "12\n\nab23\n\n" "3"

        declare_step "Trivial similar"
        validate_ex31 "$SCRIPT" "Hello" "HellO" "3"
        validate_ex31 "$SCRIPT" "HeLlo" "HellO" "3"
        validate_ex31 "$SCRIPT" "HEllo" "Hello" "3"
        validate_ex31 "$SCRIPT" "HEllO" "Hello" "3"

        declare_step "Similar with whitespaces"
        validate_ex31 "$SCRIPT" "Hello  " "He  llO" "3"
        validate_ex31 "$SCRIPT" "    HeL lo" "Hel lO   " "3"
        validate_ex31 "$SCRIPT" "HEll        o" "Hello" "3"
        validate_ex31 "$SCRIPT" "HEll O     " "Hell   o   " "3"

        declare_step "Similar with whitespaces"
        validate_ex31 "$SCRIPT" "Hello\n\n" "He\n\nllO" "3"
        validate_ex31 "$SCRIPT" "\n\n\n\nHeL\nlo" "Hel\nlO\n\n\n" "3"
        validate_ex31 "$SCRIPT" "HEll\n\n\n\n\n\n\n\no" "Hello" "3"
        validate_ex31 "$SCRIPT" "HEll\nO\n\n\n\n\n" "Hell\n\n\no\n\n\n" "3"

        declare_step "Trivial different"
        validate_ex31 "$SCRIPT" "No silly questions please" "N0 silly questions please" "2"
        validate_ex31 "$SCRIPT" "No kitbag questions please" "No silly questions please" "2"
        validate_ex31 "$SCRIPT" "No silly questions pleas" "No silly questions please" "2"
        validate_ex31 "$SCRIPT" "No silly questions please" "No silly questions pleas" "2"

        declare_step "Different with whitespaces"
        validate_ex31 "$SCRIPT" "No     silly questions please" "N0 sil   ly quest   ions ple   ase" "2"
        validate_ex31 "$SCRIPT" "No   kitbag questions please" "No   silly questions please" "2"
        validate_ex31 "$SCRIPT" "No   silly questions please       p l" "No   silly questions please p             L e" "2"
        validate_ex31 "$SCRIPT" "No   silly questions please       p l    e" "No   silly questions please p             l " "2"

        declare_step "Different with newlines"
        validate_ex31 "$SCRIPT" "No\n\n\n\n\nsilly\nquestions\nplease" "N0\nsil\n\n\nly\nquest\n\n\nions\nple\n\n\nase" "2"
        validate_ex31 "$SCRIPT" "No\n\n\nkitbag\nquestions\nplease" "No\n\n\nsilly\nquestions\nplease" "2"
        validate_ex31 "$SCRIPT" "No\n\n\nsilly\nquestions\nplease\n\n\n\n\n\n\np\nl" "No\n\n\nsilly\nquestions\nplease\np\n\n\n\n\n\n\n\n\n\n\n\n\nL\ne" "2"
        validate_ex31 "$SCRIPT" "No\n\n\nsilly\nquestions\nplease\n\n\n\n\n\n\np\nl\n\n\n\ne" "No\n\n\nsilly\nquestions\nplease\np\n\n\n\n\n\n\n\n\n\n\n\n\nl\n" "2"

        declare_step "Harder cases"
        validate_ex31 "$SCRIPT" "This\n\nis    a tougher case    \n\n\n 1\n \n \n  2" "This  is \n a    tougher case \n   \n 1  2" "3"
        validate_ex31 "$SCRIPT" "This\n\nis    a touGher case    \n\n\n 1" "This  is \n a    tougheR case \n   \n 1" "3"
        validate_ex31 "$SCRIPT" "Hard \n\n\n case    this  \n one    y   o D\nA  \n\n" "Hard case this one    Y   O D\nA  \n\n Said  " "2"
        validate_ex31 "$SCRIPT" "Hard case this one    Y   O D\nA  \n\n Said  " "Hard \n\n\n case    this  \n one    y   o D\nA  \n\n" "2"

        declare_step "Tests from exercise description"
        validate_ex31 "$SCRIPT" "12ab23" "12Ab23" "3"
        validate_ex31 "$SCRIPT" "12aB23" "12AB23" "3"
        validate_ex31 "$SCRIPT" "12AB23" "12 aB 23" "3"
        validate_ex31 "$SCRIPT" "12" "3" "2"

        declare_step "Tests from exercise files (different)"
        validate_ex31 "$SCRIPT" "We were not on a break!\n" "We were on a break!\n" "2"
        validate_ex31 "$SCRIPT" "Je\nm'ap\npelle\nLipupu!\n" "Je\nm'ap\npelle\nJe m'appelle!\n" "2"
        validate_ex31 "$SCRIPT" "Russ\n" "Ross\n" "2"

        declare_step "Tests from exercise files (identical)"
        validate_ex31 "$SCRIPT" "Could these be any more identical?\n" "Could these be any more identical?\n" "1"
        validate_ex31 "$SCRIPT" "Pivot! Pivot! Pivot!\n" "Pivot! Pivot! Pivot!\n" "1"

        declare_step "Tests from exercise files (similar)"
        validate_ex31 "$SCRIPT" "This is all a moo point.\n" "This is          all a MOO \n\n\n\npoint.\n" "3"
        validate_ex31 "$SCRIPT" "Joey doesn't share food!\n" "Joey DoeSn't\n shaRe      food!\n" "3"
else
        echo "❌ Couldn't find $SCRIPT file!"
        echo "Make sure the tests script is in the same directory as $SCRIPT"
fi

SCRIPT="./a.out"
TEST_DIR="tests_ex32"
declare_stage "$SCRIPT"
if [ -f "./$SCRIPT" ]
then
        declare_step "Tests from exercise description"
        validate_ex32 "$TEST_DIR/moodle_test" "conf.txt"

        declare_step "Tough cases"
        validate_ex32 "$TEST_DIR/tough_test" "conf.txt"

        declare_step "Messed up paths cases"
        validate_ex32 "$TEST_DIR/messed_up_paths_test" "configuration.txt"
else
        echo "❌ Couldn't find $SCRIPT file!"
        echo "Make sure the tests script is in the same directory as $SCRIPT"
fi



echo ""
echo "************ 📝  SUMMARY  📝 ************"
failed_count=$(echo $FAILED_TESTS | tr -cd ',' | wc -c)
success_count=$((COUNTER-failed_count))
echo "           $success_count/$COUNTER tests passed!"
if [ $success_count -ne $COUNTER ]
then
    echo "Failed tests are:"
    FAILED_TESTS=${FAILED_TESTS%?}
    echo $FAILED_TESTS
fi

echo "*****************************************"


echo ""
echo "*****************************************"
echo "**  This script was made by Ron Even   **"
echo "**         script version: $VERSION         **"
echo "*****************************************"