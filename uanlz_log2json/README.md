# Auditd Log Processing Application

## Overview
This application is designed to receive auditd logs via a centralized RSYSLOG connection and reassemble fragmented messages into structured JSON format. The processed data can be forwarded in real-time over TCP to downstream systems such as ELK stacks or utilized by uAuditanalyzer Alerts for generating security notifications and actions.

## Features
- **Auditd Log Reception**: Accepts auditd logs through a centralized RSYSLOG setup.
- **Message Reassembly**: Combines fragmented log messages into single, structured JSON objects.
- **Real-Time Forwarding**: Streams processed data over TCP to ELK stacks or other analytics platforms.
- **Integration Support**: Works seamlessly with uAuditanalyzer Alerts for immediate security action generation.

## Architecture
1. **Input Layer**: RSYSLOG receives incoming auditd logs from other rsyslog's+auditd and routes them to the application.
2. **Processing Pipeline**:
   - **Fragmentation Handling**: Detects and reassembles split messages using sequence identifiers.
   - **JSON Structuring**: Transforms raw log entries into standardized JSON format for easier analysis.
3. **Output Layer**: Sends processed data via TCP streams to designated endpoints like Elasticsearch or security alerting systems.

## Usage
1. Configure RSYSLOG to direct auditd logs to this application's input endpoint.
2. Run the application with appropriate configuration parameters (e.g., listening port, output destinations).
3. Monitor system performance and ensure continuous log flow for real-time analysis.

## Requirements
- **System Resources**: Adequate memory and network bandwidth to handle expected log throughput.
- **RSYSLOG Configuration**: Proper setup to forward auditd messages correctly formatted for this application.

### Compatibility:

Tested on:
- RHEL 7/8/9 and/or compatible systems.

## Configuration Overview

### Rsyslog retransmittion

For rsyslog that aggregate multiple hosts, and you may want to forward everything to uAuditAnalyzer, you can configure the file **/etc/rsyslog.d/01-remotes.conf** with the following content:

```conf
template(name="includeLocalTimestampAndSourceIP" type="list") {
    constant(value="<")
    property(name="pri")
    constant(value="><")
    property(name="fromhost-ip")
    constant(value=">")
    property(name="timereported" dateFormat="rfc3164")
    constant(value=" ")
    property(name="hostname")
    constant(value=" ")
    property(name="syslogtag")
    property(name="msg")
    constant(value="\n")
}

# RHEL 7:
:programname, isequal, "audispd"     action(type="omfwd" target="192.168.10.55" port="10514" protocol="tcp" template="includeLocalTimestampAndSourceIP")
# RHEL 8:
:programname, isequal, "audisp-syslog"     action(type="omfwd" target="192.168.10.55" port="10514" protocol="tcp" template="includeLocalTimestampAndSourceIP")
```

This configuration is designed to add the fromhost-ip property/variable to the rsyslog line, so the interpreter can add this into the JSON structure (**INFO.ip**)

> Note: it's important to change the IP address and TCP port with the location where uanlz_log2json is listening and reboot the rsyslog after changing the configuration.

### Receptors Configuration

The configuration files (**/etc/uauditanalyzer/uanlz_log2json/receptors.d/*.ini**) define the receptors responsible for receiving and processing incoming RSYSLOG TCP streams. Each `.ini` file corresponds to a separate receptor, allowing administrators to configure multiple listeners with different settings.

#### Example Receptor Configuration File:
```ini
# SYSLOG+AUDITD Receptor Configuration (TCP/10514)
[Receptor_1]
Description = "SYSLOG+AUDITD Listener on TCP Port 10514"
ListenAddr = "0.0.0.0"         # Listen on all interfaces
ListenPort = "10514"           # TCP port to listen on
ipv6 = false                   # Disable IPv6 support for IPv4 only environments
```

#### Key Receptor Configuration Parameters:
- **Description**: Human-readable name for the receptor.
- **ListenAddr**: IP address to bind the listener (e.g., `0.0.0.0` for all interfaces).
- **ListenPort**: TCP port number for incoming connections.
- **ipv6**: Boolean flag to enable IPv6 (`true`) or use IPv4 (`false`).

> Note: Each receptor must be configured manually via these `.ini` files. The web interface does not support receptor configuration at this time.

### Dispatchers Configuration

The configuration files (**/etc/uauditanalyzer/uanlz_log2json/dispatchers.d/*.ini**) define the dispatchers responsible for sending a JSON TCP streams. Each `.ini` file corresponds to a separate dispatcher, allowing administrators to configure multiple senders with different settings.

#### Example Dispatcher Configuration File:
```ini
# Output configuration: 

# Output description
Description=Output for Alert Engine
# transmition mode: 1: styled (useful for debugging), 0: one line per json
TransmitionMode=0
# Remote Host to connect
Host=127.0.0.1
# Remote port to connect
Port=9999
# Connection Timeout (in seconds) to mark a connection as failed
TimeoutSecs=10
# Sleep time (in seconds) when a connection failed to attempt a new connection
ReconnectSleepTimeInSecs=3
# Max Auditd JSON Queued events
MaxQueuedItems=500000    
# Auditd JSON Queue push timeout in milliseconds (0 for automatic discard when the queue is full)
QueuePushTimeoutInMS=0
```

- **Description**: Human-readable name for the dispatcher.
- **TransmitionMode**: Determines message formatting (`1` for human-readable styled output, `0` for compact single-line JSON).
- **Host**: The remote server IP or hostname where logs will be sent.
- **Port**: The TCP port on the remote host to connect to.
- **TimeoutSecs**: Number of seconds before a connection is considered failed.
- **ReconnectSleepTimeInSecs**: Time in seconds to wait before retrying a failed connection.
- **MaxQueuedItems**: Maximum number of JSON events stored in the queue if sending fails temporarily (default: `500,000`).
- **QueuePushTimeoutInMS**: Milliseconds allowed for pushing an event into the queue; `0` discards overflow immediately.

> Note: Each dispatcher must be configured manually via these `.ini` files. The web interface does not support dispatcher configuration at this time.

> Note2: To integrate with uAuditanalyzer Alerts, ensure that JSON output is dispatched to the uanlz_alert endpoint.


### Input/Output example:

- An rsyslog input example: [Example audisp-syslog Input](example_input.txt)
- And the output will be something like this (Using TransmissionMode=1): [Example JSON Output](example_output.txt)



### Integration Steps:

1. **Configure log2json:**
   Edit the file located at **/etc/uauditanalyzer/uanlz_log2json/config.ini** and set the following parameters under the `[RPCConnector]` section:
   - **AlertsApiKey**: change this for the same PSK in **/etc/uauditanalyzer/uanlz_web/config.ini**

2. **Enable and restart Services:**
   Apply the configuration changes and enable the required services:
    ```bash
    systemctl enable --now uanlz_log2json
    ```
    and if you want to restart the service:
    ```bash
    systemctl restart uanlz_log2json
    systemctl restart uanlz_web
    ```

3. **Open the Firewall**
    Open the firewall in RHEL (firewalld) for incomming TCP port, Eg. 10514:

    ```bash
    firewall-cmd --zone=public --permanent --add-port 10514/tcp
    # if you require another port, please repeat the last line modifiying the port
    firewall-cmd --reload
    ```
