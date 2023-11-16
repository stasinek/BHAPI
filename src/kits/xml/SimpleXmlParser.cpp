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
 * File: SimpleXmlParser.cpp
 *
 * --------------------------------------------------------------------------*/

#include <SimpleXmlParser.h>

#include <kits/support/String.h>
#include <kits/support/StringArray.h>
#include <kits/debug/Errors.h>
#include <os/debug.h>



typedef struct b_xml_attribute {
	char *name;
	char *content;
} b_xml_attribute;


BSimpleXmlNode::BSimpleXmlNode(const char *name, const char *content)
	: fName(NULL), fContent(NULL), fSuperNode(NULL)
{
	if(name) fName = bhapi::strdup(name);
	if(content) fContent = bhapi::strdup(content);
}


BSimpleXmlNode::~BSimpleXmlNode()
{
	if(fSuperNode != NULL) fSuperNode->RemoveNode(this);

	if(fName != NULL) delete[] fName;
	if(fContent != NULL) delete[] fContent;

	while(fAttributes.CountItems() > 0)
	{
		b_xml_attribute* attr = (b_xml_attribute*)fAttributes.RemoveItem((__be_int32_t)0);
		if(attr == NULL) continue;
		if(attr->name) delete[] attr->name;
		if(attr->content) delete [] attr->content;
		delete attr;
	}

	while(fNodes.CountItems() > 0)
	{
		BSimpleXmlNode* node = (BSimpleXmlNode*)fNodes.RemoveItem((__be_int32_t)0);
		if(node == NULL) continue;
		delete node;
	}
}


const char*
BSimpleXmlNode::Name() const
{
	return fName;
}


const char*
BSimpleXmlNode::Content() const
{
	return fContent;
}


const char*
BSimpleXmlNode::AttributeAt(__be_int32_t index, const char** attr_content) const
{
	b_xml_attribute* attr = (b_xml_attribute*)fAttributes.ItemAt(index);
	if(attr == NULL) return NULL;
	if(attr_content) *attr_content = attr->content;
	return attr->name;
}


__be_int32_t
BSimpleXmlNode::FindAttribute(const char *name,  __be_int32_t fromIndex) const
{
	if(name == NULL || *name == 0 || fromIndex < 0 || fromIndex >= fAttributes.CountItems()) return -1;
	for(__be_int32_t i = fromIndex; i < fAttributes.CountItems(); i++)
	{
		b_xml_attribute* attr = (b_xml_attribute*)fAttributes.ItemAt(i);
		if(attr == NULL) continue;
		if(attr->name == NULL) continue;
		if(strlen(attr->name) != strlen(name)) continue;
		if(strcmp(attr->name, name) == 0) return i;
	}
	return -1;
}


BSimpleXmlNode*
BSimpleXmlNode::NodeAt(__be_int32_t index) const
{
	return((BSimpleXmlNode*)fNodes.ItemAt(index));
}


__be_int32_t
BSimpleXmlNode::FindNode(const char *name,  __be_int32_t fromIndex) const
{
	if(name == NULL || *name == 0 || fromIndex < 0 || fromIndex >= fNodes.CountItems()) return -1;
	for(__be_int32_t i = fromIndex; i < fNodes.CountItems(); i++)
	{
		BSimpleXmlNode* node = (BSimpleXmlNode*)fNodes.ItemAt(i);
		if(node == NULL) continue;
		if(node->fName == NULL) continue;
		if(strlen(node->fName) != strlen(name)) continue;
		if(strcmp(node->fName, name) == 0) return i;
	}
	return -1;
}


void 
BSimpleXmlNode::SetName(const char *name)
{
	if(fName) delete[] fName;
	fName = (name == NULL ? NULL : bhapi::strdup(name));
}


void 
BSimpleXmlNode::SetContent(const char *content)
{
	if(fContent) delete[] fContent;
	fContent = (content == NULL ? NULL : bhapi::strdup(content));
}


bool 
BSimpleXmlNode::AddAttribute(const char *name, const char *content, bool replace_content)
{
	if(name == NULL || *name == 0) return false;
	__be_int32_t index = FindAttribute(name);
	if(index >= 0)
	{
		if(replace_content == false) return false;
		b_xml_attribute* attr = (b_xml_attribute*)fAttributes.ItemAt(index);
		if(attr->content) delete[] attr->content;
		attr->content = (content == NULL ? NULL : bhapi::strdup(content));
		return true;
	}

	b_xml_attribute* attr = new b_xml_attribute;
	if(attr == NULL) return false;

	if(fAttributes.AddItem(attr) == false)
	{
		delete attr;
		return false;
	}

	attr->name = bhapi::strdup(name);
	attr->content = (content == NULL ? NULL : bhapi::strdup(content));

	return true;
}


bool 
BSimpleXmlNode::RemoveAttribute(const char *name)
{
	__be_int32_t index = FindAttribute(name);
	if(index < 0) return false;
	b_xml_attribute* attr = (b_xml_attribute*)fAttributes.RemoveItem(index);
	if(attr == NULL) return false;

	if(attr->name) delete[] attr->name;
	if(attr->content) delete [] attr->content;
	delete attr;

	return true;
}


