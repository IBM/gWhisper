#!/bin/sh

echo Creating Server Cert-Key-Pair:
openssl req -newkey rsa:4096 \
            -x509 \
            -sha256 \
            -days 3650 \
            -nodes \
            -out cert-key-pairs/serverCert.crt \
            -keyout cert-key-pairs/serverPrivateKey.key

echo Creating Client Cert-Key-Pair:
openssl req -newkey rsa:4096 \
            -x509 \
            -sha256 \
            -days 3650 \
            -nodes \
            -out cert-key-pairs/clientCert.crt \
            -keyout cert-key-pairs/clientPrivateKey.key





