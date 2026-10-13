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

### 2.3 登录节点不等于控制节点

用户可以在配置好客户端、认证和网络的登录节点上执行 Slurm 命令，请求发送到控制服务。SSH 到控制节点不是必要步骤。能否直接登录某台机器，由管理员的访问策略决定。

此前输出表明：用户登录 `…-01-train-plt-server`，主控制服务位于 `…-04-train-plt-server`。这说明 01 可以使用 Slurm 客户端，但不能单凭这一点确定它是否还承担其他角色。

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

| 概念 | 含义 |
|---|---|
| Job：作业 | 一次提交及其资源分配、生命周期 |
| Step：作业步骤 | 作业内的一次执行，常由 `srun` 创建 |
| Task：任务进程 | 步骤中由 Slurm 启动和管理的执行进程 |

一次 `sbatch` 提交通常形成一个 Job。脚本中的两次 `srun` 可以形成两个普通 Step；批处理脚本本身也有 batch step，另有可能出现的 extern step，所以不要把步骤数简单等同于 `srun` 行数。

一个 task 可以使用多个 CPU，并创建自己的线程或子进程。子进程不会因为被 Python 创建就自动成为额外的 Slurm task。

### 3.3 CPU、内存与 GPU

Slurm 的 CPU 单位依配置可能对应物理核或硬件线程，不是整颗处理器。`--mem` 请求的是系统内存，不是 GPU 显存。GPU 通常通过 GRES/TRES 管理；GPU 数量、型号与显存规格也不是同一个概念。

TRES 是用于跟踪资源的表示方式，常见包括 CPU、内存、节点和 GPU。作业详情中的 `ReqTRES`、`AllocTRES` 可帮助区分申请与分配。

### 3.4 Account、QoS 与 Priority

Account 用于组织项目、用户关联及记账；QoS 是一组服务等级规则，可以影响限制、优先级和抢占；Priority 是调度排序使用的优先级值或相关机制。

名字叫 `P0` 不足以证明它一定比 `P1` 优先。此前 `squeue` 的 `PRIO` 列是定制输出，要结合格式、`QOS`、`Priority` 和分区配置确认。

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
| `alloc` | 节点已分配给一个或多个作业；不代表每项硬件资源都用满 |
| `comp` | 作业正在完成和清理 |
| `drng` | 正在排空，不接收新作业分配，已有作业尚未结束 |
| `drain` | 已排空，不接收新作业分配 |
| `down` | 节点不可用于新作业，可能是故障或管理员设置 |
| `resv` | 节点处于高级预留中 |
| `maint` | 节点属于维护预留 |
| `fail` / `failg` | 故障／故障处理相关状态 |
| `unk` | 状态尚未确认 |

还有电源、重启、未来节点等状态；这里只列入门常见项。节点状态不等于实时利用率，尤其不能仅用 `mix` 判断还剩几张 GPU。

`down*` 的 `*` 表示控制器当前无法获得节点响应；`small*` 中分区名后的 `*` 则表示默认分区，两者含义不同。