bool 
BSimpleXmlNode::AddNode(BSimpleXmlNode *node,  __be_int32_t atIndex)
{
	if(node == NULL || node->fSuperNode != NULL) return false;

	if(fNodes.AddItem(node, atIndex < 0 ? fNodes.CountItems() : atIndex) == false) return false;

	node->fSuperNode = this;

	return true;
}


bool 
BSimpleXmlNode::RemoveNode(BSimpleXmlNode *node)
{
	if(node == NULL || node->fSuperNode != this) return false;

	if(fNodes.RemoveItem(node) == false) return false;

	node->fSuperNode = NULL;

	return true;
}


bool 
BSimpleXmlNode::RemoveSelf()
{
	if(fSuperNode == NULL) return false;
	return fSuperNode->RemoveNode(this);
}


__be_int32_t
BSimpleXmlNode::CountAttributes() const
{
	return fAttributes.CountItems();
}


__be_int32_t
BSimpleXmlNode::CountNodes() const
{
	return fNodes.CountItems();
}


BSimpleXmlNode*
BSimpleXmlNode::SuperNode() const
{
	return fSuperNode;
}


void 
BSimpleXmlNode::PrintToStream() const
{
	__be_int32_t nSuperNode = 0;
	const BSimpleXmlNode *node = fSuperNode;
	if(node) {nSuperNode++; while((node = node->fSuperNode) != NULL) nSuperNode++;}
	if(nSuperNode == 0) BHAPI_OUTPUT("======================= BSimpleXmlNode =======================\n");
	for(__be_int32_t i = 0; i < nSuperNode; i++) BHAPI_OUTPUT("  ");
	BHAPI_OUTPUT("|-- Name: \"%s\", Content: \"%s\"\n", fName ? fName : "[NULL]", fContent ? fContent : "[NULL]");
	for(__be_int32_t i = 0; i < fAttributes.CountItems(); i++)
	{
		b_xml_attribute* attr = (b_xml_attribute*)fAttributes.ItemAt(i);
		if(attr == NULL) continue;
		for(__be_int32_t j = 0; j < nSuperNode; j++) BHAPI_OUTPUT("  ");
		BHAPI_OUTPUT("|--(Attribute %d) Name: \"%s\", Content: \"%s\"\n",
			   i, attr->name ? attr->name : "[NULL]", attr->content ? attr->content : "[NULL]");
	}
	for(__be_int32_t i = 0; i < fNodes.CountItems(); i++)
	{
        BSimpleXmlNode* t_node = (BSimpleXmlNode*)fNodes.ItemAt(i);
        if(t_node) t_node->PrintToStream();
	}
	if(nSuperNode == 0) BHAPI_OUTPUT("==============================================================\n", this);
}


inline void remove_unwanted_characters(BString &str)
{
	__be_int32_t offset = 0;
	bool quote = false;
	for(; offset < str.Length(); offset++)
	{
		char c = str[offset];
		if(c == '\"' || c == '\'')
		{
			quote = !quote;
		}
		else if(c == ' ')
		{
			if(quote)
			{
				str.Remove(offset, 1);
				str.Insert("&nbsp;", offset);
				offset += 5;
			}
			else
			{
				while(offset < str.Length() - 1)
				{
					if(str[offset + 1] != ' ') break;
					str.Remove(offset + 1, 1);
				}

				if(offset == 0 || offset == str.Length() - 1)
				{
					str.Remove(offset--, 1);
					if(offset >= 0) offset--;
				}
			}
		}
		else if(c == '\t' || c == '\r' || c == '\n')
		{
			str.Remove(offset--, 1);
			if(offset >= 0) offset--;
		}
		else
		{
			if(c == '<' || c == '=')
			{
				while(offset < str.Length() - 1)
				{
					if(str[offset + 1] == ' ') str.Remove(offset + 1, 1);
					else break;
				}
			}

			if((c == '>' || c == '=') && offset > 0)
			{
				c = str[offset - 1];
				if(c == ' ' || c == '\t' || c == '\r' || c == '\n') str.Remove(--offset, 1);
			}
		}
	}
}


