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
 * File: path-test.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../storage/VolumeRoster.h>

int main(int argc, char **argv)
{
	ETK_OUTPUT("E_MAXPATH: %d\n\n", E_MAXPATH);

	EPath path("/abc/def/ghi.jkl");
	ETK_OUTPUT("EPath path(\"/abc/def/ghi.jkl\")\n");
	ETK_OUTPUT("Path:  %s\n", path.Path());
	ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
	while(true)
	{
		e_status_t status;
		if((status = path.GetParent(&path)) == E_OK)
		{
			ETK_OUTPUT("\npath.GetParent(&path)\n");
			ETK_OUTPUT("Path:  %s\n", path.Path());
			ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
		}
		else
		{
			ETK_OUTPUT("\npath.GetParent(&path) failed: E_GENERAL_ERROR_BASE + 0x%x\n", status - E_GENERAL_ERROR_BASE);
			break;
		}
	}
	ETK_OUTPUT("\n");

	path.Unset();
	path.SetTo("/abc/def//ghi.jkl");
	ETK_OUTPUT("path.SetTo(\"/abc/def//ghi.jkl\")\n");
	ETK_OUTPUT("Path:  %s\n", path.Path());
	ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
	ETK_OUTPUT("\n");

	path.Unset();
	path.SetTo("/abc/def/ghi.jkl", NULL, true);
	ETK_OUTPUT("path.SetTo(\"/abc/def/ghi.jkl\", NULL, true)\n");
	ETK_OUTPUT("Path:  %s\n", path.Path());
	ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
	ETK_OUTPUT("\n");

	path.Unset();
	path.SetTo("/usr/lib", NULL, true);
	ETK_OUTPUT("path.SetTo(\"/usr/lib\", NULL, true)\n");
	ETK_OUTPUT("Path:  %s\n", path.Path());
	ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
	ETK_OUTPUT("\n");

	path.Unset();
	path.SetTo("/usr/include/", "sys");
	ETK_OUTPUT("path.SetTo(\"/usr/include/\", \"sys\")\n");
	ETK_OUTPUT("Path:  %s\n", path.Path());
	ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
	ETK_OUTPUT("\n");

	path.Unset();
	path.SetTo("/usr/include/", "something");
	ETK_OUTPUT("path.SetTo(\"/usr/include/\", \"something\")\n");
	ETK_OUTPUT("Path:  %s\n", path.Path());
	ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
	ETK_OUTPUT("\n");

	path.Unset();
	path.SetTo(argv[0], NULL, true);
	ETK_OUTPUT("path.SetTo(\"%s\", NULL, true)\n", argv[0]);
	ETK_OUTPUT("Path:  %s\n", path.Path());
	ETK_OUTPUT("Leaf:  %s\n", path.Leaf());
	ETK_OUTPUT("\n");

	if(argc > 1)
	{
		ETK_OUTPUT("\n");
		EDirectory dir(argv[1]);
		EEntry entry;
		dir.GetEntry(&entry);
		entry.GetPath(&path);
		ETK_OUTPUT("I'll list the content of \"%s\", items: %d\n", path.Path(), dir.CountEntries());
		while(dir.GetNextEntry(&entry, true) == E_OK)
		{
			entry.GetPath(&path);
			ETK_OUTPUT("\t%s\n", path.Path());
		}
	}

	ETK_OUTPUT("\n");

	EVolumeRoster volRoster;
	EVolume vol;
	volRoster.GetBootVolume(&vol);
	bool output_boot = true;
	do {
		EString volName;
		EDirectory volRootDir;
		EEntry entry;
		vol.GetName(&volName);
		vol.GetRootDirectory(&volRootDir);
		volRootDir.GetEntry(&entry);
		entry.GetPath(&path);

		if(output_boot == true)
		{
			if(vol.InitCheck() == E_OK)
				ETK_OUTPUT("BootVol[%d]: \"%s\" --- \"%s\"\n\n", vol.Device(), volName.String(), path.Path());
			else
				ETK_OUTPUT("BootVol: None\n\n");
			output_boot = false;
			continue;
		}

		ETK_OUTPUT("Vol[%d]: \"%s\" --- \"%s\"\n", vol.Device(), volName.String(), path.Path());
	} while(volRoster.GetNextVolume(&vol) == E_NO_ERROR);

	return 0;
}

