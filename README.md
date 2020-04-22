# uAuditAnalyzer - Unmanarc's Auditd Analyzer

Author: Aarón Mizrachi <aaron@unmanarc.com>  
License: GPLv3  
  
This program is intended to receive, reorder and reconstruct the information comming from audisp+rsyslog log from linux servers.   
  
It's known that auditd (audisp+rsyslog) provides information like this:  

```
Mar 30 21:28:32 vnode03 audispd: node=vnode03.vuln type=SYSCALL msg=audit(1585625312.197:6437): arch=c000003e syscall=59 success=yes exit=0 a0=814f30 a1=809910 a2=815590 a3=7ffd7e48c460 items=2 ppid=132442 pid=132710 auid=0 uid=0 gid=0 euid=0 suid=0 fsuid=0 egid=0 sgid=0 fsgid=0 tty=pts0 ses=313 comm="ls" exe="/usr/bin/ls" subj=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023 key="L5_EXEC_BIN"
Mar 30 21:28:32 vnode03 audispd: node=vnode03.vuln type=EXECVE msg=audit(1585625312.197:6437): argc=4 a0="ls" a1="--color" a2="-l" a3="/etc"
Mar 30 21:28:32 vnode03 audispd: node=vnode03.vuln type=CWD msg=audit(1585625312.197:6437):  cwd="/home/root"
Mar 30 21:28:32 vnode03 audispd: node=vnode03.vuln type=PATH msg=audit(1585625312.197:6437): item=0 name="/usr/bin/ls" inode=1709809 dev=fd:00 mode=0100755 ouid=0 ogid=0 rdev=00:00 obj=system_u:object_r:bin_t:s0 objtype=NORMAL cap_fp=0000000000000000 cap_fi=0000000000000000 cap_fe=0 cap_fver=0
Mar 30 21:28:32 vnode03 audispd: node=vnode03.vuln type=PATH msg=audit(1585625312.197:6437): item=1 name="/lib64/ld-linux-x86-64.so.2" inode=1709066 dev=fd:00 mode=0100755 ouid=0 ogid=0 rdev=00:00 obj=system_u:object_r:ld_so_t:s0 objtype=NORMAL cap_fp=0000000000000000 cap_fi=0000000000000000 cap_fe=0 cap_fver=0
Mar 30 21:28:32 vnode03 audispd: node=vnode03.vuln type=PROCTITLE msg=audit(1585625312.197:6437): proctitle=6C73002D2D636F6C6F72002D6C002F657463
```

But this output came with many co-related but separated lines, and even some mixed events, causing analysis nightmares.

## Operational Scheme


### Local auditd data capture

NOTE: this guide is intended to be CentOS7/RHEL7 compatible. Other linux distributions have different audisp/rsyslog/logs paths.

The auditd messages are captured by the infamous audispd daemon and sent to the local rsyslog daemon.

This can be activated using the syslog audispd module (``/etc/audisp/plugins.d/syslog.conf``):
  
Changing:
```
active = no
```
For:
```
active = yes
```

Then, configure your rules in ``/etc/audit/rules.d/``, and restart your auditd daemon

```
systemctl restart auditd.service 
```

Now you should be capturing all the auditd information to syslog. check in ``/var/log``

### Sending local syslog to a remote/centralized server

You can actually send the all the rsyslog output to a centralized rsyslog server.  
  
 But, if you only want to send the audispd information to this analyzer and saving a copy for your legal records, you just need to create a rsyslog.d configuration, by example in ``/etc/rsyslog.d/00-remotes.conf``

```
:programname, isequal, "audispd"     action(type="omfwd" target="1.2.3.4" port="10514" protocol="tcp")
:programname, isequal, "audispd"     /var/log/audispd.log
:programname, isequal, "audispd"     ~
```

Don't forget to change 1.2.3.4 for your uauditdanalyzer server ip, secure the transmission with some encryption (stunnel, vpn, etc), and create the ``/var/log/audispd.log`` file.


## Filters/Ruleset 

## Execution

## Outputs

The processed JSON output will be something like this:

