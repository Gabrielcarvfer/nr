# Copyright (c) 2022 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# SPDX-License-Identifier: GPL-2.0-only

EPSTOPDF = epstopdf
DIA = dia
SEQDIAG = seqdiag
DOT = dot
CONVERT = convert -density 250


SOURCE = source
FIGURES = $(SOURCE)/figures
FIGURES_NOBUILD = figures

# specify dia figures from which .png and .pdf figures need to be built

IMAGES_DIA = \
	$(FIGURES)/ulpc/nr-clpc.dia \
	$(FIGURES)/rbf-impl.dia \
#	$(FIGURES)/*.dia


# specify eps figures from which .png and .pdf figures need to be built

IMAGES_EPS = \
	$(FIGURES)/frame-structure-time.eps\
	$(FIGURES)/frame-structure-freq.eps\
	$(FIGURES)/frame-structure-time-WNS3.eps\
	$(FIGURES)/frame-structure-freq-WNS3.eps\
	$(FIGURES)/delay_2b.eps\
	$(FIGURES)/throughput_2.eps\
        $(FIGURES)/delay_numerologies2.eps\
        $(FIGURES)/throughput_numerologies.eps\
	$(FIGURES)/gnb-changes.eps\
        $(FIGURES)/ca-enb-data-plane-v6.eps\
	$(FIGURES)/nr-schedulers.eps\
	$(FIGURES)/nr-scheduler-collab.eps\
	$(FIGURES)/sinrs-r1-sh1-aoZ0-amGnb3GPP-amUe3GPP-scInH-ShoppingMall-sp3-bs0-ang10-gmWALL.eps\
	$(FIGURES)/snrs-r1-sh1-aoZ0-amGnb3GPP-amUe3GPP-scInH-ShoppingMall-sp3-bs0-ang10-gmWALL.eps\

# rescale pdf figures as necessary
#$(FIGURES)/lte-unlicensed-interference-test-scenario.pdf_width = 8cm
#$(FIGURES)/legacy-snir.pdf_width = 10cm


IMAGES_SEQDIAG = \
#	$(FIGURES)/figure-name.seqdiag \


IMAGES_DOT = \
#	$(FIGURES)/figure-name.dot \


IMAGES_NOBUILD = \
    $(FIGURES_NOBUILD)/csi-test-fsm.png \
#	$(FIGURES)/fading_pedestrian.png \
	${IMAGES_SEQDIAG:.seqdiag=.png} \
	${IMAGES_SEQDIAG:.seqdiag=.pdf} \
	${IMAGES_DOT:.dot=.png} \
	${IMAGES_DOT:.dot=.pdf} \
        $(FIGURES)/1x1_1x2_chan_d_fer.png \
	$(FIGURES)/2x2_chan_d_fer.png \
	$(FIGURES)/protocol-stack.png \
	$(FIGURES)/scenario-simple.png \
	$(FIGURES)/end-to-end.png \


IMAGES_BUILD = \
	${IMAGES_DIA:.dia=.eps} \
	${IMAGES_DIA:.dia=.png} \
	${IMAGES_DIA:.dia=.pdf} \
	${IMAGES_EPS:.eps=.png} \
	${IMAGES_EPS:.eps=.pdf}



IMAGES = $(IMAGES_NOBUILD) $(IMAGES_BUILD)

RESCALE = ../../../utils/rescale-pdf.sh

%.eps : %.dia; $(DIA) -t eps $< -e $@
%.png : %.dia; $(DIA) -t png $< -e $@
%.png : %.seqdiag; $(SEQDIAG) -Tpng --no-transparency -o $@ $<
%.png : %.dot; $(DOT) -Tpng -o$@ $<
%.png : %.eps; $(CONVERT) $< $@
%.pdf : %.seqdiag
	$(SEQDIAG) -Tpdf -o $@ $<
	if test x$($@_width) != x; then $(RESCALE) $($@_width) $@ ; fi
%.pdf : %.dot
	$(DOT) -Tpdf -o $@ $<
	if test x$($@_width) != x; then $(RESCALE) $($@_width) $@ ; fi
%.pdf : %.eps
	$(EPSTOPDF) $< -o=$@
	if test x$($@_width) != x; then $(RESCALE) $($@_width) $@ ; fi


# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
PAPER         =
BUILDDIR      = build

# Internal variables.
PAPEROPT_a4     = -D latex_paper_size=a4
PAPEROPT_letter = -D latex_paper_size=letter
ALLSPHINXOPTS   = -d $(BUILDDIR)/doctrees $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) $(SOURCE)

.PHONY: help clean html dirhtml singlehtml pickle json htmlhelp qthelp devhelp epub latex latexpdf text man changes linkcheck doctest

help:
	@echo "Please use \`make <target>' where <target> is one of"
	@echo "  html       to make standalone HTML files"
	@echo "  dirhtml    to make HTML files named index.html in directories"
	@echo "  singlehtml to make a single large HTML file"
	@echo "  pickle     to make pickle files"
	@echo "  json       to make JSON files"
	@echo "  htmlhelp   to make HTML files and a HTML help project"
	@echo "  qthelp     to make HTML files and a qthelp project"
	@echo "  devhelp    to make HTML files and a Devhelp project"
	@echo "  epub       to make an epub"
	@echo "  latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter"
	@echo "  latexpdf   to make LaTeX files and run them through pdflatex"
	@echo "  text       to make text files"
	@echo "  man        to make manual pages"
	@echo "  changes    to make an overview of all changed/added/deprecated items"
	@echo "  linkcheck  to check all external links for integrity"
	@echo "  doctest    to run all doctests embedded in the documentation (if enabled)"

