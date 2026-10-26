#pragma once

#include <QString>

class QAbstractItemModel;

/// CSV export of any table model: a header row, then each row's raw values (Qt::EditRole), quoted
/// and ended as RFC 4180 asks. The file is replaced only once it is completely written.
namespace warden::core::csv_export {

/// Writes `model` to `path`. Returns an empty string, or what went wrong.
[[nodiscard]] QString writeModel(const QAbstractItemModel *model, const QString &path);

} // namespace warden::core::csv_export