参考：[sinfo 状态说明](https://slurm.schedmd.com/sinfo.html)。

### 4.3 分区排空与节点排空

分区 `drain` 关闭新提交入口，但已有等待作业还可以启动。节点 `drain` 则禁止把该机器分配给新作业，对包含它的所有分区生效。

只排空分区、等待其作业全部结束后，正常节点可以变成 `idle`，而分区仍是 `drain`。如果节点同时服务其他分区，它也可能继续运行其他作业。

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
squeue -u "$USER"                # 自己的作业
scontrol show partition large   # 分区规则
scontrol show node NODE_NAME    # 节点详情
scontrol show job JOB_ID        # 作业详情
scontrol show config            # 配置
sacct -u "$USER"                 # 历史与记账记录
```

历史、统计和可见范围取决于记账及权限配置。`squeue` 主要面向当前作业；已完成作业通常从中消失。

### 5.2 理解 squeue 输出

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

`sbatch job.sh` 提交脚本；`salloc` 获得资源分配，常与 `srun` 配合交互使用；`srun` 启动作业步骤，也可以从分配外发起资源申请。获得分配不意味着当前 Shell 自动迁移到计算节点。

`scancel JOB_ID` 取消作业；`scontrol hold JOB_ID` 阻止等待作业启动；`scontrol release JOB_ID` 解除 hold。hold 不是暂停正在执行的程序。

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

这里每个 task 申请八个 CPU，每个节点申请 32G 系统内存和一张 GPU，时限两小时。`%j` 是 Job ID。CPU 数和数据加载需求必须测量，不能照抄为通用最优配置。

### 6.2 参数、默认值与覆盖

`#SBATCH` 行由 `sbatch` 解析，应放在脚本开头、实际执行语句之前；它不是 Shell 执行命令，其中的 Shell 变量也不会自动展开。命令行参数优先于环境变量，环境变量优先于脚本指令。

```bash
sbatch --partition=large --nodes=2 job.sh
```

如果脚本也设置了对应项，以这里的命令行值为准。未指定资源时使用命令默认值及集群策略，不是自动选用整个分区。

### 6.3 节点和 task 的布局

```bash
#SBATCH --nodes=2
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=2
```

表示两台节点、总共八个 task、每台四个、每个两个 CPU。随后 `srun` 才按布局启动程序。`--ntasks=8` 不会让整个批处理脚本执行八遍，直接 `python train.py` 也不会因此自动运行八份。

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

### 8.1 控制器维护一份资源账本

控制器结合配置、节点注册及状态报告，维护节点能力和资源分配。CPU、内存、GPU 如何作为可消耗资源处理，取决于选择插件和配置；常见的细粒度分配使用 `select/cons_tres`。

这个账本主要记录 Slurm 的分配，不是完全等同于操作系统的实时负载。如果有人直接 SSH 后运行训练而未纳入 Slurm 管理，控制器可能仍认为相应 GPU 未分配，从而造成争抢。

参考：[可消耗资源管理](https://slurm.schedmd.com/cons_tres.html)。

### 8.2 调度与回填

优先级可以综合等待时间、公平共享、作业规模、分区和 QoS 等因素，权重由管理员设置。公平共享通常用于考虑过去资源使用情况，并不简单等于每个人轮流运行。

回填调度会尝试利用当前空闲资源运行较低优先级作业，前提是不延误为更高优先级作业规划的启动。因此合理的时间上限有助于调度；把每个短任务都申请成很长时间可能降低回填机会。时间上限不是 Slurm 对程序运行时长的自动预测。

参考：[多因素优先级](https://slurm.schedmd.com/priority_multifactor.html)、[调度配置](https://slurm.schedmd.com/sched_config.html)。

### 8.3 资源分配与隔离是两件事

分配决定资源归哪个作业，隔离决定程序实际能访问什么。CPU 绑定影响进程在哪些 CPU 上执行；cgroup 可限制 CPU 集合、内存和设备访问。具体约束需要启用相应插件和配置，不能认为启动 `slurmd` 后所有进程自然都受控。

SSH 会话管理可使用 `pam_slurm_adopt`：在正确配置下限制无分配用户的计算节点访问，并将相关会话进程纳入作业。管理员和特殊账号可能有例外，仍需结合实际策略。

参考：[cgroup 配置](https://slurm.schedmd.com/cgroup.conf.html)、[SSH 会话纳入作业](https://slurm.schedmd.com/pam_slurm_adopt.html)。

### 8.4 认证、故障检测与主备

客户端和服务通信需要认证，传统集群常用 MUNGE，其他版本和配置也可使用不同插件。各节点的身份、凭据和网络必须一致可达。

控制器在规定时间内收不到节点服务响应时，会标记不响应；是否进一步设为 DOWN、何时恢复，取决于超时和恢复策略。原因可能是关机、网络、`slurmd` 停止、系统卡顿或认证配置异常。SSH 能登录不代表 Slurm 通信正常。

备用控制服务用于接管调度，需要正确的状态保存和主备配置；它不是另一套独立资源账本，也不增加计算资源。

参考：[官方故障排查](https://slurm.schedmd.com/troubleshoot.html)。

## 9. Slurm 与 PyTorch 分布式训练

Slurm 负责资源，启动器负责进程，PyTorch 负责训练协作。直接启动八份普通训练代码，通常只是八个独立实验，不会自动协同更新一个模型。

常见 DDP 数据并行是一张 GPU 对应一个训练进程，各进程保留完整模型、处理不同数据、同步梯度。代码通常需要初始化通信组、选择本进程的 GPU、包装 DDP 模型、切分输入数据，并限制公共日志和检查点写入。DDP 不自动切分数据，可配合 `DistributedSampler`，每轮调用 `set_epoch` 以更新打乱顺序。

| 启动布局 | 两台机器、每台四张 GPU 时的典型情况 |
|---|---|
| srun 直接启动训练进程 | 八个 Slurm task，每台四个 |
| srun 启动 torchrun | 两个 Slurm task，每台一个启动器，再各自派生四个训练进程 |

不能把两种布局的 task 数照搬混用，否则可能重复启动进程。直接使用 `srun` 时，需要把 Slurm 提供的编号和总数等信息与 PyTorch 初始化配套；使用 `torchrun` 时由它提供 `RANK`、`LOCAL_RANK`、`WORLD_SIZE` 等信息。GPU 可见设备编号还受 Slurm 绑定影响，不能永远假定本地进程编号就是物理 GPU 编号。

多机训练还需要一致的软件环境、可访问的数据和代码，以及节点间通信条件。申请 GPU 多不意味着 CPU 只需一个：数据加载、解码和增强都可能消耗 CPU，每个训练进程还可能有多个 DataLoader worker。

参考：[DDP 教程](https://docs.pytorch.org/tutorials/intermediate/ddp_tutorial.html)、[DDP 接口](https://docs.pytorch.org/docs/stable/generated/torch.nn.parallel.DistributedDataParallel.html)、[PyTorch 性能调优](https://docs.pytorch.org/tutorials/recipes/recipes/tuning_guide.html)。

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

| 误区 | 正确理解 |
|---|---|
| 所有命令必须在控制节点执行 | 配置好的客户端机器可以发送请求 |
| 分区自己分配资源 | 控制器依据分区规则统一调度 |
| 一个脚本就是一个 task | 一次提交是 Job，Job 可以包含多个 Step 和 Task |
| 分配八个 task 就自动协同训练 | 仍需要支持并行的程序 |
| alloc 表示 CPU 和 GPU 都很忙 | 它描述分配状态，不是实时利用率 |
| 分区 drain 导致所有节点 drain | 两者是独立控制项 |
| 同一节点属于两个分区就有两份资源 | 底层节点资源只有一份 |
| slurmd 能自动管住所有手工程序 | 需要登录和隔离配置配合 |
| 两份 slurmd 自动变成两台节点 | 需要不同身份和专门配置，物理资源不会翻倍 |
| 只选分区就使用全部节点 | 还要声明节点数等资源要求 |
| exclusive 自动申请所有内存 | 内存申请需单独处理 |
| bash job.sh 等同 sbatch job.sh | 前者直接在当前机器执行，后者提交给 Slurm |

后续修改可按章节进行：先完善术语和状态，再补充现有集群的真实配置及实例，最后讨论有证据支持的优化方案。
