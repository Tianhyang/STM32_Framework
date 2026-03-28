# STM32 Firmware Framework

这个工程现在已经不是单纯的一个 `STM32F103RCT6` 点灯例程，而是一套可以继续复用的嵌入式项目框架。

它当前已经具备这些基础能力：

- STM32 启动文件、链接脚本、CMSIS、HAL 基线
- FreeRTOS 任务调度
- 板级初始化和时钟配置
- LED、UART、KEY 的基础 BSP 驱动
- 统一日志入口
- 日志通过 UART DMA 异步发送
- 统一应用事件队列
- 基础 IRQ 路由层
- 基础软件定时器服务
- 应用层与平台层的初步分离

历史开发过程、每一步做了什么、为什么这么做，请看 [搭建过程记录.md](/Users/tianhyang/STM32/Project_2/搭建过程记录.md)。

## 1. Framework Overview

### 1.1 Design Goal

这套框架的目标不是“为了某一个功能临时写通”，而是：

- 让应用层尽量少碰芯片寄存器和芯片型号细节
- 让板级差异尽量收敛在 `platform/boards` 和 `drivers/bsp`
- 让以后更换芯片、换板子、加外设、加应用时，都有明确落点
- 让初学阶段也能接触到接近量产项目的分层思路

### 1.2 Layering

大体分层如下：

- `app`
  - 应用层
  - 放业务逻辑、应用任务、应用事件处理、应用服务编排
- `drivers/interface`
  - 驱动抽象接口层
  - 放和平台无关的驱动 API 定义，例如 `led.h`、`key.h`
- `drivers/bsp`
  - 板级驱动实现层
  - 用 STM32 HAL/芯片能力实现具体驱动
- `platform`
  - 平台层
  - 放芯片、板级、CMSIS、HAL、启动文件、中断入口、时钟、链接相关内容
- `rtos`
  - RTOS 配置与项目适配层
  - 放 `FreeRTOSConfig.h`、hook、包装层
- `toolchain`
  - 工具链相关文件
  - 链接脚本、OpenOCD 配置等
- `third_party`
  - 上游第三方代码和压缩包
  - STM32CubeF1、FreeRTOS 内核源码
- `docs`
  - 参考资料目录
  - 数据手册、参考手册、架构笔记、bring-up 记录

### 1.3 Folder Guide

下面是当前目录的用途说明。

- [app](/Users/tianhyang/STM32/Project_2/app)
  - 应用层总目录
- [app/include](/Users/tianhyang/STM32/Project_2/app/include)
  - 应用层头文件
  - 例如应用入口、事件、日志、任务配置
- [app/src](/Users/tianhyang/STM32/Project_2/app/src)
  - 应用层实现
  - 当前包括 `app.c`、`app_event.c`、`app_service.c`、`app_task.c`、`app_system.c`、`app_timer.c`
- [app/components](/Users/tianhyang/STM32/Project_2/app/components)
  - 预留给更大的应用组件
  - 比如状态机、协议解析、控制器组件
- [app/services](/Users/tianhyang/STM32/Project_2/app/services)
  - 预留给更重一点的应用服务
  - 比如存储服务、CLI 服务、升级服务
- [app/tasks](/Users/tianhyang/STM32/Project_2/app/tasks)
  - 预留给独立任务模块

- [drivers/interface](/Users/tianhyang/STM32/Project_2/drivers/interface)
  - 面向上层的驱动接口
  - 上层原则上依赖这里，不直接依赖 HAL 头文件
- [drivers/bsp](/Users/tianhyang/STM32/Project_2/drivers/bsp)
  - 板级驱动实现
  - 目前有 `gpio`、`led`、`uart`
- [drivers/cpp](/Users/tianhyang/STM32/Project_2/drivers/cpp)
  - 预留给 C++ 封装
  - 未来可以在这里做 `class Led`、`class Uart`、`class GpioPin`

- [platform/boards/stm32f103rct6_generic](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic)
  - 当前这块板子的板级目录
  - 放板级引脚映射、时钟策略、IRQ 路由、板级初始化
- [platform/chips/stm32f1](/Users/tianhyang/STM32/Project_2/platform/chips/stm32f1)
  - 芯片家族级配置
  - 当前放项目芯片相关配置头
- [platform/cmsis](/Users/tianhyang/STM32/Project_2/platform/cmsis)
  - CMSIS core 和 device 层
- [platform/hal/stm32f1](/Users/tianhyang/STM32/Project_2/platform/hal/stm32f1)
  - 复制到工程内的 HAL 头文件和源码
