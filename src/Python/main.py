import json
import pathlib
import time

from asst.asst import Asst
from asst.utils import Message, Version, InstanceOptionType
from asst.updater import Updater
from asst.emulator import Bluestacks

from xml.dom import minidom
from collections import defaultdict

# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
# Control Panel
# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
# Please set the path where DLL files and the resources folder are located.
path = pathlib.Path(__file__).resolve().parent.parent.parent

# enable this if you are using English client
EN_VERSION = True
# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
SANITY_CHECK = 0 # 0, 1, 2, 3
# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
# Translation
# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
if EN_VERSION:
    zone_dict = {
        "初霜湖泽": "Brisk Marshalands",
        "密静林地": "Placid Forests",
        "昧明冻土": "Dawning Frostbounds",
        "积冰岩骸": "Gelid Rockshelves",
        "无瑕花园": "Immaculate Garden",
        "远见之构": "Clayrovant Carcass",
        "深埋迷境": "Secluded Passage",
        "": "None"
    }
else:
    zone_dict = {
        "初霜湖泽": "初霜湖泽",
        "密静林地": "密静林地",
        "昧明冻土": "昧明冻土",
        "积冰岩骸": "积冰岩骸",
        "无瑕花园": "无瑕花园",
        "远见之构": "远见之构",
        "深埋迷境": "深埋迷境",
        "": "None"
    }
# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
# Control Panel
# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
if EN_VERSION:
    collapsal_paradigm_dict = {
        "去量化": "De-quantification",
        "去量深化": "Intensified De-quantification",
        "实质性坍缩": "Substantial Collapse",
        "蔓延性坍缩": "Propagating Collapse",
        "非线性移动": "Non-linear Movement",
        "非线性行动": "Non-linear Action",
        "情绪实体": "Emotional Entity",
        "恐怖实体": "Terrifying Entity",
        "泛社会悖论": "Pan-Society Paradox",
        "泛文明悖论": "Pan-Civilization Paradox",
        "气压异常": "Barometric Anomaly",
        "气压失序": "Barometric Disorder",
        "触发性损伤": "Injury Trigger",
        "触发性危殆": "Crisis Trigger",
        "趋同性消耗": "Convergence Consumption",
        "趋同性缺失": "Convergence Deficiency",
        "目空一些": "Partial Cecity",
        "睁眼瞎": "Complete Cecity",
        "图像损坏": "Image Corruption",
        "一抹黑": "Image Blackout",
        "": "None"
    }
else:
    collapsal_paradigm_dict = {
        "去量化": "去量化",
        "去量深化": "去量深化",
        "实质性坍缩": "实质性坍缩",
        "蔓延性坍缩": "蔓延性坍缩",
        "非线性移动": "非线性移动",
        "非线性行动": "非线性行动",
        "情绪实体": "情绪实体",
        "恐怖实体": "恐怖实体",
        "泛社会悖论": "泛社会悖论",
        "泛文明悖论": "泛文明悖论",
        "气压异常": "气压异常",
        "气压失序": "气压失序",
        "触发性损伤": "触发性损伤",
        "触发性危殆": "触发性危殆",
        "趋同性消耗": "趋同性消耗",
        "趋同性缺失": "趋同性缺失",
        "目空一些": "目空一些",
        "睁眼瞎": "睁眼瞎",
        "图像损坏": "图像损坏",
        "一抹黑": "一抹黑",
        "": "None"
    }
# ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

