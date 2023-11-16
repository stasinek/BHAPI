AUTOMAKE_OPTIONS = 1.7

DOCBOOK_TO_HTML = xsltproc
DOCBOOK_TO_HTML_XSL = docbook-html.xsl
DOCBOOK_CSS = docbook

ETKXX_DOC = $(top_builddir)/tools/etkxx-doc
ETKXX_DOC_LANG = zh_CN.UTF-8

XML_FILES = Definition.ent InterfaceKit.xml RenderKit.xml
RENDER_MANAUAL = LineGenerator.cpp
INTERFACE_MANAUAL = Region.cpp Button.cpp

EXTRA_DIST = 				\
	$(XML_FILES)			\
	docbook-html.xsl		\
	index.zh_CN.UTF-8.xml		\
	docbook.zh_CN.UTF-8.css

DISTCLEANFILES = Makefile.in

.PHONY: xml-manual html-manual

xml-manual: $(ETKXX_DOC)
	-mkdir xml
	-for lang in $(ETKXX_DOC_LANG) .none; do \
		(test $$lang = .none || ( \
		mkdir -p xml/$$lang; \
		cp index.$$lang.xml xml/$$lang/index.xml; \
		for f in $(RENDER_MANAUAL) .none; do \
			(test $$f = .none || ( \
				(test ! -e $(top_builddir)/etk/render/$$f) || \
				$(ETKXX_DOC) -s DocBook -t section -l $$lang -o xml/$$lang/$$f.xml $(top_builddir)/etk/render/$$f \
			)) \
		done; \
		for f in $(INTERFACE_MANAUAL) .none; do \
			(test $$f = .none || ( \
				(test ! -e $(top_builddir)/etk/interface/$$f) || \
				$(ETKXX_DOC) -s DocBook -t section -l $$lang -o xml/$$lang/$$f.xml $(top_builddir)/etk/interface/$$f \
			)) \
		done; \
		for f in $(XML_FILES) .none; do \
			(test $$f = .none || ((test ! -e $$f) || cp $$f xml/$$lang)) \
		done; \
		)) \
	done

html-manual: xml-manual
	-mkdir html
	-for lang in $(ETKXX_DOC_LANG) .none; do \
		(test $$lang = .none || ( \
		mkdir -p html/$$lang; \
		cp $(DOCBOOK_CSS).$$lang.css html/$$lang/$(DOCBOOK_CSS).css; \
		$(DOCBOOK_TO_HTML) -o html/$$lang/ $(DOCBOOK_TO_HTML_XSL) xml/$$lang/index.xml; \
		)) \
	done

clean-local:
	-rm -rf xml html