- [platform/ll/stm32f1](/Users/tianhyang/STM32/Project_2/platform/ll/stm32f1)
  - 预留给 LL 驱动
- [platform/startup/stm32f103rct6](/Users/tianhyang/STM32/Project_2/platform/startup/stm32f103rct6)
  - 启动文件

- [rtos/freertos/config](/Users/tianhyang/STM32/Project_2/rtos/freertos/config)
  - `FreeRTOSConfig.h`
- [rtos/wrappers](/Users/tianhyang/STM32/Project_2/rtos/wrappers)
  - RTOS hook 与项目适配层

- [toolchain/linker](/Users/tianhyang/STM32/Project_2/toolchain/linker)
  - 链接脚本
- [toolchain/openocd](/Users/tianhyang/STM32/Project_2/toolchain/openocd)
  - OpenOCD 配置

### 1.4 Startup Flow

当前软件启动路径大致是：

1. 启动文件设置栈和向量表
2. 进入 `main()`
3. `board_init()`
4. `app_init()`
5. `app_start()`
6. `vTaskStartScheduler()`

在应用层内部又进一步拆成：

- `app_service_init()`
  - 初始化日志、事件队列、timer service 等
- `app_task_start()`
  - 创建应用事件任务、LED 任务、KEY 任务等

这套路径的意义是：

- 早期初始化放在 scheduler 启动前
- 运行态行为放在 task 或 event 中
- 不让 `main.c` 演化成一个巨大的“所有东西都往里塞”的文件

## 2. If You Change to Another Chip

### 2.1 What Usually Changes

如果你换芯片，一般变化会落在这几层：

- 启动文件
- 链接脚本
- 芯片头文件和 HAL/LL
- 时钟配置
- 中断号和优先级
- 板级引脚映射
- 编译宏

应用层理论上尽量不改，或者只做极少量适配。

### 2.2 Files You Should Check First

如果你从 `STM32F103RCT6` 换到别的芯片，优先检查这些文件和目录：

- [Makefile](/Users/tianhyang/STM32/Project_2/Makefile)
  - 改芯片宏、源文件路径、启动文件路径
- [toolchain/linker/stm32f103rct6_flash.ld](/Users/tianhyang/STM32/Project_2/toolchain/linker/stm32f103rct6_flash.ld)
  - 改 Flash/RAM 大小、起始地址
- [platform/startup/stm32f103rct6](/Users/tianhyang/STM32/Project_2/platform/startup/stm32f103rct6)
  - 换成目标芯片对应 startup 文件
- [platform/cmsis/device/st/stm32f1xx/include](/Users/tianhyang/STM32/Project_2/platform/cmsis/device/st/stm32f1xx/include)
  - 换 device header，例如 `stm32f103xe.h`
- [platform/cmsis/device/st/stm32f1xx/src/system_stm32f1xx.c](/Users/tianhyang/STM32/Project_2/platform/cmsis/device/st/stm32f1xx/src/system_stm32f1xx.c)
  - 如果跨家族，通常也要换
- [platform/hal/stm32f1](/Users/tianhyang/STM32/Project_2/platform/hal/stm32f1)
  - 如果跨家族，比如换到 F4/G0/H7，HAL 整套都要换
- [platform/chips/stm32f1/include/project_config.h](/Users/tianhyang/STM32/Project_2/platform/chips/stm32f1/include/project_config.h)
  - 改项目芯片相关宏
- [platform/boards/stm32f103rct6_generic/include/board_config.h](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/include/board_config.h)
  - 改板级引脚、UART、LED、KEY、IRQ 映射
- [platform/boards/stm32f103rct6_generic/src/board.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/board.c)
  - 改时钟树和板级初始化
- [platform/boards/stm32f103rct6_generic/src/board_irq.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/board_irq.c)
  - 改中断入口与 IRQ 分发
- [platform/boards/stm32f103rct6_generic/src/stm32f1xx_it.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/stm32f1xx_it.c)
  - 改向量层包装函数

### 2.3 Recommended Migration Method

推荐这样迁移，不要一次性全改：

1. 新建一个新的 board 目录
   - 例如 `platform/boards/stm32xxxx_new_board`
2. 如果只是换板不换芯片家族
   - 优先改 `board_config.h`、`board.c`、`board_irq.c`
3. 如果换了芯片容量但还是同一家族
   - 再改 startup、device header、linker script、编译宏
