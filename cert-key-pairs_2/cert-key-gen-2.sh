#!/bin/bash

# Generate valid CA
echo Generate CA key:
openssl genrsa -passout pass:1234 -des3 -out ca.key 4096
echo Generate CA cert:
openssl req -passin pass:1234 -new -x509 -days 365 -key ca.key -out ca.crt

# Generate valid Server Key/Cert
echo Generate Server key:
openssl genrsa -passout pass:1234 -des3 -out server_key.pem 4096
echo Generate Server signing request:
openssl req -passin pass:1234 -new -key server_key.pem -out server_csr.pem
echo Generate selg-signed Server cert:
openssl x509 -req -passin pass:1234 -days 365 -in server_csr.pem -CA ca.crt -CAkey ca.key -CAcreateserial -out server_crt.pem

# Remove passphrase from the Server Key
openssl rsa -passin pass:1234 -in server_key.pem -out server_key.pem

# Generate valid Client Key/Cert
echo Generate Client key:
openssl genrsa -passout pass:1234 -des3 -out client_key.pem 4096
echo Generate Client signing request:
openssl req -passin pass:1234 -new -key client_key.pem -out client_csr.pem
echo Generate self-signed Client cert:
openssl x509 -passin pass:1234 -req -days 365 -in client_csr.pem -CA ca.crt -CAkey ca.key -CAcreateserial -out client_crt.pem

# Remove passphrase from Client Key
openssl rsa -passin pass:1234 -in client_key.pem -out client_key.pem