msg_dict = {
    # Global Info
    Message.InternalError:      '[InternalError]'.ljust(20, ' '),      # 内部错误
    Message.InitFailed:         '[InitFailed]'.ljust(20, ' '),         # 初始化失败
    Message.ConnectionInfo:     '[ConnectionInfo]'.ljust(20, ' '),     # 连接相关错误
    Message.AllTasksCompleted:  '[AllTasksCompleted]'.ljust(20, ' '),  # 全部任务完成
    Message.AsyncCallInfo:      '[AsyncCallInfo]'.ljust(20, ' '),      # 外部异步调用信息
    Message.Destroyed:          '[Destroyed]'.ljust(20, ' '),          # 实例已销毁
    # TaskChain Info
    Message.TaskChainError:     '[TaskChainError]'.ljust(20, ' '),     # 任务链执行/识别错误
    Message.TaskChainStart:     '[TaskChainStart]'.ljust(20, ' '),     # 任务链开始
    Message.TaskChainCompleted: '[TaskChainCompleted]'.ljust(20, ' '), # 任务链完成
    Message.TaskChainExtraInfo: '[TaskChainExtraInfo]'.ljust(20, ' '), # 任务链额外信息
    Message.TaskChainStopped:   '[TaskChainStopped]'.ljust(20, ' '),   # 任务链停止（手动停止）
    # SubTask Info
    Message.SubTaskError:       '[SubTaskError]'.ljust(20, ' '),       # 原子任务执行/识别错误
    Message.SubTaskStart:       '[SubTaskStart]'.ljust(20, ' '),       # 原子任务开始
    Message.SubTaskCompleted:   '[SubTaskCompleted]'.ljust(20, ' '),   # 原子任务完成
    Message.SubTaskExtraInfo:   '[SubTaskExtraInfo]'.ljust(20, ' '),   # 原子任务额外信息
    Message.SubTaskStopped:     '[SubTaskStopped]'.ljust(20, ' '),     # 原子任务停止（手动停止）
    # Extra
    None:                       ''
}

def PrintLog(m, msg):
    print(time.strftime('%Y-%m-%d %H:%M:%S', time.localtime()), end = ' ')
    print(msg_dict[m], end = ' ')
    print(msg)

def PrintErr(m, msg):
    print('\33[31m', end = '')
    print(time.strftime('%Y-%m-%d %H:%M:%S', time.localtime()), end = ' ')
    print(msg_dict[m], end = ' ')
    print(msg, end = '')
    print('\033[0m')

def AddLog(m, what, arg=(), extra_msg = ''):
    PrintLog(m, callback_dict[what].format(*arg) + extra_msg)

def AddErr(m, what, arg=(), extra_msg = ''):
    PrintErr(m, callback_dict[what].format(*arg) + extra_msg)

@Asst.CallBackType
def ProcMsg(msg, details, arg):
    m = Message(msg)
    d = json.loads(details.decode('utf-8'))
    match m:
        case Message.InternalError:
            pass
        case Message.InitFailed:
            AddErr(m, 'InitializationError')
            AddErr(m, 'Error')
        case Message.ConnectionInfo:
            ProcConnectInfo(m, d)
        case Message.TaskChainStart:
            ProcTaskChainMsg(m, d)
        case Message.AllTasksCompleted:
            ProcTaskChainMsg(m, d)
        case Message.TaskChainError:
            ProcTaskChainMsg(m, d)
        case Message.TaskChainCompleted:
            ProcTaskChainMsg(m, d)
        case Message.TaskChainStopped:
            ProcTaskChainMsg(m, d)
        case Message.TaskChainExtraInfo:
            ProcTaskChainMsg(m, d)
        case Message.SubTaskError:
            ProcSubTaskMsg(m, d)
        case Message.SubTaskStart:
            ProcSubTaskMsg(m, d)
        case Message.SubTaskCompleted:
            ProcSubTaskMsg(m, d)
        case Message.SubTaskExtraInfo:
            ProcSubTaskMsg(m, d)
        case Message.SubTaskStopped:
            pass
        case _:
            PrintErr(m, str(d))

def ProcConnectInfo(m, d):
    try:
        match d['what']:
            case 'Connected':
                pass
            case 'ConnectFailed':
                AddErr(m, 'ConnectFailed')
                AddErr(m, 'CheckSettings')
            case 'UnsupportedResolution':
                AddErr(m, 'UnsupportedResolution')
            case 'ResolutionError':
                AddErr(m, 'ResolutionError')
            case 'Reconnecting':
                AddLog(m, 'TryToReconnect', (int(d['details']['times']) + 1,))
            case 'Reconnected':
                AddLog(m, 'ReconnectSuccess')
            case 'Disconnect':
                AddErr(m, 'ReconnectFailed')
            case 'ScreencapFailed':
                AddErr(m, 'ScreencapFailed')
            case 'TouchModeNotAvailable':
                AddErr(m, 'TouchModeNotAvailable')
            case 'FastestWayToScreencap':
                AddLog(m, 'FastestWayToScreencap', (d['details']['method'], d['details']['cost']))
            case 'ScreencapCost':
                AddLog(m, 'ScreencapCost', (d['details']['min'], d['details']['avg'], d['details']['max'], time.strftime('%H:%M:%S', time.localtime())))
            case _:
                pass
    except:
        pass

