/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
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

#ifndef BHAPI_SIMPLE_XML_PARSER_H
#define BHAPI_SIMPLE_XML_PARSER_H

#include <kits/support/List.h>

#ifdef __cplusplus /* Just for C++ */

class BHAPI_IMPEXP BSimpleXmlNode {
public:
	BSimpleXmlNode(const char *name, const char *content = NULL);
	~BSimpleXmlNode();

	const char		*Name() const;
	void			SetName(const char *name);

	const char		*Content() const;
	void			SetContent(const char *content);

    const char		*AttributeAt(int32_t index, const char** attr_content = NULL) const;
	bool			AddAttribute(const char *name, const char *content, bool replace_content = true);
	bool			RemoveAttribute(const char *name);
     int32_t			FindAttribute(const char *name,  int32_t fromIndex = 0) const;
     int32_t			CountAttributes() const;

    BSimpleXmlNode		*NodeAt(int32_t index) const;
    bool			AddNode(BSimpleXmlNode *node,  int32_t atIndex = -1);
	bool			RemoveNode(BSimpleXmlNode *node);
	bool			RemoveSelf();
     int32_t			FindNode(const char *name,  int32_t fromIndex = 0) const;
     int32_t			CountNodes() const;
	BSimpleXmlNode		*SuperNode() const;

	void			PrintToStream() const;

private:
	char *fName;
	char *fContent;
	BList fAttributes;
	BList fNodes;

	BSimpleXmlNode *fSuperNode;
};


BHAPI_IMPEXP status_t b_parse_simple_xml(const char *simple_xml_buffer, BSimpleXmlNode *node);


#endif /* __cplusplus */

#endif /* BHAPI_SIMPLE_XML_PARSER_H */