clean:
	-rm -rf $(BUILDDIR)/*
	-rm -f $(IMAGES_BUILD)

copy-sources:  $(SOURCES)
	@mkdir -p $(FIGURES)
	@cp -r $(IMAGES_NOBUILD) $(FIGURES)
	@mkdir -p $(BUILDDIR)/html/_static
	@cp -r ./static/* $(BUILDDIR)/html/_static

images: $(IMAGES_NOBUILD) $(IMAGES_BUILD)

frag: pickle
	@if test ! -d $(BUILDDIR)/frag; then mkdir $(BUILDDIR)/frag; fi
	pushd $(BUILDDIR)/frag && ../../pickle-to-xml.py ../pickle/index.fpickle  > navigation.xml && popd
	cp -r $(BUILDDIR)/pickle/_images $(BUILDDIR)/frag

html: copy-sources $(IMAGES)
	$(SPHINXBUILD) -b html $(ALLSPHINXOPTS) $(BUILDDIR)/html
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)/html."

dirhtml: copy-sources $(IMAGES)
	$(SPHINXBUILD) -b dirhtml $(ALLSPHINXOPTS) $(BUILDDIR)/dirhtml
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)/dirhtml."

singlehtml: copy-sources $(IMAGES)
	$(SPHINXBUILD) -b singlehtml $(ALLSPHINXOPTS) $(BUILDDIR)/singlehtml
	@echo
	@echo "Build finished. The HTML page is in $(BUILDDIR)/singlehtml."

pickle:  $(IMAGES)
	$(SPHINXBUILD) -b pickle $(ALLSPHINXOPTS) $(BUILDDIR)/pickle
	@echo
	@echo "Build finished; now you can process the pickle files."

json:  $(IMAGES)
	$(SPHINXBUILD) -b json $(ALLSPHINXOPTS) $(BUILDDIR)/json
	@echo
	@echo "Build finished; now you can process the JSON files."

htmlhelp:  $(IMAGES)
	$(SPHINXBUILD) -b htmlhelp $(ALLSPHINXOPTS) $(BUILDDIR)/htmlhelp
	@echo
	@echo "Build finished; now you can run HTML Help Workshop with the" \
	      ".hhp project file in $(BUILDDIR)/htmlhelp."

qthelp:  $(IMAGES)
	$(SPHINXBUILD) -b qthelp $(ALLSPHINXOPTS) $(BUILDDIR)/qthelp
	@echo
	@echo "Build finished; now you can run "qcollectiongenerator" with the" \
	      ".qhcp project file in $(BUILDDIR)/qthelp, like this:"
	@echo "# qcollectiongenerator $(BUILDDIR)/qthelp/ns-3.qhcp"
	@echo "To view the help file:"
	@echo "# assistant -collectionFile $(BUILDDIR)/qthelp/ns-3.qhc"

devhelp:  $(IMAGES)
	$(SPHINXBUILD) -b devhelp $(ALLSPHINXOPTS) $(BUILDDIR)/devhelp
	@echo
	@echo "Build finished."
	@echo "To view the help file:"
	@echo "# mkdir -p $$HOME/.local/share/devhelp/ns-3"
	@echo "# ln -s $(BUILDDIR)/devhelp $$HOME/.local/share/devhelp/ns-3"
	@echo "# devhelp"

epub:  $(IMAGES)
	$(SPHINXBUILD) -b epub $(ALLSPHINXOPTS) $(BUILDDIR)/epub
	@echo
	@echo "Build finished. The epub file is in $(BUILDDIR)/epub."

latex: copy-sources $(IMAGES)
	$(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex
	@echo
	@echo "Build finished; the LaTeX files are in $(BUILDDIR)/latex."
	@echo "Run \`make' in that directory to run these through (pdf)latex" \
	      "(use \`make latexpdf' here to do that automatically)."

latexpdf: copy-sources $(IMAGES)
	$(SPHINXBUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex
	@echo "Running LaTeX files through pdflatex..."
	make -C $(BUILDDIR)/latex all-pdf
	@echo "pdflatex finished; the PDF files are in $(BUILDDIR)/latex."

text:  $(IMAGES)
	$(SPHINXBUILD) -b text $(ALLSPHINXOPTS) $(BUILDDIR)/text
	@echo
	@echo "Build finished. The text files are in $(BUILDDIR)/text."

man:  $(IMAGES)
	$(SPHINXBUILD) -b man $(ALLSPHINXOPTS) $(BUILDDIR)/man
	@echo
	@echo "Build finished. The manual pages are in $(BUILDDIR)/man."

changes:  $(IMAGES)
	$(SPHINXBUILD) -b changes $(ALLSPHINXOPTS) $(BUILDDIR)/changes
	@echo
	@echo "The overview file is in $(BUILDDIR)/changes."

linkcheck:  $(IMAGEs)
	$(SPHINXBUILD) -b linkcheck $(ALLSPHINXOPTS) $(BUILDDIR)/linkcheck
	@echo
	@echo "Link check complete; look for any errors in the above output " \
	      "or in $(BUILDDIR)/linkcheck/output.txt."

doctest:  $(IMAGES)
	$(SPHINXBUILD) -b doctest $(ALLSPHINXOPTS) $(BUILDDIR)/doctest
	@echo "Testing of doctests in the sources finished, look at the " \
	      "results in $(BUILDDIR)/doctest/output.txt."

