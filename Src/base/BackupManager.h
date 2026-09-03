/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
*      Copyright (c) 2026 webOS Ports
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include "Common.h"

#include <list>
#include <string>

#include <luna-service2/lunaservice.h>

struct LSHandle;
struct LSMessage;

/**
 * Serves com.palm.sysMgrDataBackup: the LunaSysMgr end of the platform
 * backup protocol described by /etc/palm/backup/com.palm.sysMgrDataBackup.
 *
 * A backup service walks /etc/palm/backup, calls preBackup on each registered
 * service, copies away whatever files come back, and calls postRestore with
 * that same list once it has put them back.
 */
class BackupManager
{
public:
    static BackupManager* instance();

    bool init(GMainLoop* mainLoop);

private:
    BackupManager();
    ~BackupManager();

    void initFilesForBackup();

    static bool preBackupCallback(LSHandle* lshandle, LSMessage* message, void* user_data);
    static bool postRestoreCallback(LSHandle* lshandle, LSMessage* message, void* user_data);

    static LSMethod s_backupServerMethods[];
    static BackupManager* s_instance;

    GMainLoop* m_mainLoop;
    LSHandle* m_service;

    std::list<std::string> m_backupFiles;
};

#endif // BACKUP_MANAGER_H
