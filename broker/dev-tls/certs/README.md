# Certificates

Copy these from Moodle into this folder:

| File | What it is |
| ---- | ---------- |
| `ca.crt` | The certificate authority (CA) clients check the broker against: give your programs this file, and copy it to the Pi |
| `server.crt` | The broker's certificate, signed by the CA |
| `server.key` | The broker's private key |

They are in `.gitignore`, so they are never committed.
