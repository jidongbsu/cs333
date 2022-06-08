## Use the RSA algorithm to encrypt and then decrypt a message.

### Setup

The provided VM.

### Steps

1. Generate a 1024-bit private/public key pair.

```console
# openssl genrsa -aes128 -out private.pem 1024
```

**Note**: pem stands for Privacy Enhanced Mail, a file format. Not human readable, try "more private.pem" to see the file.

**Note 2**: you need to provide a pass phrase here, at least 4 characters.

2. Extract the public key component from the pem file.

```console
# openssl rsa -in private.pem -pubout > public.pem
```

3. Create a message.

```console
# echo "Hack me if you can." > msg.txt
```

4. Encrypt the message, using the public key.

```console
# openssl rsautl -encrypt -inkey public.pem -pubin -in msg.txt -out msg.enc
```

**Note**: run "more msg.enc" to view the encrypted message.

5. Decrypt the message, using the private key.

```console
# openssl rsautl -decrypt -inkey private.pem -in msg.enc
```
