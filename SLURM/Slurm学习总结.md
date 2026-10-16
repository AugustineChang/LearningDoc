# Slurm 学习总结

版本：初稿，2026-10-13。本文按已确认的大纲编写，面向刚接触现有 Slurm 集群的读者，便于后续逐节修改。

文中的集群观察来自此前提供的命令输出，是当时的快照，不代表当前实时状态。已知客户端版本为 Slurm 24.11.0；官方在线文档可能对应更新版本，具体行为应结合本机手册和管理员配置确认。所有脚本均为教学示例，本文编写过程中没有连接远程集群、提交作业或修改集群配置。

## 1. Slurm 是什么

Slurm 是集群的资源管理和作业调度系统。多个用户同时需要服务器时，它记录资源申请，安排排队，选择符合条件的计算节点，并协调程序执行。

它主要回答三个问题：哪些资源可以使用、哪个作业先运行、作业在哪里运行。它不会阅读训练代码并自动推算最优资源需求，也不会把普通单机程序自动改造成多机程序。资源需求由用户声明，默认值和限制由管理员配置；程序内部如何并行仍由训练框架和代码决定。

例如，用户申请两台服务器、每台四张 GPU。Slurm 检查权限和资源，安排分配；PyTorch 再负责让多个 GPU 上的训练进程协同计算。

