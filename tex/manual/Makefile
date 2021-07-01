.DEFAULT:
	@make pdfile

pdfile: main.tex components.tex howto.tex intro.tex pipeline.tex
	@echo "Making PDF file ..."
	@xelatex -interaction=nonstopmode main.tex
	@echo "Done."

clean:
	@echo "Cleaning up auxiliary files ..."
	rm -f main.pdf
	rm -f main.aux
	rm -f main.toc
