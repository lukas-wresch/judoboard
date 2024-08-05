openssl genrsa -out private.key 2048
openssl req -new -x509 -key private.key -sha256 -out certificate.pem -days 730
