import os
import codecs

# 탐색할 확장자 목록
TARGET_EXTENSIONS = {'.cpp', '.h', '.hpp', '.c'}

def convert_to_utf8_bom(filename):
    # 1. 내용을 바이너리로 읽음
    with open(filename, 'rb') as f:
        raw_data = f.read()

    # 2. 이미 UTF-8 (BOM 포함/미포함)인지 확인
    try:
        # UTF-8로 디코딩 시도
        raw_data.decode('utf-8')
        # BOM이 있는지 확인
        if raw_data.startswith(codecs.BOM_UTF8):
            print(f"[Skip] Already UTF-8 BOM: {filename}")
            return
        else:
            # BOM이 없는 UTF-8이면 BOM을 추가하기 위해 아래 로직 진행 (선택사항)
            # 여기서는 'CP949'로 해석되는 경우만 변환하도록 함
            pass 
    except UnicodeDecodeError:
        # UTF-8이 아니므로 변환 대상임
        pass

    # 3. CP949(ANSI)로 디코딩 후 UTF-8-SIG(BOM)로 저장
    try:
        # ANSI(CP949)로 해석
        content = raw_data.decode('cp949')
        
        # utf-8-sig : BOM을 포함한 UTF-8
        with open(filename, 'w', encoding='utf-8-sig') as f:
            f.write(content)
        print(f"[Converted] {filename}")
        
    except Exception as e:
        print(f"[Error] Failed to convert {filename}: {e}")

def main():
    # 현재 디렉토리부터 하위 탐색
    for root, dirs, files in os.walk("."):
        for file in files:
            ext = os.path.splitext(file)[1]
            if ext.lower() in TARGET_EXTENSIONS:
                full_path = os.path.join(root, file)
                convert_to_utf8_bom(full_path)

if __name__ == "__main__":
    print("Converting .cpp, .h files to UTF-8 with BOM...")
    main()
    print("Done.")