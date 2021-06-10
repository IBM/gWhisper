#!/bin/bash

# Generate key-cert-pair folder in build directory, if it doesn't already exist
# $1 is the CMAKE_CURRENT_BINARY_DIR Variable
echo RUN KEYCERTGEN
mkdir -p $1/cert-key-pair
FILE_PATH = $1/cert-key-pair/

# Generate valid CA
echo Generate CA key:
openssl genrsa -passout pass:1234 -des3 -out $1/cert-key-pair/ca.key 4096
echo Generate CA cert:
openssl req -passin pass:1234 -new -x509 -days 365 -key $1/cert-key-pair/ca.key -out $1/cert-key-pair/ca.crt -subj "/C=CA/ST=Baden-Wuerttemberg/L=Boeblingen/O=IBM/OU=Test/CN=localhost"

# Generate valid Server Key/Cert
echo Generate Server key:
openssl genrsa -passout pass:1234 -des3 -out $1/cert-key-pair/server_key.pem 4096
echo Generate Server signing request:
openssl req -passin pass:1234 -new -key $1/cert-key-pair/server_key.pem -out $1/cert-key-pair/server_csr.pem -subj "/C=DE/ST=Baden-Wuerttemberg/L=Boeblingen/O=IBM/OU=Server/CN=localhost"
echo Generate selg-signed Server cert:
openssl x509 -req -passin pass:1234 -days 365 -in $1/cert-key-pair/server_csr.pem -CA $1/cert-key-pair/ca.crt -CAkey $1/cert-key-pair/ca.key -CAcreateserial -out $1/cert-key-pair/server_crt.pem

# Remove passphrase from the Server Key
openssl rsa -passin pass:1234 -in $1/cert-key-pair/server_key.pem -out $1/cert-key-pair/server_key.pem

# Generate valid Client Key/Cert
echo Generate Client key:
openssl genrsa -passout pass:1234 -des3 -out $1/cert-key-pair/client_key.pem 4096
echo Generate Client signing request:
openssl req -passin pass:1234 -new -key $1/cert-key-pair/client_key.pem -out $1/cert-key-pair/client_csr.pem -subj "/C=DE/ST=Baden-Wuerttemberg/L=Boeblingen/O=IBM/OU=Client/CN=localhost"
echo Generate self-signed Client cert:
openssl x509 -passin pass:1234 -req -days 365 -in $1/cert-key-pair/client_csr.pem -CA $1/cert-key-pair/ca.crt -CAkey $1/cert-key-pair/ca.key -CAcreateserial -out $1/cert-key-pair/client_crt.pem

# Remove passphrase from Client Key
openssl rsa -passin pass:1234 -in $1/cert-key-pair/client_key.pem -out $1/cert-key-pair/client_key.pem
