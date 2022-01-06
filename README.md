# uAuditAnalyzer2 - Unmanarc's Auditd Analyzer

![uAuditAnalyzer](art/logo.jpg)

Author: Aarón Mizrachi <aaron@unmanarc.com>  
License: GPLv3  

uAuditAnalyzer2 is intended to be a highly-efficient application for processing auditd logs received via rsyslog. It is being actively used in many enterprise-level environments for detecting potential threats and providing instant alerts using messaging applications, bots or other means.

## Applications

### uanlz_log2json

This application is in charge of receiving the auditd logs through a centralized RSYSLOG connection. After that, log2json reassembles the disaggregated auditd messages into a single JSON message that can be sent via TCP to some downstream system (eg ELK, uAuditanalyzer Alerts, etc)

### uanlz_alert

uanlz_alert receives a TCP stream with 1 JSON per line in order to filter and execute actions (usually alerting).

uanlz_alert uses a filter system based on a JSONPath mechanism integrated with an expression evaluation mechanism provided by the libMantids library, which allows comparison with advanced regular expressions, among other things.

### uanlz_web

uanlz_web was designed to manage and monitor the health of uAuditAnalyzer services. It is designed in HTML5 + JQUERY + BOOTSTRAP, using a webservices backend made in C ++ provided by the libMantids library.



***
## Installing packages (HOWTO)


- [Manual build guide](BUILD.md)
- COPR Packages (Fedora/CentOS/RHEL/etc):  
[![Copr build status](https://copr.fedorainfracloud.org/coprs/amizrachi/uAuditAnalyzer2/package/uAuditAnalyzer2/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/amizrachi/uAuditAnalyzer2/package/uAuditAnalyzer2/)

### Simple installation guide for Fedora/RHEL:

- First, proceed to install EPEL in your distribution (https://docs.fedoraproject.org/en-US/epel/), sometimes this is required for jsoncpp.

- Then, proceed to activate our repo's and download/install uAuditAnalyzer2:
```bash
# NOTE: for RHEL7 replace dnf by yum
dnf copr enable amizrachi/libMantids
dnf copr enable amizrachi/uAuditAnalyzer2

dnf -y install uAuditAnalyzer2
```

- Don't forget to open the firewall:

```bash
# EG. Log listener @10514... 
firewall-cmd --zone=public --permanent --add-port 10514/tcp
# if you require another port, please repeat the last line modifiying the port
firewall-cmd --reload
```

- You must create an application called **UAUDITANALYZER** in your uFastAuthD daemon (using the `uFastAuthD` web logged as `admin`)

- Go to file `/etc/uauditanalyzer/uanlz_web/config.ini` and fill the following fields:

    - `LoginRPCClient/LoginApiKey`: you obtain this random value from when you create the  **UAUDITANALYZER** app.
    - `LoginRPCClient/RemoteHost`: fill with the remote host address from the uFastAuthD application

- Don't forget to replace **/etc/uauditanalyzer/uanlz_web/ca.crt** file with the certificate authority public cert which signed uFastAuthD

- Don't forget to replace **/etc/uauditanalyzer/uanlz_web/snakeoil.{key,crt}** with new certificates from your certificate authority. This will provide your encryption security for the administrative website.

- Once completed the steps before, you can continue by activating/enabling the service:
```bash
systemctl enable --now uanlz_web
systemctl enable --now uanlz_alert
systemctl enable --now uanlz_log2json
```

- Now log into `uFastAuthD` as `admin` to give your user (even admin) enough privileges to access **UAUDITANALYZER** 

- Then, you can log with your `uFastAuthD` user into your uAuditAnalyzer2 Website: https://YOURHOSTIP:33000/login

- Now you are ready to operate this service


## Build Requirements 

This should be built on top of:

- libMantids libraries (2.5.2) - https://github.com/unmanarc/libMantids
- JSONCPP (AT LEAST v1.7.7, for RHEL7 you will have to build it by hand or install an external RPM)
- C++11

## Minimum System Requirements

- CPU: 1 to N processors (optimized to be multithreaded)
- MEM: Min: 256M (depending on your configuration you may want more)
- Storage: Min: 16Gb including the whole container/OS), you may want to add more storage to keep some logs for long periods of time.

So... would it run in my Raspberry PI 4? 

YES. But in our experience, RPI4 only delivers enough power to analyze some thousands of simultaneous servers with an average usage.

***

## Commercial Support
   
          
![Tekium](art/tekium_slogo.jpeg)

Tekium is a cybersecurity company specialized in red team and blue team activities based in Mexico, it has clients in the financial, telecom and retail sectors.

Tekium is an active sponsor of the project, and provides commercial support in the case you need it.

For integration with other platforms such as the Elastic stack, SIEMs, managed security providers in-house solutions, or for any other requests for extending current functionality that you wish to see included in future versions, please contact us: [info at tekium.mx](mailto:info@tekium.mx)
