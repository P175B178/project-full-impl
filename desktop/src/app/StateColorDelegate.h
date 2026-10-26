#pragma once

#include <QStyledItemDelegate>

namespace warden::app {

/// Draws a dot in the color of the device's state next to a cell's text. The model says which
/// state, through `DeviceModel::stateRole`; the delegate only draws.
class StateColorDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  using QStyledItemDelegate::QStyledItemDelegate;

protected:
  void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

} // namespace warden::app
