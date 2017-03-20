
#include <BeOSBuildCompatibility.h>
#include <../include/syscalls.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>


#include <unistd.h>
#include <../include/sys/stat.h>

#include <../include/string>

#include <fs_attr.h>

#include "fs_impl.h"
#include "fs_descriptors.h"


// Include the interface to the host platform attributes support, if it shall be
// used to tag files with unique IDs to identify their attribute directory.
#if HAIKU_HOST_USE_XATTR_REF
#	if defined(HAIKU_HOST_PLATFORM_LINUX)
#		include "fs_attr_xattr.h"
#	elif defined(HAIKU_HOST_PLATFORM_FREEBSD)
#		include "fs_attr_extattr.h"
#	elif defined(HAIKU_HOST_PLATFORM_DARWIN)
#		include "fs_attr_bsdxattr.h"
#	else
#		error No attribute support for this host platform!
#	endif
#endif


using namespace std;
using namespace BPrivate;

static const char *sAttributeDirBasePath = HAIKU_BUILD_ATTRIBUTES_DIR;

#if HAIKU_HOST_USE_XATTR_REF
static const char* const kIDAttributeName = "id";
#endif


// init_attribute_dir_base_dir
static status_t
init_attribute_dir_base_dir()
{
	static bool initialized = false;
	static status_t initError;

	if (initialized)
		return initError;

	// stat the dir
	struct stat st;
	initError = B_OK;
	if (lstat(sAttributeDirBasePath, &st) == 0) {
		if (!S_ISDIR(st.st_mode)) {
			// the attribute dir base dir is no directory
			fprintf(stderr, "init_attribute_dir_base_dir(): The Attribute "
				"directory base directory exists, but is no directory!\n");
			initError = B_FILE_ERROR;
		}

	} else {
		// doesn't exist yet: create it
		if (mkdir(sAttributeDirBasePath, S_IRWXU | S_IRWXG | S_IRWXO) < 0)
			initError = errno;
	}

	initialized = true;
	return initError;
}

// escape_attr_name
static string
escape_attr_name(const char *name)
{
	string escapedName("_");
	while (*name != '\0') {
		// we replace '/' with "_s" and '_' with "__"
		if (*name == '/')
			escapedName += "_s";
		else if (*name == '_')
			escapedName += "__";
		else
			escapedName += *name;

		name++;
	}

	return escapedName;
}

// deescape_attr_name
static string
deescape_attr_name(const char *name)
{
	if (name[0] != '_') {
		debugger("deescape_attr_name(): name doesn't start with '_'!\n");
		return "___";
	}
	name++;

	string deescapedName;
	while (*name != '\0') {
		if (*name == '_') {
			name++;
			if (*name == 's') {
				deescapedName += '/';
			} else if (*name == '_') {
				deescapedName += '_';
			} else {
				debugger("deescape_attr_name(): name contains invalid escaped "
					"sequence!\n");
				name--;
			}
		} else
			deescapedName += *name;

		name++;
	}

	return deescapedName;
}


#if HAIKU_HOST_USE_XATTR_REF


static status_t
make_unique_node_id(string& _id)
{
	// open random device
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		fd = open("/dev/random", O_RDONLY);
		if (fd < 0)
			return B_NOT_SUPPORTED;
	}

	// read bytes
	uint8 buffer[16];
	ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
	status_t error = B_OK;
	if (bytesRead < 0)
		error = errno;
	close(fd);

	if (error != B_OK)
		return error;

	if (bytesRead != (ssize_t)sizeof(buffer))
		error = B_ERROR;

	// convert to hex string
	static const char* const kHexChars = "0123456789abcdef";
	_id.clear();
	for (size_t i = 0; i < sizeof(buffer); i++) {
		_id += kHexChars[buffer[i] >> 4];
		_id += kHexChars[buffer[i] & 0xf];
	}

	return B_OK;
}


