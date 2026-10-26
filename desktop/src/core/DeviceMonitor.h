#pragma once

#include "messages/Observation.h"

#include <QObject>
#include <QString>

#include <memory>
#include <unordered_map>

namespace warden::core {

class HistoryModel;

/// What the console derives from each device's readings.
class DeviceMonitor : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DeviceMonitor)

public:
  explicit DeviceMonitor(QObject *parent = nullptr);
  ~DeviceMonitor() override;

  /// The device's history, created on first use.
  HistoryModel *historyFor(const QString &groupId);

  void addReading(const QString &groupId, const messages::observation::Reading &reading);

  /// Empties every history, for a new source of device data. The models stay valid.
  void clear();

private:
  // Views keep a model's address, so each is allocated once.
  std::unordered_map<QString, std::unique_ptr<HistoryModel>> m_histories;
};

} // namespace warden::core
