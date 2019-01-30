# unix-todo
A simple and scriptable to-do list in your terminal written in C

![Screenshot](screenshot.png?)

### Installation

You have to clone the repository (or download a release) :

	cd ~
	git clone https://github.com/geoffroymontane/unix-todo

Then, compile by using gcc :

	gcc main.c -lreadline -o uxtodo

If this command failed, just check if gcc and GNU Readline are fully installed on your system and install them otherwise.

Move the executable to a correct directory intended to store executables, such as :

	sudo mv uxtodo /bin

Then, all should be fine. Just enter :

	uxtodo help


### License

[GPLv3](LICENSE)
