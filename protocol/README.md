# Message Contract

Topics and messages exchanged between `device/` and `desktop/` through the MQTT broker. Every
group's programs must follow this contract exactly, so that any group's desktop program can
monitor any other group's device.

This file and the examples in [`examples/`](examples/) are the contract. The latest version is in
the [project template](https://github.com/P175B178/project-template/tree/main/protocol).

## Identifiers

- **`<group-id>`** names the group in every topic. It is the last segment of the group's
  repository name: `group-1` for `cpp-2027p-projektas-group-1`.
- **`deviceId`** names the device inside every message it publishes, e.g. `group-1-pi5`. It must
  be stable between runs and unique on the broker.

A group has one device: the topics carry no device id, so two devices of one group publishing at
once cannot be told apart.

## Topics

| Topic | Direction | Example |
| ----- | --------- | ------- |
| `sensors/<group-id>/temperature` | device → desktop | [observation-temperature.json](examples/observation-temperature.json) |
| `sensors/<group-id>/humidity` | device → desktop | [observation-humidity.json](examples/observation-humidity.json) |
| `sensors/<group-id>/status` | device → desktop, also the device's Last Will | [status-normal.json](examples/status-normal.json), [status-fault.json](examples/status-fault.json), [status-offline.json](examples/status-offline.json) |
| `config/<group-id>/temperature-threshold` | desktop → device | [config-temperature-threshold.json](examples/config-temperature-threshold.json) |
| `config/<group-id>/humidity-threshold` | desktop → device | [config-humidity-threshold.json](examples/config-humidity-threshold.json) |
| `config/<group-id>/interval` | desktop → device | [config-interval.json](examples/config-interval.json) |
| `config/<group-id>/state` | desktop → device | [config-state.json](examples/config-state.json) |

## Rules

For every message, in both directions:

- Every field shown in the examples is required; a message missing one is invalid.
- Unknown fields are ignored, never treated as an error.
- An invalid message is logged and dropped. It must never crash the program.
- Timestamps are UTC, ISO 8601, ending in `Z`: `2026-01-02T03:04:05Z`. Other time zone offsets
  are invalid.

## Readings

A reading is a simplified [HL7 FHIR Observation](https://www.hl7.org/fhir/observation.html), with
the fields and names of the examples. `valueQuantity.value` is the smoothed (averaged) value, the
one the device checks its thresholds against, not the sensor's raw reading.

| Metric | LOINC code | LOINC display | UCUM unit | UCUM code |
| ------ | ---------- | ------------- | --------- | --------- |
| Temperature | `8310-5` | Body temperature | °C | `Cel` |
| Humidity | `13834-7` | Humidity | % | `%` |

A device may add fields of its own, such as `"averagedSamples": 3` or `"location": "room-204"`, as
long as their names do not clash with FHIR's.

## Status

`state` is one of:

| State | Meaning |
| ----- | ------- |
| `normal` | No threshold exceeded |
| `warning` | Exactly one threshold exceeded |
| `alert` | Both thresholds exceeded |
| `fault` | The sensor keeps failing (only if the device implements the Fault state) |
| `offline` | The device is offline |

## Commands

- A threshold's `unit` must be exactly `"°C"` for temperature and `"%"` for humidity; any other
  unit makes the message invalid.
- `seconds` is a whole number of seconds.
- `state` is one of `normal`, `warning`, `alert` or `fault`. The device acts as if it were in that
  state, on its LED, its buzzer and its status, until its next valid reading decides the state
  again.

Each group decides which values its device accepts; a value it does not accept is invalid, so it
is logged and ignored. Suggested ranges:

| Command | Suggested range |
| ------- | --------------- |
| Temperature threshold | 0 to 50 °C |
| Humidity threshold | 0 to 100 % |
| Interval | 2 to 3600 s (the DHT22 needs 2 s between readings) |

## Using the Examples in Tests

The example files are meant to be used by both programs' unit tests: the device's tests can check
that the messages it produces match them, and the desktop's tests can check that it parses them.
If one side changes the format, the other side's tests then fail too.
