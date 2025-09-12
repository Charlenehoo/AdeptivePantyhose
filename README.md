# AdeptivePantyhose

**Version:** 2.0.0  
**Author:** Charlene Hoo  
**Email:** CharleneHoo@hotmail.com

AdeptivePantyhose 是一个 Skyrim **SKSE 插件**，用于动态调整女性角色脚部形态，实现高跟鞋、平底鞋与赤脚状态下的**无缝 BodyMorph 效果**。

## 功能特性

- **动态 Morph 调整**  
  根据玩家装备的鞋类自动切换 BodyMorph。
  - 高跟鞋：清除 `NoHeel` Morph
  - 平底鞋 / 赤脚：应用 `NoHeel` Morph
- **支持所有女性可玩角色**
  - 检测 NPC / 玩家角色
  - 检查种族可玩性
- **兼容 CommonLibSSE-NG 与 SKEE 接口**
- **事件驱动**  
  使用 SKSE `TESEquipEvent` 监听装备变化
- **日志记录**  
  详细日志记录 Morph 应用情况，便于调试

## 安装

1. 将插件编译生成的 `.dll` 文件放入：
   <Skyrim Mod Folder>/SKSE/Plugins/
2. 确保 CommonLibSSE-NG 已正确安装。
3. 启动游戏并观察日志以确认插件加载成功。

## 构建

项目使用 CMake + Ninja：

```bash
git clone <repo_url>
cd AdeptivePantyhose
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

依赖：CommonLibSSE-NG

C++ 标准：C++23

编译器：MSVC (cl.exe)
