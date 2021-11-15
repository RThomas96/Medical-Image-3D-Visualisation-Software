.DEFAULT:
	@make pdfile

pdfile: main.tex components.tex howto.tex intro.tex pipeline.tex
	@xelatex -interaction=nonstopmode main.tex 2>&1 > /dev/null

full_refs: main.tex components.tex howto.tex intro.tex pipeline.tex
	# Run it thrice to be sure to catch all references :
	@xelatex -interaction=nonstopmode main.tex 2>&1 > /dev/null
	@xelatex -interaction=nonstopmode main.tex 2>&1 > /dev/null
	@xelatex -interaction=nonstopmode main.tex 2>&1 > /dev/null

clean:
	@echo "Cleaning up auxiliary files ..."
	rm -f main.pdf
	rm -f main.aux
	rm -f main.toc
