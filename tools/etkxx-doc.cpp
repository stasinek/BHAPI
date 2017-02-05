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
 * File: etkxx-doc.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "./../support/StringArray.h>
#include "./../storage/Path.h>
#include "./../storage/File.h>
#include "./../xml/SimpleXmlParser.h>

#define BUFFER_SIZE	1024


inline void print_usage(const char *prog_name)
{
	ETK_OUTPUT("Usage: %s [--debug] [--showinfo] [-s style] [-t options] [-o output] [-l language] file1 file2 ...\n", prog_name);
	ETK_OUTPUT("       style         ---   DocBook/None [default=DocBook]\n");
	ETK_OUTPUT("       options       ---   options for style\n");
	ETK_OUTPUT("       output        ---   Output filename\n");
	ETK_OUTPUT("       language      ---   language of document\n");
	ETK_OUTPUT("       file1 file2   ---   files to be converted\n\n");
	ETK_OUTPUT("Tips: Run \"xsltproc -o AAA.html docbook-html.xsl AAA.xml\" to convert DocBook to HTML\n");
}


inline void quote_string(EString &str)
{
	str.ReplaceAll("&br;", "\n");
	str.ReplaceAll("&", "&amp;");
	str.ReplaceAll(" ", "&nbsp;");
	str.ReplaceAll("<", "&lt;");
	str.ReplaceAll(">", "&gt;");
	str.ReplaceAll("©", "&copy;");
	str.ReplaceAll("®", "&reg;");
	str.ReplaceAll("\"", "&quot;");
}


inline ESimpleXmlNode* find_xml_node_deep(ESimpleXmlNode* node, const char *name)
{
	if(node == NULL) return NULL;
	ESimpleXmlNode *aNode = node->NodeAt(node->FindNode(name));
	if(aNode != NULL) return aNode;
	for(eint32 i = 0; i < node->CountNodes(); i++)
	{
		if((aNode = find_xml_node_deep(node->NodeAt(i), name)) != NULL) return aNode;
	}
	return NULL;
}


inline bool foreach_xml_node(ESimpleXmlNode *node, const char *name, bool (*foreachFunc)(ESimpleXmlNode*, void*), void *userData)
{
	if(node == NULL || foreachFunc == NULL) return false;
	for(eint32 i = 0; i < node->CountNodes(); i++)
	{
		ESimpleXmlNode *aNode = node->NodeAt(i);
		if(name == NULL || !(aNode->Name() == NULL || strcmp(name, aNode->Name()) != 0))
		{
			if((*foreachFunc)(aNode, userData)) return true;
		}
		if(foreach_xml_node(aNode, name, foreachFunc, userData)) return true;
	}
	return false;
}


static bool docbook_foreach(ESimpleXmlNode *node, void *userData)
{
	if(node->Name() == NULL) return false;

	if(strcmp(node->Name(), "xref") == 0)
	{
		if(node->Content() != NULL || node->FindAttribute("endterm") >= 0) return false;

		eint32 index = node->FindAttribute("linkend");
		if(index < 0) return false;

		const char *content = NULL;
		if(node->AttributeAt(index, &content) == NULL || content == NULL) return false;

		EString str(content);
		str.Append("_TITLE");
		node->AddAttribute("endterm", str.String(), true);
	}
	else if(strcmp(node->Name(), "section") == 0)
	{
		eint32 index = node->FindAttribute("id");
		if(index < 0) return false;

		const char *content = NULL;
		ESimpleXmlNode *aNode = NULL;

		if(node->AttributeAt(index, &content) == NULL || content == NULL) return false;
		if((index = node->FindNode("title")) < 0 || (aNode = node->NodeAt(index)) == NULL) return false;

		EString str(content);
		str.Append("_TITLE");
		aNode->AddAttribute("id", str.String(), true);
	}

	return false;
}


