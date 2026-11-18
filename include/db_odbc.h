#ifndef DB_ODBC_H
#define DB_ODBC_H

#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <sqlext.h>

#include "models.h"

// 已学习课程结构体
struct LearnedCourse {
    std::string course_id;
    std::string course_name;
    int semester = 0;
    std::string status;
    bool has_score = false;
    double score = 0.0;
};
// 学生信息结构体
struct StudentInfo {
    std::string student_id;
    std::string name;
    std::string major;
    int grade = 0;
};
// 学生课程计划行结构体
struct StudentCoursePlanRow {
    std::string course_id;
    int semester = 0;
    std::string status = "PLANNED";
};
// 课程同步统计结构体
struct CourseSyncStats {
    int inserted = 0;
    int updated_or_unchanged = 0;
};
// 排课计划写入统计结构体
struct PlanWriteStats {
    int inserted = 0;
    int duplicated = 0;
};
// 课程删除检查结构体
struct CourseDeleteCheck {
    int student_course_refs = 0;
    int prereq_owner_refs = 0;
    int prereq_required_refs = 0;
};
// 数据库操作类
class OdbcDb {
public:
    OdbcDb() = default;
    ~OdbcDb();
    // 连接数据库
    bool connect(const std::string& conn_str, std::string& err);
    void disconnect();
    // 测试数据库连接是否成功
    bool ping(std::string& err);
    // 获取学生总数
    bool getStudentCount(int& count, std::string& err);
    // 获取指定学生的已学习课程
    bool listStudentCourses(const std::string& student_id, std::vector<LearnedCourse>& out, std::string& err);
    // 获取所有学生信息
    bool listStudents(std::vector<StudentInfo>& out, std::string& err);
    // 根据学生ID获取学生信息
    bool getStudentById(const std::string& student_id, StudentInfo& out, bool& found, std::string& err);
    // 添加新学生
    bool addStudent(const StudentInfo& s, std::string& err);
    // 更新学生信息
    bool updateStudent(const StudentInfo& s, std::string& err);
    // 删除学生
    bool deleteStudent(const std::string& student_id, std::string& err);

    // 插入学生课程计划行
    bool insertStudentPlanRows(const std::string& student_id,
                               const std::vector<StudentCoursePlanRow>& rows,
                               int& affected,
                               std::string& err);
    // 从数据库获取所有课程信息
    bool listCoursesFromDb(std::vector<Course>& out, std::string& err);
    // 同步课程信息到数据库
    bool upsertCourses(const std::vector<Course>& rows, CourseSyncStats& stats, std::string& err);
    // 获取课程删除检查信息
    bool getCourseDeleteCheck(const std::string& course_id, CourseDeleteCheck& check, std::string& err);
    // 从数据库删除课程
    bool deleteCourseFromDb(const std::string& course_id,
                            bool remove_prereq_refs,
                            CourseDeleteCheck* deleted_check,
                            std::string& err);
    // 删除学生的所有计划课程行
    bool deleteStudentPlannedRows(const std::string& student_id, int& deleted, std::string& err);
    // 批量插入学生课程计划行，自动去重
    bool insertStudentPlanRowsDedup(const std::string& student_id,
                                    const std::vector<StudentCoursePlanRow>& rows,
                                    PlanWriteStats& stats,
                                    std::string& err);

private:
    SQLHENV env_ = SQL_NULL_HENV;
    SQLHDBC dbc_ = SQL_NULL_HDBC;
};

#endif
