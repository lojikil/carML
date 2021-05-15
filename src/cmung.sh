if [ "$1" != "" ]
then
    sed -e 's/\([a-z]\)\-\([a-z]\)/\1_\2/g' -e 's/\?/_p/g' $1
fi
