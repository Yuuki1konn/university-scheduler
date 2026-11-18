# 教务排课系统（TUI + Qt GUI）

> 专业综合实践作业 · 题目「安排教学计划」
> C++17 / 拓扑排序 / ODBC + MySQL 8 / Qt 6

同一套业务逻辑，两个界面入口：

| 版本 | 入口 | 形态 | 说明 |
| --- | --- | --- | --- |
| TUI | `output/scheduler.exe` | 控制台菜单 | 数据结构阶段主体，含课程录入/编辑、排课、导出、学生管理 |
| GUI | `build_qt/scheduler_qt.exe` | Qt 6 窗口 | 登录数据库后进入学生 / 课程 / 计划三个管理页，可写库 |

业务层（`src/scheduler.cpp`、`course_io.cpp`、`course_edit.cpp`、`db_odbc.cpp`）两个界面共用，界面层只负责收集输入和展示结果。

---

## 1. 功能一览

**排课核心**
- 课程目录：课程号（3 位字母数字）、课程名、学分、任意多门先修课程
- 课程增删改查，删除时检查是否被其他课程当作先修（防止破坏依赖）
- 先修关系建图 + 拓扑排序，能识别先修环
- 两种编排策略：`BALANCED`（各学期负担尽量均匀）、`FRONT_LOADED`（高学分课程尽量前压）
- 无解场景明确报错：先修环、单门课学分超上限、学期数不够、总学分超容量
- 课程目录可从 TXT 导入，也可从数据库 `course` 表导入
- 排课结果导出为 TXT（默认 `output/plan.txt`）

**数据库联动（第二阶段）**
- 三张业务表 + 一张先修关系表：`student` / `course` / `student_course` / `course_prereq`
- 排课前读取某学生已修课程，自动从待排课程中剔除，并去掉已满足的先修依赖
- 写库去重：同一学生 + 同一课程 + 同一学期不重复插入
- Qt 版所有写库操作都要二次确认，写库后回显「新增 / 重复 / 删除」统计

---

## 2. 目录结构

```
practice_work/
├─ include/                业务层头文件
│  ├─ models.h             Course / PlanConfig / SemesterPlan 等核心结构
│  ├─ scheduler.h          建图、拓扑排序、按学期排课
│  ├─ course_io.h          TXT 导入导出
│  ├─ course_edit.h        课程增删改查
│  └─ db_odbc.h            ODBC 封装（OdbcDb 类）
├─ src/
│  ├─ main.cpp             TUI 入口：菜单、交互、学生管理
│  ├─ scheduler.cpp        排课算法（建图 + 贪心拓扑 + 两种策略）
│  ├─ course_io.cpp        TXT 解析与输出（含格式校验）
│  ├─ course_edit.cpp      课程增删改查与依赖检查
│  ├─ db_odbc.cpp          ODBC 连接与全部 SQL
│  └─ p2_*_test.cpp        数据库阶段的分步验证小工具（不参与主构建）
├─ qt_ui/                  Qt 6 界面层
│  ├─ CMakeLists.txt       只编译界面 + 复用 ../src 业务层
│  ├─ AppSession.h         登录会话（连接串、课程缓存、当前学生）
│  ├─ LoginDialog.*        数据库登录
│  ├─ MainWindow.*         主窗口（三个入口）
│  ├─ StudentPage.*        学生增删改查
│  ├─ StudentCoursePage.*  学生课程历史
│  ├─ CoursePage.*         课程缓存管理 / TXT 与数据库导入 / 同步写库
│  ├─ PlanPage.*           排课参数、生成课表、导出、写库
│  ├─ CourseEditDialog.*   课程编辑表单
│  └─ StudentEditDialog.*  学生编辑表单
├─ data/                   课程目录样例与 4 组测试数据
├─ docs/                   schema.sql、测试记录
├─ output/                 运行产物（plan.txt、scheduler.exe）
├─ scripts/
│  ├─ build.ps1            一键构建（tui / qt / all）
│  └─ run_gui.ps1          启动 Qt 版（固定工作目录）
├─ 需求.txt                题目原文与阶段说明
├─ TODO.md / TODO_QT.md    开发任务清单（已完成打勾）
└─ build_qt/               Qt 生成目录（可删，重新构建即可）
```

`build_qt/`、`*.exe`、`*.obj` 等构建产物均已加入 `.gitignore`，不纳入版本库。

---

## 3. 环境要求

| 项目 | 版本 / 说明 |
| --- | --- |
| 编译器 | MinGW-w64 g++（支持 C++17），默认 `C:\Mingw64\mingw64\bin\g++.exe` |
| CMake | 3.16 以上（Qt 部分） |
| Qt | 6.x `mingw_64`，默认 `C:\Qt\6.10.2\mingw_64` |
| ODBC | MySQL ODBC 9.6 Unicode Driver |
| 数据库 | MySQL 8.x，库名 `university_scheduler`（Navicat Premium 16 管理） |

