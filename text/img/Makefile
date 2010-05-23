#
#      Brno University of Technology
#      Faculty of Information Technology
# 
#      MSc Thesis	2009/2010
# 
#      $Date:: 2010-01-15 0#$:
#
#
#      Bedrich Hovorka
#-----------------------------------------------------
#
LANG         = cs_CZ
LOCALE       = cs_CZ
WMFEPSS	     = $(patsubst %.wmf,%.eps, $(wildcard *.wmf))
BMPEPSS	     = $(patsubst %.bmp,%.eps, $(wildcard *.bmp))
DIAEPSS	     = $(patsubst %.dia,%.eps, $(wildcard *.dia))
SVGEPSS	     = $(patsubst %.svg,%.eps, $(wildcard *.svg))

###########################################

all:        $(patsubst %.eps,%.pdf,$(wildcard *.eps)  $(WMFEPSS) $(BMPEPSS) $(DIAEPSS) $(SVGEPSS))

###########################################

.SUFFIXES: .bmp .eps .pdf .wmf .dia .svg

.eps.pdf:
	epstopdf $<

.wmf.eps:
	wmf2eps --wmf-xtra-fonts $< > $@

.dia.eps:
	dia -e $@ -t eps $<

.bmp.eps:
	autotrace -dpi 1200 -filter-iterations 64 $< -output-format eps > $@

.svg.eps:
	inkscape $< --export-eps=$@	
###########################################

clean:
	rm -f *.pdf $(WMFEPSS) $(BMPEPSS) $(DIAEPSS) $(SVGEPSS) *~
