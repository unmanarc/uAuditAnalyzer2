# uanlz_alert

## Overview
uanlz_alert is a system designed to process TCP streams containing JSON data, with one JSON object per line. Its primary function is to filter incoming data and execute actions—most commonly alerting—based on predefined criteria.

## Filtering/Alert Mechanism
The filtering functionality in uanlz_alert leverages **JSONPath** integrated with an expression evaluation engine from the **libMantids** library. This combination enables advanced filtering capabilities, including:
- **Advanced Regular Expressions**: Perform complex pattern matching against JSON fields.
- **Conditional Logic**: Evaluate expressions to determine if data meets specific conditions for triggering actions.

## Features
- **Real-Time Processing**: Processes TCP streams in real time, ensuring timely alerting.
- **Flexible Filtering**: Use JSONPath queries combined with libMantids' expressive syntax for precise filtering rules.
- **Modular Actions**: Execute various actions (e.g., sending alerts) based on filter outcomes.

## Usage
1. **Data Ingestion**: Receive a TCP stream where each line is a valid JSON object.
2. **Filter Application**: Apply JSONPath expressions and libMantids evaluations to determine relevance.
3. **Action Execution**: Trigger predefined actions (e.g., alert notifications) when filters match.


## Requirements
- **System Resources**: Adequate memory and CPU to handle expected log throughput.

## Configuration Overview

### Inputs Configuration

The configuration **/etc/uauditanalyzer/uanlz_alert/inputs.json** define the receptors responsible for receiving and processing incoming JSON lines to be processed. 

#### Example input Configuration File:
```json
[
	{
		"Description" : "Alert JSON TCP Input",
		"ListenAddr" : "127.0.0.1",
		"ListenPort" : 9999,
		"id" : "fpL9mjmU"
	},
	{
		"Description" : "Alert JSON TCP Input from other host",
		"ListenAddr" : "0.0.0.0",
		"ListenPort" : 9998,
		"id" : "fpL9mjmA"
	}
]
```

- **Description**: Human-readable name for the receptor.
- **ListenAddr**: IP address to bind the listener (e.g., `0.0.0.0` for all interfaces).
- **ListenPort**: TCP port number for incoming connections.
- **id**: Unique identifier for the receptor (required).

> Note: Each receptor must be configured manually via these `.json` files. The web interface does not support receptor configuration at this time.

### Actions Configuration

Actions are defined in the file **/etc/uauditanalyzer/uanlz_alert/actions.json** and can be triggered when specific filtering criteria are met. These actions allow for executing external programs or scripts, which receive arguments derived from JSON Path expressions, rule names, binary data, or other variables present in the processed JSON objects.

For example, an action might execute a program that sends an email alert using parameters extracted via JSONPath (e.g., `$.message`) along with contextual information like the triggering rule's name. The configuration specifies how each action should behave and what inputs it requires to perform its task.

#### Example actions Configuration File:

```json
[
	{
		"appendNewLineToEachArgv" : true,
		"args" : 
		[
			"sendmsg.sh",
			"%%F0%%9F%%9A%%A8 %%E2%%9A%%A0 *%#rule.name%* %%E2%%9A%%A0 %%F0%%9F%%9A%%A8 %N%#TIME=*%-.INFO.time%*",
			"#KEY=*%-.INFO.key%* / #ID=%-.INFO.unixTime%.%-.INFO.msecs%:%-.INFO.id%",
			"#HOST=*%-.INFO.host%*",
			"```",
			"PATHS= {%-.PATH[0].name%, %-.PATH[1].name%, %-.PATH[2].name%,%-.PATH[3].mode%, %-.PATH[4].mode%, %-.PATH[5].mode%}",
			"MODES= {%-.PATH[0].mode%, %-.PATH[1].mode%, %-.PATH[2].mode%,%-.PATH[3].mode%, %-.PATH[4].mode%, %-.PATH[5].mode%}",
			"DIR(CWD)= %-.CWD[0].cwd%",
			"UID/GID= %-.SYSCALL[0].uid% / %-.SYSCALL[0].gid%",
			"PID/PPID= %-.SYSCALL[0].pid% / %-.SYSCALL[0].ppid%",
			"SELINUX=%-.SYSCALL[0].subj%",
			"SES/TTY=%-.SYSCALL[0].ses% / %-.SYSCALL[0].tty%",
			"RET/SUC=%-.SYSCALL[0].exit% / %-.SYSCALL[0].success% ```",
			"PROCESS NAME: ``` %-.PROCTITLE[0].proctitle% ```",
			"EXECVE CMDLINE: ``` %-.EXECVE[0].cmdline% ```"
		],
		"description" : "Send Message via sendmsg.sh",
		"name" : "SendMSG",
		"path" : "/usr/bin/sendmsg.sh"
	}
]
```
The variables used within the `args` field of an action configuration are resolved using specific syntax patterns. Here's a breakdown of how these variables work:

1. **%%0A0D%%**: Return the  binary represenation of the hex code inside (`\n\r`).
1. **%N%**: Return a newline (`\n`). 
2. **JSONPath Styled (starts with `$`, - Eg. %$.INFO%)**:
   - Parses the substring after `$` as a JSONPath.
   - Resolves the path in the input JSON data.
   - Returns the result formatted in styled (pretty-printed multi-line) JSON.
3. **JSONPath Unstyled (starts with `-` - %-.INFO.host% )**:
   - Similar to above but returns unstyled, single-line JSON without formatting.
4. **Uppercase JSONPath (starts with `+` - %+.INFO.host% )**:
   - Resolves the path similarly to the `-` case but converts the result to uppercase.
5. **Rule Metadata (starts with `#`, - %#rule.name%)**:
   - If `var` is `"#rule.name"`, returns the current rule’s name.

This syntax allows dynamic insertion of metadata and event details into action arguments for context-rich notifications.

### Integration Steps:

1. **Configure Alerts:**
   Edit the file located at **/etc/uauditanalyzer/uanlz_alert/config.ini** and set the following parameters under the `[RPCConnector]` section:
   - **AlertsApiKey**: change this for the same PSK in **/etc/uauditanalyzer/uanlz_web/config.ini**

2. **Enable and restart Services:**
   Apply the configuration changes and enable the required services:
    ```bash
    systemctl enable --now uanlz_alert
    ```
    and if you want to restart the service:
    ```bash
    systemctl restart uanlz_alert
    systemctl restart uanlz_web
    ```

3. **Open Firewall Ports Securely**
- If log2json is running on a different host, configure your firewall (e.g., firewalld on RHEL-based systems) to allow incoming traffic on the required ports for uanlz_alert.
- **Security Best Practices**:
    - Restrict access to trusted IP addresses/subnets using firewall rules.
    - Ensure all incoming connections pass through a secure network, such as a private VLAN or a restricted VPN tunnel.
    - Avoid exposing the port to the public internet; limit access only to necessary endpoints.
