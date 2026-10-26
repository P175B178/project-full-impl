#pragma once

#include <QStyledItemDelegate>

namespace warden::app {

/// Paints a history cell above its threshold red. The model decides, through
/// `HistoryModel::violationRole`; the delegate only draws.
class ViolationHighlightDelegate : public QStyledItemDelegate {
  Q_OBJECT

public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

} // namespace warden::app
