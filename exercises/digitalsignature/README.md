## Generate and Verify a Digital Signature

This is a follow-up exercise of the rsa exercise, i.e., we assume the private/public key pair has already been generated.

### Setup

The provided Linux VM.

### Steps

1. Create a message called msg.txt

```console
# echo "I owe Jidong $2000" > msg.txt
```

2. Generate a 256 bit hash from the message.

```console
# openssl sha256 -binary msg.txt > msg.sha256
```

Note: the hash is now saved in this file msg.sha256. Use xxd to view the hash, e.g.,

```console
# xxd msg.sha256 
0000000: d9b4 a474 93b2 077d c264 0e72 9464 4ea2  ...t...}.d.r.dN.
0000010: ee9c 9070 6466 117d 534b ec44 7250 0d0b  ...pdf.}SK.DrP..
```

3. Sign the hash using the private key, the result is the digital signature.

```console
# openssl rsautl -sign -inkey private.pem -in msg.sha256 -out msg.sig
```

4. Verify the signature, using the public key.

```console
# openssl rsautl -verify -inkey public.pem -pubin -in msg.sig -raw | xxd
0000000: 0001 ffff ffff ffff ffff ffff ffff ffff  ................
0000010: ffff ffff ffff ffff ffff ffff ffff ffff  ................
0000020: ffff ffff ffff ffff ffff ffff ffff ffff  ................
0000030: ffff ffff ffff ffff ffff ffff ffff ffff  ................
0000040: ffff ffff ffff ffff ffff ffff ffff ffff  ................
0000050: ffff ffff ffff ffff ffff ffff ffff ff00  ................
0000060: d9b4 a474 93b2 077d c264 0e72 9464 4ea2  ...t...}.d.r.dN.
0000070: ee9c 9070 6466 117d 534b ec44 7250 0d0b  ...pdf.}SK.DrP..
```

As we can see, we can derive the hash value from the signature.
