"""
BMP 이미지의 특정 색상을 다른 색상으로 변환하는 스크립트
폴더 내 모든 BMP 파일을 자동 처리
"""
from PIL import Image
import os

# ============================================================
# 여기서 값을 수정하세요
# ============================================================

# 변환할 원본 색상 (R, G, B)
SOURCE_COLOR = (16, 17, 3)

# 변환될 색상 (R, G, B) - 마젠타
TARGET_COLOR = (255, 0, 255)

# 색상 허용 오차 (0이면 정확히 일치하는 색상만)
TOLERANCE = 0

# 출력 폴더 (None이면 원본 파일 덮어쓰기)
OUTPUT_FOLDER = "converted"

# ============================================================


def color_match(pixel, source, tolerance):
    """색상이 허용 오차 내에서 일치하는지 확인"""
    r, g, b = pixel
    sr, sg, sb = source
    return (abs(r - sr) <= tolerance and
            abs(g - sg) <= tolerance and
            abs(b - sb) <= tolerance)


def convert_color(input_path, output_path):
    """이미지의 특정 색상을 변환"""
    img = Image.open(input_path)
    img = img.convert("RGB")
    pixels = img.load()

    width, height = img.size
    changed_count = 0

    for y in range(height):
        for x in range(width):
            if color_match(pixels[x, y], SOURCE_COLOR, TOLERANCE):
                pixels[x, y] = TARGET_COLOR
                changed_count += 1

    img.save(output_path, "BMP")
    return changed_count


def process_folder():
    """현재 폴더의 모든 BMP 파일 처리"""
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # 출력 폴더 생성
    if OUTPUT_FOLDER:
        output_dir = os.path.join(script_dir, OUTPUT_FOLDER)
        os.makedirs(output_dir, exist_ok=True)
    else:
        output_dir = script_dir

    # BMP 파일 찾기
    bmp_files = [f for f in os.listdir(script_dir)
                 if f.lower().endswith('.bmp')]

    if not bmp_files:
        print("BMP 파일을 찾을 수 없습니다.")
        return

    print(f"설정: {SOURCE_COLOR} → {TARGET_COLOR} (허용오차: {TOLERANCE})")
    print(f"발견된 BMP 파일: {len(bmp_files)}개\n")

    total_changed = 0
    for filename in bmp_files:
        input_path = os.path.join(script_dir, filename)
        output_path = os.path.join(output_dir, filename)

        changed = convert_color(input_path, output_path)
        total_changed += changed
        print(f"  {filename}: {changed}픽셀 변환됨")

    print(f"\n완료! 총 {total_changed}픽셀 변환됨")
    print(f"출력 위치: {output_dir}")


if __name__ == "__main__":
    process_folder()
