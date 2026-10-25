# MQTT Brokers

Docker runs on the host, so the local brokers do too: open the repository in a VS Code window
**on the host** (without **Reopen in Container**) and use the tasks below (**Terminal > Run Task**).

| Setup | What it is | Start with |
| ----- | ---------- | ---------- |
| [`dev/`](dev/) | Local broker for development, no security: port 1883, anyone may connect | **Broker: Start (dev)** |
| [`dev-tls/`](dev-tls/) | Local broker for development, like the course broker: TLS on port 8883, one user and password; needs files from Moodle and a `.env` | **Broker: Start (dev TLS)** |
| [`course/`](course/) | The course broker, run by the lecturer: what your programs need to connect to it | **Broker: Explorer (course)**: only MQTT Explorer, connected to the course broker |

Only one of these runs at a time: starting one replaces the other. **Broker: Stop**,
**Broker: Restart** and **Broker: Logs** work for whichever runs. Each comes with
[MQTT Explorer](https://mqtt-explorer.com/) at http://localhost:3000, already connected.

| Connecting from | Broker address |
| --------------- | -------------- |
| a dev container | `host.docker.internal` |
| the Raspberry Pi | this computer's LAN address |

## Checking the TLS Settings

MQTT Explorer shows the messages, but it does not check the broker's certificate: it connects even
with a wrong `ca.crt`, or none. To check the settings your programs will use, run a check instead.
It connects the way they will, with the settings in the setup's `.env` and its `certs/ca.crt`, and
says in one line whether TLS and the login work, or what failed.

| Task | Checks | Before running it |
| ---- | ------ | ----------------- |
| **Broker: Check (dev TLS)** | `dev-tls/`, your own broker | Start it: **Broker: Start (dev TLS)** |
| **Broker: Check (course)** | `course/`, the course broker | Be on the university network |

For the course broker, first copy `course/.env.sample` to `course/.env` and fill it in (the address
given in class, and your group's username and password from Moodle), and save the course broker's
`ca.crt` from Moodle in `course/certs/`.

| Message | What to fix |
| ------- | ----------- |
| `TLS and login work` | Nothing: use the same settings in your programs |
| `not authorised` | `BROKER_USER` or `BROKER_PASSWORD` in the setup's `.env` |
| `certificate verify failed` | `certs/ca.crt`: not the CA of this broker |
| `File not found` | `certs/ca.crt` is missing |
| `… is missing a value` | The setup's `.env` is missing, or a setting in it is empty |
| anything else, such as `Lookup error` | The broker cannot be reached: for dev TLS, it is not running; for the course broker, you are not on the university network, or the address is wrong |