def ProcTaskChainMsg(m, d):
    try:
        match d['taskchain']:
            case 'CloseDown':
                pass
            case 'Recruit':
                if m == Message.TaskChainError:
                    AddErr(m, 'IdentifyTheMistakes')
    except:
        pass
    match m:
        case Message.TaskChainStopped:
            pass
        case Message.TaskChainError:
            AddErr(m, 'TaskError', (), d['taskchain'])
            if d['taskchain'] == 'Copilot' or d['taskchain'] == 'VideoRecognition':
                AddErr(m, 'CombatError')
        case Message.TaskChainStart:
            AddLog(m, 'StartTask', (), d['taskchain'])
        case Message.TaskChainCompleted:
            AddLog(m, 'CompleteTask', (), d['taskchain'])
            if d['taskchain'] == 'Copilot' or d['taskchain'] == 'VideoRecognition':
                AddLog(m, 'CompleteCombat')
        case Message.TaskChainExtraInfo:
            pass
        case Message.AllTasksCompleted:
            AddLog(m, 'AllTasksComplete')
        case Message.InternalError:
            pass
        case Message.InitFailed:
            pass
        case Message.ConnectionInfo:
            pass
        case Message.SubTaskError:
            pass
        case Message.SubTaskStart:
            pass
        case Message.SubTaskCompleted:
            pass
        case Message.SubTaskExtraInfo:
            pass
        case Message.SubTaskStopped:
            pass
        case _:
            PrintErr(m, str(d))

def ProcSubTaskMsg(m, d):
    match m:
        case Message.SubTaskError:
            ProcSubTaskError(m, d)
        case Message.SubTaskStart:
            ProcSubTaskStart(m, d)
        case Message.SubTaskCompleted:
            ProcSubTaskCompleted(m, d)
        case Message.SubTaskExtraInfo:
            ProcSubTaskExtraInfo(m, d)
        case Message.InternalError:
            pass
        case Message.InitFailed:
            pass
        case Message.ConnectionInfo:
            pass
        case Message.AllTasksCompleted:
            pass
        case Message.TaskChainError:
            pass
        case Message.TaskChainStart:
            pass
        case Message.TaskChainCompleted:
            pass
        case Message.TaskChainExtraInfo:
            pass
        case Message.TaskChainStopped:
            pass
        case Message.SubTaskStopped:
            pass
        case _:
            PrintErr(m, str(d))

def ProcSubTaskError(m, d):
    try:
        match d['details']['subtask']:
            case 'StartGameTask':
                AddErr(m, 'FailedToOpenClient')
            case 'AutoRecruitTask':
                AddErr(m, 'ErrorOccurred')
                AddErr(m, 'HasReturned')
            case 'RecognizeDrops':
                AddErr(m, 'DropRecognitionError')
            case 'ReportToPenguinStats':
                AddErr(m, 'GiveUpUploadingPenguins')
            case 'CheckStageValid':
                AddErr(m, 'TheEx')
            case 'BattleFormationTask':
                try:
                    AddErr(m, 'MissingOperators', (), str(d['details']['opers']))
                except:
                    AddErr(m, 'MissingOperators')
    except:
        pass

def ProcSubTaskStart(m, d):
    try:
        match d['subtask']:
            case 'ProcessTask':
                match d['details']['task']:
                    case 'StartButton2':
                        AddLog(m, 'MissionStart', (), ' ' + str(d['details']['exec_times'] + callback_dict['UnitTime']))
                    case 'AnnihilationConfirm':
                        AddLog(m, 'MissionStart', (), ' ' + str(d['details']['exec_times'] + callback_dict['UnitTime']))
                    case 'StoneConfirm':
                        AddLog(m, 'StoneUsed', (), ' ' + str(d['details']['exec_times'] + callback_dict['UnitTime']))
                    case 'AbandonAction':
                        AddErr(m, 'ActingCommandError')
                    case 'RecruitRefreshConfirm':
                        AddLog(m, 'LabelsRefreshed')
                    case 'RecruitConfirm':
                        AddLog(m, 'RecruitConfirm')
                    case 'InfrastDormDoubleConfirmButton':
                        AddErr(m, 'InfrastDormDoubleConfirmed')
                    case 'StartExplore':
                        AddLog(m, 'BegunToExplore', (), ' ' + str(d['details']['exec_times']) + callback_dict['UnitTime'])
                    case 'ExitThenAbandon':
                        AddLog(m, 'ExplorationAbandoned')
                    case 'MissionCompletedFlag':
                        AddLog(m, 'FightCompleted')
                    case 'MissionFailedFlag':
                        AddLog(m, 'FightFailed')
                    case 'StageTraderEnter':
                        AddLog(m, 'Trader')
                    case 'StageSafeHouseEnter':
                        AddLog(m, 'SafeHouse')
                    case 'StageCombatDpsEnter':
                        AddLog(m, 'CombatDps')
                    case 'StageEmergencyDps':
                        AddLog(m, 'EmergencyDps')
                    case 'StageDreadfulFoe':
                        AddLog(m, 'DreadfulFoe')
                    case 'StageDreadfulFoe-5Enter':
                        AddLog(m, 'DreadfulFoe')
                    case 'StageTraderInvestSystemFull':
                        AddLog(m, 'UpperLimit')
                    case 'OfflineConfirm':
                        AddLog(m, 'GameDrop')
                    case 'GamePass':
                        AddLog(m, 'RoguelikeGamePass')
                    case 'BattleStartAll':
                        AddLog(m, 'MissionStart')
                    case 'StageTraderSpecialShoppingAfterRefresh':
                        AddLog(m, 'RoguelikeSpecialItemBought')
            case 'CombatRecordRecognitionTask':
                PrintLog(m, d['what'])
    except:
        pass

