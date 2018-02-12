#!/usr/bin/env bash

## reference: https://github.com/mascii/WiFiClientSecure-IFTTT/blob/master/convcert.sh

cat <(echo "const char* infura_ca_cert = \\") <(cat $1 | tr -d "\r" | sed -e "s/^/\"/" -e "s/$/\\\n\" \\\/") <(echo ";") > certificate.h

exit 0