inline void convert_document_to_docbook(const ESimpleXmlNode *node, EString *buffer)
{
	if(node == NULL || buffer == NULL) return;

	EString str;

	if(node->Name() == NULL)
	{
		if(node->Content() != NULL)
		{
			str.SetTo(node->Content());
			quote_string(str);
			buffer->Append(str);
		}
		return;
	}
	else if(strcmp(node->Name(), "comment") == 0)
	{
		if(node->Content() != NULL)
		{
			str.SetTo(node->Content());
			quote_string(str);
			buffer->AppendFormat("<!-- %s -->", str.String());
		}
		return;
	}
	else if(strcmp(node->Name(), "legalnotice") == 0)
	{
		buffer->Append("<legalnotice><para>");
	}
	else if(strcmp(node->Name(), "abstract") == 0)
	{
		buffer->Append("<abstract><para>");
	}
	else if(strcmp(node->Name(), "document") != 0)
	{
		buffer->AppendFormat("<%s", node->Name());

		for(eint32 i = 0; i < node->CountAttributes(); i++)
		{
			const char *attr_content = NULL;
			const char *attr_name = node->AttributeAt(i, &attr_content);
			if(attr_name != NULL && attr_content != NULL) buffer->AppendFormat(" %s=\"%s\"", attr_name, attr_content);
		}

		buffer->AppendFormat(">");
	}

	if(node->Content() != NULL)
	{
		str.SetTo(node->Content());
		quote_string(str);
		buffer->Append(str);
	}

	for(eint32 i = 0; i < node->CountNodes(); i++)
		convert_document_to_docbook(node->NodeAt(i), buffer);

	if(strcmp(node->Name(), "legalnotice") == 0)
	{
		buffer->Append("</para></legalnotice>");
	}
	else if(strcmp(node->Name(), "abstract") == 0)
	{
		buffer->Append("</para></abstract>");
	}
	else if(strcmp(node->Name(), "document") != 0)
	{
		buffer->AppendFormat("</%s>", node->Name());
	}
}


inline e_status_t convert_to_docbook(ESimpleXmlNode *node, EString *buffer, const char *options, const char *lang)
{
	if(node == NULL || buffer == NULL || lang == NULL || strlen(lang) == 0) return E_ERROR;

	ESimpleXmlNode *aNode = node->NodeAt(node->FindNode("documentinfo"));

	if(options == NULL) options = "article";
	buffer->MakeEmpty();

	buffer->Append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	if(strcmp(options, "book") == 0 || strcmp(options, "article") == 0)
	{
		buffer->AppendFormat("<!DOCTYPE %s PUBLIC \"-//OASIS//DTD DocBook XML V4.2//EN\"\n", options);
		buffer->Append("\t\"http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd\">\n");
//		buffer->Append("[\n]>\n\n");

		if(aNode)
		{
			EString str;
			str << options << "info";
			aNode->SetName(str.String());
			aNode->RemoveSelf();
		}
	}
	else if(aNode)
	{
		ESimpleXmlNode *cNode = aNode->NodeAt(aNode->FindNode("title"));
		if(cNode) cNode->RemoveSelf();
		aNode->RemoveSelf();
		delete aNode;
		aNode = cNode;
	}

	buffer->Append("<!-- ***** This is a generated file by \"etkxx-doc\", please modify the sources. ***** -->\n");
	buffer->AppendFormat("<%s lang=\"%s\">", options, lang);

	if(aNode)
	{
		convert_document_to_docbook(aNode, buffer);
		delete aNode;
		aNode = NULL;
	}

	eint32 offset = 0;
	while(offset >= 0 && offset < node->CountNodes())
	{
		if((offset = node->FindNode("document", offset)) < 0) break;
		if((aNode = node->NodeAt(offset)) == NULL) break;

		offset++;

		buffer->Append("\n");
		convert_document_to_docbook(aNode, buffer);
	}

	buffer->AppendFormat("\n</%s>\n\n", options);

	return E_OK;
}


