cc=gcc
flags=-Wall -Wextra

default:
	$(cc) main.c -o ml $(flags)

tex:
	pdflatex example.tex

clean:
	rm -rf ml *.tex *.pdf *.log *.aux