inline bool parse_simple_xml(BString *buffer,  __be_int32_t *offset, BSimpleXmlNode *node)
{
	if(buffer == NULL || offset == NULL || node == NULL || *offset < 0 || *offset >= buffer->Length()) return false;

	__be_int32_t start = buffer->FindFirst("<", *offset) + 1;
	if(start <= 0) return false;
	*offset = start;

	__be_int32_t end = buffer->FindFirst(">", start) - 1;
	if(end < 0) return false;
	*offset = end + 2;
	if(end < start) return false;

	BString str;
	buffer->CopyInto(str, start, end - start + 1);
	remove_unwanted_characters(str);

	if(str.ByteAt(str.Length() - 1) == '/') str.Insert(" ", str.Length() - 1);

//	BHAPI_DEBUG("[XML]: \"%s\"", str.String());

	if(str.Compare("!-- ", 4) == 0)
	{
		if(str.FindLast(" --") == str.Length() - 3)
		{
			BString tmp;
			str.CopyInto(tmp, 4, str.Length() - 7);

			tmp.ReplaceAll("&nbsp;", " ");
			tmp.ReplaceAll("&lt;", "<");
			tmp.ReplaceAll("&gt;", ">");
			tmp.ReplaceAll("&amp;", "&");
			tmp.ReplaceAll("&copy;", "©");
			tmp.ReplaceAll("&reg;", "®");
			tmp.ReplaceAll("&quot;", "\"");

			BSimpleXmlNode *aNode = new BSimpleXmlNode("comment", tmp.String());
			if(!(aNode == NULL || node->AddNode(aNode))) delete aNode;
			return true;
		}
		return false;
	}

	BStringArray *array = str.Split(' ');
	if(array == NULL) return false;

	bool retVal = false;

	do
	{
		if(array->CountItems() < 1) {delete array; break;}
		if(array->ItemAt(0) == NULL || array->ItemAt(0)->FindFirst('/') >= 0) {delete array; break;}
		BSimpleXmlNode *aNode = new BSimpleXmlNode(array->ItemAt(0)->String());
		if(aNode == NULL) {delete array; break;}
		for(__be_int32_t i = 1; i < array->CountItems(); i++)
		{
			if(array->ItemAt(i) == NULL) continue;
			if(array->ItemAt(i)->FindFirst('/') >= 0) continue;
			__be_int32_t sepIndex = array->ItemAt(i)->FindFirst('=');
			if(array->ItemAt(i)->FindLast('=') != sepIndex || sepIndex == 0) continue;
			str.SetTo(array->ItemAt(i)->String(), sepIndex);

			BString attr_content;
			array->ItemAt(i)->CopyInto(attr_content, sepIndex + 1, -1);
			attr_content.RemoveSet("\"\'");
			attr_content.ReplaceAll("&nbsp;", " ");
			attr_content.ReplaceAll("&lt;", "<");
			attr_content.ReplaceAll("&gt;", ">");
			attr_content.ReplaceAll("&amp;", "&");
			attr_content.ReplaceAll("&copy;", "©");
			attr_content.ReplaceAll("&reg;", "®");
			attr_content.ReplaceAll("&quot;", "\"");

			aNode->AddAttribute(str.String(), attr_content.String());
		}

		bool isEnded = !(array->LastItem() == NULL || *(array->LastItem()) != "/");
		delete array;

		while(!isEnded && *offset < buffer->Length())
		{
			start = buffer->FindFirst("<", *offset) + 1;
			if(start > *offset + 1 || start <= 0)
			{
				str.SetTo(buffer->String() + *offset, start - *offset - 1);
				*offset += max_c(str.Length(), 0);
				remove_unwanted_characters(str);
				if(!(str.Length() <= 0 || (str.Length() == 1 && str.ByteAt(0) == ' ')))
				{
					str.ReplaceAll("&nbsp;", " ");
					str.ReplaceAll("&lt;", "<");
					str.ReplaceAll("&gt;", ">");
					str.ReplaceAll("&amp;", "&");
					str.ReplaceAll("&copy;", "©");
					str.ReplaceAll("&reg;", "®");
					str.ReplaceAll("&quot;", "\"");

					if(aNode->Content() == NULL && aNode->CountNodes() <= 0)
					{
						aNode->SetContent(str.String());
					}
					else
					{
						BSimpleXmlNode *cNode = new BSimpleXmlNode(NULL, str.String());
						if(cNode != NULL) if(aNode->AddNode(cNode) == false) delete cNode;
					}
				}
				if(start <= 0) break;
			}
			*offset = start;

			end = buffer->FindFirst(">", start) - 1;
			if(end < 0) break;
			*offset = end + 2;
			if(end < start) break;

			buffer->CopyInto(str, start, end - start + 1);
			remove_unwanted_characters(str);

//			BHAPI_DEBUG("[XML]: \"%s\"", str.String());

			if(str.Length() <= 0 || (str.Length() == 1 && str.ByteAt(0) == ' ')) continue;

			if(str.ByteAt(0) == '/' &&
			   str.FindLast(aNode->Name()) == 1 &&
			   str.Length() - (__be_int32_t)strlen(aNode->Name()) == 1) break;

			*offset = start - 1;

			if(parse_simple_xml(buffer, offset, aNode) == true) continue;
			break;
		}

		retVal = true;
		if(node->AddNode(aNode) == false) delete aNode;
	} while(false);

	return retVal;
}


BHAPI_IMPEXP status_t b_parse_simple_xml(const char *buf, BSimpleXmlNode *node)
{
	if(buf == NULL || *buf == 0 || node == NULL) return B_BAD_VALUE;

	BString buffer(buf);
	if(buffer.Length() <= 0) return B_ERROR;

	__be_int32_t offset = 0;

	while(offset < buffer.Length())
	{
		if(parse_simple_xml(&buffer, &offset, node) == false) break;
	}

	return B_OK;
}

