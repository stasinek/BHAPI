﻿/*
 * Copyright 2001-2015 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_ROSTER_H
#define BHAPI_ROSTER_H

#ifdef __cplusplus
namespace bhapi {
#endif
// app flags
#define B_SINGLE_LAUNCH			(0x0)
#define B_MULTIPLE_LAUNCH		(0x1)
#define B_EXCLUSIVE_LAUNCH		(0x2)
#define B_LAUNCH_MASK			(0x3)
#define B_BACKGROUND_APP		(0x4)
#define B_ARGV_ONLY				(0x8)
#define _B_APP_INFO_RESERVED1_	(0x10000000)

// watching request flags
enum {
    B_REQUEST_LAUNCHED	= 0x00000001,
    B_REQUEST_QUIT		= 0x00000002,
    B_REQUEST_ACTIVATED	= 0x00000004,
};
// notification message "what"
enum {
    B_SOME_APP_LAUNCHED		= 'BRAS',
    B_SOME_APP_QUIT			= 'BRAQ',
    B_SOME_APP_ACTIVATED	= 'BRAW',
};
#ifdef __cplusplus
}
#endif

#ifndef BFILE_I
#define BFILE_I
class BFile;
#endif
#ifndef BODEINFO_I
#define BNODEINFO_I
class BNodeInfo;
#endif
#ifndef BMIMETYPE_I
#define BMIMETYPE_I
class BMimeType;
#endif
#ifndef BLIST_I
#define BLIST_I
class BList;
#endif
#include <kits/storage/Entry.h>
#include <kits/app/Messenger.h>
#include <Haiku.h>

#ifdef __cplusplus
namespace bhapi {
#endif
struct app_info {
    app_info();
    ~app_info();

    thread_id	thread;
    team_id		team;
    port_id		port;
    uint32_t		flags;
    entry_ref	ref;
    char		signature[B_MIME_TYPE_LENGTH];
};
#ifdef __cplusplus
}
#endif

class BHAPI_IMPEXP BRoster {
public:
                                BRoster();
                                ~BRoster();

    // running apps
            bool				IsRunning(const char* signature) const;
            bool				IsRunning(bhapi::entry_ref* ref) const;

            bhapi::team_id				TeamFor(const char* signature) const;
            bhapi::team_id				TeamFor(bhapi::entry_ref* ref) const;

            void				GetAppList(BList* teamIDList) const;
            void				GetAppList(const char* signature,
                                    BList* teamIDList) const;

    // app infos
            status_t			GetAppInfo(const char* signature,
                                    bhapi::app_info* info) const;
            status_t			GetAppInfo(bhapi::entry_ref* ref,
                                    bhapi::app_info* info) const;

            status_t			GetRunningAppInfo(team_id team,
                                    bhapi::app_info* info) const;
            status_t			GetActiveAppInfo(app_info* info) const;

    // find app
            status_t			FindApp(const char* mimeType,
                                    bhapi::entry_ref* app) const;
            status_t			FindApp(bhapi::entry_ref* ref, bhapi::entry_ref* app) const;

    // broadcast
            status_t			Broadcast(BMessage* message) const;
            status_t			Broadcast(BMessage* message,
                                    BMessenger replyTo) const;

    // watching
            status_t			StartWatching(BMessenger target,
                                    uint32_t eventMask
                                        = B_REQUEST_LAUNCHED
                                            | B_REQUEST_QUIT) const;
            status_t			StopWatching(BMessenger target) const;

            status_t			ActivateApp(bhapi::team_id team) const;

    // launch app
            status_t			Launch(const char* mimeType,
                                    BMessage* initialMessage = NULL,
                                    bhapi::team_id* _appTeam = NULL) const;
            status_t			Launch(const char* mimeType, BList* messageList,
                                    bhapi::team_id* _appTeam = NULL) const;
            status_t			Launch(const char* mimeType, int argc,
                                    const char* const* args,
                                    bhapi::team_id* _appTeam = NULL) const;
            status_t			Launch(const bhapi::entry_ref* ref,
                                    const BMessage* initialMessage = NULL,
                                    bhapi::team_id* _appTeam = NULL) const;
            status_t			Launch(const bhapi::entry_ref* ref, const BList* messageList, team_id* _appTeam = NULL) const;
            status_t			Launch(const bhapi::entry_ref* ref, int argc, const char* const* args, team_id* _appTeam = NULL) const;

    // recent documents, folders, apps
            void				GetRecentDocuments(BMessage* refList, int32_t maxCount, const char* fileType = NULL, const char* signature = NULL) const;
            void				GetRecentDocuments(BMessage* refList, int32_t maxCount, const char* fileTypes[], int32_t fileTypesCount, const char* signature = NULL) const;
            void				GetRecentFolders(BMessage* refList, int32_t maxCount, const char* signature = NULL) const;
            void				GetRecentApps(BMessage* refList,int32_t maxCount) const;

            void				AddToRecentDocuments(const bhapi::entry_ref* document, const char* signature = NULL) const;
            void				AddToRecentFolders(const bhapi::entry_ref* folder, const char* signature = NULL) const;

    // private/reserved stuff starts here
    class Private;

private:
    class ArgVector;
    friend class Private;

            status_t			_ShutDown(bool reboot, bool confirm, bool synchronous);

            status_t			_AddApplication(const char* signature, const bhapi::entry_ref* ref, uint32_t flags,bhapi::team_id team, bhapi::thread_id thread, bhapi::port_id port, bool fullRegistration, uint32_t* pToken, team_id* otherTeam) const;
            status_t			_SetSignature(bhapi::team_id team,const char* signature) const;
            void				_SetThread(bhapi::team_id team, bhapi::thread_id thread) const;
            status_t			_SetThreadAndTeam(uint32_t entryToken, bhapi::thread_id thread, team_id team, bhapi::port_id* _port) const;

            status_t			_CompleteRegistration(team_id team, thread_id thread, port_id port) const;

            bool				_IsAppPreRegistered(const bhapi::entry_ref* ref, team_id team, app_info* info) const;

            status_t			_IsAppRegistered(const bhapi::entry_ref* ref, team_id team, uint32_t token, bool* preRegistered, app_info* info) const;

            status_t			_RemovePreRegApp(uint32_t entryToken) const;
            status_t			_RemoveApp(team_id team) const;

            void				_ApplicationCrashed(team_id team);

            status_t			_LaunchApp(const char* mimeType,
                                    const bhapi::entry_ref* ref,
                                    const BList* messageList, int argc,
                                    const char* const* args,
                                    const char** environment,
                                    team_id* _appTeam, thread_id* _appThread,
                                    port_id* _appPort, uint32_t* _appToken,
                                    bool launchSuspended) const;

            status_t			_UpdateActiveApp(team_id team) const;

            void				_SetAppFlags(team_id team, uint32_t flags) const;

            void				_DumpRoster() const;

            status_t			_ResolveApp(const char* inType, bhapi::entry_ref* ref,
                                    bhapi::entry_ref* appRef,
                                    char* signature,
                                    uint32_t* appFlags,
                                    bool* wasDocument) const;

            status_t			_TranslateRef(bhapi::entry_ref* ref,
                                    BMimeType* appMeta, bhapi::entry_ref* appRef,
                                    BFile* appFile, bool* wasDocument) const;

            status_t			_TranslateType(const char* mimeType,
                                    BMimeType* appMeta, bhapi::entry_ref* appRef,
                                    BFile* appFile) const;

            status_t			_GetFileType(const bhapi::entry_ref* file,
                                    BNodeInfo* nodeInfo, char* mimeType) const;
            status_t			_SendToRunning(team_id team, int argc,
                                    const char* const* args,
                                    const BList* messageList,
                                    const bhapi::entry_ref* ref,
                                    bool readyToRun) const;

            void				_SetWithoutRegistrar(bool noRegistrar);

            void				_InitMessenger();

    static	status_t			_InitMimeMessenger(void* data);

            BMessenger&			_MimeMessenger();

            void				_AddToRecentApps(const char* signature) const;

            void				_ClearRecentDocuments() const;
            void				_ClearRecentFolders() const;
            void				_ClearRecentApps() const;
            void				_LoadRecentLists(const char* filename) const;
            void				_SaveRecentLists(const char* filename) const;

            BMessenger			fMessenger;
            BMessenger			fMimeMessenger;
            int32_t				fMimeMessengerInitOnce;
            bool				fNoRegistrar;
            uint32_t				_reserved[1];
};
// global BRoster instance
extern const BRoster *__be_roster, *be_roster;
#define BROSTER_DEF
#endif	// BHAPI_ROSTER_H
