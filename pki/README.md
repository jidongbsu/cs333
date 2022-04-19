## Man-in-the-middle attack against PKI

Requirement: In this lab, you will first see how PKI defeats man-in-the-middle attack, but then if the CA is compromised - its private key is exposed, then attackers launching a man-in-the-middle attack can defeat PKI. The attacker's goal is, when the victim visits https://www.cnn.com, the victim will actually be taken to fakenews.com (not the REAL fakenews.com, but a website created by the attacker).

Setup: Two VMs: victim VM and attacker VM.
Assumptions: We assume there is a trusted CA called GoMommy. And GoMommy's private key and certificate are both listed on the course page (named ca.key, ca.crt). We also assume we have used this CA to certify a website called fakenews.com, the private key and certificate for this website is also listed on the course page (named fakenews.key and fakenews.pem)

0. on both the victim VM and the attacker VM: Download GoMommy's certificate ca.crt (from here: http://cs.boisestate.edu/~jxiao/cs333/info/pki/ca.crt), and then load it into the firefox browser:

Edit -> Preferences -> Privacy & Security -> Certificates -> View Certificates -> Import.

(if you don't see Import, use the view->zoom out option of your browser)

Note: select "Trust this CA to identify websites."

Note 2: we import this because we assume GoMommy is a trusted CA, and for trusted CA, its certificate is supposed to be pre-loaded in the browser.

1: on the attacker VM, attacker setting up fakenews.com.

Step 1.0: on the attacker VM: download fakenews.key and fakenews.pem into the home directory - i.e., /home/seed/ directory. (download fakenews.key from http://cs.boisestate.edu/~jxiao/cs333/info/pki/fakenews.key, and download fakenews.pem from http://cs.boisestate.edu/~jxiao/cs333/info/pki/fakenews.pem)

Step 1.1: we setup a website called fakenews.com on the attacker's VM.

$ sudo mkdir /var/www/fakenews

Step 1.1.1. we create the home page for fakenews.com. Inside /var/www/fakenews, we create a file called index.html, with the following content:

$ sudo vi index.html
<html>
<body>
	Welcome to fakenews.com! Every day we provide you with the latest and most authentic fake news!
</body>
</html>

Step 1.2: we then setup at virtual host so that we host fakenews.com via https. To achieve this, we add the following content at the end of this file: /etc/apache2/sites-available/000-default.conf.

<VirtualHost *:443>
ServerName fakenews.com
DocumentRoot /var/www/fakenews
DirectoryIndex index.html

SSLEngine On
SSLCertificateFile /home/seed/fakenews.pem
SSLCertificateKeyFile /home/seed/fakenews.key
</VirtualHost>

Step 1.3: run the following commands to configure and enable SSL.

$ sudo a2enmod ssl	// this command enables ssl, a2enmod means "apache2 enable module", the opposite is a2dismod, which means "apache2 disable module".
$ sudo a2ensite default-ssl	// this command a2ensite enables an apache site, i.e., a virtual host, which is specified in the above 000-default.conf file. The opposite command is called a2dissite.
$ sudo apachectl configtest	// this command apachectl checks apache configuration file for valid syntax.
$ sudo service apache2 restart // this command actually starts the apache web server.

Note the passphrase here is 1234.

At this moment, if you, still on the attacker's VM, add "127.0.0.1 fakenews.com" in /etc/hosts, and you type https://fakenews.com in the browser, you should be able to access the fakenews.com we just created.

Warning: if you don't see the "Welcome to fakenews.com!" page, then your website setup is not successful, don't need to move forward.

2. on the victim VM, we emulate the result of a DNS cache poisoning attack. So that www.cnn.com points to the attacker's VM. We achieve this by editing /etc/hosts so as to have the following entry:

ATTACKER_IP	www.cnn.com

Replace ATTACKER_IP with the attacker VM's IP address.

Step 2.1. we now type https://www.cnn.com in the browser and see if the man-in-the-middle attack is successful - if so, we should be visiting the attacker's fakenews.com.

Note: the attack here will not be successful, and you, as the victim client, are expected get a warning message saying "Your connection is not secure".

3. attacker stole the CA's privacy key.

Step 3.1. on the attacker VM, now we assume the attacker has compromised the CA and stole the CA's (i.e., GoMommy) private key ca.key. With this key, we, as an attacker, can sign any certificates in the name of GoMommy. Assume we, as the attacker, have created a private key for www.cnn.com, and have signed a certificate for www.cnn.com. The private key (named cnn.key) and the certificate (named cnn.pem) are here: http://cs.boisestate.edu/~jxiao/cs333/info/pki/cnn.key and http://cs.boisestate.edu/~jxiao/cs333/info/pki/cnn.pem. The attacker downloads these two files to its home directory, i.e., /home/seed.

Step 3.2. Now edit the file we mentioned in Step 1.2, but change the certificate and the key from fakenews to cnn. i.e.:

<VirtualHost *:443>
ServerName fakenews.com
DocumentRoot /var/www/fakenews
DirectoryIndex index.html

SSLEngine On
SSLCertificateFile /home/seed/cnn.pem
SSLCertificateKeyFile /home/seed/cnn.key
</VirtualHost>

Step 3.3. Run the following command to restart the apache web server:

# sudo service apache2 restart

Note: once again the passphrase here is 1234.

4. On the victim's VM, repeat step 2.1. Now the attack should be successful: the victim who types https://www.cnn.com should be redirected to the attacker's fakenews.com.

