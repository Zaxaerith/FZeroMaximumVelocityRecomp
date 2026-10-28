# F-Zero: Maximum Velocity Recomp — 接管与开发恢复完整记录

## 1. 已有成果核验 (M0 & M1)
- **工程目录**: `D:\Project\GameRecomp\FZeroMaximumVelocityRecomp`。
- **ROM 完整性与身份**:
  - 文件名: `F-Zero - Maximum Velocity (USA, Europe).gba` (4,194,304 字节)
  - 内部标题: `F-ZERO ADVAN`，游戏代码: `AFZE`，制造商代码: `01` (Nintendo)
  - 存档类型: `SRAM_V111` (32KB SRAM)
  - 校验值: SHA1=`8a08e29ec987f9cbdde21c34d5f7657aa7ba0be6`, CRC32=`0xBD5E9798` (标准 No-Intro ROM)。
- **GBARecomp 版本**: v0.3.0（记录于 `gbarecomp-project.json`）。
- **AOT 生成文件**: 
  - 4 个 C++ 分片 (`recompiled_000.cpp` ~ `recompiled_003.cpp`，共约 96.5MB，包含 10,232 个重编译函数)。
  - `dispatch_table.cpp` (5.5MB，112,695 项入口)、`symbol_map.cpp` (412KB)、`recompiled.h` (568KB)。
- **静态库 `gbarecomp_game.lib`**:
  - 路径: `build/Release/gbarecomp_game.lib` (51,211,736 字节)。
  - 架构: MSVC x64。
  - 完全复用已有产物，未重新生成 AOT，未重复冷编。

## 2. 旧会话执行状态与残留清理
- **残留进程排查**: 检查 `cl.exe`, `link.exe`, `gcc.exe`, `g++.exe`, `cc1plus.exe`, `ninja.exe`, `cmake.exe`, `MSBuild.exe`, `python.exe`，无残留孤立进程。
- **构建环境排查**: 无任何 MinGW 污染，环境为纯净 MSVC x64。

## 3. 本阶段开发推进 (M2 & M3 验证)
- **工具链固定**: 严格固定 MSVC x64 (`Visual Studio 2026` + C++20 + 动态 CRT `/MD`)，编译并行度固定 `--parallel 1`。
- **Windows 宿主与桥接**:
  - 编写原生宿主入口 `src/main.cpp`，配置 ROM 自动寻址、BIOS 自动加载、SRAM 存档重定向与窗口/无头参数控制。
  - 解决框架升级缺失符号：编写 `src/runtime_shim.cpp` 补全 GBARecomp v0.3.0 的原生 RAM 模板注册与等待周期模型（`runtime_register_native_ram_templates`, `runtime_prefetch_stall_delta`, `runtime_exception_return_refill` 等 8 个接口，含官方 ARM 立即数循环右移解码器）。
  - 更新 `CMakeLists.txt`，添加目标 `FZeroMaximumVelocityRecomp`，配置 64MB 堆栈限制 (`/STACK:67108864`)，并完成 Windows x64 原生宿主程序链接。
- **可执行程序产物**:
  - `build/Release/FZeroMaximumVelocityRecomp.exe` (20,747,264 字节，~20.7 MB)。
  - `build/Release/SDL2.dll` (配套 x64 运行时库)。
- **MIC 权限修复与窗口运行 (M3)**:
  - 经用户授权执行 `icacls $root /setintegritylevel "(OI)(CI)M" /T /C`，将项目目录及文件的强制完整性级别从 `Low` 恢复为 `Medium`。
  - `0xC0000142` 与 USER32 错误 1114 彻底消除，Direct3D11 SDL 窗口成功创建 (720x480)。

## 4. 游戏全流程验证 (M4, M5, M6)
- **M4: 标题画面验证**:
  - 启动后依次正常渲染任天堂官方椭圆 Logo 与 NdCube 开场。
  - 顺利进入真实标题画面："F-ZERO MAXIMUM VELOCITY"、"PRESS START"、"© 2001 Nintendo"，文字、背景与配色 100% 准确。
- **M5: 第一场比赛加载**:
  - 响应 START 键进入存档管理与档案创建画面。
  - 顺利通过键盘输入并持久化 Slot 1 档案。
  - 正式载入第一条赛道："BIANCA CITY: STRETCH CIRCUIT"。
  - 成功展示赛道全景 3D Mode 7 漫游，切入发车起跑线，展示玩家赛车（Hot Violet）及周边竞争对手 Sprite 与 HUD 界面。
- **M6: 基本可玩性验证**:
  - 发车起跑后，A 键油门加速正常，时速突破 197 km/h。
  - 赛车左转、右转正常，Mode 7 赛道随视角实时旋转平滑，逆向行驶时正确触发 "TURN BACK" 警告。
  - 比赛计时器（已跑出 0'09"30）与排名（Rank 4/15）动态跳动。
  - START 键暂停正常，弹出 "CONTINUE / GIVE UP" 菜单。
  - SRAM 存档（`saves/fzero_maximum_velocity.sav`，32,768 字节）在开机与建档时正常写入。
- **综合状态码**: `FIRST_RACE_PLAYABLE`
