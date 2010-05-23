#
#      Brno University of Technology
#      Faculty of Information Technology
# 
#      MSc Thesis	2009/2010
# 
#      Design of S-Boxes Using Genetic Algorithms
#
#
#      Bedrich Hovorka
#-----------------------------------------------------
#
LANG         = cs_CZ.iso-8859-2
LOCALE       = cs_CZ
DOC          = diplomka
TEX_CODES    = $(wildcard *.utf8.tex)
TEX_FILES    = $(TEX_CODES:.utf8.tex=)
PLOTS        = $(wildcard *.gp)
PLOTS_TEX  = $(PLOTS:.gp=.tex)
PLOTS_EPS  = $(PLOTS:.gp=.eps)
PLOTS_PDF  = $(PLOTS:.gp=.pdf)

###########################################

all:		$(DOC).pdf

rebuild:	clean all

###########################################

.SUFFIXES: .eps .pdf .ps .dvi .utf8.tex .tex

.eps.pdf:
	epstopdf $<

.dvi.ps:
	dvips $<

.utf8.tex.tex:
	cstocs utf8 il2 $< > $@
	vlna -l -r $@

###########################################

pdf: $(DOC).pdf

$(PLOTS_TEX) $(PLOTS_EPS) : $(PLOTS)
	gnuplot $(PLOTS)

experimentDetails.tex: experimentDetails.utf8.tex
	cstocs utf8 il2 $< > $@

zkratky.tex: zkratky.dat
	sort zkratky.dat | sed -e 's/^\(.*\) \"\(.*\)\"\(.*\)/\\paragraph{\1} \2 -- \3/g' > zkratky.tex

images: $(PLOTS_PDF)
	make -C img all

$(DOC).pdf: $(DOC).tex zkratky.tex $(DOC).bib images $(TEX_FILES:=.tex)
	pdflatex $(DOC).tex
	bibtex $(DOC)
	pdflatex $(DOC).tex
	pdflatex $(DOC).tex

clean:
	rm -f *.dvi *.log $(DOC).blg $(DOC).bbl $(DOC).toc *.aux $(DOC).out $(TEX_FILES:=.tex)
	rm -f $(DOC).lof $(PLOTS_TEX) $(PLOTS_EPS) $(PLOTS_PDF) zkratky.tex *~ *.backup *.bak
	make -C img clean

mrproper: clean
	rm $(DOC).pdf

#spell:
#	export LANG=cs_CZ.UTF-8; for FILE in *.utf8.tex; do aspell --lang=cs --encoding=utf-8 --mode=tex check $FILE; done
#	export LANG=cs_CZ.UTF-8; aspell --lang=cs --encoding=iso-8859-2 --mode=tex check zkratky.dat

