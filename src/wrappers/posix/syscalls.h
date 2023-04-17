/*
 * Copyright 2004-2005, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KERNEL_SYSCALLS_H
#define _KERNEL_SYSCALLS_H


#include <kits/kernel/OS.h>
#include <image.h>
//#include <disk_device_manager/ddm_userland_interface.h>
#include <storage/DiskDeviceDefs.h>

//#include <sys/select.h>
#include <sys/uio.h>


#ifdef __cplusplus
extern "C" {
#endif

struct sigaction;
struct rlimit;
struct stat;
struct pollfd;
struct fs_info;
struct dirent;

// This marks the beginning of the syscalls prototypes for gensyscallinfos.
// NOTE:
// * Nothing but those prototypes may live here.
// * The arguments of the functions must be named to be processed properly.

#ifdef GEN_SYSCALL_INFOS_PROCESSING
#pragma syscalls begin
#endif

extern int     		_kern_null();
extern status_t		_kern_generic_syscall(const char *subsystem, uint32_t function,
						void *buffer, size_t bufferSize);

extern int			_kern_getrlimit(int resource, struct rlimit * rlp);
extern int			_kern_setrlimit(int resource, const struct rlimit * rlp);

extern status_t		_kern_shutdown(bool reboot);
extern status_t		_kern_get_safemode_option(const char *parameter, char *buffer, size_t *_bufferSize);

/* sem functions */
extern sem_id		_kern_create_sem(int count, const char *name);
extern status_t		_kern_delete_sem(sem_id id);
extern status_t		_kern_switch_sem(sem_id releaseSem, sem_id id);
extern status_t		_kern_switch_sem_etc(sem_id releaseSem, sem_id id, uint32_t count,
						uint32_t flags, bigtime_t timeout);
extern status_t		_kern_acquire_sem(sem_id id);
extern status_t		_kern_acquire_sem_etc(sem_id id, uint32_t count, uint32_t flags, bigtime_t timeout);
extern status_t		_kern_release_sem(sem_id id);
extern status_t		_kern_release_sem_etc(sem_id id, uint32_t count, uint32_t flags);
extern status_t		_kern_get_sem_count(sem_id id, int32_t* thread_count);
extern status_t		_kern_get_sem_info(sem_id semaphore, struct sem_info *info, size_t size);
extern status_t		_kern_get_next_sem_info(team_id team, uint32_t *cookie,
						struct sem_info *info, size_t size);
extern status_t		_kern_set_sem_owner(sem_id id, team_id proc);

/* team & thread syscalls */

// extern thread_id	_kern_load_image(int32_t argCount, const char **args,
// 						int32_t envCount, const char **envp, int32_t priority,
// 						uint32_t flags);
// extern void			_kern_exit_team(status_t returnValue);
// extern status_t		_kern_kill_team(team_id team);
// extern team_id		_kern_get_current_team();
// extern status_t		_kern_wait_for_team(team_id team, status_t *_returnCode);
// extern thread_id	_kern_wait_for_child(thread_id child, uint32_t flags,
// 						int32_t *_reason, status_t *_returnCode);
// extern status_t		_kern_exec(const char *path, int32_t argc, char * const *argv, int32_t envCount,
// 						char * const *environment);
// extern thread_id	_kern_fork(void);
// extern pid_t		_kern_process_info(pid_t process, int32_t which);
// extern pid_t		_kern_setpgid(pid_t process, pid_t group);
// extern pid_t		_kern_setsid(void);

// extern thread_id	_kern_spawn_thread(int32_t (*func)(thread_func, void *),
// 						const char *name, int32_t priority, void *data1, void *data2);
// extern thread_id	_kern_find_thread(const char *name);
// extern status_t		_kern_suspend_thread(thread_id thread);
// extern status_t		_kern_resume_thread(thread_id thread);
// extern status_t		_kern_rename_thread(thread_id thread, const char *newName);
// extern status_t		_kern_set_thread_priority(thread_id thread, int32_t newPriority);
// extern status_t		_kern_kill_thread(thread_id thread);
// extern void			_kern_exit_thread(status_t returnValue);
// extern status_t		_kern_wait_for_thread(thread_id thread, status_t *_returnCode);
// extern bool			_kern_has_data(thread_id thread);
// extern status_t		_kern_send_data(thread_id thread, int32_t code, const void *buffer, size_t bufferSize);
// extern int32_t		_kern_receive_data(thread_id *_sender, void *buffer, size_t bufferSize);
// extern int64		_kern_restore_signal_frame();

