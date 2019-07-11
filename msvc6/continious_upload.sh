set +x

##################################
#
#        GitHub Releases
#
##################################

curl -o upload.sh "https://raw.githubusercontent.com/FWGS/uploadtool/master/upload.sh"

export GITHUB_TOKEN=$GH_TOKEN
sh upload.sh $*
