#!/bin/bash
# Tring to create rtcnofig.h from .config
HEADER_STR=XS_CONFIG_H__


function is_pkg_special_config()
{
    echo -ne $1 | egrep '^PKG_|_PATH$|_VER$' >/dev/null 2>/dev/null
}

function is_config()
{
    echo -ne $1 | egrep '^XS_' >/dev/null 2>/dev/null
}

function make_config_h()
{
    local CONFIG_NAME=${1}

    # destination file using file descriptor 8
    exec 8>${2}

    echo -ne "#ifndef ${HEADER_STR}\n" >&8
    echo -ne "#define ${HEADER_STR}\n\n" >&8

    EMPTY_LINE='true'

    while read LN
    do
        LINE=`echo $LN | sed 's/[ \t\r\n]*$//g'`

        if [ -z "$LINE" ]; then
            continue
        fi

        if [ '#' = ${LINE:0:1} ]; then
            if [ ${#LINE} -eq 1 ]; then
                if $EMPTY_LINE; then
                    continue
                fi
                echo >&8
                EMPTY_LINE='true'
                continue
            fi

            if echo -ne "$LINE" | egrep '^# CONFIG_' >/dev/null 2>/dev/null; then
                LINE=`printf ' %s' ${LINE:9}`
            else
                LINE=${LINE:1}
                echo -ne "/* ${LINE} */\n" >&8
            fi

            EMPTY_LINE='false'
        else
            EMPTY_LINE='false'

            OLD_IFS="$IFS"
            IFS='='
            SETTINGS=($LINE)
            IFS="$OLD_IFS"

            if [ ${#SETTINGS[@]} -ge 2 ]; then
                if echo -ne "$SETTINGS[0]" | egrep '^CONFIG_' >/dev/null 2>/dev/null; then
                    SETTINGS[0]="${SETTINGS[0]:7}"
                fi

                if is_pkg_special_config "${SETTINGS[0]}"; then
                    continue
                fi

#                echo "DBG---: ${SETTINGS[@]}, ${SETTINGS[*]}"
                if [ "${SETTINGS[1]}" = 'y' ]; then
                    echo -ne "#define ${SETTINGS[0]}\n" >&8
                else
                    echo -ne "#define ${SETTINGS[0]} ${LINE#*=}\n" >&8
                fi
            fi
        fi

    done < $CONFIG_NAME

    if [ -f xsconfig_project.h ]; then
        echo -ne "#include \"xsconfig_project.h\"\n" >&8
    fi

    echo -ne "\n#endif\n" >&8
    exec 8<&-
}

make_config_h $1 $BSP_ROOT/xsconfig.h