// extern status_t		_kern_get_thread_info(thread_id id, thread_info *info);
// extern status_t		_kern_get_next_thread_info(team_id team, int32_t *cookie, thread_info *info);
// extern status_t		_kern_get_team_info(team_id id, team_info *info);
// extern status_t		_kern_get_next_team_info(int32_t *cookie, team_info *info);
// extern status_t		_kern_get_team_usage_info(team_id team, int32_t who, team_usage_info *info, size_t size);

// signal functions
// extern int			_kern_send_signal(pid_t tid, uint sig);
// extern int			_kern_sigprocmask(int how, const sigset_t *set, sigset_t *oldSet);
// extern int			_kern_sigaction(int sig, const struct sigaction *action, struct sigaction *oldAction);
// extern bigtime_t	_kern_set_alarm(bigtime_t time, uint32_t mode);

// image functions
// extern image_id		_kern_register_image(image_info *info, size_t size);
// extern status_t		_kern_unregister_image(image_id id);
// extern void			_kern_image_relocated(image_id id);
// extern void			_kern_loading_app_failed(status_t error);
// extern status_t		_kern_get_image_info(image_id id, image_info *info, size_t size);
// extern status_t		_kern_get_next_image_info(team_id team, int32_t *cookie, image_info *info, size_t size);

// VFS functions
// extern status_t		_kern_mount(const char *path, const char *device,
// 						const char *fs_name, uint32_t flags, const char *args);
// extern status_t		_kern_unmount(const char *path, uint32_t flags);
// extern status_t		_kern_read_fs_info(dev_t device, struct fs_info *info);
// extern status_t		_kern_write_fs_info(dev_t device, const struct fs_info *info, int mask);
// extern dev_t		_kern_next_device(int32_t *_cookie);
// extern status_t		_kern_sync(void);
extern status_t		_kern_entry_ref_to_path(dev_t device, ino_t inode,
						const char *leaf, char *userPath, size_t pathLength);
extern int			_kern_open_entry_ref(dev_t device, ino_t inode, const char *name, int openMode, int perms);
extern int			_kern_open(int fd, const char *path, int openMode, int perms);
extern int			_kern_open_dir_entry_ref(dev_t device, ino_t inode, const char *name);
extern int			_kern_open_dir(int fd, const char *path);
extern int			_kern_open_parent_dir(int fd, char *name,
						size_t nameLength);
// extern status_t		_kern_fcntl(int fd, int op, uint32_t argument);
extern status_t		_kern_fsync(int fd);
extern off_t		_kern_seek(int fd, off_t pos, int seekType);
extern status_t		_kern_create_dir_entry_ref(dev_t device, ino_t inode, const char *name, int perms);
extern status_t		_kern_create_dir(int fd, const char *path, int perms);
// extern status_t		_kern_remove_dir(const char *path);
extern status_t		_kern_read_link(int fd, const char *path, char *buffer,
						size_t *_bufferSize);
// extern status_t		_kern_write_link(const char *path, const char *toPath);
extern status_t		_kern_create_symlink(int fd, const char *path,
						const char *toPath, int mode);
// extern status_t		_kern_create_link(const char *path, const char *toPath);
extern status_t		_kern_unlink(int fd, const char *path);
extern status_t		_kern_rename(int oldDir, const char *oldpath, int newDir,
						const char *newpath);