4. 如果跨家族
   - 新建新的 `platform/chips/...`
   - 新建新的 `platform/hal/...`
   - 新建新的 `platform/cmsis/device/...`
5. 在 `Makefile` 中切换到新的路径和宏
6. 先验证最小工程
   - 空跑
   - SysTick
   - LED
   - UART
7. 最后再接回 FreeRTOS、事件、应用层

### 2.4 Migration Rule of Thumb

迁移时尽量遵守下面这条经验：

- 先让底层重新“亮灯 + 串口打印”
- 再让 FreeRTOS 跑起来
- 再把应用层接回去

不要一上来就指望整个旧应用无缝跑起来，那样不好定位问题。

## 3. If You Want to Add a New Peripheral

### 3.1 Recommended Placement

新增外设时，建议按下面方式放文件。

如果是“对上层暴露一个通用驱动接口”：

- 在 [drivers/interface/include](/Users/tianhyang/STM32/Project_2/drivers/interface/include) 新增接口头文件
  - 例如 `spi_bus.h`
  - 例如 `i2c_bus.h`
  - 例如 `pwm.h`

如果是“这块板子上这个外设的 STM32 实现”：

- 在 [drivers/bsp](/Users/tianhyang/STM32/Project_2/drivers/bsp) 下新增对应目录
  - 例如 `drivers/bsp/spi`
  - 例如 `drivers/bsp/i2c`
  - 例如 `drivers/bsp/pwm`

如果有引脚和实例映射：

- 在 [board_config.h](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/include/board_config.h) 里新增宏

如果需要初始化流程：

- 在对应 BSP 驱动里实现 `xxx_init()`
- 由应用或服务层决定何时调用

如果涉及中断：

- 在 [board_irq.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/board_irq.c) 加 IRQ 路由
- 在 [stm32f1xx_it.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/stm32f1xx_it.c) 加向量入口包装

### 3.2 Example Procedure

比如你要新增 `SPI1`：

1. 在 `drivers/interface/include` 增加 `spi_bus.h`
2. 在 `drivers/bsp/spi/include` 增加 `bsp_spi.h`
3. 在 `drivers/bsp/spi/src` 增加 `bsp_spi.c`
4. 在 `board_config.h` 写 `SPI1` 的 SCK/MISO/MOSI/CS 引脚
5. 在 `Makefile` 把新文件加进 `C_SOURCES`
6. 如需中断，在 `board_irq.c` 和 `stm32f1xx_it.c` 加 IRQ 路由
7. 先写最小收发验证
8. 再让上层业务去调用

### 3.3 Suggested Naming

建议统一命名风格：

- 抽象接口：`xxx.h`
- 板级实现：`bsp_xxx.h/.c`
- 应用层使用：只 include 抽象接口，尽量不直接 include `bsp_*.h`

### 3.4 What Not to Do

尽量不要这样做：

- 在应用层直接调用 `HAL_GPIO_WritePin()`、`HAL_UART_Transmit()`
- 在多个业务文件里散落相同的引脚定义
- 把 IRQ 逻辑直接写满到 `stm32f1xx_it.c`
- 让驱动直接依赖某个具体 app 任务

## 4. If You Want to Add a New Application Based on Existing Drivers

### 4.1 Recommended Flow

如果你已经有现成驱动，比如 LED、KEY、UART，现在想基于它们做一个新的应用，建议这样做：

1. 在 [app/include](/Users/tianhyang/STM32/Project_2/app/include) 增加应用头文件
2. 在 [app/src](/Users/tianhyang/STM32/Project_2/app/src) 增加应用实现
3. 如果它是一个独立任务
   - 在 [app_task_config.h](/Users/tianhyang/STM32/Project_2/app/include/app_task_config.h) 增加栈和优先级定义
4. 在 [app_task.c](/Users/tianhyang/STM32/Project_2/app/src/app_task.c) 统一启动它
5. 如果它有事件输入
   - 优先接入 [app_event.c](/Users/tianhyang/STM32/Project_2/app/src/app_event.c) 统一分发
6. 如果它是周期性工作
   - 优先考虑挂到 [app_timer.c](/Users/tianhyang/STM32/Project_2/app/src/app_timer.c) 或一个独立 task，而不是随手在别的模块里加 delay

### 4.2 Current Examples You Can Follow

你现在可以参考已有例子：

- LED 任务
  - [app_led_demo.c](/Users/tianhyang/STM32/Project_2/app/src/app_led_demo.c)
