#include "core/DeviceMonitor.h"

#include "core/HistoryModel.h"

namespace warden::core {

DeviceMonitor::DeviceMonitor(QObject *parent)
      : QObject(parent) {}

DeviceMonitor::~DeviceMonitor() = default;

HistoryModel *DeviceMonitor::historyFor(const QString &groupId) {
  std::unique_ptr<HistoryModel> &history = m_histories[groupId];
  if (!history) {
    history = std::make_unique<HistoryModel>();
  }
  return history.get();
}

void DeviceMonitor::addReading(const QString &groupId, const messages::observation::Reading &reading) {
  historyFor(groupId)->addReading(reading);
}

void DeviceMonitor::clear() {
  for (const auto &entry : m_histories) {
    entry.second->clear();
  }
}

} // namespace warden::core