// extern status_t		_kern_access(const char *path, int mode);
// extern ssize_t		_kern_select(int numfds, fd_set *readSet, fd_set *writeSet, fd_set *errorSet,
// 						bigtime_t timeout, const sigset_t *sigMask);
// extern ssize_t		_kern_poll(struct pollfd *fds, int numfds, bigtime_t timeout);
extern int			_kern_open_attr_dir(int fd, const char *path);
// extern int			_kern_create_attr(int fd, const char *name, uint32_t type, int openMode);
// extern int			_kern_open_attr(int fd, const char *name, int openMode);
extern status_t		_kern_remove_attr(int fd, const char *name);
extern status_t		_kern_rename_attr(int fromFile, const char *fromName, int toFile, const char *toName);
// extern int			_kern_open_index_dir(dev_t device);
// extern status_t		_kern_create_index(dev_t device, const char *name, uint32_t type, uint32_t flags);
// extern status_t		_kern_read_index_stat(dev_t device, const char *name, struct stat *stat);
// extern status_t		_kern_remove_index(dev_t device, const char *name);
// extern status_t		_kern_getcwd(char *buffer, size_t size);
// extern status_t		_kern_setcwd(int fd, const char *path);
// extern int			_kern_open_query(dev_t device, const char *query, size_t queryLength,
// 						uint32_t flags, port_id port, int32_t token);

// file descriptor functions
extern ssize_t		_kern_read(int fd, off_t pos, void *buffer, size_t bufferSize);
//extern ssize_t		_kern_readv(int fd, off_t pos, const iovec *vecs, size_t count);
extern ssize_t		_kern_write(int fd, off_t pos, const void *buffer, size_t bufferSize);
//extern ssize_t		_kern_writev(int fd, off_t pos, const iovec *vecs, size_t count);
// extern status_t		_kern_ioctl(int fd, ulong cmd, void *data, size_t length);
extern ssize_t		_kern_read_dir(int fd, struct dirent *buffer, size_t bufferSize, uint32_t maxCount);
extern status_t		_kern_rewind_dir(int fd);
extern status_t		_kern_read_stat(int fd, const char *path, bool traverseLink,
						struct stat *stat, size_t statSize);
extern status_t		_kern_write_stat(int fd, const char *path,
						bool traverseLink, const struct stat *stat,
						size_t statSize, int statMask);
extern status_t		_kern_close(int fd);
extern int			_kern_dup(int fd);
// extern int			_kern_dup2(int ofd, int nfd);
extern status_t		_kern_lock_node(int fd);
extern status_t		_kern_unlock_node(int fd);

// node monitor functions
// extern status_t		_kern_stop_notifying(port_id port, uint32_t token);
// extern status_t		_kern_start_watching(dev_t device, ino_t node, uint32_t flags,
// 						port_id port, uint32_t token);
// extern status_t		_kern_stop_watching(dev_t device, ino_t node, uint32_t flags,
// 						port_id port, uint32_t token);

// time functions
// extern status_t		_kern_set_real_time_clock(uint32_t time);
// extern status_t		_kern_set_timezone(int32_t timezoneOffset, bool daylightSavingTime);
// extern status_t		_kern_set_tzfilename(const char *filename, size_t length, bool isGMT);
// extern status_t		_kern_get_tzfilename(char *filename, size_t length, bool *_isGMT);
// 
// extern bigtime_t	_kern_system_time();
// extern status_t		_kern_snooze_etc(bigtime_t time, int timebase, int32_t flags);

// area functions
// extern area_id		_kern_create_area(const char *name, void **address, uint32_t addressSpec,
// 						size_t size, uint32_t lock, uint32_t protection);
// extern status_t		_kern_delete_area(area_id area);
// extern area_id		_kern_area_for(void *address);
// extern area_id		_kern_find_area(const char *name);
// extern status_t		_kern_get_area_info(area_id area, area_info *info);
// extern status_t		_kern_get_next_area_info(team_id team, int32_t *cookie, area_info *info);
// extern status_t		_kern_resize_area(area_id area, size_t newSize);
// extern area_id		_kern_transfer_area(area_id area, void **_address, uint32_t addressSpec,
// 						team_id target);
// extern status_t		_kern_set_area_protection(area_id area, uint32_t newProtection);
// extern area_id		_kern_clone_area(const char *name, void **_address, uint32_t addressSpec, 
// 						uint32_t protection, area_id sourceArea);
// extern status_t		_kern_init_heap_address_range(addr_t base, addr_t size);
// 
// area_id sys_vm_map_file(const char *name, void **address, int addr_type,
// 			addr_t size, int lock, int mapping, const char *path, off_t offset);

