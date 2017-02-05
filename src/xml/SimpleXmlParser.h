/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: SimpleXmlParser.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_SIMPLE_XML_PARSER_H__
#define __ETK_SIMPLE_XML_PARSER_H__

#include "./../support/List.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK ESimpleXmlNode {
public:
	ESimpleXmlNode(const char *name, const char *content = NULL);
	~ESimpleXmlNode();

	const char		*Name() const;
	void			SetName(const char *name);

	const char		*Content() const;
	void			SetContent(const char *content);

	const char		*AttributeAt(eint32 index, const char** attr_content = NULL) const;
	bool			AddAttribute(const char *name, const char *content, bool replace_content = true);
	bool			RemoveAttribute(const char *name);
	eint32			FindAttribute(const char *name, eint32 fromIndex = 0) const;
	eint32			CountAttributes() const;

	ESimpleXmlNode		*NodeAt(eint32 index) const;
	bool			AddNode(ESimpleXmlNode *node, eint32 atIndex = -1);
	bool			RemoveNode(ESimpleXmlNode *node);
	bool			RemoveSelf();
	eint32			FindNode(const char *name, eint32 fromIndex = 0) const;
	eint32			CountNodes() const;
	ESimpleXmlNode		*SuperNode() const;

	void			PrintToStream() const;

private:
	char *fName;
	char *fContent;
	EList fAttributes;
	EList fNodes;

	ESimpleXmlNode *fSuperNode;
};


_IMPEXP_ETK e_status_t etk_parse_simple_xml(const char *simple_xml_buffer, ESimpleXmlNode *node);


#endif /* __cplusplus */

#endif /* __ETK_SIMPLE_XML_PARSER_H__ */

