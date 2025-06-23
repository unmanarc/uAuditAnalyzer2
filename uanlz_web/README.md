# uanlz_web - Unified Audit Analysis Web Interface

## Overview
uanlz_web provides a user-friendly web-based interface to monitor and manage the health of **uAuditAnalyzer services** and configure/maintain filters for **uanlz_alert**. Built with modern front-end technologies, it seamlessly integrates with a backend service implemented in C++ using the **libMantids library's web services capabilities**, ensuring robust performance and efficient communication.

## Features
- Real-time monitoring of uAuditAnalyzer service health.
- Comprehensive configuration and management tools for uanlz_alert filters.
- Responsive design optimized for all screen.
- Secure, high-performance backend leveraging C++ and libMantids library.

## Configuration

### uFastAuthD integration

uFastAuthD (https://github.com/unmanarc/uFastAuthD) is an Identity and Access Management (IAM) solution designed specifically for applications built with the libMantids framework. To ensure seamless operation, this application requires a properly installed and configured instance of uFastAuthD.

#### Integration Steps:

1. **Register Application in uFastAuthD:**
   - Log into your uFastAuthD admin panel.
   - Navigate to the "Applications" section and create a new application named **UAUDITANALYZER**.
   - Record the **Application API Key** generated during this process; it will be needed for configuration.

2. **Configure `config.ini`:**
   Edit the file located at `/etc/uauditanalyzer/uanlz_web/config.ini` and set the following parameters under the `[LoginRPCClient]` section:
   - `LoginApiKey`: Use the API key obtained from the newly created **UAUDITANALYZER** application.
   - `RemoteHost`: Specify the network address (IP or domain) where uFastAuthD is running.

3. **Certificate Configuration:**
   - Replace the default CA certificate located at `/etc/uauditanalyzer/uanlz_web/ca.crt` with your organization's trusted Certificate Authority (CA) public certificate to ensure secure communication with uFastAuthD.
   - Substitute the self-signed Snakeoil certificates (`snakeoil.key`, `snakeoil.crt`) in the same directory with valid SSL/TLS certificates issued by your trusted CA. This ensures encrypted and authenticated access to the administrative web interface.

4. **Enable Services:**
   Apply the configuration changes and enable the required services:
    ```bash
    systemctl enable --now uanlz_web
    ```
    and if you want to restart the service:
    ```bash
    systemctl restart uanlz_web
    ```

5. **Open the Firewall**
    Open the firewall in RHEL (firewalld) for incomming TCP port 33000:

    ```bash
    firewall-cmd --zone=public --permanent --add-port 33000/tcp
    # if you require another port, please repeat the last line modifiying the port
    firewall-cmd --reload
    ```
