# Development Broker with TLS

A local broker set up like the course broker: TLS on port 8883, and only a known user may connect.
It needs, from Moodle:

| Where | What |
| ----- | ---- |
| [`certs/`](certs/) | `ca.crt`, `server.crt` and `server.key`: the certificates, see its README |
| `passwd`, here | The user and its password, hashed |
| `.env`, here | The same user and password in plain text: copy `.env.sample` and fill it in |

All of them are in `.gitignore`, so they are never committed. Then run the task
**Broker: Start (dev TLS)**. Your programs log in with the user from `.env` and check the broker
against `certs/ca.crt`; **Broker: Check (dev TLS)** checks that these work, see
[Checking the TLS Settings](../README.md#checking-the-tls-settings).

The certificate is valid for the names `localhost`, `host.docker.internal` and `broker`. The Pi
connects through this computer's LAN address, which no ready-made certificate can list, so there
the client has to skip the check of the name; the certificate itself is still checked.