static status_t
get_id_attribute(const char *path, int fd, string& _id)
{
	// list_attributes() and remove_attribute() are unused here -- prevent the
	// warning
	(void)list_attributes;
	(void)remove_attribute;

	string attributeName(kAttributeNamespace);
	attributeName += kIDAttributeName;

	char buffer[64];
	ssize_t bytesRead = get_attribute(fd, path, attributeName.c_str(), buffer,
		sizeof(buffer));
	if (bytesRead < 0) {
		// On Linux only priviledged users are allowed to set attributes on
		// symlinks. So, if this is a symlink, we don't even try and instead
		// construct a symlink specific node ID.
		status_t error = errno;
		struct stat st;
		if (path == NULL || lstat(path, &st) < 0 || !S_ISLNK(st.st_mode))
			return error;

		char buffer[32];
		snprintf(buffer, sizeof(buffer), "symlink-%" B_PRIdINO, st.st_ino);
		_id = buffer;
		return B_OK;
	}

	_id = string(buffer, bytesRead);
	return B_OK;
}


static status_t
set_id_attribute(const char *path, int fd, const char* id)
{
	string attributeName(kAttributeNamespace);
	attributeName += kIDAttributeName;

	if (set_attribute(fd, path, attributeName.c_str(), id, strlen(id)) < 0)
		return errno;
	return B_OK;
}


static string
get_attribute_dir_path(node_ref ref, const char *path, int fd)
{
	string id;
	status_t error = get_id_attribute(path, fd, id);
	if (error != B_OK)
		id = "_no_attributes_";

	string attrDirPath(sAttributeDirBasePath);
	attrDirPath += '/';
	attrDirPath += id;
	return attrDirPath;
}


static status_t
get_attribute_dir_path_needed(node_ref ref, const char *path, int fd,
	string& _attrDirPath)
{
	string id;
	status_t error = get_id_attribute(path, fd, id);
	if (error != B_OK) {
		error = make_unique_node_id(id);
		if (error != B_OK)
			return error;

		error = set_id_attribute(path, fd, id.c_str());
		if (error != B_OK)
			return error;
	}

	_attrDirPath = sAttributeDirBasePath;
	_attrDirPath += '/';
	_attrDirPath += id;
	return B_OK;
}


#else


static string
get_attribute_dir_path(node_ref ref, const char *path, int fd)
{
	string attrDirPath(sAttributeDirBasePath);
	char buffer[32];
	sprintf(buffer, "/%" B_PRIdINO, ref.node);
	attrDirPath += buffer;
	return attrDirPath;
}


static status_t
get_attribute_dir_path_needed(node_ref ref, const char *path, int fd,
	string& _attrDirPath)
{
	_attrDirPath = get_attribute_dir_path(ref, path, fd);
	return B_OK;
}


#endif


// ensure_attribute_dir_exists
static status_t
ensure_attribute_dir_exists(node_ref ref, const char *path, int fd)
{
	// init the base directory and get the attribute directory path
	status_t error = init_attribute_dir_base_dir();
	if (error != B_OK)
		return error;

	string attrDirPath;
	error = get_attribute_dir_path_needed(ref, path, fd, attrDirPath);
	if (error != B_OK)
		return error;

	// stat the dir
	struct stat st;
	if (lstat(attrDirPath.c_str(), &st) == 0) {
		if (!S_ISDIR(st.st_mode)) {
			// the attribute dir is no directory
			fprintf(stderr, "ensure_attribute_dir_exists(): Attribute "
				"directory for node %lld exists, but is no directory!\n",
				(long long)ref.node);
			return B_FILE_ERROR;
		}

		return B_OK;
	}

	// doesn't exist yet: create it
	if (mkdir(attrDirPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) < 0)
		return errno;

	return B_OK;
}

