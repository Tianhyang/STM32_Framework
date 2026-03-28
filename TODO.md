# TODO

这个文件记录当前框架“已经能用”，但我认为后续仍然值得继续完善的地方。

## High Priority

- 增加统一错误码体系
  - 现在很多地方还是 `configASSERT()` 或直接返回
  - 后面应逐步补 `status/error` 定义

- 增加更完整的 fault 诊断
  - 目前 fault trap 还是最基础形态
  - 后面可以补寄存器现场抓取、fault 原因输出、重启策略

- 日志模块继续升级
  - 目前已经切到 UART DMA 异步发送
  - 后面仍建议补 ring buffer 水位监控、丢包统计、日志级别控制

- UART 接收框架
  - 现在只有发送和 IRQ 骨架
  - 后面应补接收中断、接收缓冲、命令行或协议收包能力

- 事件队列分层
  - 当前 `app_event` 还是最小总线
  - 后面可拆成 input/service/system/comm 等更清晰的事件域

- 参数与配置管理
  - 还没有运行参数、出厂参数、持久化参数模块

- 构建配置切换
  - 当前还没有 debug/release/profile 等构建档位

## Medium Priority

- 增加 C++ 驱动封装示例
  - 当前只预留了目录
  - 还没有真正的 `Led`、`Uart`、`GpioPin` 类封装

- 增加 board 选择机制
  - 当前 Makefile 还是固定指向一个 board
  - 后面可以改为 `BOARD=`、`CHIP=` 参数化构建

- 增加 chip family 选择机制
  - 当前更适合 STM32F1
  - 后面如果想跨 F1/F4/G0，需要更强的目录和构建抽象

- 增加驱动初始化统一入口
  - 当前驱动初始化分散在使用处
  - 后面可收成 `driver manager` 或更清晰的 service bring-up

- 按键框架继续演进
  - 目前已支持唤醒抽象和基础消抖
  - 后面可加长按、连按、双击、组合键

- 软件定时器服务继续扩展
  - 现在只有一个 heartbeat 示例
  - 后面可做统一 timer registration 接口

- 日志级别与模块过滤
  - 现在没有 `debug/info/warn/error` 分级控制

- 增加版本信息
  - 建议在启动日志中输出 firmware version、build time、git hash

## Lower Priority

- 补充 `docs/architecture`
  - 当前目录已留好，但还没有系统化架构文档

- 补充 `docs/bringup`
  - 建议把硬件连线、下载、串口验证方法记录进去

- 增加测试支撑
  - 可以考虑 host-side 单元测试
  - 或至少增加关键模块的编译级验证

- 增加脚本工具
  - 比如 size 报告、map 分析、自动烧录、串口监控辅助脚本

- Makefile 再整理
  - 后面可以拆成多个 `.mk` 文件，让结构更清晰

- 清理保留目录的使用策略
  - `app/components`
  - `app/services`
  - `app/tasks`
  - `drivers/cpp`
  - 这些目录已经预留，但还没充分使用

## Product-Oriented Next Steps

- 增加 watchdog 策略
- 增加电源上电/复位原因记录
- 增加 non-volatile storage 抽象
- 增加 bootloader / app 分区考虑
- 增加生产测试接口
- 增加通信协议层
- 增加更严格的内存使用监控
- 增加任务运行状态统计

## Not Urgent, But Worth Remembering

- 当前框架更偏“稳定的起始模板”
- 它已经适合继续承载真实项目
- 但如果未来项目复杂度明显上升，建议再引入：
  - 更明确的 service manager
  - 更系统的 module lifecycle
  - 更完整的 config/build/profile 体系
