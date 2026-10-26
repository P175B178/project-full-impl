#include "app/ViolationHighlightDelegate.h"

#include "app/Colors.h"
#include "core/HistoryModel.h"

#include <QStyleOptionViewItem>

namespace warden::app {

void ViolationHighlightDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
  QStyledItemDelegate::initStyleOption(option, index);
  if (index.data(core::HistoryModel::violationRole).toBool()) {
    option->backgroundBrush = violationBackground();
  }
}

} // namespace warden::app