/* kernel port functions */
// extern port_id		_kern_create_port(int32_t queue_length, const char *name);
// extern status_t		_kern_close_port(port_id id);
// extern status_t		_kern_delete_port(port_id id);
// extern port_id		_kern_find_port(const char *port_name);
// extern status_t		_kern_get_port_info(port_id id, struct port_info *info);
// extern status_t	 	_kern_get_next_port_info(team_id team, uint32_t *cookie, struct port_info *info);
// extern ssize_t		_kern_port_buffer_size_etc(port_id port, uint32_t flags, bigtime_t timeout);
// extern int32_t		_kern_port_count(port_id port);
// extern ssize_t		_kern_read_port_etc(port_id port, int32_t *msgCode, void *msgBuffer,
// 						size_t bufferSize, uint32_t flags, bigtime_t timeout);
// extern status_t		_kern_set_port_owner(port_id port, team_id team);
// extern status_t		_kern_write_port_etc(port_id port, int32_t msgCode, const void *msgBuffer,
// 						size_t bufferSize, uint32_t flags, bigtime_t timeout);
// extern status_t		_kern_writev_port_etc(port_id id, int32_t msgCode, const iovec *msgVecs,
// 						size_t vecCount, size_t bufferSize, uint32_t flags, bigtime_t timeout);

// debug support functions
// extern void			_kern_debugger(const char *message);
// extern int			_kern_disable_debugger(int state);
// 
// extern status_t		_kern_install_default_debugger(port_id debuggerPort);
// extern port_id		_kern_install_team_debugger(team_id team,
// 						port_id debuggerPort);
// extern status_t		_kern_remove_team_debugger(team_id team);
// extern status_t		_kern_debug_thread(thread_id thread);
// extern void			_kern_wait_for_debugger(void);


/* atomic_* ops (needed for CPUs that don't support them directly) */
// #ifdef ATOMIC_FUNCS_ARE_SYSCALLS
// extern int32_t		_kern_atomic_set(vint32_t *value, int32_t newValue);
// extern int32_t		_kern_atomic_test_and_set(vint32_t *value, int32_t newValue, int32_t testAgainst);
// extern int32_t		_kern_atomic_add(vint32_t *value, int32_t addValue);
// extern int32_t		_kern_atomic_and(vint32_t *value, int32_t andValue);
// extern int32_t		_kern_atomic_or(vint32_t *value, int32_t orValue);	
// extern int32_t		_kern_atomic_get(vint32_t *value);
// #endif	// ATOMIC_FUNCS_ARE_SYSCALLS

// #ifdef ATOMIC64_FUNCS_ARE_SYSCALLS
// extern int64		_kern_atomic_set64(vint64 *value, int64 newValue);
// extern int64		_kern_atomic_test_and_set64(vint64 *value, int64 newValue, int64 testAgainst);
// extern int64		_kern_atomic_add64(vint64 *value, int64 addValue);
// extern int64		_kern_atomic_and64(vint64 *value, int64 andValue);
// extern int64		_kern_atomic_or64(vint64 *value, int64 orValue);	
// extern int64		_kern_atomic_get64(vint64 *value);
// #endif	// ATOMIC64_FUNCS_ARE_SYSCALLS

// int sys_sysctl(int *name, uint namlen, void *oldp, size_t *oldlen,
// 		void *newp, size_t newlen);
// int sys_socket(int family, int type, int proto);

/* System informations */
// extern status_t		_kern_get_system_info(system_info *info, size_t size);

/* Debug output */
// extern void			_kern_debug_output(const char *message);
// extern status_t		_kern_frame_buffer_update(addr_t baseAddress, int32_t width,
// 						int32_t height, int32_t depth, int32_t bytesPerRow);

