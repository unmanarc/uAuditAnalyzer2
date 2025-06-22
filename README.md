# uAuditAnalyzer2 - Unmanarc's Auditd Analyzer

![uAuditAnalyzer](art/logo.jpg)

Author: Aarón Mizrachi <aaron@unmanarc.com>  
License: GPLv3  

uAuditAnalyzer2 is a high-performance tool designed to efficiently process auditd logs received through rsyslog. Widely deployed in enterprise environments, it excels at identifying potential security threats and delivering real-time alerts via messaging platforms, bots, or other notification systems.

Additionally, uAuditAnalyzer2 can aggregate and decode auditd messages into JSON structures, making it easy to forward the data to other systems such as ELK stacks for further analysis and visualization.

## Applications

### uanlz_log2json

This application is responsible for receiving auditd logs via a centralized RSYSLOG connection. It then reassembles fragmented auditd messages into a single, structured JSON format. The processed data can be forwarded in real-time over TCP to downstream systems such as ELK stacks or used by uAuditanalyzer Alerts for generating security notifications or actions.

### uanlz_alert

uanlz_alert receives a TCP stream with 1 JSON per line in order to filter and execute actions (usually alerting).

uanlz_alert uses a filter system based on a JSONPath mechanism integrated with an expression evaluation mechanism provided by the libMantids library, which allows comparison with advanced regular expressions, among other things.

### uanlz_web

uanlz_web is a web-based interface designed to monitor and manage the health of uAuditAnalyzer services as well as configure and maintain filters for uanlz_alert. Built using HTML5, jQuery, and Bootstrap for the frontend, it communicates with a backend service implemented in C++ leveraging the libMantids library's web services capabilities.

***
## Installing packages (HOWTO)


- [Manual build guide](BUILD.md)
- COPR Packages (Fedora/CentOS/RHEL/etc):  
[![Copr build status](https://copr.fedorainfracloud.org/coprs/amizrachi/unmanarc/package/uAuditAnalyzer2/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/amizrachi/unmanarc/package/uAuditAnalyzer2/)

### Simple installation guide for Fedora/RHEL:

To activate our repo's and download/install the software:

In RHEL7:
```bash
# Install EPEL Repo + COPR
yum -y install epel-release
yum -y install yum-plugin-copr

# Install unmanarc's copr
yum copr enable amizrachi/unmanarc -y
yum -y install uAuditAnalyzer2
```

In RHEL8:
```bash
# Install EPEL Repo
dnf -y install 'dnf-command(config-manager)'
dnf config-manager --set-enabled powertools
dnf -y install epel-release

# Install unmanarc's copr
dnf copr enable amizrachi/unmanarc -y
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

- libMantids libraries (2.8.26) - https://github.com/unmanarc/libMantids
- JSONCPP (AT LEAST v1.7.7, for RHEL7 you will have to build it by hand or install an external RPM)
- BOOST libs
- C++11

## Minimum System Requirements

- CPU: 1 to N processors (optimized to be multithreaded)
- MEM: Min: 256M (depending on your configuration you may want more)
- Storage: Min: 16Gb including the whole container/OS), you may want to add more storage to keep some logs for long periods of time.

So... would it run in my Raspberry PI 4? 

YES. But in our experience, RPI4 only delivers enough power to analyze some thousands of simultaneous servers with an average usage.

***

## Professional Support
          
![Tekium](art/tekium_slogo.jpeg)

Tekium is a cybersecurity company specializing in incident response and threat management based in Mexico. Tekium serves clients across various sectors including finance, telecommunications, and retail. As an active contributor to this project, Tekium offers professional installation support for organizations requiring assistance with deployment.

For integration with other platforms such as the Elastic stack, SIEMs, managed security providers in-house solutions, or for any other requests for extending current functionality that you wish to see included in future versions, please contact: [info at tekium.mx](mailto:info@tekium.mx)