参考：[Slurm 架构概览](https://slurm.schedmd.com/overview.html)。

## 2. Slurm 与集群由什么组成

### 2.1 节点角色与后台程序

| 角色 | 主要职责 | 常见程序 |
|---|---|---|
| 主控制节点 | 接收请求、维护全局状态、调度资源 | `slurmctld` |
| 备用控制节点 | 主控制服务故障时接管 | `slurmctld` |
| 计算节点 | 报告本机状态、执行计算 | `slurmd`、运行期间的 `slurmstepd` |
| 登录／提交节点 | 用户 SSH 登录、编辑代码、提交和查询作业 | `sbatch`、`squeue` 等客户端 |
| 记账服务节点 | 保存和查询用量、作业历史、账号关联等 | 可选的 `slurmdbd`，连接数据库 |
| 存储节点 | 提供代码、数据和结果的共享存储 | 文件系统服务，属于配套基础设施 |

这些是部署角色，不要求每种角色对应一台独立机器。控制节点可以兼任计算节点，也可以允许用户直接登录；大型集群通常把不同负载分开。

### 2.2 三个容易混淆的名字

`slurmctld`、`slurmd`、`slurmstepd` 是程序名称，运行后成为进程，并各自拥有操作系统分配的 PID。它们不是保存 PID 的变量，也不是配置中的 key。末尾的 `d` 表示 daemon，即后台服务进程。

控制器 `slurmctld` 管理整个集群；每台计算节点的 `slurmd` 管理本机；`slurmd` 启动 `slurmstepd` 管理具体的作业步骤。`slurmstepd` 负责启动、跟踪任务进程及处理相关输入输出和清理。前两个通常长期运行，后者随步骤创建和退出。

```text
用户客户端 → slurmctld：资源申请与调度
计算节点上的 slurmd → slurmstepd → Python 等任务进程
```

这是职责图，不是所有消息都必须经过控制器的完整网络通信图。作业步骤启动还涉及 `srun` 与计算节点之间的通信。

### 2.3 登录节点的角色与权限

登录节点是用户进入集群、编辑代码、提交作业和查看结果的入口。通常需要允许 SSH 登录，安装 Slurm 客户端，并具备正确的集群配置、认证、用户身份和网络访问。它不需要 GPU，也不要求运行 `slurmctld` 或 `slurmd`。

登录节点可以专门部署，也可以由控制节点或计算节点兼任。用户从这里向控制服务发送请求，不必先登录控制节点；能否直接登录其他节点，由管理员决定。

**SSH 登录权限不等于 Slurm 资源分配。** 直接执行 `python train.py` 会在当前机器运行，用 `sbatch` 提交才进入调度。若登录节点兼任计算节点，需要通过访问限制和资源隔离，避免手工程序干扰作业，相关原理见 8.4。

此外，`sbatch` 只传送作业脚本，不会自动传送代码、数据和运行环境，计算节点需要另外能够访问这些内容。

### 2.4 一个物理服务器能否对应多个 Slurm 节点

通常一个计算节点对应一台服务器。但 Slurm 节点更准确地说是调度系统中的资源实体，可以对应虚拟机或特殊配置的逻辑节点。

简单启动两份 `slurmd` 不会自动创建两个节点，相同端口和节点身份通常会冲突。同机多实例需要专门的构建支持、不同的 `NodeName`、端口和独立运行文件。硬件不会因此增加；必须避免两个逻辑节点重复声明、使用同一批资源。

参考：[作业启动原理](https://slurm.schedmd.com/job_launch.html)、[同机多 slurmd 支持](https://slurm.schedmd.com/programmer_guide.html)。

## 3. 核心概念及其关系

### 3.1 分区 Partition

分区是节点集合及其使用规则，类似任务队列入口。规则可以包含运行时间上限、用户权限、节点范围和调度优先层级。分区本身不是独立的调度程序，真正分配资源的是控制器及其调度插件。

同一节点可以属于多个分区。Slurm 统一记录它的资源，不会为每个分区复制一套硬件。例如八张 GPU 的节点，两个分区的作业分别申请两张和四张，正常的独占 GPU 分配下只剩两张可分配。

允许 CPU 或 GPU 共享的特殊配置另有规则，不能把所有集群都理解成完全独占。

### 3.2 Job、Step、Task

三者是包含关系：**一个 Job 可以包含多个 Step，一个 Step 可以启动多个 Task。**

- **Job（作业）**：一次提交的整体工作，向 Slurm 申请资源。作业必须提交向某一分区。
- **Step（步骤）**：作业中的一次执行，通常由一条 `srun` 命令创建。
- **Task（任务进程）**：这次执行中启动的进程，可以有一个或多个。

例如，一个作业先预处理数据，再启动四个进程训练：

```bash
# 在同一个 sbatch 作业脚本中
srun --ntasks=1 python preprocess.py
srun --ntasks=4 python train.py
```

```text
一个 Job
├─ Step 1：预处理 → 1 个 Task
└─ Step 2：训练   → 4 个 Task
```

这里展示的是两次 `srun` 创建的执行步骤。每个 Task 可以使用多个 CPU；四个训练进程要协同训练，程序还需要分布式支持。

### 3.3 CPU、内存与 GPU

Slurm 的 CPU 单位依配置可能对应物理核或硬件线程，不是整颗处理器。`--mem` 请求的是系统内存，不是 GPU 显存。GPU 通常通过 GRES/TRES 管理；GPU 数量、型号与显存规格也不是同一个概念。

**TRES（Trackable RESources，可跟踪资源）是 Slurm 统一表示和跟踪资源的机制，不是命令。** 在查询结果中，它常显示为一串“资源名=数量”的文本，可以理解为资源清单：

```text
cpu=8,mem=32G,node=1,gres/gpu=2
```

| 表示 | 含义 |
|---|---|
| `cpu=8` | 8 个 CPU 单位 |
| `mem=32G` | 32G 系统内存，不是 GPU 显存 |
| `node=1` | 1 个节点 |
| `gres/gpu=2` | 2 张 GPU |
| `gres/gpu:a800=2` | 2 张类型标签为 `a800` 的 GPU |

`gres/` 表示 GPU 属于通用资源（Generic Resources）。字段名则说明这份清单描述的是配置、申请还是分配：

```text
# 节点配置的资源：scontrol show node 节点名
CfgTRES=cpu=64,mem=512G,gres/gpu=8

# 作业申请和分配的资源：scontrol show job 作业ID
ReqTRES=cpu=8,mem=32G,node=1,gres/gpu=2
AllocTRES=cpu=8,mem=32G,node=1,gres/gpu=2
```

以上为简化示例，实际输出可能包含其他字段，内存单位也可能显示为 `M`。节点资源由管理员配置，作业申请由用户声明，实际分配由 Slurm 决定。**这些数量不等于实时使用率**：分配了两张 GPU，不代表程序充分使用了它们。

### 3.4 Account、QoS 与 Priority

**Account（记账账户）**通常代表项目或团队，不是 SSH 登录用户。它用于统计资源用量，也可以关联额度和限制；一个 Account 可以包含多个用户，一个用户也可以使用多个获授权的 Account。例如，用户 `aaaa` 提交作业，用量记在 `research-team` 名下。

**QoS（服务质量）**是一组作业待遇和限制规则，可以设置优先级影响、CPU/GPU 额度、运行时限及作业数量限制，也可以配合抢占策略。例如 `normal` 和 `urgent` 可以使用同一分区，但享受不同待遇；实际规则由管理员配置，用户只能选择获授权的 QoS。

**Priority（优先级）**用于决定作业的调度顺序。QoS 可以影响它，但最终排序还可能考虑等待时间、公平共享和分区等因素；优先级高也不代表立即运行，仍需满足资源和其他条件。

简单区分：**Account 管用量归属，QoS 管待遇与限制，Priority 管调度排序。** 分区则主要规定可使用的节点范围及相关规则。

## 4. 各类状态及其含义

### 4.1 分区状态：看 AVAIL 列

| 状态 | 接收新提交进入该分区 | 启动已有等待作业 |
|---|---|---|
| `up` | 是 | 是 |
| `down` | 是 | 否 |
| `drain` | 否 | 是 |
| `inact`，即 INACTIVE | 否 | 否 |

表格描述分区状态本身的规则，其他权限和资源条件仍须满足。配置了备用分区时，被关闭入口的请求可能转到备用分区。改变分区状态本身通常不会终止正在运行的作业。

参考：[分区 State 配置](https://slurm.schedmd.com/slurm.conf.html)。

### 4.2 常见节点状态：看 STATE 列

| 简写 | 含义 |
|---|---|
| `idle` | 没有作业分配，正常情况下可调度 |
| `mix` | 部分 CPU 已分配，部分空闲；也可反映某些暂停作业情况 |
| `alloc` | 节点已分配给一个或多个作业；只代表不可再分配，不代表硬件资源都用满 |
| `comp` | 作业正在完成和清理 |
| `drng` | 正在排空，不接收新作业分配，已有作业尚未结束 |
| `drain` | 已排空，不接收新作业分配 |
| `down` | 节点不可用于新作业，可能是故障或管理员设置 |
| `resv` | 节点处于高级预留中 |
| `maint` | 节点属于维护预留 |
| `fail` / `failg` | 故障／故障处理相关状态 |
| `unk` | 状态尚未确认 |

还有电源、重启、未来节点等状态；这里只列入门常见项。节点状态不等于实时利用率，尤其不能仅用 `mix` 判断还剩几张 GPU。

节点中的 `*` 表示控制器当前无法获得节点响应；分区名后的 `*` 则表示默认分区，两者含义不同。

参考：[sinfo 状态说明](https://slurm.schedmd.com/sinfo.html)。

### 4.3 分区排空与节点排空

分区 `drain` 关闭新提交入口，但已有等待作业还可以启动。节点 `drain` 则禁止把该机器分配给新作业，对包含它的所有分区生效。

只排空分区、等待其作业全部结束后，正常节点可以变成 `idle`，而分区是 `drain`。如果节点同时服务其他分区，它也可能继续运行其他作业。

若目标是消化完原有队列再维护，应先关闭相关入口，等等待作业启动或全部完成，再排空节点。若因故障需要立即停止新分配，可直接排空节点，但原有等待作业可能无法启动。多个分区共用节点时，还要考虑其他入口，否则某个队列为空不代表机器已经空闲。

### 4.4 常见作业状态

| 状态 | 含义 |
|---|---|
| `PD` PENDING | 等待资源或其他条件 |
| `R` RUNNING | 正在运行 |
| `CG` COMPLETING | 正在收尾、清理 |
| `CD` COMPLETED | 正常完成 |
| `F` FAILED | 执行失败 |
| `CA` CANCELLED | 被取消 |
| `TO` TIMEOUT | 超过时间上限 |
| `NF` NODE_FAIL | 节点故障相关失败 |
| `OOM` OUT_OF_MEMORY | 内存不足 |
| `S` SUSPENDED | 已暂停 |

状态说明作业所处阶段，Reason 说明为什么等待或发生某种情况。例如 `PD (Resources)` 表示等待资源，`PD (Dependency)` 表示等待前置条件。Reason 不一定列出全部阻塞因素。

## 5. 如何使用和管理 Slurm

### 5.1 只读查询

```bash
scontrol ping                   # 控制服务是否响应
sinfo                           # 分区与节点
sinfo -s                        # 分区概要
sinfo -N                        # 按节点展示
sinfo -R                        # 不可用原因记录
squeue                          # 当前可见作业
squeue -u "$USER"               # 自己的作业
scontrol show partition NAME    # 分区规则
scontrol show node show NAME    # 节点详情
scontrol show job JOB_ID        # 作业详情
scontrol show config            # 配置
sacct -u "$USER"                # 历史与记账记录
```

历史、统计和可见范围取决于记账及权限配置。`squeue` 主要面向当前作业；已完成作业通常从中消失。

### 5.2 理解 squeue 输出

下面是简化的输出示例，实际列可由集群定制：

```text
JOBID  PARTITION  ST  USER    TIME         NODES  NODELIST(REASON)
19481  large      R   aaa     1-11:18:28       2  node[01-02]
19482  small      PD  bbb     0:00             1  (Resources)
```

第一行表示作业 `19481` 在两台节点上运行；第二行表示作业 `19482` 申请一台节点，正在等待资源，尚未分配节点。

| 字段 | 含义 |
|---|---|
| JOBID | 作业编号 |
| PARTITION | 作业所在分区 |
| ST | 作业状态 |
| USER | 提交用户 |
| TIME | 已运行时间，通常不是剩余时间 |
| NODES | 分配节点数量，等待作业时含义需结合申请看 |
| NODELIST(REASON) | 已分配节点，或等待原因 |

例如 `1-11:18:28` 是一天十一小时十八分二十八秒。两个节点不等于两个 task，也不说明 GPU 数量或节点独占情况。

### 5.3 提交和操作作业

**方法一：手动交互式操作**

在登录节点的命令行先申请资源，分配成功后再执行程序：

```bash
salloc -p small -N 1 -n 1 -c 4 -t 00:30:00  # 申请 1 台节点、1 个 task、4 个 CPU，时限 30 分钟
srun hostname                              # 在已分配的计算节点上执行命令
srun python main.py                        # 在同一分配内执行程序
exit                                       # 退出 salloc 创建的 Shell，释放资源
```

`salloc` 通常不会将当前 Shell 迁移到计算节点，因此分配成功后应通过 `srun` 启动计算。上述示例没有申请 GPU。

也可以在没有现有分配时直接申请资源并执行命令：

```bash
srun -p small -N 1 -n 1 -t 00:05:00 hostname  # 等待资源，运行命令，结束后释放分配
```

**方法二：脚本提交**

将资源需求写在脚本的 `#SBATCH` 行中，将执行命令写在脚本正文中，再提交：

```bash
sbatch job.sh                              # 提交脚本，返回 Job ID；不等待作业执行完
squeue -j JOB_ID                           # 查看排队或运行状态
tail -f slurm-JOB_ID.out                    # 若采用默认输出文件名，可在文件生成后跟踪日志
```

脚本中通常用 `srun python main.py` 执行程序，不需要再调用 `salloc`。完整脚本示例见第 6 节；`JOB_ID` 应替换为实际编号。

**两种方法都可使用的作业管理命令**

```bash
scancel JOB_ID                             # 取消作业
scontrol hold JOB_ID                       # 阻止等待中的作业启动
scontrol release JOB_ID                    # 解除 hold
```

hold 不是暂停正在运行的程序。

### 5.4 管理员操作与权限

管理员可以修改分区、排空节点、恢复调度，例如以下仅作为语法说明：

```bash
scontrol update PartitionName=example State=DRAIN
scontrol update NodeName=node-a State=DRAIN Reason="maintenance"
scontrol update NodeName=node-a State=RESUME
```

这些是修改命令，和 `show`、`ping` 不同。能运行客户端不意味着有权限修改集群；节点恢复也不等于修复了原来的故障。

## 6. 如何申请资源和编写作业脚本

### 6.1 资源由用户声明

Slurm 主要依据提交参数和默认策略，而不是分析程序需求。申请过多可能占用不用的资源，申请过少则可能降低速度或触发限制。

下面是单节点、单训练进程的教学示例：

```bash
#!/bin/bash
#SBATCH --job-name=train-demo
#SBATCH --partition=small
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=32G
#SBATCH --gres=gpu:1
#SBATCH --time=02:00:00
#SBATCH --output=slurm-%j.out

set -e
# 此处按集群实际环境激活 Python 环境
srun python train.py
```

各参数的含义如下：

| 参数 | 含义 |
|---|---|
| `--job-name=train-demo` | 作业名称，方便识别，不是作业 ID |
| `--partition=small` | 从 `small` 分区申请资源 |
| `--nodes=1` | 申请 1 台节点，不代表独占整台机器 |
| `--ntasks=1` | 总共申请 1 个 task；由后面的 `srun` 启动 |
| `--cpus-per-task=8` | 每个 task 申请 8 个 CPU 单位，具体含义见 3.3 |
| `--mem=32G` | 每台节点申请 32G 系统内存，不是 GPU 显存 |
| `--gres=gpu:1` | 每台节点申请 1 张 GPU，未指定 GPU 类型 |
| `--time=02:00:00` | 运行时间上限为 2 小时，超时通常会被终止 |
| `--output=slurm-%j.out` | 输出文件名，`%j` 替换为作业 ID；未单独指定错误输出时，标准错误也写入这里 |

`#!/bin/bash` 指定使用 Bash 执行脚本；`set -e` 让脚本在普通命令失败时退出；`srun python train.py` 在分配的资源内启动训练程序。

本例总共申请 1 台节点、1 个 task、8 个 CPU、32G 系统内存和 1 张 GPU。CPU 数和数据加载需求必须测量，不能照抄为通用最优配置。

### 6.2 参数、默认值与覆盖

`#SBATCH` 行由 `sbatch` 解析，应放在脚本开头、实际执行语句之前；它不是 Shell 执行命令，其中的 Shell 变量也不会自动展开。命令行参数优先于环境变量，环境变量优先于脚本指令。

```bash
sbatch --partition=large --nodes=2 job.sh
```

如果脚本也设置了对应项，以这里的命令行值为准。未指定资源时使用命令默认值及集群策略，不是自动选用整个分区。
作业需要归属某个分区，但提交时不一定要手动指定。指定 `--partition=large` 时使用所选分区；不指定时通常使用管理员设置的默认分区。如果没有默认分区且未指定分区，通常会提交失败。`sinfo` 中分区名后的 `*` 标识默认分区，例如此前的 `small*`。管理员的提交规则可能调整分区选择；默认分区不代表自动选择资源最多、启动最快或最适合程序的分区。

### 6.3 节点和 task 的布局

```bash
#SBATCH --nodes=2
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=2
```

表示两台节点、总共八个 task、每台四个、每个两个 CPU。**批处理脚本本身只执行一次，`--ntasks=8` 只是声明要运行八个 task。** 脚本中写 `python train.py`，只直接启动一个 Python 进程；写 `srun python train.py`，才会按该配置启动八个 Python 进程，分布在两台节点上。

### 6.4 指定节点、全部节点与独占

`--nodelist` 可以约束节点选择，但仍须经过调度。只设置分区不会使用其全部节点；若分区确实有六台节点，申请六台才是要求六台，没有通用的 `--nodes=all`。

申请全部节点与独占每台节点是不同需求。普通 `--exclusive` 通常禁止与其他作业共享分配节点，并分配全部 CPU 和 GRES，但内存仍按申请量分配；分区共享策略及特殊 GRES 配置会影响行为。申请全部内存通常使用 `--mem=0`，仍需确认集群规则。

参考：[sbatch 参数与脚本规则](https://slurm.schedmd.com/sbatch.html)、[GPU/GRES 管理](https://slurm.schedmd.com/gres.html)。

## 7. 一个作业从提交到结束的全过程

以单节点训练为例：

1. 用户在登录节点准备脚本，通过 `sbatch` 提交。
2. 控制器检查申请及站点规则，接收后生成 Job ID。
3. 作业等待优先级、资源、依赖等条件，通常显示 `PD`。
4. 调度器选择节点，记录资源分配，协调启动批处理脚本。
5. 脚本中的 `srun` 创建步骤，计算节点上的 `slurmstepd` 管理训练进程。
6. 程序结束后，Slurm 清理进程和执行结束处理，可能短暂显示 `CG`。
7. 资源释放，记录退出状态；配置了记账时，可用 `sacct` 查询历史。

```text
提交 → 检查 → 等待 → 分配 → 执行 → 清理 → 释放与记账
```

资源申请、程序退出和作业完成是不同阶段。程序不再计算，不代表清理已经结束；控制器故障也不一定立即让所有运行程序停止。

## 8. Slurm 的底层工作原理

可以把 Slurm 理解为一个闭环：**建立资源账本 → 做出调度决定 → 启动进程 → 管理运行 → 清理并归还资源**。

```text
用户：声明需求，提交作业
           ↓
控制器 slurmctld：维护账本，选择资源，批准分配
           ↓
计算节点 slurmd / slurmstepd：启动和管理进程
           ↓
完成或故障：上报状态，清理，释放资源，更新账本
```

这是职责示意。实际通信不全都经过控制器：步骤获批后，`srun` 会与计算节点的服务通信，训练数据和梯度也不需要经过控制器。

### 8.1 系统怎样建立资源账本

首先，管理员声明节点有哪些可调度资源及其使用规则，例如 CPU 数、内存、GPU、分区和权限。硬件可以通过工具探测，但哪些资源供作业使用、哪些要为系统预留，仍由配置决定。

计算节点上的 `slurmd` 启动后向控制器注册，报告节点信息，控制器检查它与预期配置是否一致。运行期间，双方通过状态报告、查询和事件通知维护节点及作业状态；失去响应时，控制器会记录异常。

`slurmctld` 维护的是一套全局记录，主要包含：

| 记录 | 回答的问题 |
|---|---|
| 节点能力与状态 | 哪些机器能用，各有什么资源？ |
| 作业申请与队列 | 谁申请了什么，还在等什么？ |
| 资源分配 | 哪些资源已经归某个作业使用？ |
| 分区、账号和 QoS 规则 | 这个用户和作业能用哪些资源，受什么限制？ |

例如，一台节点配置八张 GPU，作业 A 分配两张、作业 B 分配四张，正常的不共享 GPU 配置下还剩两张可分配。即使节点属于多个分区，这份资源记录仍是统一的。

**分配账本与实时利用率不同。** 作业获分配的 GPU 即使暂时不计算，通常仍归该作业；绕过 Slurm 手工启动的程序则未必记入分配。因此，不能只看实际负载就判断可调度量，也不能只看分配量就判断使用效率。

CPU、内存等资源如何细分、共享，由选择插件和配置决定；常见的 `select/cons_tres` 支持以可消耗资源进行管理。参考：[可消耗资源管理](https://slurm.schedmd.com/cons_tres.html)。

### 8.2 调度器怎样决定何时、在哪里运行

客户端把节点数、CPU、内存、GPU、时限等请求发送给控制器。控制器检查请求及权限，站点提交插件还可以补充或调整申请。接收请求并返回 Job ID，不代表资源已经分配成功。

调度时，可以按三类问题理解判断过程：

1. **是否允许运行**：账号、QoS、分区、依赖和额度是否满足？
2. **哪些节点符合要求**：GPU 类型、节点状态、指定节点和拓扑等是否匹配？
3. **现在轮到它且资源足够吗**：结合优先级、预留、可用资源和调度策略作出决定。

例如，一个作业要求四台节点、每台八张 GPU，即使集群总共剩三十二张 GPU，也不保证能启动：这些 GPU 可能分散在更多机器上，没有四台各具备八张可用 GPU 的节点。

作业提交、结束和配置变化会触发调度尝试，系统也有周期性调度。优先级可考虑等待时间、公平共享、分区、QoS 等；公平共享考虑历史使用情况，具体因素和权重由管理员配置。

**回填调度**用于利用等待期间的空档。例如，高优先级作业要等两小时才能凑齐资源，另一个作业只需一小时，且能在所需资源释放前结束，就可能先运行后者。原则是避免延误更高优先级作业的预计启动时间，因此合理的运行时限很重要。这里的未来安排依据申请时限和资源记录，不是对程序耗时的自动预测。

参考：[多因素优先级](https://slurm.schedmd.com/priority_multifactor.html)、[调度配置](https://slurm.schedmd.com/sched_config.html)。

### 8.3 何时申请、分配、使用和释放资源

**通常先为整个 Job 分配资源，再由各个 Step 使用；不是执行到哪条 `srun`，才为整个作业重新申请资源。** 要区分四个时刻：

| 阶段 | 发生什么 | 是否已经占用作业分配 |
|---|---|---|
| 提交申请 | `sbatch` 读取 `#SBATCH` 参数，向控制器提交整个 Job 的需求 | 尚未；返回 Job ID 不代表已分配 |
| 获得分配 | 调度条件满足，控制器将节点及相关资源分配给 Job | 是，即使程序还未开始计算 |
| 执行步骤 | 脚本中的 `srun` 在已有分配内创建 Step，启动任务进程 | 是；步骤可以只使用其中一部分 |
| 作业结束 | 脚本结束，相关进程和节点清理完成，归还资源 | 分配释放，其他 Job 才能重新获得这些资源 |

**批处理脚本本身只在分配的一台节点上执行一份。** 脚本里的 `srun` 再按 task 和节点布局，将程序启动到相应计算节点；不会把整个脚本在所有节点各执行一遍。

**例子：先预处理，再训练**

```bash
#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=1

srun --exact --ntasks=2 --cpus-per-task=1 python preprocess.py
srun --exact --ntasks=8 --cpus-per-task=1 python train.py
```

这个 Job 申请一台节点、八个 CPU。两条 `srun` 没有放到后台，因此依次执行；`--exact` 让 Step 按明确请求使用资源，避免默认步骤行为把节点上分给 Job 的更多 CPU 一并纳入步骤。

```text
提交：申请整个 Job 所需的 8 个 CPU
    ↓ 排队，此时尚未获得分配
获分配：8 个 CPU 保留给这个 Job，开始执行一份脚本
    ↓
Step 1：2 个 task，每个 1 个 CPU
        其余 6 个 CPU 暂未使用，仍保留给这个 Job
    ↓ Step 1 结束，不释放整个 Job 的分配
Step 2：8 个 task，每个 1 个 CPU，复用已有资源
    ↓
脚本结束、清理完成：归还 8 个 CPU 的作业分配
```

所以，资源“分配给 Job”和“被某个 Step 实际使用”不是一回事。在通常不允许超额共享的配置下，Step 1 闲置的六个 CPU 不会自动交给其他 Job。如果申请中还有 GPU，CPU 预处理期间未使用的 GPU 通常也会保留。

顺序执行的步骤通常按最大同时需求申请，而不是逐步相加；并发步骤则需要考虑同时需求，并明确各 Step 的资源划分。Step 不能靠填写更大的参数，自动获得超出 Job 分配的资源。如果不同步骤需求差异很大且持续时间长，可以拆成多个 Job，分别申请资源，再通过依赖关系衔接。

**三种命令入口如何对应这个过程**

| 入口 | 申请与使用方式 |
|---|---|
| `sbatch job.sh` | 提交 Job 申请；获分配后执行脚本，脚本中的 `srun` 使用已有资源 |
| `salloc ...` | 先申请分配，成功后执行指定命令或通常启动一个 Shell；之后可多次 `srun`，退出后释放分配 |
| 没有已有分配时直接 `srun ...` | 先申请 Job 分配，再创建 Step 执行程序，结束后释放分配 |

在已有 Job 中执行 `srun`，请求的是该 Job 内的 Step 资源，不是另一份独立 Job 分配。控制器批准步骤后，`srun` 与计算节点上的 `slurmd` 协调启动，由 `slurmstepd` 设置环境、输入输出并管理 task 进程。每个参与节点通常有一个对应步骤的 `slurmstepd`，可管理多个 task。

控制器负责资源和授权，计算进程负责实际计算；多个训练进程是否协同，仍由 PyTorch 等程序决定。

参考：[作业分配和启动设计](https://slurm.schedmd.com/job_launch.html)、[srun 步骤资源参数](https://slurm.schedmd.com/srun.html)。

### 8.4 运行期间怎样管理资源与跟踪状态

获得分配只是账本层面的约定。要让程序实际遵守约定，还需要操作系统层面的约束：

| 机制 | 作用 |
|---|---|
| CPU 绑定 | 指定进程或线程在哪些 CPU 上执行 |
| cgroup | 按配置限制 CPU 集合、内存和 GPU 等设备访问 |
| 进程跟踪 | 将任务及相关子进程纳入管理，便于退出时清理 |
| 资源采集 | 按配置收集 CPU 时间、内存和部分 GPU 指标 |

`slurmstepd` 与相关插件负责本地任务管理，`slurmd` 和控制器协调节点、作业状态。控制器不需要逐条接收训练计算结果；训练进程之间的通信、数据读取由程序和相应网络完成。

例如，某 task 分配八个 CPU，程序可以创建线程或数据加载子进程，共同使用这组资源。申请八个 CPU 不会自动产生八个线程；是否严格限制内存和 GPU 访问，也要看隔离配置。

如果用户绕过 Slurm 直接 SSH 运行程序，它可能不在作业约束中。正确配置的 `pam_slurm_adopt` 可以结合登录策略，把相应 SSH 会话进程纳入已有作业；单独启动 `slurmd` 不能保证所有程序都受控。

参考：[cgroup 配置](https://slurm.schedmd.com/cgroup.conf.html)、[SSH 会话纳入作业](https://slurm.schedmd.com/pam_slurm_adopt.html)。

### 8.5 作业结束或系统故障时怎样闭环

**正常结束时**，任务退出，步骤上报结果，作业进行剩余清理和结束处理。一个 Step 结束不一定释放整个 Job 的分配：批处理脚本可能还有下一步，交互式分配也可能继续使用。Job 真正结束并完成节点清理后，控制器才能归还相关资源并安排后续作业。

**取消或超时时**，系统协调终止受管理的进程，必要时先发送终止信号，再强制结束，执行配置的结束脚本并等待清理确认。作业可能在此期间显示 `CG`。若进程卡在存储 I/O 或节点失去响应，清理可能延迟，不能只因为主程序已经退出就认为所有资源已释放。

配置了记账服务时，结束状态和用量会形成历史记录供 `sacct` 查询。控制器用于恢复调度的状态保存，与记账数据库保存的历史记录是不同用途。

**节点不响应时**，控制器首先知道的是通信异常，而不是直接知道硬件坏在哪里。节点可能关机、网络断开、`slurmd` 停止或系统卡顿；进一步设为 DOWN、作业失败或重新排队以及节点恢复，都受配置和故障类型影响，不会保证所有故障作业自动重跑。

**主控制服务故障时**，正确配置的备用控制器可以依据保存的状态接管。正在运行的计算进程不一定立即停止，但新分配、步骤请求和部分管理操作可能受影响。主备控制器管理同一套集群，不会各自重复分配一份资源。

因此，整个闭环的核心是：**分配时记账，执行时约束和跟踪，结束时确认清理并归还资源，异常时维护一致状态。**

参考：[故障排查](https://slurm.schedmd.com/troubleshoot.html)、[控制服务说明](https://slurm.schedmd.com/slurmctld.html)。

## 9. Slurm 与 PyTorch 分布式训练

### 9.1 srun、torchrun、DDP 分别负责什么

**Slurm 分配资源，启动器创建进程，DDP 让进程协同训练。** 这三件事相互配合，但不会自动代替彼此。

| 工具 | 主要作用 |
|---|---|
| `sbatch` / `salloc` | 向 Slurm 申请节点、CPU、内存和 GPU |
| `srun` | 在分配的节点上启动 Slurm task；没有已有分配时也能申请资源 |
| `torchrun` | PyTorch 提供的启动命令，创建本机训练子进程，并为它们设置分布式环境信息 |
| DDP（DistributedDataParallel） | 在训练代码中包装模型，同步各训练进程的梯度 |

`torchrun` 会提供 `RANK`（全局进程编号）、`LOCAL_RANK`（本机进程编号）、`WORLD_SIZE`（训练进程总数）等环境信息，供程序初始化分布式通信。它不会向 Slurm 申请 GPU，也不会把普通单进程代码自动改成分布式代码。

常见 DDP 训练是一张 GPU 对应一个训练进程，每个进程保存完整模型、处理不同的数据，并在反向传播中同步梯度。代码仍需选择 GPU、初始化通信组、使用 DDP，并自行切分输入，例如使用 `DistributedSampler`；按轮打乱数据时还应调用采样器的 `set_epoch`。

### 9.2 简单例子：单节点四张 GPU

下面的教学脚本先申请资源，再启动一个 `torchrun`，由它创建四个训练进程。假设 `train.py` 已支持 torchrun 和 DDP，且已激活对应 Python 环境：

```bash
#!/bin/bash
#SBATCH --job-name=ddp-demo
#SBATCH --partition=large
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --gres=gpu:4
#SBATCH --time=01:00:00

srun --ntasks=1 torchrun --standalone --nnodes=1 --nproc-per-node=4 train.py
```

| 配置 | 含义 |
|---|---|
| `--ntasks=1` | Slurm 启动一个 task，执行 torchrun 启动器 |
| `--gres=gpu:4` | 给这个 Job 在本节点分配四张 GPU |
| `--cpus-per-task=16` | 给启动器及其训练子进程共同使用的 CPU 资源，不是每个子进程各十六个 |
| `--standalone --nnodes=1` | 使用单节点的分布式初始化方式 |
| `--nproc-per-node=4` | torchrun 在本机启动四个执行 train.py 的训练进程 |

```text
一个 Slurm Job：1 台节点，4 张 GPU
└─ srun 启动 1 个 Slurm task：torchrun
   ├─ train.py 进程 0 → GPU 0
   ├─ train.py 进程 1 → GPU 1
   ├─ train.py 进程 2 → GPU 2
   └─ train.py 进程 3 → GPU 3
          ↕ DDP 同步梯度，协同训练
```

图中的 GPU 编号指分配后进程可见的本地编号，不一定等于机器的物理编号。**这里是一个 Slurm task，但有四个训练子进程**，不要把 task 数直接等同于 GPU 数。

训练代码中常见的关键写法如下，仅展示初始化结构，并非完整训练程序：

```python
import os
import torch
import torch.distributed as dist
from torch.nn.parallel import DistributedDataParallel as DDP

local_rank = int(os.environ["LOCAL_RANK"])
torch.cuda.set_device(local_rank)
dist.init_process_group(backend="nccl")

# model 是已创建的模型
model = model.to(local_rank)
model = DDP(model, device_ids=[local_rank])
# 后续还需数据切分、训练循环和退出清理
```

训练进程根据 torchrun 提供的编号选择 GPU、建立通信；DDP 在计算梯度时负责同步。保存公共检查点和写公共日志时，通常只让全局进程 0 执行。

### 9.3 多节点启动与通信

| 布局 | 两台节点、每台四张 GPU 时的典型情况 |
|---|---|
| srun 直接启动训练进程 | 八个 Slurm task，每台四个，代码适配 Slurm 的进程编号等信息 |
| srun 启动 torchrun | 两个 Slurm task，每台一个启动器，各自再创建四个训练进程 |

两种布局都可以，不能混用进程数量配置。例如已经让四个 Slurm task 分别执行 torchrun，再让每个 torchrun 创建四个进程，就可能意外启动十六个训练进程。

多节点 torchrun 的通信可以分成两个阶段：**先通过共同的集合地址建立训练组，再由训练进程交换梯度等数据。** 不能直接把单节点例子的 `--standalone` 照搬过去。

**第一阶段：各节点集合，建立训练组**

假设 Slurm 已分配两台节点 `node-a` 和 `node-b`，每台四张 GPU。在每台节点上各启动一个 torchrun，使用相同的参数：

```bash
# 在 node-a、node-b 上分别执行一次；在 Slurm 中通常由 srun 每台启动一次
torchrun \
  --nnodes=2 \
  --nproc-per-node=4 \
  --rdzv-backend=c10d \
  --rdzv-endpoint=node-a:29400 \
  --rdzv-id=training-001 \
  train.py
```

| 参数 | 含义 |
|---|---|
| `--nnodes=2` | 本次训练需要两台节点加入 |
| `--nproc-per-node=4` | 每台节点启动四个训练进程，总共八个 |
| `--rdzv-backend=c10d` | 使用 c10d 集合机制 |
| `--rdzv-endpoint=node-a:29400` | 两边共同访问的集合服务地址，示例由 node-a 承载 |
| `--rdzv-id=training-001` | 本次训练的集合标识，两边必须一致，不同作业应区分 |

`rdzv` 是 rendezvous，即“集合”。各节点通过集合服务发现彼此，协调训练组成员和进程编号。torchrun 为本机训练进程提供 `RANK`、`LOCAL_RANK`、`WORLD_SIZE` 等信息。这里使用 c10d 集合方式，不需要手工为每台节点指定静态 node rank。

这是通信参数示例，不会自动申请资源或在远程机器启动程序。实际 Slurm 脚本需先申请两台节点、每台一个 task 和四张 GPU，再由 `srun` 启动上述命令；集合地址应选用实际分配节点中可达的主机名和可用端口。

**第二阶段：训练进程通信，同步梯度**

训练代码调用 `dist.init_process_group(backend="nccl")`，根据启动器提供的信息建立通信组。之后 DDP 在反向传播时，通过 NCCL 执行梯度同步：

```text
集合阶段：node-a 的 torchrun ─┐
                            ├→ node-a:29400，协调加入同一训练组
          node-b 的 torchrun ─┘

训练阶段：
node-a：4 个训练进程 ←── NCCL 跨节点通信 ──→ node-b：4 个训练进程
         各使用本机 GPU                         各使用本机 GPU
```

**梯度不会先汇总给 torchrun，也不经过 Slurm 控制节点转发。** torchrun 负责启动和监控本地训练进程，实际集合通信由训练进程调用通信后端完成。NCCL 会根据硬件和网络条件使用相应通信路径，并非固定由一个进程中转全部数据。

因此，集合地址可达只是第一步，计算节点之间的训练通信也必须畅通；仅开放集合端口 `29400` 不一定足够。各节点还需一致的软件环境、可访问的代码和数据，必要时正确选择用于通信的网卡。

GPU 多也不意味着 CPU 只需一个。每个训练进程可能有数据加载子进程，解码、增强及存储读取都可能成为瓶颈；应根据实际吞吐调整资源和 worker 数量。

参考：[torchrun 启动器](https://docs.pytorch.org/docs/stable/elastic/run.html)、[DDP 教程](https://docs.pytorch.org/tutorials/intermediate/ddp_tutorial.html)、[DDP 接口](https://docs.pytorch.org/docs/stable/generated/torch.nn.parallel.DistributedDataParallel.html)、[PyTorch 性能调优](https://docs.pytorch.org/tutorials/recipes/recipes/tuning_guide.html)。

## 10. 如何观察问题和寻找优化空间

### 10.1 先区分现象和原因

| 现象 | 优先检查 |
|---|---|
| 作业一直 PD | 等待原因、资源需求、QoS、依赖、预留 |
| 有 idle 节点却不启动 | 节点型号和资源是否匹配、权限、时限、优先级、预留 |
| 节点 down* | Reason、地址及网络、slurmd 状态、日志、认证 |
| 作业长期 CG | 残留进程、存储 I/O、结束脚本、节点是否响应 |
| GPU 利用率低 | 数据等待、CPU、存储、通信、计算规模 |
| 实际 GPU 有进程，Slurm 却显示未分配 | 是否有绕过 Slurm 的进程，统计范围是否正确 |

### 10.2 比较申请、分配与实际使用

作业详情的 `ReqTRES`、`AllocTRES` 是起点；历史中的 CPU 时间、内存峰值等可用于判断效率，但必须注意 Job 与 Step 的统计范围。`MaxRSS` 不应直接当成整个多节点作业的内存总峰值。GPU 利用率和显存统计是否可用，取决于 GPU 记账插件及硬件支持。

```bash
sacct -j JOB_ID --format=JobID,State,Elapsed,AllocCPUS,ReqMem,MaxRSS,TotalCPU
```

该命令只读取已有记录，字段可能因集群配置而为空。

### 10.3 按证据选择优化方向

- 申请资源长期远高于实际使用：调整资源请求和作业模板。
- CPU 数据处理瓶颈：测量并调整 CPU 数、DataLoader worker 和预处理方式。
- 存储瓶颈：检查小文件数量、共享存储吞吐和数据缓存策略。
- 通信瓶颈：检查网络、GPU 拓扑、跨节点通信及并行策略。
- 排队长但零散空闲多：检查请求规模、合理时限与回填配置。
- 资源账本与实际进程不一致：检查 SSH 访问及隔离策略。

不要仅因存在 drain/down 节点就判断配置需要优化，它们可能是有意维护。先积累只读证据，再判断是否需要调整。

### 10.4 当前集群的已知观察

此前 `sinfo` 显示 `small` 为默认分区，时限三天；`large` 同样三天；`large-p0` 显示无限时限。`large` 与 `large-p0` 共享节点，两个旧名称分区处于 drain。06 节点为 drain，57 节点为 down*。

这些能说明状态和共享关系，但不能确定排空原因、QoS 权限、GPU 隔离或实际利用率。需要 `scontrol show partition`、`show node`、`show job` 等进一步核实。

## 11. 命令速查与常见误区

### 11.1 命令速查

| 需求 | 命令 |
|---|---|
| 分区概要 | `sinfo -s` |
| 节点配置与 GPU 声明 | `sinfo -N -o "%N %P %t %c %m %G"` |
| 不可用原因 | `sinfo -R` |
| 当前作业 | `squeue` |
| 只看自己 | `squeue -u "$USER"` |
| 某个作业详情 | `scontrol show job JOB_ID` |
| 历史记录 | `sacct -j JOB_ID` |
| 分区规则 | `scontrol show partition PARTITION_NAME` |
| 节点详情 | `scontrol show node NODE_NAME` |
| 本机服务进程 | `pgrep -a -x slurmd` |
| 本机 CPU 拓扑 | `lscpu` |

`%G` 展示配置的 GRES，不等于实时剩余 GPU；本机进程命令和 `lscpu` 查看的是当前登录机器，不会自动查询其他节点。

### 11.2 常见误区

**节点、分区与状态**

| 误区 | 正确理解 |
|---|---|
| 所有命令必须在控制节点执行 | 配置好的客户端机器可以发送请求 |
| 登录节点不能兼任计算节点 | 可以兼任，但 SSH 登录权限不等于获得作业资源 |
| 分区自己分配资源 | 控制器依据分区规则统一调度 |
| 同一节点属于两个分区就有两份资源 | 底层节点资源只有一份，统一管理 |
| alloc 表示 CPU、内存和 GPU 全部分配完且很忙 | 它是节点分配状态，不代表所有类型资源都已分完，更不代表实际利用率 |
| 分区 drain 导致所有节点 drain | 两者是独立控制项 |
| 分区 drain 后，已有等待作业也不能启动 | 分区停止接收新提交，但已有等待作业仍可启动，前提是节点等条件满足 |
| slurmd 能自动管住所有手工程序 | 需要登录和隔离配置配合 |
| 两份 slurmd 自动变成两台节点 | 需要不同身份和专门配置，物理资源不会翻倍 |

**资源申请与执行**

| 误区 | 正确理解 |
|---|---|
| 提交后返回 Job ID 就已获得资源 | 只表示请求已接收，可能仍在排队 |
| 每条 srun 都重新申请整个 Job 的资源 | 在已有分配内，srun 创建 Step，使用 Job 已获得的资源 |
| Step 结束就把资源归还给其他 Job | 通常仍保留给当前 Job，直到整个分配结束并完成清理 |
| salloc 成功后 Shell 就在计算节点上 | 默认方式下通常仍在登录节点，通过 srun 才把程序启动到计算节点 |
| ntasks=8 会把批处理脚本执行八遍 | 脚本只执行一份；srun 才按配置启动八个 task |
| 一个 task 只能用一个 CPU | task 可以有多个线程或子进程，共同使用申请的 CPU |
| cpus-per-task=8 会自动创建八个线程 | 只分配 CPU 资源，程序要自行实现并行才能利用它们 |
| 只选分区就使用全部节点 | 还要声明节点数等资源要求 |
| 每次提交都必须手动填写分区 | 未指定时通常使用默认分区，前提是集群已配置 |
| exclusive 自动申请所有内存 | 内存申请需单独处理 |
| bash job.sh 等同 sbatch job.sh | 前者直接在当前机器执行，后者提交给 Slurm |
| TRES 是命令，或表示实时用量 | 它是资源跟踪机制；CfgTRES、ReqTRES、AllocTRES 分别表示配置、申请和分配 |
| 分区 TRES 没显示 GPU 就没有配置 GPU | 还需检查节点 GRES 和 TRES 跟踪配置 |

**PyTorch 分布式训练**

| 误区 | 正确理解 |
|---|---|
| 分配多个 GPU、启动多个进程就自动协同训练 | 程序仍需初始化通信、使用 DDP 等机制，并正确切分数据 |
| Slurm task 数总等于训练进程数 | 使用 torchrun 时，一个 task 可以启动多个训练子进程 |
| torchrun 会向 Slurm 申请 GPU | 它只负责训练进程启动和相关协调，资源需先由 Slurm 分配 |
| srun 和 torchrun 都按 GPU 数启动就行 | 应明确启动层级，避免多个启动器重复派生过多进程 |
| 多机梯度经 torchrun 或 Slurm 控制器转发 | 训练进程通过 NCCL 等后端执行梯度通信 |
| 集合端口可达就保证多机训练能通信 | 还需满足训练进程之间的网络通信条件 |

后续修改可按章节进行：先完善术语和状态，再补充现有集群的真实配置及实例，最后讨论有证据支持的优化方案。
