#pragma once

#include "messages/Observation.h"
#include "messages/StatusMessage.h"

#include <QByteArray>
#include <QObject>
#include <QString>

namespace warden::messages {

/// Where the console's device data comes from: the devices' messages on their topics, from a
/// broker or a simulator. The window depends only on this interface, so the two are
/// interchangeable.
class DeviceGateway : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceGateway)

public:
  enum class State { Disconnected, Connecting, Connected };

  explicit DeviceGateway(QObject *parent = nullptr);
  ~DeviceGateway() override;

  /// (Re)start the source: from then on its messages arrive as the signals below.
  virtual void start() = 0;

  /// Stop the source.
  virtual void stop() = 0;

  [[nodiscard]] virtual State state() const = 0;

signals:
  /// A parsed reading for a device.
  void readingReceived(const QString &groupId, const observation::Reading &reading);

  /// A parsed status message for a device.
  void statusReceived(const QString &groupId, const status_message::Status &status);

  /// The source's lifecycle, for the status bar.
  void stateChanged(DeviceGateway::State state);

protected:
  /// Turns the message on `topic` into its signal, or drops it when it cannot be used: what a
  /// broker delivers and what a simulator makes up go the same way.
  void route(const QString &topic, const QByteArray &payload);
};

} // namespace warden::messages
