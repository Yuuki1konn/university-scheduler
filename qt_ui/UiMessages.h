#pragma once

#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QString>
#include <QWidget>

namespace UiMessages {

inline QString formatError(const QString& action, const QString& detail) {
    if (detail.trimmed().isEmpty()) {
        return QString("%1失败，请稍后重试。").arg(action);
    }
    return QString("%1失败。\n原因：%2").arg(action, detail);
}

class BusyGuard {
public:
    BusyGuard(QWidget* parent, const QString& text)
        : dialog_(parent) {
        dialog_.setWindowTitle("请稍候");
        dialog_.setLabelText(text);
        dialog_.setCancelButton(nullptr);
        dialog_.setRange(0, 0);
        dialog_.setMinimumDuration(0);
        dialog_.setAutoClose(false);
        dialog_.setAutoReset(false);
        dialog_.setWindowModality(Qt::WindowModal);
        dialog_.show();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QApplication::processEvents();
    }

    ~BusyGuard() {
        dialog_.hide();
        QApplication::restoreOverrideCursor();
        QApplication::processEvents();
    }

private:
    QProgressDialog dialog_;
};

inline void info(QWidget* parent, const QString& title, const QString& text) {
    QMessageBox::information(parent, title, text);
}

inline void warning(QWidget* parent, const QString& title, const QString& text) {
    QMessageBox::warning(parent, title, text);
}

inline void error(QWidget* parent, const QString& title, const QString& text) {
    QMessageBox::critical(parent, title, text);
}

inline bool confirm(QWidget* parent,
                    const QString& title,
                    const QString& text,
                    QMessageBox::StandardButton default_button = QMessageBox::No) {
    const auto reply = QMessageBox::question(
        parent,
        title,
        text,
        QMessageBox::Yes | QMessageBox::No,
        default_button
    );
    return reply == QMessageBox::Yes;
}

inline bool confirmUpload(QWidget* parent, const QString& target_name, int row_count) {
    return confirm(
        parent,
        "确认上传",
        QString("是否上传到数据库？\n将同步 %1 条%2记录。").arg(row_count).arg(target_name),
        QMessageBox::No
    );
}

}  // namespace UiMessages
