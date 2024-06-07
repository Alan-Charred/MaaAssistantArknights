from typing import Union, Dict, List, Any, Type
from enum import Enum, IntEnum, unique, auto

JSON = Union[Dict[str, Any], List[Any], int, str, float, bool, Type[None]]


class InstanceOptionType(IntEnum):
    touch_type = 2
    deployment_with_pause = 3


@unique
class Message(Enum):
    """
    回调消息

    请参考 docs/回调消息.md
    """
    # Global Info
    # ––––––––––––––––––––––––––––––––
    # 内部错误
    InternalError = 0
    # 初始化失败
    InitFailed = auto()
    # 连接相关错误
    ConnectionInfo = auto()
    # 全部任务完成
    AllTasksCompleted = auto()
    # 外部异步调用信息
    AsyncCallInfo = auto()
    # 实例已销毁
    Destroyed = auto()

    # TaskChain Info
    # ––––––––––––––––––––––––––––––––
    # 任务链执行/识别错误
    TaskChainError = 10000
    # 任务链开始
    TaskChainStart = auto()
    # 任务链完成
    TaskChainCompleted = auto()
    # 任务链额外信息
    TaskChainExtraInfo = auto()
    # 任务链停止（手动停止）
    TaskChainStopped = auto()

    # SubTask Info
    # ––––––––––––––––––––––––––––––––
     # 原子任务执行/识别错误
    SubTaskError = 20000
    # 原子任务开始
    SubTaskStart = auto()
    # 原子任务完成
    SubTaskCompleted = auto()
    # 原子任务额外信息
    SubTaskExtraInfo = auto()
    # 原子任务停止（手动停止）
    SubTaskStopped = auto()


@unique
class Version(Enum):
    """
    目标版本
    """
    Nightly = auto()

    Beta = auto()

    Stable = auto()