/* messaging service */
// extern area_id		_kern_register_messaging_service(sem_id lockingSem,
// 						sem_id counterSem);
// extern status_t		_kern_unregister_messaging_service();
// 
// extern void			_kern_clear_caches(void *address, size_t length, uint32_t flags);

// #ifdef __INTEL__
// // our only x86 only syscall
// extern status_t		_kern_get_cpuid(cpuid_info *info, uint32_t eax, uint32_t cpu);
// #endif


/* Disk Device Manager syscalls */

// iterating, retrieving device/partition data
extern partition_id	_kern_get_next_disk_device_id(int32_t *cookie, size_t *neededSize);
extern partition_id	_kern_find_disk_device(const char *filename, size_t *neededSize);
extern partition_id	_kern_find_partition(const char *filename, size_t *neededSize);
extern status_t		_kern_get_disk_device_data(partition_id deviceID, bool deviceOnly,
						bool shadow, struct user_disk_device_data *buffer,
						size_t bufferSize, size_t *neededSize);
extern partition_id	_kern_register_file_device(const char *filename);
extern status_t		_kern_unregister_file_device(partition_id deviceID,
						const char *filename);
	// Only a valid deviceID or filename need to be passed. The other one
	// is -1/NULL. If both is given only filename is ignored.

// disk systems
extern status_t		_kern_get_disk_system_info(disk_system_id id,
						struct user_disk_system_info *info);
extern status_t		_kern_get_next_disk_system_info(int32_t *cookie,
						struct user_disk_system_info *info);
extern status_t		_kern_find_disk_system(const char *name,
						struct user_disk_system_info *info);
extern bool			_kern_supports_defragmenting_partition(partition_id partitionID,
						int32_t changeCounter, bool *whileMounted);
extern bool			_kern_supports_repairing_partition(partition_id partitionID,
						int32_t changeCounter, bool checkOnly, bool *whileMounted);
extern bool			_kern_supports_resizing_partition(partition_id partitionID,
						int32_t changeCounter, bool *canResizeContents, bool *whileMounted);
extern bool			_kern_supports_moving_partition(partition_id partitionID,
						int32_t changeCounter, partition_id *unmovable,
						partition_id *needUnmounting, size_t bufferSize);
extern bool			_kern_supports_setting_partition_name(partition_id partitionID,
						int32_t changeCounter);
extern bool			_kern_supports_setting_partition_content_name(partition_id partitionID,
						int32_t changeCounter, bool *whileMounted);
extern bool			_kern_supports_setting_partition_type(partition_id partitionID,
						int32_t changeCounter);
extern bool			_kern_supports_setting_partition_parameters(partition_id partitionID,
						int32_t changeCounter);
extern bool			_kern_supports_setting_partition_content_parameters(
						partition_id partitionID, int32_t changeCounter, bool *whileMounted);
extern bool			_kern_supports_initializing_partition(partition_id partitionID,
						int32_t changeCounter, const char *diskSystemName);
extern bool			_kern_supports_creating_child_partition(partition_id partitionID,
						int32_t changeCounter);
extern bool			_kern_supports_deleting_child_partition(partition_id partitionID,
						int32_t changeCounter);
extern bool			_kern_is_sub_disk_system_for(disk_system_id diskSystemID,
						partition_id partitionID, int32_t changeCounter);

extern status_t		_kern_validate_resize_partition(partition_id partitionID,
						int32_t changeCounter, off_t *size);
extern status_t		_kern_validate_move_partition(partition_id partitionID,
						int32_t changeCounter, off_t *newOffset);
extern status_t		_kern_validate_set_partition_name(partition_id partitionID,
						int32_t changeCounter, char *name);
extern status_t		_kern_validate_set_partition_content_name(partition_id partitionID,
						int32_t changeCounter, char *name);
extern status_t		_kern_validate_set_partition_type(partition_id partitionID,
						int32_t changeCounter, const char *type);
