import os
import re
import sys
import shutil
import subprocess
import glob

# Windows CMD 이모지 출력을 위한 설정
sys.stdout.reconfigure(encoding='utf-8')

SCRIPT_DIR    = os.path.dirname(os.path.abspath(__file__))
BASE_DIR      = os.path.dirname(SCRIPT_DIR)

# ── 기기 저장소에 맞게 변경 ─────────────────────────────────
SKETCH_FILE   = os.path.join(BASE_DIR, "updated_itembox.ino")
VERSION_MACRO = "FIRMWARE_VER"
# ────────────────────────────────────────────────────────────

OUTPUT_BIN    = os.path.join(BASE_DIR, "update.bin")
OUTPUT_SIG    = os.path.join(BASE_DIR, "update.sig")
VERSION_TXT   = os.path.join(BASE_DIR, "version.txt")

PARTITION_MACRO   = "PARTITION_VER"
OUTPUT_PART_BIN   = os.path.join(BASE_DIR, "partitions.bin")
OUTPUT_PART_SIG   = os.path.join(BASE_DIR, "partitions.sig")
PARTITION_VER_TXT = os.path.join(BASE_DIR, "partition_version.txt")

try:
    sys.path.insert(0, SCRIPT_DIR)
    from secrets import HMAC_SECRET
except ImportError:
    print("❌ 오류: scripts/secrets.py 파일이 없습니다.")
    print("   secrets.py.example 을 secrets.py 로 복사한 뒤 비밀키를 설정하세요.")
    sys.exit(1)

def get_current_version():
    with open(SKETCH_FILE, "r", encoding="utf-8") as f:
        content = f.read()
    pattern = rf'#define\s+{VERSION_MACRO}\s+(\d+)'
    match = re.search(pattern, content)
    if match:
        return int(match.group(1))
    return None

def increment_version(current_ver):
    new_ver = current_ver + 1
    with open(SKETCH_FILE, "r", encoding="utf-8") as f:
        content = f.read()
    new_content = re.sub(
        rf'#define\s+{VERSION_MACRO}\s+\d+',
        f'#define {VERSION_MACRO} {new_ver}',
        content
    )
    with open(SKETCH_FILE, "w", encoding="utf-8") as f:
        f.write(new_content)
    return new_ver

def get_current_partition_version():
    with open(SKETCH_FILE, "r", encoding="utf-8") as f:
        content = f.read()
    match = re.search(rf'#define\s+{PARTITION_MACRO}\s+(\d+)', content)
    return int(match.group(1)) if match else None

def increment_partition_version(current_ver):
    new_ver = current_ver + 1
    with open(SKETCH_FILE, "r", encoding="utf-8") as f:
        content = f.read()
    new_content = re.sub(
        rf'#define\s+{PARTITION_MACRO}\s+\d+',
        f'#define {PARTITION_MACRO} {new_ver}',
        content
    )
    with open(SKETCH_FILE, "w", encoding="utf-8") as f:
        f.write(new_content)
    return new_ver

def find_partitions_bin():
    sketch_dir = os.path.dirname(SKETCH_FILE)
    search_patterns = [
        os.path.join(sketch_dir, "build", "**", "*partitions*.bin"),
        os.path.join(BASE_DIR, "build", "**", "*partitions*.bin"),
    ]
    candidates = []
    for pattern in search_patterns:
        candidates.extend(glob.glob(pattern, recursive=True))
    candidates = [f for f in candidates if os.path.abspath(f) != os.path.abspath(OUTPUT_PART_BIN)]
    if not candidates:
        return None
    return max(candidates, key=os.path.getmtime)

def find_newest_bin():
    sketch_dir = os.path.dirname(SKETCH_FILE)
    search_patterns = [
        os.path.join(sketch_dir, "build", "**", "*.bin"),
        os.path.join(sketch_dir, "**", "*.bin"),
        os.path.join(BASE_DIR, "build", "**", "*.bin"),
    ]
    candidates = []
    for pattern in search_patterns:
        candidates.extend(glob.glob(pattern, recursive=True))
    exclude_keywords = ["merged", "bootloader", "partitions", "boot_app"]
    candidates = [
        f for f in candidates
        if not any(kw in os.path.basename(f).lower() for kw in exclude_keywords)
    ]
    if not candidates:
        return None
    return max(candidates, key=os.path.getmtime)

