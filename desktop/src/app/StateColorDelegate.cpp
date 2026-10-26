#include "app/StateColorDelegate.h"

#include "app/Colors.h"
#include "core/DeviceModel.h"
#include "messages/StatusMessage.h"

#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionViewItem>
#include <QVariant>

#include <optional>

namespace warden::app {

namespace {

constexpr int dotSize = 12;

} // namespace

void StateColorDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
  QStyledItemDelegate::initStyleOption(option, index);

  // No state in the model yet: the dot is gray.
  const QVariant stored = index.data(core::DeviceModel::stateRole);
  std::optional<messages::status_message::State> state;
  if (stored.isValid()) {
    state = static_cast<messages::status_message::State>(stored.toInt());
  }

  QPixmap dot(dotSize, dotSize);
  dot.fill(Qt::transparent);
  QPainter painter(&dot);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(stateColor(state));
  painter.drawEllipse(dot.rect());
  painter.end();

  option->icon           = QIcon(dot);
  option->decorationSize = dot.size();
  option->features |= QStyleOptionViewItem::HasDecoration;
}

} // namespace warden::app
