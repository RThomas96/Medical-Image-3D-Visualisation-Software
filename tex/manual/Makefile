.DEFAULT:
	@make pdfile

pdfile: main.tex components.tex howto.tex intro.tex pipeline.tex
	@xelatex -interaction=nonstopmode main.tex 2>&1 > /dev/null

clean:
	@echo "Cleaning up auxiliary files ..."
	rm -f main.pdf
	rm -f main.aux
	rm -f main.toc