- 按键输入和事件
  - [app_key_demo.c](/Users/tianhyang/STM32/Project_2/app/src/app_key_demo.c)
  - [app_key_input.c](/Users/tianhyang/STM32/Project_2/app/src/app_key_input.c)
  - [app_key.c](/Users/tianhyang/STM32/Project_2/app/src/app_key.c)
- 应用事件总线
  - [app_event.c](/Users/tianhyang/STM32/Project_2/app/src/app_event.c)
- 系统事件处理
  - [app_system.c](/Users/tianhyang/STM32/Project_2/app/src/app_system.c)

### 4.3 A Good Pattern

推荐你新增应用时沿着下面的思路：

- 驱动负责“硬件访问”
- 输入模块负责“采样/唤醒”
- 事件模块负责“分发”
- 应用模块负责“业务决策”

例如未来做一个菜单系统：

- `key` 驱动读按键
- `app_key_input` 负责唤醒
- `app_key_demo` 或未来的 `input service` 负责发布事件
- `menu_app` 负责消费事件和切换菜单状态

## 5. If You Want to Write an Interrupt

### 5.1 Recommended IRQ Path

建议始终沿着这条路径加中断：

1. 在启动文件里确认该中断向量存在
2. 在 [stm32f1xx_it.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/stm32f1xx_it.c) 增加一个很薄的入口函数
3. 在 [board_irq.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/board_irq.c) 增加对应路由处理函数
4. 在具体驱动里实现真正的 IRQ 处理
5. 如果要和任务交互
   - 尽量用 `queue`、`task notification`、`semaphore`
   - 不要在 ISR 里做复杂业务

### 5.2 Example

例如增加 `TIM2_IRQHandler`：

1. 在 `stm32f1xx_it.c` 增加 `TIM2_IRQHandler()`
2. 在 `board_irq.h/.c` 增加 `board_irq_handle_tim2()`
3. 在 `board_irq_handle_tim2()` 里调用 `bsp_timer_irq_handler()` 或 `HAL_TIM_IRQHandler()`
4. 在驱动层或服务层里决定中断结束后怎么通知任务

### 5.3 ISR Best Practices

建议遵守这些规则：

- ISR 里只做最少的事情
- 优先清标志、搬最少量数据、发通知
- 不在 ISR 里打印日志
- 不在 ISR 里做格式化字符串
- 不在 ISR 里做阻塞操作
- 如果和 FreeRTOS 交互，确认该中断优先级满足 `configMAX_SYSCALL_INTERRUPT_PRIORITY`

### 5.4 Files to Check for IRQ Work

- [platform/boards/stm32f103rct6_generic/src/stm32f1xx_it.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/stm32f1xx_it.c)
- [platform/boards/stm32f103rct6_generic/src/board_irq.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/board_irq.c)
- [platform/boards/stm32f103rct6_generic/include/board_config.h](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/include/board_config.h)
- [rtos/freertos/config/FreeRTOSConfig.h](/Users/tianhyang/STM32/Project_2/rtos/freertos/config/FreeRTOSConfig.h)

## 6. Common Future Scenarios

### 6.1 If You Want to Add C++ Wrappers

当前工程已经预留了 [drivers/cpp](/Users/tianhyang/STM32/Project_2/drivers/cpp)。

建议做法：

- 先保证底层 C 接口稳定
- 再用 C++ 在 `drivers/cpp` 做轻量封装
- 不要让 C++ 类直接把 HAL 细节暴露给 app

一个比较合适的方向是：

- `drivers/interface/include/led.h` 保持 C 接口
- `drivers/cpp/bsp/Led.hpp` 做 RAII 或对象化包装
- 上层应用可以逐步切到 C++，底层 C 保持稳定

### 6.2 If You Want to Add CLI or Serial Command Processing

建议放置方式：

- 驱动层继续只管 `uart`
- 在 `app/services` 新增 `cli_service`
- 命令解析出来的结果优先转成 app event

### 6.3 If You Want to Add Storage or Flash Parameters

建议新增：

- `drivers/interface/include/storage.h`
- `drivers/bsp/flash` 或类似目录
- `app/services/config_service`

不要让业务代码到处直接写 Flash 地址常量。

### 6.4 If You Want to Add a Communication Protocol

比如 Modbus、自定义串口协议、CAN 协议：

- 驱动层处理底层收发
- 协议层放在 `app/components` 或 `app/services`
- 解析结果再进入 app event 或 service 内部状态机

### 6.5 If You Want to Add Another Board Based on the Same MCU