def ProcSubTaskCompleted(m, d):
    pass

def ProcSubTaskExtraInfo(m, d):
    try:
        match d['taskchain']:
            case 'Recruit':
                pass
            case 'VideoRecognition':
                pass
            case 'Depot':
                pass
            case 'OperBox':
                pass
    except:
        pass
    try:
        match d['what']:
            case 'StageDrops':
                pass
            case 'EnterFacility':
                AddLog(m, 'ThisFacility', (), d['details']['facility'] + " " + str(d['details']['index']))
            case 'ProductIncorrect':
                AddErr(m, 'ProductIncorrect')
            case 'ProductUnknown':
                AddErr(m, 'ProductUnknown')
            case 'ProductChanged':
                AddLog(m, 'ProductChanged')
            case 'RecruitTagsDetected':
                pass
            case 'RecruitSpecialTag':
                pass
            case 'RecruitRobotTag':
                pass
            case 'RecruitResult':
                pass
            case 'RecruitTagsSelected':
                pass
            case 'RecruitTagsRefreshed':
                AddLog(m, 'Refreshed', (), str(d['details']['count']) + callback_dict['UnitTime'])
            case 'RecruitNoPermit':
                if d['details']['continue']:
                    AddLog(m, 'ContinueRefresh')
                else:
                    AddLog(m, 'NoRecruitmentPermit')
            case 'NotEnoughStaff':
                AddErr(m, 'NotEnoughStaff')
            case 'CreditFullOnlyBuyDiscount':
                AddLog(m, 'CreditFullOnlyBuyDiscount', (), d['details']['credit'])
            case 'RoguelikeInvestment':
                AddLog(m, 'RoguelikeInvestment', (d['details']['count'], d['details']['total'], d['details']['deposit']))
            case 'RoguelikeSettlement':
                AddLog(m, 'RoguelikeSettlement', ('✓' if d['details']['game_pass'] else '✗', 
                                                  d['details']['floor'],
                                                  d['details']['step'],
                                                  d['details']['combat'],
                                                  d['details']['emergency'],
                                                  d['details']['boss'],
                                                  d['details']['recruit'],
                                                  d['details']['collection'],
                                                  d['details']['difficulty'],
                                                  d['details']['score'],
                                                  d['details']['exp'],
                                                  d['details']['skill']))
            case 'StageInfo':
                AddLog(m, 'StartCombat', (), d['details']['name'])
            case 'StageInfoError':
                AddErr(m, 'StageInfoError')
            case 'RoguelikeCombatEnd':
                pass
            case 'RoguelikeEvent':
                AddLog(m, 'RoguelikeEvent', (), ' ' + d['details']['name'])
            case 'FoldartalGainOcrNextLevel':
                AddLog(m, 'FoldartalGainOcrNextLevel', (), ' ' + d['details']['foldartal'])
            case 'PenguinId':
                pass
            case 'BattleFormation':
                AddLog(m, 'BattleFormation', (), '\n' + str(d['details']['formation']))
            case 'BattleFormationSelected':
                AddLog(m, 'BattleFormationSelected', (), d['details']['selected'])
            case 'CopilotAction':
                AddLog(m, 'CurrentSteps', (d['details']['action'], d['details']['target']))
            case 'CopilotListLoadTaskFileSuccess':
                pass
            case 'SSSStage':
                AddLog(m, 'CurrentStage', (), d['details']['stage'])
            case 'SSSSettlement':
                PrintLog(m + d['details']['why'])
            case 'SSSGamePass':
                AddLog(m, 'SSSGamePass')
            case 'UnsupportedLevel':
                AddErr(m, 'UnsupportedLevel', (), d['details']['level'])
            case 'CustomInfrastRoomGroupsMatch':
                AddLog(m, 'RoomGroupsMatch', (), d['details']['group'])
            case 'CustomInfrastRoomGroupsMatchFailed':
                if d['details']['groups']:
                    AddErr(m, 'RoomGroupsMatchFailed', (), ' ,'.join(d['details']['groups']))
            case 'CustomInfrastRoomOperators':
                pass
            case 'InfrastTrainingIdle':
                AddLog(m, 'TrainingIdle')
            case 'InfrastTrainingCompleted':
                pass
            case 'InfrastTrainingTimeLeft':
                pass
            case 'ReclamationReport':
                AddLog(m, 'AlgorithmFinish', (), '\n' 
                       + callback_dict['AlgorithmBadge'] + ': ' + str(d['details']['total_badges']) + '(+' + str(d['details']['badges']) + ')\n' +
                       + callback_dict['AlgorithmConstructionPoint'] + ': ' + str(d['details']['total_construction_points']) + '(+' + str(d['details']['construction_points']) + ')')
            case 'ReclamationProcedureStart':
                AddLog(m, 'MissionStart', (), ' ' + str(d['details']['times']) + callback_dict['UnitTime'])
            case 'ReclamationSmeltGold':
                AddLog(m, 'AlgorithmDoneSmeltGold', (), ' ' + str(d['details']['times']) + callback_dict['UnitTime'])
            case 'SanityBeforeStage':
                pass
            case 'UseMedicine':
                pass
            case 'StageQueueUnableToAgent':
                AddLog(m, 'StageQueue', (), ' ' +d['details']['stage_code'] + callback_dict['UnableToAgent'])
            case 'StageQueueMissionCompleted':
                AddLog(m, 'StageQueue', (), ' ' + d['details']['stage_code'] + ' - ' + d['details']['stars'] + ' ★')
            case "AccountSwitch":
                AddLog(m, 'StageQueue', (), ' ' + d['details']['current_account'] + ' -->> ' + d['details']['account_name'])
            case 'RoguelikeCollapsalParadigms':
                if d['details']['deepen_or_weaken']:
                    PrintLog(m, collapsal_paradigm_dict[d['details']['cur']] + ' ' 
                             + str(d['details']['deepen_or_weaken']) + ' '
                             + collapsal_paradigm_dict[d['details']['prev']])
                elif d['details']['prev'] == 'CollapsalParadigmInfo' and d['details']['cur'].startswith('Current Zone: '):
                    PrintLog(m, 'Current Zone: ' + zone_dict[d['details']['cur'][14 :]] + ' ' 
                             + str(d['details']['deepen_or_weaken']) + ' '
                             + d['details']['prev'])
                else:
                    PrintLog(m, d['details']['cur'] + ' ' 
                             + str(d['details']['deepen_or_weaken']) + ' '
                             + d['details']['prev'])
    except:
        pass


