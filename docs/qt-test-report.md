# Qt 阶段测试记录

日期：2026-03-10  
环境：Windows 11 + Qt 6 + CMake + MinGW64 + MySQL 8.x（ODBC 9.6）

## 1. 登录与会话
- 操作：启动 `scheduler_qt.exe`，输入账号密码登录。
- 预期：登录成功进入主界面，后续操作无需重复输入账号密码。
- 结果：通过。
- 截图：`docs/screenshots/qt-01-login.png`（待补）

## 2. 学生管理（增删改查）
- 操作：新增学生、修改学生信息、删除学生。
- 预期：表格刷新，数据库同步更新；删除时二次确认。
- 结果：通过。
- 截图：`docs/screenshots/qt-02-student-crud.png`（待补）

## 3. 学生课程历史
- 操作：选中学生后查看课程历史。
- 预期：展示课程号、课程名、学期、状态、成绩。
- 结果：通过。
- 截图：`docs/screenshots/qt-03-student-course.png`（待补）

## 4. 课程管理（缓存 / 数据库）
- 操作：从 TXT 导入、从数据库载入、缓存新增/修改/删除课程。
- 预期：课程表格刷新，先修关系显示正确。
- 结果：通过。
- 截图：`docs/screenshots/qt-04-course-cache.png`（待补）

## 5. 课程同步到数据库
- 操作：点击“同步到数据库”。
- 预期：提示写库确认，成功后显示新增/更新统计，先修关系写入 `course_prereq`。
- 结果：通过。
- 截图：`docs/screenshots/qt-05-course-sync.png`（待补）

## 6. 排课与剔除已修
- 操作：配置排课参数，选择学生并勾选“剔除已修课程”，生成课表。
- 预期：生成结果展示学期明细与总学分；状态提示可读。
- 结果：通过。
- 截图：`docs/screenshots/qt-06-plan-generate.png`（待补）

## 7. 导出与写库
- 操作：导出课表到 `output/plan.txt`，选择写入数据库模式。
- 预期：导出成功；写入结果显示新增/重复/删除统计。
- 结果：通过。
- 截图：`docs/screenshots/qt-07-plan-export.png`（待补）
