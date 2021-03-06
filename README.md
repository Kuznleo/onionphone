OnionPhone
==========

VOIP tool for calling over Tor v0.2a (not compatible with IKE in v0.1a, see docs)

OnionPhone (OPH) is a VOIP tool for calling over Tor network which can be used as a VOIP plugin for TorChat. Call is targeted to the onion address of the recipient (its hidden service HS). The recipient can install a reverse onion connection to the originator’s HS and use a faster channel periodically resetting the slower channel to reduce overall latency. 
Also provided is the ability to switch to a direct UDP connection (with NAT traversal) after the connection is established over Tor (Tor network opposed to a SIP server requiring registration and collecting metadata). And also OnionPhone can establish a direct UDP or TCP connection to the specified port on IP-address or host. 
OPH uses a custom protocol (not RTP) with only one byte unencrypted header that can provide some obfuscation against a DPI. 

OPH provides independent level of p2p encryption and authentication which employs modern cryptographic primitives: Diffe-Hellmann key exchange on Elliptic Curve 25519 and Keccak Sponge Duplexing encryption. In the case of a call to the onion address Tor protects against MitM attacks. Also the recipient can verify identity of the originator’s onion address (only with the permission of the sender) similarly to the TorChat authentication. Otherwise possible mways of ulti-factor authentication are: 

• voice (biometrics);

• using previously shared password (with the possibility of hidden notification of enforcement); 

• using long-term public keys signed with PGP. 

OPH provides Perfect Forward Secrecy (uses a fresh key for each call) and Full Deniability for using long-term public keys of participants (as a fact and a content of the conversation): like a deniable SKEME protocol uses for initial key exchange. 

OPH can use a wide range of in-build voice codec (C source is included) from ultra low-bitrates up to high quality. The full list is: MELPE-1200, MELP-2400, CODEC2-1300, CODEC2-3200, LPC10-2400, CELP4800, AMR-4750/12200+DTX,LPC-5600+VOCODER,G723- 6400, G729-8000, GSM-HR-5600, GSM-FR-13200, GSM-EFR-12400, ILBC-13333, BV16-16000, OPUS-6000VBR, SILK-10000VBR, SPEEX-15200VBR+R. Some of them are free, some require a license (check regional laws). Implemented noise suppressors (NPP7 and SPEEX) of environment sounds and automatic mic gain control. Built-in LPC vocoder with the ability of irreversible change of voice (robot, whisper etc.). Specially designed dynamic adaptive buffer useful for smart compensation of jitter in high latency Onion environment is implemented. Radio-mode (Push-to-Talk) and voice control mode (Voice Active Detector) with generation a short signal when transmission is completed are available.

OPH:

• is a command line application which does not require installation and can be run from removable disk or True-Crypt container; 

• is fully open source, developed in pure C at the possible lowest level and carefully commented;

• statically linked, does not require additional third-party libraries and uses a minimum of system functions;

• can be compiled under Linux OS (Debian, Ubuntu etc.) using GCC or under Win32 OS (from Windows 98 up to 8) using MinGW. 

Compilation: 

Source code designed for compilation in 32-bits mode. On Linux you need ALSA headers for this to work; on Ubuntu you can install them with the following command: 'sudo apt-get install libasound2-dev'. 
For compile the source code use 'make'. Executable binaries 'oph' and 'addkey' (or with '.exe' extension for Win32  using MinGW) will be created. Integrity of cryptography library can be checked running 'test' (or 'test.exe')  in '/cr'’ subfolder.  
To clear the source use 'make clean'.
For normal functionality the OnionPhone folder must contain created binaries, default configuration files ('conf.txt', 'menu.tx' and 'audiocfg') and '/key' subfolder with default files ('contacts.txt', 'guest.sec' and 'guest').

Installation: 

the easiest way to use OnionOhine as a VOIP plugin for TorChat : 

• Step 1: Put the OnionPhone folder on the hard disk, removable media or TrueCrypt container (preferred).

• Step 2: Edit the TorChat configuration file '/torchat/bin/Tor/torrc.txt': immediately following the line

	HiddenServicePort 11009 127.0.0.1:11009
	
add the a new line

	HiddenServicePort 17447 127.0.0.1:17447
	
then run the TorChat. 

• Step 3: Right click on 'myself' icon of TorChat contact list and copy the ID to clipboard. Edit the OnionPhone configuration file 'conf.txt': specify our onion address using copied ID, for example: 
	Our_onion=gegelcy5fw7dsnsn 
Once OnionPhone is started it is ready to receive incoming and make outgoing calls. 

Usage:

• To accept an incoming call press Enter. 

• To make an outgoing call as a guest to guest (without using of personal public keys) type command: 	-Oremote_onion_address
  and press Enter then wait 10-30 sec for connecting over Tor. 
  
• To toggle continious voice transmission press Enter. Hold down / release Tab for Push-to-Talk mode. 

• To apply the voice codec from 1 to 18 use the command -Ccodec number. Smaller numbers correspond to lower bitrate codecs, greater ones correspond to higher quality. Numbers from 16 to 18 correspond to the variable bit rate codec. 

• To enable security vocoder use the command -Qmode, where mode=3 correspond "whisper" voice (preferred), modes 6-255 correspond "robot" etc). For deactivation of vocoder use the command -Q-3.

• To use the chat feature type a message and send it by pressing Enter.

• To switch to direct UDP connection use the command -S (both parties must do this).

• To return into Tor from direct UDP connection use the command -O.

• To end the call use the command -H. 

• To exit the OnioPhone use the command -X or click Esc twice for emergence exiting.

Alternatively use Up, Down, left and Right arrows to navigate in menu and apply frequently used commands quickly.
  
Detailed description of OninPhone modes, commands, cryptography etc. can be found inside the document 'oph.pdf':

http://torfone.org/download/oph.pdf

Project homepage: http://torfone.org/onionphone/

Also you can always ask the author directly (Van Gegel): torfone@ukr.net or gegelcopy@ukr.net

My TorChat ID: gegelcy5fw7dsnsn.

My PGP key available at: 

https://pgp.mit.edu/pks/lookup?search=gegelcopy%40ukr.net&op=index

http://pool.sks-keyservers.net:11371/pks/lookup?search=0xB5FA66FFEFDE1F49&fingerprint=on&hash=on&op=vindex

https://www.pgpru.com/proekt/poljzovateli?profile=gegel&getkey=8C23C721D54B3AF21EF119FBB5FA66FFEFDE1F49

Note: Alpha version is presented "as is" and has not yet been checked by anyone except the developer. Do not use in "life or death" cases.
