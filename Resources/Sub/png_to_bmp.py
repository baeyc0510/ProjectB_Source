"""
PNG 이미지의 투명 배경을 특정 색상(마젠타)으로 변환하여 BMP로 저장하는 스크립트
폴더 내 모든 PNG 파일을 자동 처리
"""
from PIL import Image
import os

# ============================================================
# 여기서 값을 수정하세요
# ============================================================

# 투명 영역을 채울 색상 (R, G, B) - 마젠타
BACKGROUND_COLOR = (255, 0, 255)

# 투명도 임계값 (이 값 이하의 알파값은 투명으로 처리)
# 0 = 완전 투명만, 127 = 반투명도 포함, 255 = 모든 픽셀
ALPHA_THRESHOLD = 127

# 출력 폴더 (None이면 현재 폴더에 저장)
OUTPUT_FOLDER = "converted"

# ============================================================


def convert_png_to_bmp(input_path, output_path):
    """PNG의 투명 영역을 지정 색상으로 채우고 BMP로 저장"""
    img = Image.open(input_path)

    # RGBA로 변환 (알파 채널 확보)
    if img.mode != 'RGBA':
        img = img.convert('RGBA')

    # 배경 이미지 생성 (마젠타)
    background = Image.new('RGB', img.size, BACKGROUND_COLOR)

    # 알파 채널 기준으로 합성
    # 투명 영역은 배경색, 불투명 영역은 원본 이미지
    pixels = img.load()
    bg_pixels = background.load()

    width, height = img.size
    transparent_count = 0

    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a <= ALPHA_THRESHOLD:
                # 투명 픽셀 - 배경색 유지
                transparent_count += 1
            else:
                # 불투명 픽셀 - 원본 색상 사용
                bg_pixels[x, y] = (r, g, b)

    background.save(output_path, "BMP")
    return transparent_count


def process_folder():
    """현재 폴더의 모든 PNG 파일 처리"""
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # 출력 폴더 생성
    if OUTPUT_FOLDER:
        output_dir = os.path.join(script_dir, OUTPUT_FOLDER)
        os.makedirs(output_dir, exist_ok=True)
    else:
        output_dir = script_dir

    # PNG 파일 찾기
    png_files = [f for f in os.listdir(script_dir)
                 if f.lower().endswith('.png')]

    if not png_files:
        print("PNG 파일을 찾을 수 없습니다.")
        return

    print(f"설정: 투명 영역 → {BACKGROUND_COLOR} (알파 임계값: {ALPHA_THRESHOLD})")
    print(f"발견된 PNG 파일: {len(png_files)}개\n")

    total_transparent = 0
    for filename in png_files:
        input_path = os.path.join(script_dir, filename)
        # 확장자를 .bmp로 변경
        output_filename = os.path.splitext(filename)[0] + ".bmp"
        output_path = os.path.join(output_dir, output_filename)

        transparent = convert_png_to_bmp(input_path, output_path)
        total_transparent += transparent
        print(f"  {filename} → {output_filename}: {transparent}픽셀 투명→마젠타")

    print(f"\n완료! 총 {total_transparent}픽셀 변환됨")
    print(f"출력 위치: {output_dir}")


if __name__ == "__main__":
    process_folder()