// open_attr_dir
static DIR *
open_attr_dir(node_ref ref, const char *path, int fd)
{
	// make sure the directory exists
	status_t error = ensure_attribute_dir_exists(ref, path, fd);
	if (error != B_OK) {
		errno = error;
		return NULL;
	}

	// open it
	string dirPath(get_attribute_dir_path(ref, path, fd));
	return opendir(dirPath.c_str());
}

// get_attribute_path
static status_t
get_attribute_path(node_ref ref, const char *path, int fd,
	const char *attribute, string &attrPath, string &typePath)
{
	if (!attribute || strlen(attribute) == 0)
		return B_BAD_VALUE;

	// make sure the attribute dir for the node exits
	status_t error = ensure_attribute_dir_exists(ref, path, fd);
	if (error != B_OK) {
		errno = error;
		return -1;
	}

	// construct the attribute path
	attrPath = get_attribute_dir_path(ref, path, fd) + '/';
	string attrName(escape_attr_name(attribute));
	typePath = attrPath + "t" + attrName;
	attrPath += attrName;

	return B_OK;
}


// get_attribute_path_virtual_fd
static status_t
get_attribute_path_virtual_fd(int fd, const char *attribute, string &attrPath,
	string &typePath)
{
	// stat the file to get a node_ref
	struct stat st;
	status_t error = _kern_read_stat(fd, NULL, false, &st, sizeof(st));
	if (error != B_OK)
		return error;
	node_ref ref(st);

	// Try to get a path. If we can't get a path, this is must be a "real"
	// (i.e. system) file descriptor, which is just as well.
	string path;
	bool pathValid = (get_path(fd, NULL, path) == B_OK);

	// get the attribute path
	return get_attribute_path(ref, (pathValid ? path.c_str() : NULL),
		(pathValid ? -1 : fd), attribute, attrPath, typePath);
}


// get_attribute_path
static status_t
get_attribute_path(int fd, const char *attribute, string &attrPath,
	string &typePath)
{
	if (get_descriptor(fd)) {
		// This is a virtual file descriptor -- we have a special function
		// for handling it.
		return  get_attribute_path_virtual_fd(fd, attribute, attrPath,
			typePath);
	} else {
		// This is a real (i.e. system) file descriptor -- fstat() it and
		// build the path.

		// stat the file to get a node_ref
		struct stat st;
		if (fstat(fd, &st) < 0)
			return errno;
		node_ref ref(st);

		return get_attribute_path(ref, NULL, fd, attribute, attrPath, typePath);
	}
}


// # pragma mark - Public API


// fs_open_attr_dir
DIR *
fs_open_attr_dir(const char *path)
{
	struct stat st;
	if (lstat(path, &st))
		return NULL;

	return open_attr_dir(node_ref(st), path, -1);
}

// fs_fopen_attr_dir
DIR *
fs_fopen_attr_dir(int fd)
{
	struct stat st;

	status_t error = _kern_read_stat(fd, NULL, false, &st,
		sizeof(struct stat));
	if (error != B_OK) {
		errno = error;
		return NULL;
	}

	// Try to get a path. If we can't get a path, this is must be a "real"
	// (i.e. system) file descriptor, which is just as well.
	string path;
	bool pathValid = (get_path(fd, NULL, path) == B_OK);

	// get the attribute path
	return open_attr_dir(node_ref(st), (pathValid ? path.c_str() : NULL),
		(pathValid ? -1 : fd));
}

// fs_close_attr_dir
int
fs_close_attr_dir(DIR *dir)
{
	return closedir(dir);
}

// fs_read_attr_dir
struct dirent *
fs_read_attr_dir(DIR *dir)
{
	struct dirent *entry = NULL;
	while (true) {
		// read the next entry
		entry = readdir(dir);
		if (!entry)
			return NULL;

		// ignore administrative entries; the
		if (entry->d_name[0] == '_') {
			string attrName = deescape_attr_name(entry->d_name);
			strcpy(entry->d_name, attrName.c_str());
			return entry;
		}
	}
}

