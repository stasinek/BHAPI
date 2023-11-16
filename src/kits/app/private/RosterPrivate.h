/*
 * Copyright 2001-2015, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ingo Weinhold (bonefish@users.sf.net)
 */
#ifndef _ROSTER_PRIVATE_H
#define _ROSTER_PRIVATE_H


#include <kits/app/Messenger.h>
#include <kits/netRoster.h>


const int32_t kMsgAppServerRestarted = 'ASRe';
const int32_t kMsgRestartAppServer = 'ReAS';


class BRoster::Private {
    public:
        Private() : fRoster(const_cast<BRoster*>(__be_roster)) {}
        Private(BRoster &roster) : fRoster(&roster) {}
        Private(BRoster *roster) : fRoster(roster) {}

        void SetTo(BMessenger mainMessenger, BMessenger mimeMessenger);

        status_t SendTo(BMessage *message, BMessage *reply, bool mime);
        bool IsMessengerValid(bool mime) const;

        status_t Launch(const char* mimeType, const bhapi::entry_ref* ref,
                    const BList* messageList, int argc, const char* const* args,
                    const char** environment, team_id* appTeam,
                    thread_id* appThread, port_id* appPort, uint32_t* appToken,
                    bool launchSuspended)
            { return fRoster->_LaunchApp(mimeType, ref, messageList, argc,
                    args, environment, appTeam, appThread, appPort, appToken,
                    launchSuspended); }

        status_t ShutDown(bool reboot, bool confirm, bool synchronous)
            { return fRoster->_ShutDown(reboot, confirm, synchronous); }

        // needed by BApplication

        status_t AddApplication(const char *mimeSig, const bhapi::entry_ref *ref,
                    uint32_t flags, team_id team, thread_id thread,
                    port_id port, bool fullReg, uint32_t *token,
                    team_id *otherTeam) const
            { return fRoster->_AddApplication(mimeSig, ref, flags, team, thread,
                    port, fullReg, token, otherTeam); }

        status_t SetSignature(team_id team, const char *mimeSig) const
            { return fRoster->_SetSignature(team, mimeSig); }

        status_t CompleteRegistration(team_id team, thread_id thread,
                    port_id port) const
            { return fRoster->_CompleteRegistration(team, thread, port); }

        status_t IsAppRegistered(const bhapi::entry_ref *ref, team_id team,
                    uint32_t token, bool *preRegistered, app_info *info) const
            { return fRoster->_IsAppRegistered(ref, team, token, preRegistered,
                    info); }

        void SetWithoutRegistrar(bool noRegistrar) const
            { fRoster->_SetWithoutRegistrar(noRegistrar); }

        status_t RemoveApp(team_id team) const
            { return fRoster->_RemoveApp(team); }

        // needed by GetRecentTester

        void AddToRecentApps(const char *appSig) const
            { fRoster->_AddToRecentApps(appSig); }

        void ClearRecentDocuments() const
            { fRoster->_ClearRecentDocuments(); }

        void ClearRecentFolders() const
            { fRoster->_ClearRecentFolders(); }

        void ClearRecentApps() const
            { fRoster->_ClearRecentApps(); }

        void LoadRecentLists(const char *file) const
            { fRoster->_LoadRecentLists(file); }

        void SaveRecentLists(const char *file) const
            { fRoster->_SaveRecentLists(file); }

        // needed by the debug server
        void ApplicationCrashed(team_id team) const
            { fRoster->_ApplicationCrashed(team); }

        void UpdateActiveApp(team_id team) const
            { fRoster->_UpdateActiveApp(team); }

        static void InitBeRoster();
        static void DeleteBeRoster();

    private:
        BRoster	*fRoster;
};

#endif	// _ROSTER_PRIVATE_H
