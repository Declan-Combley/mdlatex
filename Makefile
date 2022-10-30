cc=gcc
flags=-Wall -Wextra

default:
	$(cc) main.c -o ml $(flags)

clean:
	rm -rf ml *.tex *.log *.aux
