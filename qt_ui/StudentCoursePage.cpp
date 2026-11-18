#include "StudentCoursePage.h"
#include "AppSession.h"
#include "UiMessages.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QAbstractItemView>

StudentCoursePage::StudentCoursePage(AppSession* session,
                                     const StudentInfo& student,
                                     QWidget* parent)
    : QDialog(parent), session_(session), student_(student) {
    setWindowTitle("学生课程情况");
    resize(860, 480);

    infoLabel_ = new QLabel(this);
    infoLabel_->setText(QString("学生：%1 | %2 | %3 | %4")
        .arg(QString::fromStdString(student_.student_id))
        .arg(QString::fromStdString(student_.name))
        .arg(QString::fromStdString(student_.major))
        .arg(student_.grade));

    table_ = new QTableWidget(this);
    table_->setColumnCount(5);
    table_->setHorizontalHeaderLabels({"学期", "课程号", "课程名", "状态", "成绩"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setSortingEnabled(true);
    table_->horizontalHeader()->setSortIndicatorShown(true);

    statusLabel_ = new QLabel(this);
    auto* btnRefresh = new QPushButton("刷新", this);
    auto* btnClose = new QPushButton("关闭", this);
    
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(infoLabel_);
    layout->addWidget(table_);
    layout->addWidget(statusLabel_);
    auto* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(btnRefresh);
    btnRow->addWidget(btnClose);
    layout->addLayout(btnRow);

    connect(btnRefresh, &QPushButton::clicked, this, &StudentCoursePage::loadCourses);
    connect(btnClose, &QPushButton::clicked, this, &StudentCoursePage::accept);

    loadCourses();
}

void StudentCoursePage::loadCourses() {
    UiMessages::BusyGuard busy(this, "正在读取课程记录...");
    const bool sorting = table_->isSortingEnabled();
    table_->setSortingEnabled(false);
    table_->setRowCount(0);

    if(!session_ || !session_->connected) {
        statusLabel_->setText("数据库未连接");
        table_->setSortingEnabled(sorting);
        return;
    }
    
    std::vector<LearnedCourse> history;
    std::string err;
    if(!session_->db.listStudentCourses(student_.student_id, history, err)){
        statusLabel_->setText(UiMessages::formatError("读取课程记录", QString::fromStdString(err)));
        table_->setSortingEnabled(sorting);
        return;
    }
    
    table_->setRowCount(static_cast<int>(history.size()));
    for(int i = 0; i< static_cast<int>(history.size()); ++i) {
        const auto& r = history[i];
        table_->setItem(i, 0, new QTableWidgetItem(QString::number(r.semester)));
        table_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(r.course_id)));
        table_->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(r.course_name)));
        table_->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(r.status)));
        if(r.has_score){
            table_->setItem(i, 4, new QTableWidgetItem(QString::number(r.score)));
        }else{
            table_->setItem(i, 4, new QTableWidgetItem("NULL"));
        }
    }

    statusLabel_->setText(QString("共 %1 条课程记录").arg(history.size()));
    table_->setSortingEnabled(sorting);
}
