import os
import re
from typing import Dict, List, Optional, Set

# ========= 可調整設定 =========
START_LINE = 26022

# 手動指定「一定算已完成」的 class 清單
# 只要 class 名稱在這裡，就直接視為已完成（優先權高於 FORCE_MISSING_CLASSES）
FORCE_IMPLEMENTED_CLASSES = [
    "Device_Reset_Manager"
]

# 手動指定「一定算未完成」的 class 清單
# 只要 class 名稱在這裡，就直接視為未完成
FORCE_MISSING_CLASSES = [
    "ClientCharacter",
    "ClientCharacterManager",
    "cltUsingSkillSystem",
    "cltCharKindInfo",
    "cltNPCManager",
    "CUIManager",
    "CUITutorial",
    "cltMyCharData",
    "cltWorkingPassiveSkillSystem",
    "Map",
    "clClientTransportKindInfo",
    "clTransportAnilInfo",
    "clTransportKindInfo",
    "CCA",
    "cltChattingMgr",
    "CUIBase",
    "CUIMessageBoxTypes",
    "CUIMessageBoxList",
    "CUIMessageBoxBase"
    "CUIMessageBoxOK",
    "CUIMessageBoxRadioList",
    "CUIMessageBoxSortList",
    "CUIMessageBoxMultLineOK",
    "CUIMessageBoxMultLineOKLarge",
    "CMessageBoxManager",
    "cltShopInfo",
    "CQuizEventParser",
    "CShortKey",
    "XJoyStick",
    "cltSystemMessage",
    "CControlCA",
    "cltSkillKindInfo",
    "cltRegenMonsterKindInfo"
]

# 例外清單：放 class 名稱字串，這些 class 會被跳過
EXCLUDE_CLASSES = [
    "D3DXShader",
    "std",
    "D3DXTex",
    "D3DXCore",
    "_D3DXMATRIXA16",
    "ID3DXBuffer",
    "ID3DXConstantTable",
    "ID3DXFont",
    "ID3DXLine",
    "ID3DXRenderToEnvMap",
    "ID3DXRenderToSurface",
    "ID3DXSprite",
    "D3DXVECTOR4",
    "D3DXPLANE",
    "D3DX",
    "D3DXQUATERNION",
    "D3DXVECTOR2",
    "D3DXVECTOR3",
    "D3DXCOLOR",
    "D3DXMATRIX",
    "ATL"
]

# sort 方式：
# True  -> 依總行數由大到小
# False -> 依 class 名稱排序
SORT_BY_LINES_DESC = True

C_FILE_PATH = "./mofclient.c"
INC_DIR = "./inc"
# ============================


def extract_class_name(signature_line: str) -> Optional[str]:
    """
    從函式宣告行中抓 class 名稱。
    只接受包含 ClassName::FunctionName 的情況。
    """
    m = re.search(r'([A-Za-z_]\w*)::', signature_line)
    if not m:
        return None
    return m.group(1)


def count_code_lines(block_lines: List[str]) -> int:
    """
    計算一個函式區塊的程式碼行數。
    規則：
    - 忽略空行
    - 其餘都算進去（包含註解行、{、}）
    """
    count = 0
    for line in block_lines:
        if line.strip() == "":
            continue
        count += 1
    return count


