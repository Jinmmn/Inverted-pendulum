# Inverted Pendulum

基于 NXP Kinetis KL25Z（MKL25Z128xxx4 / Cortex-M0+）的倒立摆控制课程设计工程。项目使用 Keil MDK uVision 管理，采用裸机 C 编写，包含角度采样、编码器测速、PWM 电机驱动、起摆逻辑和双环 PID 平衡控制。

## 项目概览

本工程面向单电机倒立摆小车或类似实验平台，控制流程如下：

1. 初始化角度传感器、编码器和电机 PWM。
2. 通过固定方向脉冲完成起摆。
3. 检测摆杆进入中心角附近后切换到平衡控制。
4. 使用角度 PID 控制电机输出，使用位置 PID 修正角度目标值，抑制小车位置漂移。
5. 摆杆偏离安全范围时自动停止电机。

## 硬件与开发环境

- MCU：NXP MKL25Z128xxx4
- CPU：ARM Cortex-M0+
- IDE：Keil MDK uVision
- 编译器：ARMCC 5（当前构建日志为 V5.06 update 4）
- Device Pack：Keil Kinetis_KLxx_DFP 1.7.0
- CMSIS：5.0.0
- 工程文件：`project1.uvprojx`
- 目标名：`Target 1`

## 引脚分配

| 功能 | MCU 外设 | 引脚 | 说明 |
| --- | --- | --- | --- |
| 角度采样 | ADC0_SE8 | PTB0 | 读取摆杆角度原始 ADC 值 |
| 电机 IN1 | TPM1_CH0 | PTA12 | PWM 输出，控制一个方向 |
| 电机 IN2 | TPM1_CH1 | PTA13 | PWM 输出，控制反方向 |
| 编码器 A 相 | GPIO/PORTD IRQ | PTD0 | 双边沿中断计数 |
| 编码器 B 相 | GPIO/PORTD IRQ | PTD2 | 双边沿中断计数 |

## 目录结构

```text
.
├── main.c                  # 程序入口、系统初始化、SysTick 调度
├── control.c/.h            # 起摆状态机、平衡控制、位置控制
├── pid.c/.h                # 通用 PID 控制器
├── motor.c/.h              # TPM1 PWM 电机驱动
├── encoder.c/.h            # 增量式编码器读取
├── angle_sensor.c/.h       # ADC 角度采样
├── RTE/                    # Keil/CMSIS 运行时环境与启动文件
├── project1.uvprojx        # Keil uVision 工程文件
└── project1.uvoptx         # Keil 工程选项文件
```

`Objects/`、`Listings/`、编译日志和 uVision 用户界面状态文件属于生成文件或个人配置，已在 `.gitignore` 中排除，不建议提交到 GitHub。

## 控制逻辑

核心控制代码位于 `control.c`。

- `CONTROL_SWING_POS_*` / `CONTROL_SWING_NEG_*`：起摆阶段，按固定 PWM 和持续时间向两侧施加驱动。
- `CONTROL_SWING_WATCH`：采样角度变化趋势，判断是否接近中心位置。
- `CONTROL_BALANCE`：进入闭环平衡控制。
- `CONTROL_STOP`：停止电机输出。

主要控制参数：

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `CENTER_ANGLE_DEFAULT` | `2010` | 平衡中心角 ADC 值，需要按实际机械零位校准 |
| `CENTER_RANGE` | `500` | 允许进入平衡控制的角度窗口 |
| `START_PWM` | `35` | 起摆 PWM 输出 |
| `START_TIME_MS` | `100` | 起摆单次驱动持续时间 |
| `BALANCE_PERIOD_MS` | `5` | 角度 PID 更新周期 |
| `LOCATION_PERIOD_MS` | `50` | 位置 PID 更新周期 |
| `MOTOR_POLARITY` | `-1` | 电机方向极性修正 |

PID 默认参数：

| 控制环 | Kp | Ki | Kd | 输出范围 |
| --- | --- | --- | --- | --- |
| 角度环 | `0.3` | `0.01` | `0.4` | `[-100, 100]` |
| 位置环 | `0.4` | `0.0` | `4.0` | `[-100, 100]` |

## 构建方法

1. 安装 Keil MDK uVision。
2. 安装 `Keil.Kinetis_KLxx_DFP.1.7.0` 或兼容版本的 Kinetis KLxx Device Pack。
3. 使用 uVision 打开 `project1.uvprojx`。
4. 选择 `Target 1`。
5. 执行 `Build`。

当前工程最近一次构建结果：

```text
Program Size: Code=4456 RO-data=240 RW-data=128 ZI-data=1120
0 Error(s), 0 Warning(s)
```

## 烧录与调试

在 Keil 中连接调试器后，可使用 `Download` 或 `Start/Stop Debug Session` 烧录和调试。工程目标芯片为 `MKL25Z128xxx4`，Flash 范围为 `0x00000000 - 0x00020000`，RAM 范围为 `0x1FFFF000 - 0x20002FFF`。

调试建议：

- 先确认角度传感器在直立位置附近的 ADC 值，再修改 `control.h` 中的 `CENTER_ANGLE_DEFAULT`。
- 如果电机方向相反，优先修改 `control.c` 中的 `MOTOR_POLARITY`。
- 起摆力度不足时，适当调整 `START_PWM` 和 `START_TIME_MS`。
- 平衡后位置漂移明显时，重点调节位置环 PID 参数。

## 上传 GitHub 前建议

推荐提交以下文件：

- `*.c` / `*.h`
- `RTE/`
- `project1.uvprojx`
- `project1.uvoptx`
- `.gitignore`
- `README.md`

不推荐提交以下文件：

- `Objects/`
- `Listings/`
- `build_output.log`
- `project1.uvguix.*`
- 调试器日志、编辑器缓存和临时文件

首次上传可参考：

```bash
git add .
git commit -m "Initial inverted pendulum project"
git branch -M main
git remote add origin https://github.com/<your-name>/<repo-name>.git
git push -u origin main
```

如果仓库已经配置过远程地址，只需要执行：

```bash
git add .
git commit -m "Prepare project for GitHub"
git push
```

## 注意事项

- 本项目为嵌入式裸机工程，不能直接在 PC 上运行。
- PID 参数和中心角与具体机械结构、电机、供电和传感器安装方式强相关，需要实机调试。
- 电机调试时建议架空小车或断开负载，避免起摆阶段突然动作造成机械损伤。
