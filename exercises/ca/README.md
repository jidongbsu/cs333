## Sign a certificate (for fakenews.com) as a CA

### Setup

The provided Linux VM.

### Preparation

- Download these 3 files:

```console
# wget http://cs.boisestate.edu/~jxiao/cs333/info/pki/ca.crt
# wget http://cs.boisestate.edu/~jxiao/cs333/info/pki/ca.key
# wget http://cs.boisestate.edu/~jxiao/cs333/info/pki/openssl.cnf
```

- Create a folder called demoCA, and then enter into demoCA, and then create 3 empty folders: crl, certs, and newcerts. After that, create an empty file called index.txt. After that create a file called serial, write a number called "1000" into this file.

```console
[04/18/22]seed@VM:~/pki/demoCA$ ls
certs  crl  index.txt  newcerts  serial
```

You can create the above folders and files with the following commands:

```console
# mkdir demoCA
# cd demoCA
# mkdir certs
# mkdir crl
# mkdir newcerts
# touch index.txt
# echo "1000" > serial
# cd ..
```

### Sign the Certificate.

1. generate public/private key pair:

```console
# openssl genrsa -aes128 -out fakenews.key 1024
```

Note: you need to provide a pass phrase here, at least 4 characters.

2. generate a certificate signing request (csr):

```console
# openssl req -new -key fakenews.key -out fakenews.csr -config openssl.cnf
```

When asked, provide inputs like the following:

```console
[04/18/22]seed@VM:~/pki$ openssl req -new -key fakenews.key -out fakenews.csr -config openssl.cnf
Enter pass phrase for fakenews.key:
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:US
State or Province Name (full name) [Some-State]:Idaho
Locality Name (eg, city) []:Boise
Organization Name (eg, company) [Internet Widgits Pty Ltd]:GoMommy
Organizational Unit Name (eg, section) []:GoMommy
Common Name (e.g. server FQDN or YOUR name) []:fakenews.com
Email Address []:admin@fakenews.com

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:1234
An optional company name []:fakenews.com
```

3. generate the certificate:

```console
# openssl ca -in fakenews.csr -out fakenews.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
```

4. combine the key and the certificate into one pem file:

```console
# cp fakenews.key fakenews.pem
# cat fakenews.crt >> fakenews.pem
```

Now fakenews.pem will be the certificate, and fakenews.key will be the private key. With these two files, one can setup an HTTPS website, for fakenews.com. Refer to the lab instructions to see how to setup such a website.

===

### Extra Notes

When the above website is setup and if you don't have graphic browser, you can access the website from command line like this:

```console
# curl --cacert ca.crt https://fakenews.com
<html>
<body>
	Welcome to fakenews.com! Every day we provide you with the latest and most authentic fake news!
</body>
</html>
```