int main(int argc, char **argv)
{
	bool printUsageAndExit = true;
	bool showDebug = false;
	bool showInfo = false;

	EPath prog(argv[0]);
	EStringArray files;
	const char *lang = "C";
	const char *style = "DocBook";
	const char *options = NULL;
	files.AddItem(NULL);

	const char *tmp_env = getenv("LC_ALL");
	if(tmp_env == NULL) tmp_env = getenv("LANG");
	if(tmp_env != NULL) lang = tmp_env;

	do {
		if(argc < 2) break;

		for(int n = 1; n < argc; n++)
		{
			if(strcmp(argv[n], "-s") == 0)
			{
				if(argc - n < 2) break;
				n++;
				style = argv[n];
			}
			else if(strcmp(argv[n], "-t") == 0)
			{
				if(argc - n < 2) break;
				n++;
				options = argv[n];
			}
			else if(strcmp(argv[n], "-o") == 0)
			{
				if(argc - n < 2) break;
				n++;
				if(files.ReplaceItem(0, argv[n]) == false) break;
			}
			else if(strcmp(argv[n], "-l") == 0)
			{
				if(argc - n < 2) break;
				n++;
				lang = argv[n];
			}
			else if(strcmp(argv[n], "--debug") == 0)
			{
				showDebug = true;
			}
			else if(strcmp(argv[n], "--showinfo") == 0)
			{
				showInfo = true;
			}
			else
			{
				files.AddItem(argv[n]);
			}
		}

		if(files.CountItems() < 2) break;

		printUsageAndExit = false;
	} while(false);

	if(printUsageAndExit)
	{
		print_usage(prog.Leaf());
		exit(1);
	}

	EString xml_buffer;
	EString strDocStart = "<document ";
	EString strDocEnd = "</document>";

	for(eint32 i = 1; i < files.CountItems(); i++)
	{
		if(files.ItemAt(i) == NULL) continue;
		EPath readInPath(files.ItemAt(i)->String(), NULL, true);
		EFile readIn(readInPath.Path(), E_READ_ONLY);
		if(readIn.InitCheck() != E_OK)
		{
			ETK_DEBUG("[%s] --- Unable to read \"%s\".", prog.Leaf(), files.ItemAt(i)->String());
			continue;
		}

		eint32 old_length = xml_buffer.Length();

		char buffer[BUFFER_SIZE];
		bool foundDocEnd = true;
		size_t nLeave = 0;
		xml_buffer.AppendFormat("<!-- convert from \"%s\" -->\n", readInPath.Leaf());
		while(true)
		{
			ssize_t len = readIn.Read(buffer + nLeave, BUFFER_SIZE - nLeave);
			if(len <= 0) break;
			EString str;
			str.SetTo(buffer, len + nLeave);
			str.RemoveAll("\r");
			eint32 offset = 0;
			while(offset >= 0 && offset < str.Length())
			{
				nLeave = 0;
				if(foundDocEnd)
				{
					offset = str.FindFirst("/*", offset);
					if(offset < 0)
					{
						if(str.Length() < 2) break;
						if(str[str.Length() - 1] == '/' && str[str.Length() - 2] != '*') nLeave = 1; break;
					}

					nLeave = str.Length() - offset;
					offset = str.FindFirst("\n", offset);
					if(offset < 0)
					{
						if(nLeave > 80) nLeave = 0;
						break;
					}
					nLeave = 0;

					offset++;
					if(offset >= str.Length()) break;

					if(strDocStart.Compare(str.String() + offset, strDocStart.Length()) != 0)
					{
						eint32 tmp = str.FindLast("<");
						if(tmp >= 0 && str.Length() - tmp < strDocStart.Length())
						{
							nLeave = str.Length() - tmp;
						}
						else
						{
							nLeave = 0;
						}
						continue;
					}

					foundDocEnd = false;
				}

				eint32 endOffset = str.FindFirst(strDocEnd, offset);
				if(endOffset >= 0)
				{
					endOffset += strDocEnd.Length();
					foundDocEnd = true;
					nLeave = 0;
				}
				else
				{
					eint32 tmp = str.FindLast("<");
					if(tmp >= 0 && str.Length() - tmp < strDocEnd.Length())
					{
						nLeave = str.Length() - tmp;
					}
					else
					{
						nLeave = 0;
					}
				}

				xml_buffer.Append(str.String() + offset, (endOffset >= 0 ? endOffset : str.Length()) - offset - nLeave);
				if(foundDocEnd) xml_buffer.Append("\n");
				offset = endOffset;
			}
			if(nLeave > 0) str.CopyInto(buffer, BUFFER_SIZE, str.Length() - nLeave, nLeave);
		}

		if(foundDocEnd == false)
		{
			xml_buffer.Remove(old_length, -1);
			ETK_DEBUG("[%s] --- Invalid document \"%s\".", prog.Leaf(), readInPath.Path());
		}
	}

	EString output_buffer;

	if(strcmp(style, "None") == 0)
	{
		output_buffer.Adopt(xml_buffer);
	}
	else if(strcmp(style, "DocBook") == 0)
	{
		xml_buffer.ReplaceAll("&", "&amp;");
		xml_buffer.ReplaceAll("&amp;lt;", "&lt;");
		xml_buffer.ReplaceAll("&amp;gt;", "&gt;");
		xml_buffer.ReplaceAll("&amp;nbsp;", "&nbsp;");
		xml_buffer.ReplaceAll("©", "&copy;");
		xml_buffer.ReplaceAll("®", "&reg;");
		xml_buffer.ReplaceAll("\n", "&br;");

		eint32 offset = 0;
		while(offset >= 0 && offset < xml_buffer.Length())
		{
			if((offset = xml_buffer.FindFirst(">", offset)) < 0) break;
			eint32 tmp = xml_buffer.FindFirst("<", offset);
			if(tmp < 0 || tmp - offset <= 1) {offset = tmp; continue;}
			EString str;
			xml_buffer.MoveInto(str, offset + 1, tmp - offset);
			str.ReplaceAll(" ", "&nbsp;");
			xml_buffer.Insert(str, offset + 1);
			offset += str.Length() + 1;
		}

		ESimpleXmlNode node(NULL, NULL);
		if(etk_parse_simple_xml(xml_buffer.String(), &node) != E_OK)
		{
			ETK_OUTPUT("[%s] --- Unable to parse.\n", prog.Leaf());
			exit(1);
		}

		ESimpleXmlNode *aNode = NULL;
		offset = 0;
		while(offset >= 0 && offset < node.CountNodes())
		{
			if((offset = node.FindNode("document", offset)) < 0) break;
			if((aNode = node.NodeAt(offset)) == NULL) break;

			eint32 index = aNode->FindAttribute("lang");
			const char *tmp = NULL;
			if(index < 0 || aNode->AttributeAt(index, &tmp) == NULL || tmp == NULL || strcmp(tmp, lang) != 0)
			{
				aNode->RemoveSelf();
				delete aNode;
				continue;
			}

			offset++;
		}

		aNode = find_xml_node_deep(&node, "documentinfo");
		if(aNode != NULL)
		{
			if(!showInfo)
			{
				ESimpleXmlNode *cNode = aNode->NodeAt(aNode->FindNode("title"));
				if(cNode) cNode->RemoveSelf();
				ESimpleXmlNode *nNode;
				while((nNode = aNode->NodeAt(0)) != NULL) {nNode->RemoveSelf(); delete nNode;}
				if(cNode) aNode->AddNode(cNode);
			}
			aNode->RemoveSelf();
			if(node.AddNode(aNode) == false) delete aNode;
		}

		foreach_xml_node(&node, NULL, docbook_foreach, NULL);

		if(showDebug) node.PrintToStream();

		if(convert_to_docbook(&node, &output_buffer, options, lang) != E_OK)
		{
			ETK_OUTPUT("[%s] --- Unable to convert to \"DocBook\" style.\n", prog.Leaf());
			exit(1);
		}
	}
	else
	{
		ETK_OUTPUT("[%s] --- style \"%s\" unsupport yet.\n", prog.Leaf(), style);
		exit(1);
	}

	if(files.ItemAt(0) == NULL || files.ItemAt(0)->String() == NULL)
	{
		for(eint32 offset = 0; offset < output_buffer.Length(); offset += BUFFER_SIZE)
		{
			EString str(output_buffer.String() + offset, BUFFER_SIZE);
			fprintf(stdout, "%s", str.String());
		}
	}
	else
	{
		EFile writeOut(files.ItemAt(0)->String(), E_WRITE_ONLY | E_CREATE_FILE | E_ERASE_FILE);
		if(writeOut.InitCheck() != E_OK)
		{
			ETK_OUTPUT("[%s] --- Unable to write \"%s\".\n", prog.Leaf(), files.ItemAt(0)->String());
			exit(1);
		}
		else
		{
			for(eint32 offset = 0; offset < output_buffer.Length(); offset += BUFFER_SIZE)
				writeOut.Write(output_buffer.String() + offset, min_c(BUFFER_SIZE, output_buffer.Length() - offset));
		}
	}

	return 0;
}