```
[
  {
    "CWD": [
      {
        "cwd": "/home/root"
      }
    ],
    "EXECVE": [
      {
        "a0": "ls",
        "a1": "--color",
        "a2": "-l",
        "a3": "/etc",
        "argc": "4"
      }
    ],
    "INFO": {
      "host": "vnode03.vuln",
      "id": 6437,
      "key": "L5_EXEC_BIN",
      "msecs": 197,
      "time": "Mon Mar 30 21:28:32 2020",
      "unixTime": 1585625312
    },
    "PATH": [
      {
        "cap_fe": "0",
        "cap_fi": "0000000000000000",
        "cap_fp": "0000000000000000",
        "cap_fver": "0",
        "dev": "fd:00",
        "inode": "1709809",
        "item": "0",
        "mode": "0100755",
        "name": "/usr/bin/ls",
        "obj": "system_u:object_r:bin_t:s0",
        "objtype": "NORMAL",
        "ogid": "0",
        "ouid": "0",
        "rdev": "00:00"
      },
      {
        "cap_fe": "0",
        "cap_fi": "0000000000000000",
        "cap_fp": "0000000000000000",
        "cap_fver": "0",
        "dev": "fd:00",
        "inode": "1709066",
        "item": "1",
        "mode": "0100755",
        "name": "/lib64/ld-linux-x86-64.so.2",
        "obj": "system_u:object_r:ld_so_t:s0",
        "objtype": "NORMAL",
        "ogid": "0",
        "ouid": "0",
        "rdev": "00:00"
      }
    ],
    "PROCTITLE": [
      {
        "proctitle": "ls --color -l /etc"
      }
    ],
    "SYSCALL": [
      {
        "a0": "814f30",
        "a1": "809910",
        "a2": "815590",
        "a3": "7ffd7e48c460",
        "arch": "c000003e",
        "auid": "0",
        "comm": "ls",
        "egid": "0",
        "euid": "0",
        "exe": "/usr/bin/ls",
        "exit": "0",
        "fsgid": "0",
        "fsuid": "0",
        "gid": "0",
        "items": "2",
        "key": "L5_EXEC_BIN",
        "pid": "132710",
        "ppid": "132442",
        "ses": "313",
        "sgid": "0",
        "subj": "unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023",
        "success": "yes",
        "suid": "0",
        "syscall": "59",
        "tty": "pts0",
        "uid": "0"
      }
    ]
  }
```

This is a JSON formatted event with all the event messages/lines integrated and properly decoded.

### Output JSON

We can use uauditdanalyzer for sending this processed JSON to another service (like ELK) using a per-line JSON message protocol.
  
for this, you should configure the service as follows in ``config.ini``:

```
########################################################
# Output data via JSON TCP Channel
[OUTPUT/JSONTCP]
# JSON-TCP Output Enabled/Disabled
Enabled=true
# transmition mode: 1: styled (useful for debugging), 0: one line per json
TransmitionMode=0
# Remote Host to connect
Host=127.0.0.1
# Remote port to connect
Port=9200
# Connection Timeout (in seconds) to mark a connection as failed
TimeoutSecs=10
# Sleep time (in seconds) when a connection failed to attempt a new connection
ReconnectSleepTimeInSecs=3
```

Just change the destination host and port given your needs, and restart the service.

## External Interactions

Reloading/Modifying things should not imply a full program reset, so the program includes a control unix socket to interact.

### Rules/Actions Reset

To reload all rules and actions:

``$ echo -n 'E' | nc -U /run/uauditanalyzer/control.sock``

To reload all actions:

``$ echo -n 'A' | nc -U /run/uauditanalyzer/control.sock``

To reload all rules:

``$ echo -n 'R' | nc -U /run/uauditanalyzer/control.sock``

### Answer

You should monitor the application for detailed output (eg. ``journalctl -xef``), and you also have to check the return code, ``0`` for error, and ``1`` for success.

## Requirements 

This should be built on top of:

- cxFramework libraries - https://github.com/unmanarc/cxFramework 
- POCO Libraries
- JSONCPP (AT LEAST v1.7.7, for RHEL7 you will have to build it by hand or install an external RPM)
- C++11

## Build

## Alternatives