if __name__ == "__main__":

    # 请设置为存放 dll 文件及资源的路径
    # path = pathlib.Path(__file__).resolve().parent.parent.parent

    # ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    # 创建UI字典
    # ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    with open(path.joinpath('src/MaaWpfGui/Res/Localizations/' + ('en-us' if EN_VERSION else 'zh-cn') + '.xaml')) as fp:
        dom = minidom.parse(fp)
    attributeList = filter(lambda x: x is not None, map(lambda x: x.attributes, dom.childNodes[0].childNodes))
    callback_dict = defaultdict(lambda key: 'Unknown Callback Message: ' + key,
                        map(lambda x: (x.attributes['x:Key'].nodeValue, x.childNodes[0].nodeValue),
                            filter(lambda x: x.attributes is not None, dom.childNodes[0].childNodes)
                        )
                    )
    # ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––

    # 设置更新器的路径和目标版本并更新
    # Updater(path, Version.Stable).update()

    # 加载 dll 及资源
    #
    # incremental_path 参数表示增量资源所在路径。两种用法举例：
    # 1. 传入外服的增量资源路径：
    #     Asst.load(path=path, incremental_path=path / 'resource' / 'global' / 'YoStarEN')
    # 2. 加载活动关导航（需额外下载）：
    #     # 下载活动关导航
    #     import urllib.request
    #     ota_tasks_url = 'https://ota.maa.plus/MaaAssistantArknights/api/resource/tasks.json'
    #     ota_tasks_path = path / 'cache' / 'resource' / 'tasks.json'
    #     ota_tasks_path.parent.mkdir(parents=True, exist_ok=True)
    #     with open(ota_tasks_path, 'w', encoding='utf-8') as f:
    #         with urllib.request.urlopen(ota_tasks_url) as u:
    #             f.write(u.read().decode('utf-8'))
    #     # 加载
    #     Asst.load(path=path, incremental_path=path / 'cache')
    Asst.load(path=path)

    # 若需要获取详细执行信息，请传入 callback 参数
    # 例如 asst = Asst(callback=my_callback)
    asst = Asst(callback=ProcMsg)

    # 设置额外配置
    # 触控方案配置
    asst.set_instance_option(InstanceOptionType.touch_type, 'maatouch')
    # 暂停下干员
    # asst.set_instance_option(InstanceOptionType.deployment_with_pause, '1')

    # 启动模拟器。例如启动蓝叠模拟器的多开Pie64_1，并等待30s
    # Bluestacks.launch_emulator_win(r'C:\Program Files\BlueStacks_nxt\HD-Player.exe', 30, "Pie64_1")

    # 获取Hyper-v蓝叠的adb port
    # port = Bluestacks.get_hyperv_port(r"C:\ProgramData\BlueStacks_nxt\bluestacks.conf", "Pie64_1")

    # 请自行配置 adb 环境变量，或修改为 adb 可执行程序的路径
    AddLog(Message.ConnectionInfo, 'ConnectingToEmulator')
    for address in ['emulator-5554', '127.0.0.1:5555']:
        PrintLog(Message.ConnectionInfo, 'Attempt to connect to: ' + address)
        if asst.connect('adb', address):
            AddLog(Message.ConnectionInfo, 'Running')
            break
    else:
        exit()

    # 任务及参数请参考 docs/集成文档.md

    # 开始唤醒
    # asst.append_task('StartUp', {
    #     'client_type': 'Official',
    #     'start_game_enabled': True
    # })

    # # 刷理智
    # asst.append_task('Fight', {
    #     'medicine': 5,
    #     'stone': 16,
    #     'drops': {
    #         "30073": 46
    #     }
    # })

    if SANITY_CHECK == 0:
        asst.append_task('Roguelike', {
            'theme': 'Sami',
            'mode': 5,
            'squad': '远程战术分队',
            'roles': '取长补短',
            'core_char': '焰影苇草',
            'expected_collapsal_paradigms': ['去量化', '目空一些', '睁眼瞎', '图像损坏', '一抹黑'],
        })
    elif SANITY_CHECK == 1:
        asst.append_task('Roguelike', {
            'theme': 'Sami',
            'mode': 5,
            'squad': '远程战术分队',
            'roles': '稳扎稳打',
            'core_char': '提丰',
            'expected_collapsal_paradigms': ['去量化', '目空一些', '睁眼瞎', '图像损坏', '一抹黑'],
            'double_check_collapsal_paradigms': False
        })
    elif SANITY_CHECK == 2:
        asst.append_task('Roguelike', {
            'theme': 'Sami',
            'mode': 0,
            'squad': '远程战术分队',
            'roles': '取长补短',
            'core_char': '焰影苇草',
            'investment_enabled': True,
            'use_foldartal': True,
            'expected_collapsal_paradigms': ['目空一些', '睁眼瞎', '图像损坏', '一抹黑'],
            'check_collapsal_paradigms': True,
            'double_check_collapsal_paradigms': True
        })
    elif SANITY_CHECK == 3:
        asst.append_task('Roguelike', {
            'theme': 'Sami',
            'mode': 5,
            'squad': '突击战术分队',
            'roles': '取长补短',
            'core_char': '锏',
            'expected_collapsal_paradigms': ['去量化', '目空一些', '睁眼瞎', '图像损坏', '一抹黑'],
        })

    asst.start()

    while asst.running():
        time.sleep(0)