def git_push(version, partition_version=None):
    print("\n☁️ GitHub 에 업로드 중...")
    try:
        with open(VERSION_TXT, "w", encoding="utf-8") as f:
            f.write(str(version))
        print(f"📝 version.txt → v{version}")
        files_to_add = [
            "update.bin",
            "update.sig",
            "version.txt",
            os.path.relpath(SKETCH_FILE, BASE_DIR).replace("\\", "/"),
        ]
        commit_msg = f"Firmware Update v{version}"
        if partition_version is not None:
            with open(PARTITION_VER_TXT, "w", encoding="utf-8") as f:
                f.write(str(partition_version))
            print(f"📝 partition_version.txt → v{partition_version}")
            files_to_add += ["partitions.bin", "partitions.sig", "partition_version.txt"]
            commit_msg += f" + Partition v{partition_version}"
        subprocess.run(["git", "-C", BASE_DIR, "add"] + files_to_add, check=True)
        subprocess.run(
            ["git", "-C", BASE_DIR, "commit", "-m", commit_msg],
            check=True
        )
        subprocess.run(["git", "-C", BASE_DIR, "push"], check=True)
        print("✅ GitHub 업로드 완료!")
    except subprocess.CalledProcessError as e:
        print(f"❌ Git 오류: {e}")

def main():
    print("🚀 SecureOTA 배포 자동화 시작...")
    if HMAC_SECRET == "CHANGE_THIS_TO_YOUR_SECRET":
        print("❌ 오류: scripts/secrets.py 의 HMAC_SECRET 을 설정하세요.")
        return
    cur_ver = get_current_version()
    if cur_ver is None:
        print(f"❌ 오류: {SKETCH_FILE} 에서 '#define {VERSION_MACRO}' 를 찾을 수 없습니다.")
        return
    print(f"\n현재 버전: v{cur_ver}")
    new_ver = increment_version(cur_ver)
    print(f"🔼 버전 변경: v{cur_ver} → v{new_ver}")
    print("\n" + "="*55)
    print("⚠️  [필수] 아두이노 IDE 재로드 후 컴파일!")
    print("   1) 아두이노 IDE 파일 변경 알림 → [Reload] 클릭")
    print("      알림 없으면 → IDE 완전 종료 후 스케치 다시 열기")
    print("   2) Ctrl+Alt+S (컴파일된 바이너리 내보내기)")
    print("="*55)
    print("   완료되면 Enter 를 누르세요...")
    input()
    print("🔎 빌드 파일 탐색 중...")
    bin_file = find_newest_bin()
    if not bin_file:
        print("❌ .bin 파일을 찾을 수 없습니다.")
        return
    print(f"   발견: {os.path.relpath(bin_file, BASE_DIR)}")
    try:
        shutil.copy2(bin_file, OUTPUT_BIN)
        print(f"📦 → update.bin 복사 완료")
    except Exception as e:
        print(f"❌ 파일 복사 실패: {e}")
        return
    sign_script = os.path.join(SCRIPT_DIR, "sign_firmware.py")
    result = subprocess.run(
        [sys.executable, sign_script, OUTPUT_BIN, HMAC_SECRET, OUTPUT_SIG],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print(f"❌ 서명 실패:\n{result.stderr}")
        return
    print("🔏 서명 완료 → update.sig")

    new_partition_ver = None
    print("\n🗂️ 파티션 스키마도 업데이트하시겠습니까? (y/N): ", end="", flush=True)
    if input().strip().lower() == "y":
        cur_partition_ver = get_current_partition_version()
        if cur_partition_ver is None:
            print(f"❌ {SKETCH_FILE} 에서 '#define {PARTITION_MACRO}' 를 찾을 수 없습니다.")
        else:
            part_bin_file = find_partitions_bin()
            if not part_bin_file:
                print("❌ partitions.bin 을 찾을 수 없습니다. Ctrl+Alt+S 후 다시 시도하세요.")
            else:
                print(f"   발견: {os.path.relpath(part_bin_file, BASE_DIR)}")
                try:
                    shutil.copy2(part_bin_file, OUTPUT_PART_BIN)
                    print("📦 → partitions.bin 복사 완료")
                except Exception as e:
                    print(f"❌ 파일 복사 실패: {e}")
                    part_bin_file = None
                if part_bin_file:
                    sign_script = os.path.join(SCRIPT_DIR, "sign_firmware.py")
                    result = subprocess.run(
                        [sys.executable, sign_script, OUTPUT_PART_BIN, HMAC_SECRET, OUTPUT_PART_SIG],
                        capture_output=True, text=True
                    )
                    if result.returncode != 0:
                        print(f"❌ 서명 실패:\n{result.stderr}")
                    else:
                        print("🔏 서명 완료 → partitions.sig")
                        new_partition_ver = increment_partition_version(cur_partition_ver)
                        print(f"🔼 파티션 버전: v{cur_partition_ver} → v{new_partition_ver}")

    git_push(new_ver, new_partition_ver)
    print(f"\n🎉 배포 완료! 펌웨어 v{new_ver}", end="")
    if new_partition_ver is not None:
        print(f" + 파티션 v{new_partition_ver}", end="")
    print(" 이(가) GitHub 에 업로드되었습니다.")
    print("   서버에서 device_state = \"github\" 를 전송하면 기기가 업데이트됩니다.")

if __name__ == "__main__":
    main()
