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

## Build Requirements 

This should be built on top of:

- libMantids libraries (2.3.0) - https://github.com/unmanarc/libMantids
- JSONCPP (AT LEAST v1.7.7, for RHEL7 you will have to build it by hand or install an external RPM)
- C++11

## Minimum System Requirements

- CPU: 1 to N processors (optimized to be multithreaded)
- MEM: Min: 256M (depending on your configuration you may want more)
- Storage: Min: 16Gb including the whole container/OS), you may want to add more storage to keep some logs for long periods of time.

So... would it run in my Raspberry PI 4? 

YES. But in our experience, RPI4 only delivers enough power to analyze some thousands of simultaneous servers with an average usage.

## Commercial Support
   
          
![Tekium](art/tekium_slogo.jpeg)

Tekium is a cybersecurity company specialized in red team and blue team activities based in Mexico, it has clients in the financial, telecom and retail sectors.

Tekium is an active sponsor of the project, and provides commercial support in the case you need it.

For integration with other platforms such as the Elastic stack, SIEMs, managed security providers in-house solutions, or for any other requests for extending current functionality that you wish to see included in future versions, please contact us: [info at tekium.mx](mailto:info@tekium.mx)
