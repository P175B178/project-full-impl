# Course Broker

The shared broker at the university, run by the lecturer, used for the demo. It is like
[`../dev-tls/`](../dev-tls/): TLS and a username and password. Your programs need:

| Setting | Value |
| ------- | ----- |
| Address | given in class |
| Port | `8883` |
| TLS | on, checked against `ca.crt` from Moodle: save it in [`certs/`](certs/) |
| Username and password | your group's, from Moodle |

To check them before the demo, put them in `.env` (copy `.env.sample`) and run the task
**Broker: Check (course)**: see [Checking the TLS Settings](../README.md#checking-the-tls-settings).