TUI 版即使没有数据库也能跑「导入 / 编辑 / 排课 / 导出」；只有「学生管理」「同步课程到数据库」这些入口会要求登录。

---

## 4. 构建与运行

```powershell
# 只编译 TUI（约 10 秒）
pwsh -File scripts\build.ps1 -Target tui

# 只编译 Qt 版（首次会走 CMake 配置 + windeployqt）
pwsh -File scripts\build.ps1 -Target qt

# 两个都编译
pwsh -File scripts\build.ps1 -Target all

# 清空产物后重建
pwsh -File scripts\build.ps1 -Target all -Clean
```

运行：

```powershell
# TUI：必须在仓库根目录运行，程序按相对路径找 data/ 和 output/
.\output\scheduler.exe

# GUI
pwsh -File scripts\run_gui.ps1
```

> Qt 版本若单独拷走 `scheduler_qt.exe`，记得把 `platforms\`、`Qt6*.dll` 等运行时文件一起带上（`build.ps1` 首次构建会自动 `windeployqt`）。

---

## 5. 课程数据格式

第一行是课程数量，之后每行一门课，字段用 `|` 分隔，最后一列是先修课程号（逗号分隔，没有就先修留空）：

```
5
C01|高等数学|4|
C02|程序设计|3|
C03|数据结构|4|C02
C04|离散数学|3|C01
C05|算法设计|4|C03,C04
```

导入时会校验：课程数为非负整数、课程号为 3 位字母数字且不重复、学分为正、先修课程号必须已定义。

排课结果导出格式：

```
课程安排
总学期数: 5

学期 1
ID|课程名|学分
C02|程序设计|3
总学分: 3
...
```

---

## 6. 排课算法要点

1. **建图**：边为「先修课程 → 后继课程」，同时统计每门课的入度；先修课程号未定义直接报错。
2. **可行性**：每门课学分不能超过单学期上限；总学分不能超过 `学期上限 × 单学期学分上限`。
3. **逐学期贪心**：每学期从「入度为 0 且未排」的课程里挑课，选完后把这些课的后继入度减一，入度归零的课程进入下一学期候选池。
4. **两种策略的差别只在每学期的挑课顺序**：
   - `FRONT_LOADED`：按学分从大到小排，能放进本学期就放，高学分课尽量前压。
   - `BALANCED`：先算目标学分 = ⌈剩余学分 / 剩余学期数⌉，再按学分从小到大往目标上凑，让各学期负担接近。
5. **失败原因**：超过最大学期数、候选池为空（环或依赖不可达）、某学期挑不出课（学分上限太小）。

复杂度：建图与拓扑部分为 `O(V + E)`，按学期扫描候选池为 `O(学期数 × V log V)`。

---

## 7. 数据库配置

1. 打开 Navicat，执行 `docs/schema.sql`，它会建库 `university_scheduler`、四张表并插入少量示例数据。
2. 确认系统里有 `MySQL ODBC 9.6 Unicode Driver`（Windows「ODBC 数据源」里能看到）。
3. 连接参数写死在代码里，如需改动请看这两处：
   - TUI：`src/main.cpp` 的 `buildOdbcConnStr()`
   - GUI：`qt_ui/AppSession.h` 的 `buildConnStr()`

```text
DRIVER={MySQL ODBC 9.6 Unicode Driver};SERVER=127.0.0.1;PORT=3306;
DATABASE=university_scheduler;UID=<你的账号>;PWD=<你的密码>;OPTION=3;
```

程序首次同步课程时会自动补建 `course_prereq` 表，所以即使只执行了旧脚本也能跑起来。

---

## 8. 测试数据

| 文件 | 场景 | 预期 |
| --- | --- | --- |
| `data/test1_no_prereq.txt` | 4 门课无先修 | 可排 |
| `data/test2_multilevel.txt` | 多层先修链 | 可排且先修顺序正确 |
| `data/test3_cycle.txt` | 先修成环 | 报「存在先修环」 |
| `data/test4_capacity_conflict.txt` | 学期/学分容量不足 | 报容量不足 |

四组用例的「输入 / 预期 / 实际输出」记录在 `docs/test-report.md`，Qt 阶段的界面测试记录在 `docs/qt-test-report.md`。

---

## 9. 已知限制

- 课程号固定 3 位字母数字，导入时不做中文课号的兼容。
- `FRONT_LOADED` 用「学分大优先」近似「集中在前几个学期」，不是严格的最少学期数求解。
- `BALANCED` 是单遍贪心，追求「够用且均衡」，不保证全局最优解。
- 数据库连接串写死在本机 `127.0.0.1:3306`，换机器需要改代码或加配置项。
- `src/main.cpp` 约 1000 行，交互逻辑与业务调用混在一起，后续可按菜单项拆分。
- `docs/qt-test-report.md` 里的截图目录 `docs/screenshots/` 尚未补齐。