def analyze_c_file(file_path: str) -> List[dict]:
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        all_lines = f.readlines()

    lines = all_lines[START_LINE - 1:]

    # class_stats[class_name] = {
    #     "total_lines": int,
    #     "functions": int,
    #     "details": [(func_header, line_count), ...]
    # }
    class_stats: Dict[str, dict] = {}

    separator_indexes = []
    for i, line in enumerate(lines):
        if line.startswith("//-----"):
            separator_indexes.append(i)

    if not separator_indexes:
        return []

    for idx, start_idx in enumerate(separator_indexes):
        end_idx = separator_indexes[idx + 1] if idx + 1 < len(separator_indexes) else len(lines)
        block = lines[start_idx:end_idx]

        if not block:
            continue

        signature_line = None
        for line in block[1:10]:
            stripped = line.strip()
            if not stripped:
                continue
            if "(" in stripped and ")" in stripped and "::" in stripped:
                signature_line = stripped
                break

        if signature_line is None:
            continue

        # __stdcall -> 跳過
        # __thiscall -> 算
        # __cdecl -> 算（視為 class 靜態函式）
        if "__stdcall" in signature_line:
            continue

        if "__thiscall" not in signature_line and "__cdecl" not in signature_line:
            continue

        class_name = extract_class_name(signature_line)
        if class_name is None:
            continue

        if class_name in EXCLUDE_CLASSES:
            continue

        line_count = count_code_lines(block)

        if class_name not in class_stats:
            class_stats[class_name] = {
                "total_lines": 0,
                "functions": 0,
                "details": []
            }

        class_stats[class_name]["total_lines"] += line_count
        class_stats[class_name]["functions"] += 1
        class_stats[class_name]["details"].append((signature_line, line_count))

    result_list = []
    for class_name, info in class_stats.items():
        result_list.append({
            "class_name": class_name,
            "function_count": info["functions"],
            "total_lines": info["total_lines"],
            "details": info["details"],
        })

    if SORT_BY_LINES_DESC:
        result_list.sort(key=lambda x: (-x["total_lines"], x["class_name"]))
    else:
        result_list.sort(key=lambda x: x["class_name"])

    return result_list


def find_all_header_files(root_dir: str) -> List[str]:
    """
    遞迴找出 root_dir 底下所有 .h 檔案
    """
    header_files = []
    for base, _, files in os.walk(root_dir):
        for name in files:
            if name.lower().endswith(".h"):
                header_files.append(os.path.join(base, name))
    header_files.sort()
    return header_files


def analyze_header_file(file_path: str) -> List[str]:
    """
    從單一 .h 檔中抓出 class/struct 名稱。
    只抓有本體的 class/struct，跳過 forward declaration：
        class Foo;
        struct Bar;
    """
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    class_names = []
    n = len(lines)
    i = 0

    class_decl_re = re.compile(r'^\s*(class|struct)\s+([A-Za-z_]\w*)\b')

    while i < n:
        line = lines[i]
        m = class_decl_re.match(line)
        if not m:
            i += 1
            continue

        class_name = m.group(2)

        if class_name in EXCLUDE_CLASSES:
            i += 1
            continue

        j = i
        header_text = lines[j]
        found_semicolon_before_brace = (';' in lines[j] and '{' not in lines[j])

        while j + 1 < n and '{' not in header_text and not found_semicolon_before_brace:
            j += 1
            header_text += lines[j]
            if ';' in lines[j] and '{' not in header_text:
                found_semicolon_before_brace = True
                break

        # forward declaration 跳過
        if found_semicolon_before_brace and '{' not in header_text:
            i = j + 1
            continue

        if '{' not in header_text:
            i = j + 1
            continue

        class_names.append(class_name)
        i = j + 1

    return class_names


def analyze_inc_headers(inc_dir: str) -> List[str]:
    """
    掃描 ./inc 底下所有 .h 檔，彙整 class/struct 名稱
    """
    header_files = find_all_header_files(inc_dir)
    class_name_set: Set[str] = set()

    for header_file in header_files:
        class_names = analyze_header_file(header_file)
        for class_name in class_names:
            class_name_set.add(class_name)

    return sorted(class_name_set)


