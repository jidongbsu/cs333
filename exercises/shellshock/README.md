## Getting a Reverse Shell via Shellshock Attack

### Setup

The provided Linux VM.

### Preparation

1. copy the cgi script (myprog.cgi) into /usr/lib/cgi-bin.

```console
[05/16/22]seed@VM:~$ ls /usr/lib/cgi-bin/
[05/16/22]seed@VM:~$ sudo cp myprog.cgi /usr/lib/cgi-bin/
[05/16/22]seed@VM:~$ ls /usr/lib/cgi-bin/
myprog.cgi
```

2. go to the /usr/lib/cgi-bin folder and change the cgi script's permission to 755.

```console
[05/16/22]seed@VM:~$ cd /usr/lib/cgi-bin/
[05/16/22]seed@VM:.../cgi-bin$ ls -l
total 136
-rw-r--r-- 1 root root 135992 May 16 10:17 myprog.cgi
[05/16/22]seed@VM:.../cgi-bin$ sudo chmod 755 myprog.cgi
[05/16/22]seed@VM:.../cgi-bin$ ls -l
total 136
-rwxr-xr-x 1 root root 135992 May 16 10:17 myprog.cgi
```

### Launch the Attack

