# Retropie Wiki IRC Bot
This bot is created from the irc bot I have already created in perl, simply
transformed and updated in C for speed, readability, and portability
(statically linked binaries are a great thing).

# Compiling
There are multiple ways to compile the bot.

1. Running `make` will compile the code and leave an executable called brimbot.
2. Running `make clean` will remove the executable and only leave the code files.

# How to Use
The program logs the messages, joins and leaves to filenames in the format
`nick.log.txt` or `#channel.log.txt`. It adds the current date and time to
every line. It allows configuration with the config.txt file.

# The Config File
The program can be configured by editing the config.txt file. The file is in the `key = value` format. The settings are:

1. **server**: Used to set the server ip to connect to.
2. **port**: Used to set the port of the server. The most commonly used irc
   port for unencrypted traffic is 6667.
3. **nick**: Used to set the nick of the bot.
4. **channels**: Used to set the channel list for the bot, comma seperated.

Example Config File

    server = 71.11.84.232
    port = 6667
    nick = BrimBot
    channels = ##pentest

## More Information
More information can be found [here](https://tools.ietf.org/html/rfc1459#section-4.1.6).
