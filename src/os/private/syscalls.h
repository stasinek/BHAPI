/*
 * Copyright 2004-2015, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_SYSTEM_SYSCALLS_H
#define BHAPI_SYSTEM_SYSCALLS_H

#include <DiskDeviceDefs.h>
#include <elf_private.h>
#include <image.h>
#include <kits/kernel/OS.h>
#include <signal.h>
//#include <sys/socket.h>

// POSIX
#define key_t int32
#define sigset_t uint32
#define siginfo_t uint32
#define stack_t uint32
#define socklen_t uint32

struct attr_info;
struct dirent;
struct fd_info;
struct fd_set;
struct fs_info;
struct iovec;
struct msqid_ds;
struct net_stat;
struct pollfd;
struct rlimit;
struct scheduling_analysis;
struct sem_t;
struct sembuf;
 union semun;
struct sigaction;
struct signal_frame_data;
struct stat;
struct system_profiler_parameters;
struct user_timer_info;

struct disk_device_job_progress_info;
struct partitionable_space_data;
struct thread_creation_attributes;
struct user_disk_device_data;
struct user_disk_device_job_info;
struct user_disk_system_info;
using namespace bhapi;

#include <sys/stat.h>

// This marks the beginning of the syscalls prototypes for gensyscallinfos.
// NOTE:
// * Nothing but those prototypes may live here.
// * The arguments of the functions must be named to be processed properly.

#ifdef GEN_SYSCALL_INFOS_PROCESSING
#	define __NO_RETURN
#	pragma syscalls begin
#else
#	define __NO_RETURN	__attribute__((noreturn))
#endif

BHAPI_IMPEXP int			_kern_is_computer_on(void);
BHAPI_IMPEXP status_t		_kern_generic_syscall(const char *subsystem, uint32 function,
                        void *buffer, size_t bufferSize);

BHAPI_IMPEXP int			_kern_getrlimit(int resource, struct rlimit * rlp);
BHAPI_IMPEXP int			_kern_setrlimit(int resource, const struct rlimit * rlp);

BHAPI_IMPEXP status_t		_kern_shutdown(bool reboot);
BHAPI_IMPEXP status_t		_kern_get_safemode_option(const char *parameter,
                        char *buffer, size_t *_bufferSize);

BHAPI_IMPEXP ssize_t		_kern_wait_for_objects(object_wait_info* infos, int numInfos,
                        uint32 flags, bigtime_t timeout);

/* user mutex functions */
BHAPI_IMPEXP status_t		_kern_mutex_lock(int32* mutex, const char* name,
                        uint32 flags, bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_mutex_unlock(int32* mutex, uint32 flags);
BHAPI_IMPEXP status_t		_kern_mutex_switch_lock(int32* fromMutex, int32* toMutex,
                        const char* name, uint32 flags, bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_mutex_sem_acquire(int32* sem, const char* name,
                        uint32 flags, bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_mutex_sem_release(int32* sem);

/* sem functions */
BHAPI_IMPEXP sem_id		_kern_create_sem(int count, const char *name);
BHAPI_IMPEXP status_t		_kern_delete_sem(sem_id id);
BHAPI_IMPEXP status_t		_kern_switch_sem(sem_id releaseSem, sem_id id);
BHAPI_IMPEXP status_t		_kern_switch_sem_etc(sem_id releaseSem, sem_id id,
                        uint32 count, uint32 flags, bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_acquire_sem(sem_id id);
BHAPI_IMPEXP status_t		_kern_acquire_sem_etc(sem_id id, uint32 count, uint32 flags,
                        bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_release_sem(sem_id id);
BHAPI_IMPEXP status_t		_kern_release_sem_etc(sem_id id, uint32 count, uint32 flags);
BHAPI_IMPEXP status_t		_kern_get_sem_count(sem_id id, int32* thread_count);
BHAPI_IMPEXP status_t		_kern_get_sem_info(sem_id semaphore, struct sem_info *info,
                        size_t size);
BHAPI_IMPEXP status_t		_kern_get_next_sem_info(team_id team, int32 *cookie,
                        struct sem_info *info, size_t size);
BHAPI_IMPEXP status_t		_kern_set_sem_owner(sem_id id, team_id proc);

/* POSIX realtime sem syscalls */
BHAPI_IMPEXP status_t		_kern_realtime_sem_open(const char* name,
                        int openFlagsOrShared, mode_t mode, uint32 semCount,
                        struct _sem_t* userSem, struct _sem_t** _usedUserSem);
BHAPI_IMPEXP status_t		_kern_realtime_sem_close(sem_id semID,
                        struct _sem_t** _deleteUserSem);
BHAPI_IMPEXP status_t		_kern_realtime_sem_unlink(const char* name);

BHAPI_IMPEXP status_t		_kern_realtime_sem_get_value(sem_id semID, int* value);
BHAPI_IMPEXP status_t		_kern_realtime_sem_post(sem_id semID);
BHAPI_IMPEXP status_t		_kern_realtime_sem_wait(sem_id semID, bigtime_t timeout);

/* POSIX XSI semaphore syscalls */
BHAPI_IMPEXP int			_kern_xsi_semget(key_t key, int numSems, int flags);
BHAPI_IMPEXP int			_kern_xsi_semctl(int semID, int semNumber, int command,
                        union semun* args);
BHAPI_IMPEXP status_t		_kern_xsi_semop(int semID, struct sembuf *semOps,
                        size_t numSemOps);

/* POSIX XSI message queue syscalls */
BHAPI_IMPEXP int			_kern_xsi_msgctl(int messageQueueID, int command,
                        struct msqid_ds *buffer);
BHAPI_IMPEXP int			_kern_xsi_msgget(key_t key, int messageQueueFlags);
BHAPI_IMPEXP ssize_t		_kern_xsi_msgrcv(int messageQueueID, void *messagePointer,
                        size_t messageSize, long messageType, int messageFlags);
BHAPI_IMPEXP int			_kern_xsi_msgsnd(int messageQueueID,
                        const void *messagePointer, size_t messageSize,
                        int messageFlags);

/* team & thread syscalls */
BHAPI_IMPEXP thread_id	_kern_load_image(const char* const* flatArgs,
                        size_t flatArgsSize, int32 argCount, int32 envCount,
                        int32 priority, uint32 flags, port_id errorPort,
                        uint32 errorToken);
BHAPI_IMPEXP void __NO_RETURN _kern_exit_team(status_t returnValue);
BHAPI_IMPEXP status_t		_kern_kill_team(team_id team);
BHAPI_IMPEXP team_id		_kern_get_current_team();
BHAPI_IMPEXP status_t		_kern_wait_for_team(team_id team, status_t *_returnCode);
BHAPI_IMPEXP pid_t		_kern_wait_for_child(thread_id child, uint32 flags,
                        siginfo_t* info);
BHAPI_IMPEXP status_t		_kern_exec(const char *path, const char* const* flatArgs,
                        size_t flatArgsSize, int32 argCount, int32 envCount,
                        mode_t umask);
BHAPI_IMPEXP thread_id	_kern_fork(void);
BHAPI_IMPEXP pid_t		_kern_process_info(pid_t process, int32 which);
BHAPI_IMPEXP pid_t		_kern_setpgid(pid_t process, pid_t group);
BHAPI_IMPEXP pid_t		_kern_setsid(void);
BHAPI_IMPEXP status_t		_kern_change_root(const char *path);

BHAPI_IMPEXP thread_id	_kern_spawn_thread(
                        struct thread_creation_attributes* attributes);
BHAPI_IMPEXP thread_id	_kern_find_thread(const char *name);
BHAPI_IMPEXP status_t		_kern_suspend_thread(thread_id thread);
BHAPI_IMPEXP status_t		_kern_resume_thread(thread_id thread);
BHAPI_IMPEXP status_t		_kern_rename_thread(thread_id thread, const char *newName);
BHAPI_IMPEXP status_t		_kern_set_thread_priority(thread_id thread,
                        int32 newPriority);
BHAPI_IMPEXP status_t		_kern_kill_thread(thread_id thread);
BHAPI_IMPEXP void			_kern_exit_thread(status_t returnValue);
BHAPI_IMPEXP status_t		_kern_cancel_thread(thread_id threadID,
                        void (*cancelFunction)(int));
BHAPI_IMPEXP void			_kern_thread_yield(void);
BHAPI_IMPEXP status_t		_kern_wait_for_thread(thread_id thread,
                        status_t *_returnCode);
BHAPI_IMPEXP bool			_kern_has_data(thread_id thread);
BHAPI_IMPEXP status_t		_kern_send_data(thread_id thread, int32 code,
                        const void *buffer, size_t bufferSize);
BHAPI_IMPEXP int32		_kern_receive_data(thread_id *_sender, void *buffer,
                        size_t bufferSize);
BHAPI_IMPEXP int64		_kern_restore_signal_frame(
                        struct signal_frame_data* signalFrameData);

BHAPI_IMPEXP status_t		_kern_get_thread_info(thread_id id, thread_info *info);
BHAPI_IMPEXP status_t		_kern_get_next_thread_info(team_id team, int32 *cookie,
                        thread_info *info);
BHAPI_IMPEXP status_t		_kern_get_team_info(team_id id, team_info *info);
BHAPI_IMPEXP status_t		_kern_get_next_team_info(int32 *cookie, team_info *info);
BHAPI_IMPEXP status_t		_kern_get_team_usage_info(team_id team, int32 who,
                        team_usage_info *info, size_t size);
BHAPI_IMPEXP status_t		_kern_get_extended_team_info(team_id teamID, uint32 flags,
                        void* buffer, size_t size, size_t* _sizeNeeded);

BHAPI_IMPEXP status_t		_kern_start_watching_system(int32 object, uint32 flags,
                        port_id port, int32 token);
BHAPI_IMPEXP status_t		_kern_stop_watching_system(int32 object, uint32 flags,
                        port_id port, int32 token);

BHAPI_IMPEXP status_t		_kern_block_thread(uint32 flags, bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_unblock_thread(thread_id thread, status_t status);
BHAPI_IMPEXP status_t		_kern_unblock_threads(thread_id* threads, uint32 count,
                        status_t status);

BHAPI_IMPEXP bigtime_t	_kern_estimate_max_scheduling_latency(thread_id thread);

BHAPI_IMPEXP status_t		_kern_set_scheduler_mode(int32 mode);
BHAPI_IMPEXP int32		_kern_get_scheduler_mode(void);

// user/group functions
BHAPI_IMPEXP gid_t		_kern_getgid(bool effective);
BHAPI_IMPEXP uid_t		_kern_getuid(bool effective);
BHAPI_IMPEXP status_t		_kern_setregid(gid_t rgid, gid_t egid,
                        bool setAllIfPrivileged);
BHAPI_IMPEXP status_t		_kern_setreuid(uid_t ruid, uid_t euid,
                        bool setAllIfPrivileged);
BHAPI_IMPEXP ssize_t		_kern_getgroups(int groupCount, gid_t* groupList);
BHAPI_IMPEXP status_t		_kern_setgroups(int groupCount, const gid_t* groupList);

// signal functions
BHAPI_IMPEXP status_t		_kern_send_signal(int32 id, uint32 signal,
                        const union sigval* userValue, uint32 flags);
BHAPI_IMPEXP status_t		_kern_set_signal_mask(int how, const sigset_t *set,
                        sigset_t *oldSet);
BHAPI_IMPEXP status_t		_kern_sigaction(int sig, const struct sigaction *action,
                        struct sigaction *oldAction);
BHAPI_IMPEXP status_t		_kern_sigwait(const sigset_t *set, siginfo_t *info,
                        uint32 flags, bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_sigsuspend(const sigset_t *mask);
BHAPI_IMPEXP status_t		_kern_sigpending(sigset_t *set);
BHAPI_IMPEXP status_t		_kern_set_signal_stack(const stack_t *newStack,
                        stack_t *oldStack);

// image functions
BHAPI_IMPEXP image_id		_kern_register_image(image_info *info, size_t size);
BHAPI_IMPEXP status_t		_kern_unregister_image(image_id id);
BHAPI_IMPEXP void			_kern_image_relocated(image_id id);
BHAPI_IMPEXP void			_kern_loading_app_failed(status_t error);
BHAPI_IMPEXP status_t		_kern_get_image_info(image_id id, image_info *info,
                        size_t size);
BHAPI_IMPEXP status_t		_kern_get_next_image_info(team_id team, int32 *cookie,
                        image_info *info, size_t size);
BHAPI_IMPEXP status_t		_kern_read_kernel_image_symbols(image_id id,
                        elf_sym* symbolTable, int32* _symbolCount,
                        char* stringTable, size_t* _stringTableSize,
                        addr_t* _imageDelta);

// VFS functions
BHAPI_IMPEXP dev_t		_kern_mount(const char *path, const char *device,
                        const char *fs_name, uint32 flags, const char *args,
                        size_t argsLength);
BHAPI_IMPEXP status_t		_kern_unmount(const char *path, uint32 flags);
BHAPI_IMPEXP status_t		_kern_read_fs_info(dev_t device, struct fs_info *info);
BHAPI_IMPEXP status_t		_kern_write_fs_info(dev_t device, const struct fs_info *info,
                        int mask);
BHAPI_IMPEXP dev_t		_kern_next_device(int32 *_cookie);
BHAPI_IMPEXP status_t		_kern_sync(void);
BHAPI_IMPEXP status_t		_kern_entry_ref_to_path(dev_t device, ino_t inode,
                        const char *leaf, char *userPath, size_t pathLength);
BHAPI_IMPEXP status_t		_kern_normalize_path(const char* userPath,
                        bool traverseLink, char* buffer);
BHAPI_IMPEXP int			_kern_open_entry_ref(dev_t device, ino_t inode,
                        const char *name, int openMode, int perms);
BHAPI_IMPEXP int			_kern_open(int fd, const char *path, int openMode,
                        int perms);
BHAPI_IMPEXP int			_kern_open_dir_entry_ref(dev_t device, ino_t inode,
                        const char *name);
BHAPI_IMPEXP int			_kern_open_dir(int fd, const char *path);
BHAPI_IMPEXP int			_kern_open_parent_dir(int fd, char *name,
                        size_t nameLength);
BHAPI_IMPEXP status_t		_kern_fcntl(int fd, int op, size_t argument);
BHAPI_IMPEXP status_t		_kern_fsync(int fd);
BHAPI_IMPEXP status_t		_kern_flock(int fd, int op);
BHAPI_IMPEXP off_t		_kern_seek(int fd, off_t pos, int seekType);
BHAPI_IMPEXP status_t		_kern_create_dir_entry_ref(dev_t device, ino_t inode,
                        const char *name, int perms);
BHAPI_IMPEXP status_t		_kern_create_dir(int fd, const char *path, int perms);
BHAPI_IMPEXP status_t		_kern_remove_dir(int fd, const char *path);
BHAPI_IMPEXP status_t		_kern_read_link(int fd, const char *path, char *buffer,
                        size_t *_bufferSize);
BHAPI_IMPEXP status_t		_kern_create_symlink(int fd, const char *path,
                        const char *toPath, int mode);
BHAPI_IMPEXP status_t		_kern_create_link(int pathFD, const char *path, int toFD,
                        const char *toPath, bool traverseLeafLink);
BHAPI_IMPEXP status_t		_kern_unlink(int fd, const char *path);
BHAPI_IMPEXP status_t		_kern_rename(int oldDir, const char *oldpath, int newDir,
                        const char *newpath);
BHAPI_IMPEXP status_t		_kern_create_fifo(int fd, const char *path, mode_t perms);
BHAPI_IMPEXP status_t		_kern_create_pipe(int *fds);
BHAPI_IMPEXP status_t		_kern_access(int fd, const char *path, int mode,
                        bool effectiveUserGroup);
BHAPI_IMPEXP ssize_t		_kern_select(int numfds, struct fd_set *readSet,
                        struct fd_set *writeSet, struct fd_set *errorSet,
                        bigtime_t timeout, const sigset_t *sigMask);
BHAPI_IMPEXP ssize_t		_kern_poll(struct pollfd *fds, int numFDs,
                        bigtime_t timeout);

BHAPI_IMPEXP int			_kern_open_attr_dir(int fd, const char *path,
                        bool traverseLeafLink);
BHAPI_IMPEXP ssize_t		_kern_read_attr(int fd, const char *attribute, off_t pos,
                        void *buffer, size_t readBytes);
BHAPI_IMPEXP ssize_t		_kern_write_attr(int fd, const char *attribute, uint32 type,
                        off_t pos, const void *buffer, size_t readBytes);
BHAPI_IMPEXP status_t		_kern_stat_attr(int fd, const char *attribute,
                        struct attr_info *attrInfo);
BHAPI_IMPEXP int			_kern_open_attr(int fd, const char* path, const char *name,
                        uint32 type, int openMode);
BHAPI_IMPEXP status_t		_kern_remove_attr(int fd, const char *name);
BHAPI_IMPEXP status_t		_kern_rename_attr(int fromFile, const char *fromName,
                        int toFile, const char *toName);
BHAPI_IMPEXP int			_kern_open_index_dir(dev_t device);
BHAPI_IMPEXP status_t		_kern_create_index(dev_t device, const char *name,
                        uint32 type, uint32 flags);
BHAPI_IMPEXP status_t		_kern_read_index_stat(dev_t device, const char *name,
                        struct stat *stat);
BHAPI_IMPEXP status_t		_kern_remove_index(dev_t device, const char *name);
BHAPI_IMPEXP status_t		_kern_getcwd(char *buffer, size_t size);
BHAPI_IMPEXP status_t		_kern_setcwd(int fd, const char *path);
BHAPI_IMPEXP int			_kern_open_query(dev_t device, const char *query,
                        size_t queryLength, uint32 flags, port_id port,
                        int32 token);

// file descriptor functions
BHAPI_IMPEXP ssize_t		_kern_read(int fd, off_t pos, void *buffer,
                        size_t bufferSize);
BHAPI_IMPEXP ssize_t		_kern_readv(int fd, off_t pos, const struct iovec *vecs,
                        size_t count);
BHAPI_IMPEXP ssize_t		_kern_write(int fd, off_t pos, const void *buffer,
                        size_t bufferSize);
BHAPI_IMPEXP ssize_t		_kern_writev(int fd, off_t pos, const struct iovec *vecs,
                        size_t count);
BHAPI_IMPEXP status_t		_kern_ioctl(int fd, uint32 cmd, void *data, size_t length);
BHAPI_IMPEXP ssize_t		_kern_read_dir(int fd, struct dirent *buffer,
                        size_t bufferSize, uint32 maxCount);
BHAPI_IMPEXP status_t		_kern_rewind_dir(int fd);
BHAPI_IMPEXP status_t		_kern_read_stat(int fd, const char *path, bool traverseLink,
                        struct stat *stat, size_t statSize);
BHAPI_IMPEXP status_t		_kern_write_stat(int fd, const char *path,
                        bool traverseLink, const struct stat *stat,
                        size_t statSize, int statMask);
BHAPI_IMPEXP status_t		_kern_close(int fd);
BHAPI_IMPEXP int			_kern_dup(int fd);
BHAPI_IMPEXP int			_kern_dup2(int ofd, int nfd);
BHAPI_IMPEXP status_t		_kern_lock_node(int fd);
BHAPI_IMPEXP status_t		_kern_unlock_node(int fd);
BHAPI_IMPEXP status_t		_kern_get_next_fd_info(team_id team, uint32 *_cookie,
                        struct fd_info *info, size_t infoSize);

// socket functions
BHAPI_IMPEXP int			_kern_socket(int family, int type, int protocol);
BHAPI_IMPEXP status_t		_kern_bind(int socket, const struct sockaddr *address,
                        socklen_t addressLength);
BHAPI_IMPEXP status_t		_kern_shutdown_socket(int socket, int how);
BHAPI_IMPEXP status_t		_kern_connect(int socket, const struct sockaddr *address,
                        socklen_t addressLength);
BHAPI_IMPEXP status_t		_kern_listen(int socket, int backlog);
BHAPI_IMPEXP int			_kern_accept(int socket, struct sockaddr *address,
                        socklen_t *_addressLength);
BHAPI_IMPEXP ssize_t		_kern_recv(int socket, void *data, size_t length,
                        int flags);
BHAPI_IMPEXP ssize_t		_kern_recvfrom(int socket, void *data, size_t length,
                        int flags, struct sockaddr *address,
                        socklen_t *_addressLength);
BHAPI_IMPEXP ssize_t		_kern_recvmsg(int socket, struct msghdr *message,
                        int flags);
BHAPI_IMPEXP ssize_t		_kern_send(int socket, const void *data, size_t length,
                        int flags);
BHAPI_IMPEXP ssize_t		_kern_sendto(int socket, const void *data, size_t length,
                        int flags, const struct sockaddr *address,
                        socklen_t addressLength);
BHAPI_IMPEXP ssize_t		_kern_sendmsg(int socket, const struct msghdr *message,
                        int flags);
BHAPI_IMPEXP status_t		_kern_getsockopt(int socket, int level, int option,
                        void *value, socklen_t *_length);
BHAPI_IMPEXP status_t		_kern_setsockopt(int socket, int level, int option,
                        const void *value, socklen_t length);
BHAPI_IMPEXP status_t		_kern_getpeername(int socket, struct sockaddr *address,
                        socklen_t *_addressLength);
BHAPI_IMPEXP status_t		_kern_getsockname(int socket, struct sockaddr *address,
                        socklen_t *_addressLength);
BHAPI_IMPEXP int			_kern_sockatmark(int socket);
BHAPI_IMPEXP status_t		_kern_socketpair(int family, int type, int protocol,
                        int *socketVector);
BHAPI_IMPEXP status_t		_kern_get_next_socket_stat(int family, uint32 *cookie,
                        struct net_stat *stat);

// node monitor functions
BHAPI_IMPEXP status_t		_kern_stop_notifying(port_id port, uint32 token);
BHAPI_IMPEXP status_t		_kern_start_watching(dev_t device, ino_t node, uint32 flags,
                        port_id port, uint32 token);
BHAPI_IMPEXP status_t		_kern_stop_watching(dev_t device, ino_t node, port_id port,
                        uint32 token);

// time functions
BHAPI_IMPEXP status_t		_kern_set_real_time_clock(bigtime_t time);
BHAPI_IMPEXP status_t		_kern_set_timezone(int32 timezoneOffset, const char *name,
                        size_t nameLength);
BHAPI_IMPEXP status_t		_kern_get_timezone(int32 *_timezoneOffset, char *name,
                        size_t nameLength);
BHAPI_IMPEXP status_t		_kern_set_real_time_clock_is_gmt(bool isGMT);
BHAPI_IMPEXP status_t		_kern_get_real_time_clock_is_gmt(bool *_isGMT);

BHAPI_IMPEXP status_t		_kern_get_clock(clockid_t clockID, bigtime_t* _time);
BHAPI_IMPEXP status_t		_kern_set_clock(clockid_t clockID, bigtime_t time);

BHAPI_IMPEXP bigtime_t	_kern_system_time();
BHAPI_IMPEXP status_t		_kern_snooze_etc(bigtime_t time, int timebase, int32 flags,
                        bigtime_t* _remainingTime);

BHAPI_IMPEXP int32		_kern_create_timer(clockid_t clockID, thread_id threadID,
                        uint32 flags, const struct sigevent* event,
                        const struct thread_creation_attributes*
                            threadAttributes);
BHAPI_IMPEXP status_t		_kern_delete_timer(int32 timerID, thread_id threadID);
BHAPI_IMPEXP status_t		_kern_get_timer(int32 timerID, thread_id threadID,
                        struct user_timer_info* info);
BHAPI_IMPEXP status_t		_kern_set_timer(int32 timerID, thread_id threadID,
                        bigtime_t startTime, bigtime_t interval, uint32 flags,
                        struct user_timer_info* oldInfo);

// area functions
BHAPI_IMPEXP area_id		_kern_create_area(const char *name, void **address,
                        uint32 addressSpec, size_t size, uint32 lock,
                        uint32 protection);
BHAPI_IMPEXP status_t		_kern_delete_area(area_id area);
BHAPI_IMPEXP area_id		_kern_area_for(void *address);
BHAPI_IMPEXP area_id		_kern_find_area(const char *name);
BHAPI_IMPEXP status_t		_kern_get_area_info(area_id area, area_info *info);
BHAPI_IMPEXP status_t		_kern_get_next_area_info(team_id team, ssize_t *cookie,
                        area_info *info);
BHAPI_IMPEXP status_t		_kern_resize_area(area_id area, size_t newSize);
BHAPI_IMPEXP area_id		_kern_transfer_area(area_id area, void **_address,
                        uint32 addressSpec, team_id target);
BHAPI_IMPEXP status_t		_kern_set_area_protection(area_id area,
                        uint32 newProtection);
BHAPI_IMPEXP area_id		_kern_clone_area(const char *name, void **_address,
                        uint32 addressSpec, uint32 protection,
                        area_id sourceArea);
BHAPI_IMPEXP status_t		_kern_reserve_address_range(addr_t* _address,
                        uint32 addressSpec, addr_t size);
BHAPI_IMPEXP status_t		_kern_unreserve_address_range(addr_t address, addr_t size);

BHAPI_IMPEXP area_id		_kern_map_file(const char *name, void **address,
                        uint32 addressSpec, size_t size, uint32 protection,
                        uint32 mapping, bool unmapAddressRange, int fd,
                        off_t offset);
BHAPI_IMPEXP status_t		_kern_unmap_memory(void *address, size_t size);
BHAPI_IMPEXP status_t		_kern_set_memory_protection(void *address, size_t size,
                        uint32 protection);
BHAPI_IMPEXP status_t		_kern_sync_memory(void *address, size_t size, int flags);
BHAPI_IMPEXP status_t		_kern_memory_advice(void *address, size_t size,
                        uint32 advice);

BHAPI_IMPEXP status_t		_kern_get_memory_properties(team_id teamID,
                        const void *address, uint32* _protected, uint32* _lock);

/* kernel port functions */
BHAPI_IMPEXP port_id		_kern_create_port(int32 queue_length, const char *name);
BHAPI_IMPEXP status_t		_kern_close_port(port_id id);
BHAPI_IMPEXP status_t		_kern_delete_port(port_id id);
BHAPI_IMPEXP port_id		_kern_find_port(const char *port_name);
BHAPI_IMPEXP status_t		_kern_get_port_info(port_id id, struct port_info *info);
BHAPI_IMPEXP status_t		_kern_get_next_port_info(team_id team, int32 *cookie,
                        struct port_info *info);
BHAPI_IMPEXP ssize_t		_kern_port_buffer_size_etc(port_id port, uint32 flags,
                        bigtime_t timeout);
BHAPI_IMPEXP int32		_kern_port_count(port_id port);
BHAPI_IMPEXP ssize_t		_kern_read_port_etc(port_id port, int32 *msgCode,
                        void *msgBuffer, size_t bufferSize, uint32 flags,
                        bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_set_port_owner(port_id port, team_id team);
BHAPI_IMPEXP status_t		_kern_write_port_etc(port_id port, int32 msgCode,
                        const void *msgBuffer, size_t bufferSize, uint32 flags,
                        bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_writev_port_etc(port_id id, int32 msgCode,
                        const struct iovec *msgVecs, size_t vecCount,
                        size_t bufferSize, uint32 flags, bigtime_t timeout);
BHAPI_IMPEXP status_t		_kern_get_port_message_info_etc(port_id port,
                        port_message_info *info, size_t infoSize, uint32 flags,
                        bigtime_t timeout);

// debug support functions
BHAPI_IMPEXP status_t		_kern_kernel_debugger(const char *message);
BHAPI_IMPEXP void			_kern_register_syslog_daemon(port_id port);
BHAPI_IMPEXP void			_kern_debugger(const char *message);
BHAPI_IMPEXP int			_kern_disable_debugger(int state);

BHAPI_IMPEXP status_t		_kern_install_default_debugger(port_id debuggerPort);
BHAPI_IMPEXP port_id		_kern_install_team_debugger(team_id team,
                        port_id debuggerPort);
BHAPI_IMPEXP status_t		_kern_remove_team_debugger(team_id team);
BHAPI_IMPEXP status_t		_kern_debug_thread(thread_id thread);
BHAPI_IMPEXP void			_kern_wait_for_debugger(void);

BHAPI_IMPEXP status_t		_kern_set_debugger_breakpoint(void *address, uint32 type,
                        int32 length, bool watchpoint);
BHAPI_IMPEXP status_t		_kern_clear_debugger_breakpoint(void *address,
                        bool watchpoint);

BHAPI_IMPEXP status_t		_kern_system_profiler_start(
                        struct system_profiler_parameters* parameters);
BHAPI_IMPEXP status_t		_kern_system_profiler_next_buffer(size_t bytesRead,
                        uint64* _droppedEvents);
BHAPI_IMPEXP status_t		_kern_system_profiler_stop();
BHAPI_IMPEXP status_t		_kern_system_profiler_recorded(
                        struct system_profiler_parameters* parameters);

/* atomic_* ops (needed for CPUs that don't support them directly) */
#ifdef ATOMIC_FUNCS_ARE_SYSCALLS
BHAPI_IMPEXP void		_kern_atomic_set(int32 *value, int32 newValue);
BHAPI_IMPEXP int32		_kern_atomic_get_and_set(int32 *value, int32 newValue);
BHAPI_IMPEXP int32		_kern_atomic_test_and_set(int32 *value, int32 newValue,
                        int32 testAgainst);
BHAPI_IMPEXP int32		_kern_atomic_add(int32 *value, int32 addValue);
BHAPI_IMPEXP int32		_kern_atomic_and(int32 *value, int32 andValue);
BHAPI_IMPEXP int32		_kern_atomic_or(int32 *value, int32 orValue);
BHAPI_IMPEXP int32		_kern_atomic_get(int32 *value);
#endif	// ATOMIC_FUNCS_ARE_SYSCALLS

#ifdef ATOMIC64_FUNCS_ARE_SYSCALLS
BHAPI_IMPEXP void		_kern_atomic_set64(int64 *value, int64 newValue);
BHAPI_IMPEXP int64		_kern_atomic_get_and_set64(int64 *value, int64 newValue);
BHAPI_IMPEXP int64		_kern_atomic_test_and_set64(int64 *value, int64 newValue,
                        int64 testAgainst);
BHAPI_IMPEXP int64		_kern_atomic_add64(int64 *value, int64 addValue);
BHAPI_IMPEXP int64		_kern_atomic_and64(int64 *value, int64 andValue);
BHAPI_IMPEXP int64		_kern_atomic_or64(int64 *value, int64 orValue);
BHAPI_IMPEXP int64		_kern_atomic_get64(int64 *value);
#endif	// ATOMIC64_FUNCS_ARE_SYSCALLS

/* System informations */
BHAPI_IMPEXP status_t		_kern_get_system_info(system_info* info);
BHAPI_IMPEXP status_t		_kern_get_cpu_info(uint32 firstCPU, uint32 cpuCount,
                        cpu_info* info);
BHAPI_IMPEXP status_t		_kern_get_cpu_topology_info(
                        cpu_topology_node_info* topologyInfos,
                        uint32* topologyInfoCount);

BHAPI_IMPEXP status_t		_kern_analyze_scheduling(bigtime_t from, bigtime_t until,
                        void* buffer, size_t size,
                        struct scheduling_analysis* analysis);

/* Debug output */
BHAPI_IMPEXP void			_kern_debug_output(const char *message);
BHAPI_IMPEXP void			_kern_ktrace_output(const char *message);
BHAPI_IMPEXP status_t		_kern_frame_buffer_update(addr_t baseAddress, int32 width,
                        int32 height, int32 depth, int32 bytesPerRow);

/* messaging service */
BHAPI_IMPEXP area_id		_kern_register_messaging_service(sem_id lockingSem,
                        sem_id counterSem);
BHAPI_IMPEXP status_t		_kern_unregister_messaging_service();

BHAPI_IMPEXP void			_kern_clear_caches(void *address, size_t length,
                        uint32 flags);
BHAPI_IMPEXP bool			_kern_cpu_enabled(int32 cpu);
BHAPI_IMPEXP status_t		_kern_set_cpu_enabled(int32 cpu, bool enabled);

#if defined(__INTEL__) || defined(__x86_64__)
// our only x86 only syscall
BHAPI_IMPEXP status_t		_kern_get_cpuid(cpuid_info *info, uint32 eax, uint32 cpu);
#endif


/* Disk Device Manager syscalls */

// iterating, retrieving device/partition data
BHAPI_IMPEXP partition_id	_kern_get_next_disk_device_id(int32 *cookie,
                        size_t *neededSize);
BHAPI_IMPEXP partition_id	_kern_find_disk_device(const char *filename,
                        size_t *neededSize);
BHAPI_IMPEXP partition_id	_kern_find_partition(const char *filename,
                        size_t *neededSize);
BHAPI_IMPEXP partition_id	_kern_find_file_disk_device(const char *filename,
                        size_t *neededSize);
BHAPI_IMPEXP status_t		_kern_get_disk_device_data(partition_id deviceID,
                        bool deviceOnly, struct user_disk_device_data *buffer,
                        size_t bufferSize, size_t *neededSize);
BHAPI_IMPEXP partition_id	_kern_register_file_device(const char *filename);
BHAPI_IMPEXP status_t		_kern_unregister_file_device(partition_id deviceID,
                        const char *filename);
    // Only a valid deviceID or filename need to be passed. The other one
    // is -1/NULL. If both is given only filename is ignored.
BHAPI_IMPEXP status_t		_kern_get_file_disk_device_path(partition_id id,
                        char* buffer, size_t bufferSize);

// disk systems
BHAPI_IMPEXP status_t		_kern_get_disk_system_info(disk_system_id id,
                        struct user_disk_system_info *info);
BHAPI_IMPEXP status_t		_kern_get_next_disk_system_info(int32 *cookie,
                        struct user_disk_system_info *info);
BHAPI_IMPEXP status_t		_kern_find_disk_system(const char *name,
                        struct user_disk_system_info *info);

// disk device modification
BHAPI_IMPEXP status_t		_kern_defragment_partition(partition_id partitionID,
                        int32* changeCounter);
BHAPI_IMPEXP status_t		_kern_repair_partition(partition_id partitionID,
                        int32* changeCounter, bool checkOnly);
BHAPI_IMPEXP status_t		_kern_resize_partition(partition_id partitionID,
                        int32* changeCounter, partition_id childID,
                        int32* childChangeCounter, off_t size,
                        off_t contentSize);
BHAPI_IMPEXP status_t		_kern_move_partition(partition_id partitionID,
                        int32* changeCounter, partition_id childID,
                        int32* childChangeCounter, off_t newOffset,
                        partition_id* descendantIDs,
                        int32* descendantChangeCounters, int32 descendantCount);
BHAPI_IMPEXP status_t		_kern_set_partition_name(partition_id partitionID,
                        int32* changeCounter, partition_id childID,
                        int32* childChangeCounter, const char* name);
BHAPI_IMPEXP status_t		_kern_set_partition_content_name(partition_id partitionID,
                        int32* changeCounter, const char* name);
BHAPI_IMPEXP status_t		_kern_set_partition_type(partition_id partitionID,
                        int32* changeCounter, partition_id childID,
                        int32* childChangeCounter, const char* type);
BHAPI_IMPEXP status_t		_kern_set_partition_parameters(partition_id partitionID,
                        int32* changeCounter, partition_id childID,
                        int32* childChangeCounter, const char* parameters);
BHAPI_IMPEXP status_t		_kern_set_partition_content_parameters(
                        partition_id partitionID, int32* changeCounter,
                        const char* parameters);
BHAPI_IMPEXP status_t		_kern_initialize_partition(partition_id partitionID,
                        int32* changeCounter, const char* diskSystemName,
                        const char* name, const char* parameters);
BHAPI_IMPEXP status_t		_kern_uninitialize_partition(partition_id partitionID,
                        int32* changeCounter);
BHAPI_IMPEXP status_t		_kern_create_child_partition(partition_id partitionID,
                        int32* changeCounter, off_t offset, off_t size,
                        const char* type, const char* name,
                        const char* parameters, partition_id* childID,
                        int32* childChangeCounter);
BHAPI_IMPEXP status_t		_kern_delete_child_partition(partition_id partitionID,
                        int32* changeCounter, partition_id childID,
                        int32 childChangeCounter);

// disk change notification
BHAPI_IMPEXP status_t		_kern_start_watching_disks(uint32 eventMask, port_id port,
                        int32 token);
BHAPI_IMPEXP status_t		_kern_stop_watching_disks(port_id port, int32 token);


// The end mark for gensyscallinfos.
#ifdef GEN_SYSCALL_INFOS_PROCESSING
#pragma syscalls end
#endif

#undef __NO_RETURN

#ifdef __cplusplus
#endif

#endif	/* _SYSTEM_SYSCALLS_H */
