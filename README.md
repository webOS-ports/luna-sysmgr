luna-sysmgr
===========

**This component has been dissolved.** The webOS System Manager began as the
single process that ran the whole webOS user experience; LG stripped the UI
out of it upstream, LuneOS moved the shell to luna-next-cardshell and the
window server to the compositor, and what remained here was four small
services sharing one process name. They are now four daemons:

| Functionality | Now lives in | Bus name |
|---|---|---|
| Display states, brightness, ALS, suspend handshake | [luna-displaymanager](https://github.com/webOS-ports/luna-displaymanager) | `com.palm.display`, `com.palm.ambientLightSensor`, `com.palm.SuspendBlocker*` |
| Device lock, EAS policy, keymanager sync | [luna-authmanager](https://github.com/webOS-ports/luna-authmanager) | `com.palm.systemmanager`, `com.webos.service.auth` |
| Vibration | [luna-haptics](https://github.com/webOS-ports/luna-haptics) | `com.palm.vibrate` |
| Preference-file backup | [luna-backupagent](https://github.com/webOS-ports/luna-backupagent) | `com.webos.service.backupagent` (was `com.palm.sysMgrDataBackup`) |
| System sounds (`/usr/palm/sounds`) | [luneos-system-sounds](https://github.com/webOS-ports/luneos-system-sounds) | — |

Long before that, other parts of this repository had already become:

* `com.palm.applicationManager` / `com.palm.appinstaller` → [luna-appmanager](https://github.com/webOS-ports/luna-appmanager)
* The shell and window management → [luna-next-cardshell](https://github.com/webOS-ports/luna-next-cardshell) and luna-next
* Web app running → WebAppMgr, then WAM
* The shared library → [luna-sysmgr-common](https://github.com/webOS-ports/luna-sysmgr-common), which lives on

The full history of every file is preserved in this repository's git
history; the `webOS-ports/master` branch holds the last complete tree, and
`master` holds the openwebos original.

# Copyright and License Information

All content, including all source code files and documentation files in
this repository except otherwise noted are:

Copyright (c) 2008-2013 LG Electronics, Inc.
Copyright (c) 2026 Herman van Hazendonk <github.com@herrie.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this content except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