最推荐的方式不是改原目录，而是复制一个新 board 目录，例如：

- `platform/boards/stm32f103rct6_board_a`
- `platform/boards/stm32f103rct6_board_b`

然后通过 `Makefile` 切换。这样比在一个 `board_config.h` 里堆很多 `#ifdef` 更清晰。

### 6.6 If You Want to Move from Demo to Product

建议演进顺序：

1. 稳定时钟和最小驱动
2. 增加日志和错误码
3. 增加参数管理
4. 增加 CLI 或调试接口
5. 增加单元级验证和板级回归验证脚本
6. 最后再做复杂业务功能

## 7. Current Important Files

如果你以后快速回看工程，优先看这些文件：

- 顶层入口
  - [app/src/main.c](/Users/tianhyang/STM32/Project_2/app/src/main.c)
- 应用编排
  - [app/src/app.c](/Users/tianhyang/STM32/Project_2/app/src/app.c)
  - [app/src/app_service.c](/Users/tianhyang/STM32/Project_2/app/src/app_service.c)
  - [app/src/app_task.c](/Users/tianhyang/STM32/Project_2/app/src/app_task.c)
- 事件与系统服务
  - [app/src/app_event.c](/Users/tianhyang/STM32/Project_2/app/src/app_event.c)
  - [app/src/app_system.c](/Users/tianhyang/STM32/Project_2/app/src/app_system.c)
  - [app/src/app_timer.c](/Users/tianhyang/STM32/Project_2/app/src/app_timer.c)
- 输入路径
  - [app/src/app_key_input.c](/Users/tianhyang/STM32/Project_2/app/src/app_key_input.c)
  - [app/src/app_key_demo.c](/Users/tianhyang/STM32/Project_2/app/src/app_key_demo.c)
  - [app/src/app_key.c](/Users/tianhyang/STM32/Project_2/app/src/app_key.c)
- 日志和 UART
  - [app/src/log.c](/Users/tianhyang/STM32/Project_2/app/src/log.c)
  - [drivers/bsp/uart/src/bsp_uart.c](/Users/tianhyang/STM32/Project_2/drivers/bsp/uart/src/bsp_uart.c)
- 板级层
  - [platform/boards/stm32f103rct6_generic/src/board.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/board.c)
  - [platform/boards/stm32f103rct6_generic/include/board_config.h](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/include/board_config.h)
  - [platform/boards/stm32f103rct6_generic/src/board_irq.c](/Users/tianhyang/STM32/Project_2/platform/boards/stm32f103rct6_generic/src/board_irq.c)
- RTOS 配置
  - [rtos/freertos/config/FreeRTOSConfig.h](/Users/tianhyang/STM32/Project_2/rtos/freertos/config/FreeRTOSConfig.h)
- 构建与下载
  - [Makefile](/Users/tianhyang/STM32/Project_2/Makefile)
  - [toolchain/linker/stm32f103rct6_flash.ld](/Users/tianhyang/STM32/Project_2/toolchain/linker/stm32f103rct6_flash.ld)

## 8. Suggested Rules for Reusing This Framework

为了让这个框架真的能复用，建议你以后尽量坚持下面这些规则：

- 新业务先想清楚它属于 `driver`、`service`、`component` 还是 `app task`
- 应用层尽量不要直接 include HAL 头文件
- 引脚、UART、IRQ、时钟映射尽量统一收敛在 `board_config.h` 和 `board.c`
- 中断入口薄，业务不进 ISR
- 用事件队列和服务层组织复杂逻辑，不要跨模块直接乱调用
- 加新芯片时，先让 LED 和 UART 通，再去接业务
- README 负责“怎么复用这个框架”
- [搭建过程记录.md](/Users/tianhyang/STM32/Project_2/搭建过程记录.md) 负责“这次是怎么一步步搭出来的”
- [TODO.md](/Users/tianhyang/STM32/Project_2/TODO.md) 负责“还没做但值得做的事”

## 9. Build and Flash

常用命令：

```bash
make
make flash
make erase
make reset
```

可选 OpenOCD 命令：

```bash
make flash-openocd
make erase-openocd
make debug-server
```

串口当前默认：

- `USART1`
- `115200`
- `8N1`

## 10. Final Note

现在这套工程已经适合作为你后续 STM32 项目的起始模板，但它还不是“完全产品级”的最终形态。哪些地方下一步还值得继续完善，请看 [TODO.md](/Users/tianhyang/STM32/Project_2/TODO.md)。