def compare_tool_vs_headers(tool_result_list: List[dict], header_class_names: List[str]) -> dict:
    """
    正確口徑：
    - 總 class / 總行數 都以 .c tool 結果為準
    - header 只拿來判斷哪些 class 算完成
    - 完成 = class_name 同時存在於 tool 結果與 header 名單，或被手動指定為已完成
    - 未完成 = tool 有，但 header 沒有；或被手動指定為未完成
    - 優先權：FORCE_IMPLEMENTED > FORCE_MISSING > header 判斷
    """
    header_set = set(header_class_names)
    force_implemented_set = set(FORCE_IMPLEMENTED_CLASSES)
    force_missing_set = set(FORCE_MISSING_CLASSES)

    implemented_classes = []
    missing_classes = []

    for item in tool_result_list:
        class_name = item["class_name"]

        # 手動指定已完成的優先權最高
        if class_name in force_implemented_set:
            implemented_classes.append(item)
            continue

        # 手動指定未完成次之
        if class_name in force_missing_set:
            missing_classes.append(item)
            continue

        if class_name in header_set:
            implemented_classes.append(item)
        else:
            missing_classes.append(item)

    total_classes = len(tool_result_list)
    implemented_class_count = len(implemented_classes)
    missing_class_count = len(missing_classes)

    total_lines = sum(item["total_lines"] for item in tool_result_list)
    implemented_lines = sum(item["total_lines"] for item in implemented_classes)
    missing_lines = sum(item["total_lines"] for item in missing_classes)

    class_completion = (implemented_class_count / total_classes * 100.0) if total_classes > 0 else 0.0
    line_completion = (implemented_lines / total_lines * 100.0) if total_lines > 0 else 0.0

    if SORT_BY_LINES_DESC:
        missing_classes.sort(key=lambda x: (-x["total_lines"], x["class_name"]))
        implemented_classes.sort(key=lambda x: (-x["total_lines"], x["class_name"]))
    else:
        missing_classes.sort(key=lambda x: x["class_name"])
        implemented_classes.sort(key=lambda x: x["class_name"])

    return {
        "implemented_classes": implemented_classes,
        "missing_classes": missing_classes,
        "total_classes": total_classes,
        "implemented_class_count": implemented_class_count,
        "missing_class_count": missing_class_count,
        "total_lines": total_lines,
        "implemented_lines": implemented_lines,
        "missing_lines": missing_lines,
        "class_completion": class_completion,
        "line_completion": line_completion,
    }

def main():
    tool_result_list = analyze_c_file(C_FILE_PATH)
    header_class_names = analyze_inc_headers(INC_DIR)
    comparison = compare_tool_vs_headers(tool_result_list, header_class_names)

    print("=" * 100)
    print(f"從第 {START_LINE} 行開始解析: {C_FILE_PATH}")
    print(f"header 掃描目錄: {INC_DIR}")
    print(f"排除清單: {EXCLUDE_CLASSES}")
    print("=" * 100)

    total_tool_functions = sum(item["function_count"] for item in tool_result_list)

    print("[.c tool 統計總覽]")
    print(f".c 總 class 數: {comparison['total_classes']}")
    print(f".c 總函式數: {total_tool_functions}")
    print(f".c 總行數: {comparison['total_lines']}")
    print("-" * 100)

    print("[完成度統計]")
    print(f"已完成 class 數: {comparison['implemented_class_count']}")
    print(f"未完成 class 數: {comparison['missing_class_count']}")
    print(
        f"class 完成度: "
        f"{comparison['implemented_class_count']}/{comparison['total_classes']} "
        f"({comparison['class_completion']:.2f}%)"
    )
    print("-" * 100)
    print(f"已完成行數: {comparison['implemented_lines']}")
    print(f"未完成行數: {comparison['missing_lines']}")
    print(
        f"行數完成度: "
        f"{comparison['implemented_lines']}/{comparison['total_lines']} "
        f"({comparison['line_completion']:.2f}%)"
    )
    print("=" * 100)

    print("[未完成 class 清單（.c 有、但 ./inc 沒有）]")
    if not comparison["missing_classes"]:
        print("全部 class 都已在 ./inc 找到。")
    else:
        for item in comparison["missing_classes"]:
            print(
                f"{item['class_name']:40s} "
                f"funcs={item['function_count']:4d} "
                f"lines={item['total_lines']:6d}"
            )

    print("=" * 100)

    print("[已完成 class 清單（.c 有、且 ./inc 也有）]")
    for item in comparison["implemented_classes"]:
        print(
            f"{item['class_name']:40s} "
            f"funcs={item['function_count']:4d} "
            f"lines={item['total_lines']:6d}"
        )

    print("=" * 100)

    # 保留原本整理後的 list
    simple_list = [
        (item["class_name"], item["function_count"], item["total_lines"])
        for item in tool_result_list
    ]
    # print("[tool sorted result list]")
    # for item in simple_list:
    #     print(item)


if __name__ == "__main__":
    main()