extern status_t		_kern_validate_initialize_partition(partition_id partitionID,
						int32_t changeCounter, const char *diskSystemName,
						char *name, const char *parameters, size_t parametersSize);
extern status_t		_kern_validate_create_child_partition(partition_id partitionID,
						int32_t changeCounter, off_t *offset, off_t *size,
						const char *type, const char *parameters,
						size_t parametersSize);
extern status_t		_kern_get_partitionable_spaces(partition_id partitionID,
						int32_t changeCounter,
						struct partitionable_space_data *buffer, int32_t count,
						int32_t *actualCount);
extern status_t		_kern_get_next_supported_partition_type(partition_id partitionID,
						int32_t changeCounter, int32_t *cookie, char *type);
extern status_t		_kern_get_partition_type_for_content_type(disk_system_id diskSystemID,
						const char *contentType, char *type);

// disk device modification
extern status_t		_kern_prepare_disk_device_modifications(partition_id deviceID);
extern status_t		_kern_commit_disk_device_modifications(partition_id deviceID,
						port_id port, int32_t token, bool completeProgress);
extern status_t		_kern_cancel_disk_device_modifications(partition_id deviceID);
extern bool			_kern_is_disk_device_modified(partition_id deviceID);
extern status_t		_kern_defragment_partition(partition_id partitionID,
						int32_t changeCounter);
extern status_t		_kern_repair_partition(partition_id partitionID, int32_t changeCounter,
						bool checkOnly);
extern status_t		_kern_resize_partition(partition_id partitionID, int32_t changeCounter,
						off_t size);
extern status_t		_kern_move_partition(partition_id partitionID, int32_t changeCounter,
						off_t newOffset);
extern status_t		_kern_set_partition_name(partition_id partitionID,
						int32_t changeCounter, const char *name);
extern status_t		_kern_set_partition_content_name(partition_id partitionID,
						int32_t changeCounter, const char *name);
extern status_t		_kern_set_partition_type(partition_id partitionID,
						int32_t changeCounter, const char *type);
extern status_t		_kern_set_partition_parameters(partition_id partitionID,
						int32_t changeCounter, const char *parameters,
						size_t parametersSize);
extern status_t		_kern_set_partition_content_parameters(partition_id partitionID,
						int32_t changeCounter, const char *parameters,
						size_t parametersSize);
extern status_t		_kern_initialize_partition(partition_id partitionID,
						int32_t changeCounter, const char *diskSystemName,
						const char *name, const char *parameters,
						size_t parametersSize);
extern status_t		_kern_uninitialize_partition(partition_id partitionID,
						int32_t changeCounter);
extern status_t		_kern_create_child_partition(partition_id partitionID,
						int32_t changeCounter, off_t offset, off_t size, const char *type,
						const char *parameters, size_t parametersSize,
						partition_id *childID);
extern status_t		_kern_delete_partition(partition_id partitionID, int32_t changeCounter);
extern status_t		_kern_delete_child_partition(partition_id partitionID,
						int32_t* changeCounter, partition_id childID,
						int32_t childChangeCounter);

// jobs
extern status_t		_kern_get_next_disk_device_job_info(int32_t *cookie,
						struct user_disk_device_job_info *info);
extern status_t		_kern_get_disk_device_job_info(disk_job_id id,
						struct user_disk_device_job_info *info);
extern status_t		_kern_get_disk_device_job_progress_info(disk_job_id id,
						struct disk_device_job_progress_info *info);
extern status_t		_kern_pause_disk_device_job(disk_job_id id);
extern status_t		_kern_cancel_disk_device_job(disk_job_id id, bool reverse);

#if 0

// watching
status_t start_disk_device_watching(port_id, int32_t token, uint32_t flags);
status_t start_disk_device_job_watching(disk_job_id job, port_id, int32_t token,
										uint32_t flags);
status_t stop_disk_device_watching(port_id, int32_t token);

#endif	// 0


// The end mark for gensyscallinfos.
#ifdef GEN_SYSCALL_INFOS_PROCESSING
#pragma syscalls end
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _KERNEL_SYSCALLS_H */