// fs_rewind_attr_dir
void
fs_rewind_attr_dir(DIR *dir)
{
	rewinddir(dir);
}

// fs_fopen_attr
int
fs_fopen_attr(int fd, const char *attribute, uint32 type, int openMode)
{
	if (!attribute) {
		errno = B_BAD_VALUE;
		return -1;
	}

	// get the attribute path
	string attrPath;
	string typePath;
	status_t error = get_attribute_path(fd, attribute, attrPath, typePath);
	if (error != B_OK) {
		errno = error;
		return -1;
	}

	// check, if the attribute already exists
	struct stat st;
	bool exists = (lstat(attrPath.c_str(), &st) == 0);

	// open the attribute
	int attrFD = open(attrPath.c_str(), openMode, S_IRWXU | S_IRWXG | S_IRWXO);
	if (attrFD < 0)
		return -1;

	// set the type, if the attribute didn't exist yet
	if (!exists) {
		// create a file prefixed "t"
		int typeFD = creat(typePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
		if (typeFD >= 0) {
			// write the type into the file
			if (write(typeFD, &type, sizeof(type)) < 0)
				error = errno;

			close(typeFD);

		} else
			error = errno;

		// remove type and attribute file, if something went wrong
		if (error != B_OK) {
			if (typeFD > 0) {
				unlink(typePath.c_str());
			}

			close(attrFD);
			unlink(attrPath.c_str());

			errno = error;
			return -1;
		}
	}

	return attrFD;
}

// fs_close_attr
int
fs_close_attr(int fd)
{
	return close(fd);
}

// fs_read_attr
ssize_t
fs_read_attr(int fd, const char *attribute, uint32 type, off_t pos,
	void *buffer, size_t readBytes)
{
	// open the attribute
	int attrFD = fs_fopen_attr(fd, attribute, type, O_RDONLY);
	if (attrFD < 0)
		return attrFD;

	// read
	ssize_t bytesRead = read_pos(attrFD, pos, buffer, readBytes);
	status_t error = errno;

	// close the attribute
	fs_close_attr(attrFD);

	if (bytesRead < 0) {
		errno = error;
		return -1;
	}

	return bytesRead;
}

// fs_write_attr
ssize_t
fs_write_attr(int fd, const char *attribute, uint32 type, off_t pos,
	const void *buffer, size_t readBytes)
{
	// open the attribute
	int attrFD = fs_fopen_attr(fd, attribute, type,
		O_WRONLY | O_CREAT | O_TRUNC);
	if (attrFD < 0) {
		// Setting user attributes on symlinks is not allowed (xattr). So, if
		// this is a symlink and we're only supposed to write a "BEOS:TYPE"
		// attribute we silently pretend to have succeeded.
		struct stat st;
		if (strcmp(attribute, "BEOS:TYPE") == 0 && fstat(fd, &st) == 0
			&& S_ISLNK(st.st_mode)) {
			return readBytes;
		}
		return attrFD;
	}

	// read
	ssize_t bytesWritten = write_pos(attrFD, pos, buffer, readBytes);
	status_t error = errno;

	// close the attribute
	fs_close_attr(attrFD);

	if (bytesWritten < 0) {
		errno = error;
		return -1;
	}

	return bytesWritten;
}

// fs_remove_attr
int
fs_remove_attr(int fd, const char *attribute)
{
	if (!attribute) {
		errno = B_BAD_VALUE;
		return -1;
	}

	// get the attribute path
	string attrPath;
	string typePath;
	status_t error = get_attribute_path(fd, attribute, attrPath, typePath);
	if (error != B_OK) {
		errno = error;
		return -1;
	}

	// remove the attribute
	if (unlink(attrPath.c_str()) < 0)
		return -1;

	unlink(typePath.c_str());

	return B_OK;
}

// fs_stat_attr
int
fs_stat_attr(int fd, const char *attribute, struct attr_info *attrInfo)
{
	if (!attribute || !attrInfo) {
		errno = B_BAD_VALUE;
		return -1;
	}

	// get the attribute path
	string attrPath;
	string typePath;
	status_t error = get_attribute_path(fd, attribute, attrPath, typePath);
	if (error != B_OK) {
		errno = error;
		return -1;
	}

	// stat the attribute file to get the size of the attribute
	struct stat st;
	if (lstat(attrPath.c_str(), &st) < 0)
		return -1;

	attrInfo->size = st.st_size;

	// now open the attribute type file and read the attribute's type
	int typeFD = open(typePath.c_str(), O_RDONLY);
	if (typeFD < 0)
		return -1;

	ssize_t bytesRead = read(typeFD, &attrInfo->type, sizeof(attrInfo->type));
	if (bytesRead < 0)
		error = errno;
	else if (bytesRead < (ssize_t)sizeof(attrInfo->type))
		error = B_FILE_ERROR;

	close(typeFD);

	// fail on error
	if (error != B_OK) {
		errno = error;
		return -1;
	}

	return 0;
}


// #pragma mark - Private Syscalls


// _kern_open_attr_dir
int
_kern_open_attr_dir(int fd, const char *path)
{
	// get node ref for the node
	struct stat st;
	status_t error = _kern_read_stat(fd, path, false, &st,
		sizeof(struct stat));
	if (error != B_OK) {
		errno = error;
		return -1;
	}
	node_ref ref(st);

	// If a path was given, get a usable path.
	string realPath;
	if (path) {
		error = get_path(fd, path, realPath);
		if (error != B_OK)
			return error;
	}

	// open the attr dir
	DIR *dir = open_attr_dir(ref, (path ? realPath.c_str() : NULL),
		(path ? -1 : fd));
	if (!dir)
		return errno;

	// create descriptor
	AttrDirDescriptor *descriptor = new AttrDirDescriptor(dir, ref);
	return add_descriptor(descriptor);
}

// _kern_rename_attr
status_t
_kern_rename_attr(int fromFile, const char *fromName, int toFile,
	const char *toName)
{
	if (!fromName || !toName)
		return B_BAD_VALUE;

	// get the attribute paths
	string fromAttrPath;
	string fromTypePath;
	status_t error = get_attribute_path_virtual_fd(fromFile, fromName,
		fromAttrPath, fromTypePath);
	if (error != B_OK)
		return error;

	string toAttrPath;
	string toTypePath;
	error = get_attribute_path_virtual_fd(toFile, toName, toAttrPath,
		toTypePath);
	if (error != B_OK)
		return error;

	// rename the attribute and type files
	if (rename(fromAttrPath.c_str(), toAttrPath.c_str()) < 0)
		return errno;

	if (rename(fromTypePath.c_str(), toTypePath.c_str()) < 0) {
		// renaming the type file failed: try to rename back the attribute file
		error = errno;

		rename(toAttrPath.c_str(), fromAttrPath.c_str());

		return error;
	}

	return B_OK;
}

// _kern_remove_attr
status_t
_kern_remove_attr(int fd, const char *name)
{
	if (!name)
		return B_BAD_VALUE;

	// get the attribute path
	string attrPath;
	string typePath;
	status_t error = get_attribute_path_virtual_fd(fd, name, attrPath,
		typePath);
	if (error != B_OK)
		return error;

	// remove the attribute
	if (unlink(attrPath.c_str()) < 0)
		return errno;

	unlink(typePath.c_str());

	return B_OK;
}


// __get_attribute_dir_path
extern "C" bool __get_attribute_dir_path(const struct stat* st,
	const char* path, char* buffer);
bool
__get_attribute_dir_path(const struct stat* st, const char* path, char* buffer)
{
	node_ref ref(*st);
	string dirPath = get_attribute_dir_path(ref, path, -1);
	strcpy(buffer, dirPath.c_str());
	return true;
}