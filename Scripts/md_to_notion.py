#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Markdown to Notion Style Converter
==================================
마크다운 파일을 노션에 붙여넣기 좋은 형태로 변환합니다.

사용법:
    python md_to_notion.py [input_dir] [output_dir]
    python md_to_notion.py  # 기본: ../Docs → ../Docs/Notion

출력:
    - 각 .md 파일에 대해 _notion.txt 파일 생성
"""

import os
import re
import sys
from pathlib import Path
from datetime import datetime


class NotionConverter:
    """마크다운을 노션 스타일로 변환하는 클래스"""

    def __init__(self):
        # 이모지 매핑 (섹션 타입별)
        self.section_emojis = {
            '개요': '📋',
            'overview': '📋',
            '목차': '📑',
            '플레이어': '🎮',
            'player': '🎮',
            '전투': '⚔️',
            'combat': '⚔️',
            '적': '👹',
            'enemy': '👹',
            '시스템': '⚙️',
            'system': '⚙️',
            'ui': '🖥️',
            '레벨': '🗺️',
            'level': '🗺️',
            '카메라': '📷',
            'camera': '📷',
            '게임': '🎯',
            'game': '🎯',
            '도전': '🔥',
            'challenge': '🔥',
            '해결': '💡',
            'solution': '💡',
            '권장': '✅',
            'recommend': '✅',
            '부록': '📎',
            'appendix': '📎',
            '요약': '📝',
            'summary': '📝',
            '참고': '📚',
            'reference': '📚',
        }

        # 우선순위/상태 이모지
        self.priority_emojis = {
            '필수': '🔴',
            '권장': '🟡',
            '선택': '🟢',
            '높음': '🔴',
            '중간': '🟡',
            '낮음': '🟢',
        }

    def convert_file(self, input_path: str, output_path: str) -> bool:
        """단일 파일 변환"""
        try:
            with open(input_path, 'r', encoding='utf-8') as f:
                content = f.read()

            converted = self.convert(content)

            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(converted)

            return True
        except Exception as e:
            print(f"Error converting {input_path}: {e}")
            return False

    def convert(self, content: str) -> str:
        """마크다운 콘텐츠를 노션 스타일로 변환"""

        # 1. 전처리
        content = self._preprocess(content)

        # 2. 헤딩 변환 (이모지 추가)
        content = self._convert_headings(content)

        # 3. 테이블 정리
        content = self._convert_tables(content)

        # 4. 코드 블록 정리
        content = self._convert_code_blocks(content)

        # 5. 리스트 정리
        content = self._convert_lists(content)

        # 6. 인용문/콜아웃 변환
        content = self._convert_callouts(content)

        # 7. 구분선 정리
        content = self._convert_dividers(content)

        # 8. 강조 표시 정리
        content = self._convert_emphasis(content)

        # 9. 후처리
        content = self._postprocess(content)

        return content

    def _preprocess(self, content: str) -> str:
        """전처리: 불필요한 요소 정리"""

        # HTML 주석 제거
        content = re.sub(r'<!--.*?-->', '', content, flags=re.DOTALL)

        # 연속 빈 줄을 2줄로 제한
        content = re.sub(r'\n{4,}', '\n\n\n', content)

        return content

    def _convert_headings(self, content: str) -> str:
        """헤딩 변환: 이모지 추가 및 정리"""

        lines = content.split('\n')
        result = []

        for line in lines:
            # H1: # Title
            match = re.match(r'^# (.+)$', line)
            if match:
                title = match.group(1).strip()
                result.append(f"# {title}")
                result.append("")  # 빈 줄 추가
                continue

            # H2: ## Section
            match = re.match(r'^## (.+)$', line)
            if match:
                title = match.group(1).strip()
                emoji = self._get_section_emoji(title)
                if emoji and not self._has_emoji(title):
                    result.append(f"\n## {emoji} {title}")
                else:
                    result.append(f"\n## {title}")
                continue

            # H3: ### Subsection
            match = re.match(r'^### (.+)$', line)
            if match:
                title = match.group(1).strip()
                result.append(f"\n### {title}")
                continue

            # H4 이하는 볼드로 변환 (노션에서 더 깔끔)
            match = re.match(r'^####+ (.+)$', line)
            if match:
                title = match.group(1).strip()
                result.append(f"\n**{title}**\n")
                continue

            result.append(line)

        return '\n'.join(result)

    def _get_section_emoji(self, title: str) -> str:
        """섹션 제목에 맞는 이모지 반환"""
        title_lower = title.lower()

        for keyword, emoji in self.section_emojis.items():
            if keyword in title_lower:
                return emoji

        # 숫자로 시작하는 섹션
        if re.match(r'^\d+\.?\s', title):
            return '📌'

        return ''

    def _has_emoji(self, text: str) -> bool:
        """텍스트에 이모지가 있는지 확인"""
        emoji_pattern = re.compile(
            "["
            "\U0001F600-\U0001F64F"
            "\U0001F300-\U0001F5FF"
            "\U0001F680-\U0001F6FF"
            "\U0001F1E0-\U0001F1FF"
            "\U00002702-\U000027B0"
            "\U000024C2-\U0001F251"
            "]+",
            flags=re.UNICODE
        )
        return bool(emoji_pattern.search(text))

    def _convert_tables(self, content: str) -> str:
        """테이블 변환: 노션 호환 형식으로"""

        lines = content.split('\n')
        result = []
        in_table = False
        table_lines = []

        for line in lines:
            # 테이블 행 감지
            if re.match(r'^\|.+\|$', line.strip()):
                in_table = True
                # 구분선(|---|---|) 건너뛰기
                if not re.match(r'^\|[\s\-:]+\|$', line.strip()):
                    table_lines.append(line)
            else:
                if in_table:
                    # 테이블 종료 - 정리하여 추가
                    if table_lines:
                        result.append("")  # 테이블 전 빈 줄
                        result.extend(table_lines)
                        result.append("")  # 테이블 후 빈 줄
                    table_lines = []
                    in_table = False
                result.append(line)

        # 마지막 테이블 처리
        if table_lines:
            result.append("")
            result.extend(table_lines)
            result.append("")

        return '\n'.join(result)

    def _convert_code_blocks(self, content: str) -> str:
        """코드 블록 변환: 언어 태그 정리"""

        # 언어가 없는 코드 블록에 기본값 추가
        content = re.sub(r'```\n', '```text\n', content)

        # 의사 코드 블록 정리
        content = re.sub(r'```(\s*)(struct|class|enum|void|int|float)', r'```cpp\n\2', content)

        return content

    def _convert_lists(self, content: str) -> str:
        """리스트 변환: 들여쓰기 및 이모지 추가"""

        lines = content.split('\n')
        result = []

        for line in lines:
            # 체크박스 변환
            line = re.sub(r'^(\s*)-\s*\[\s*\]', r'\1☐', line)
            line = re.sub(r'^(\s*)-\s*\[x\]', r'\1☑', line, flags=re.IGNORECASE)

            # 우선순위 표시에 이모지 추가
            for keyword, emoji in self.priority_emojis.items():
                if keyword in line and emoji not in line:
                    line = line.replace(keyword, f"{emoji} {keyword}")
                    break

            result.append(line)

        return '\n'.join(result)

    def _convert_callouts(self, content: str) -> str:
        """인용문/콜아웃 변환"""

        lines = content.split('\n')
        result = []

        for line in lines:
            # > 로 시작하는 인용문
            match = re.match(r'^>\s*\*\*(.+?)\*\*:?\s*(.*)$', line)
            if match:
                # **제목**: 내용 형태 → 콜아웃 스타일
                title = match.group(1)
                content_text = match.group(2)

                # 제목에 따른 이모지
                if '주의' in title or 'warning' in title.lower():
                    emoji = '⚠️'
                elif '팁' in title or 'tip' in title.lower():
                    emoji = '💡'
                elif '노트' in title or 'note' in title.lower():
                    emoji = '📝'
                elif '중요' in title or 'important' in title.lower():
                    emoji = '❗'
                else:
                    emoji = '💬'

                result.append(f"> {emoji} **{title}**")
                if content_text:
                    result.append(f"> {content_text}")
                continue

            # 일반 인용문
            if line.startswith('>'):
                # 이미 이모지가 없으면 추가
                if not self._has_emoji(line):
                    line = re.sub(r'^>\s*', '> 💬 ', line, count=1)

            result.append(line)

        return '\n'.join(result)

    def _convert_dividers(self, content: str) -> str:
        """구분선 변환"""

        # --- 또는 *** 또는 ___ → 노션 구분선
        content = re.sub(r'\n[-*_]{3,}\n', '\n\n---\n\n', content)

        return content

    def _convert_emphasis(self, content: str) -> str:
        """강조 표시 정리"""

        # 인라인 코드 주변 공백 정리
        content = re.sub(r'`\s+', '`', content)
        content = re.sub(r'\s+`', '`', content)

        return content

    def _postprocess(self, content: str) -> str:
        """후처리: 최종 정리"""

        # 연속 빈 줄 정리 (최대 2줄)
        content = re.sub(r'\n{3,}', '\n\n', content)

        # 파일 시작/끝 공백 정리
        content = content.strip()

        # 노션 메타 정보 추가
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M")
        header = f"""📄 **Notion Import Ready**
> 이 문서는 노션에 붙여넣기 위해 변환되었습니다.
> 변환 시간: {timestamp}

---

"""

        return header + content


def main():
    """메인 함수"""

    # Windows 콘솔 UTF-8 설정
    if sys.platform == 'win32':
        import io
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

    # 경로 설정
    script_dir = Path(__file__).parent
    default_input = script_dir.parent / "Docs"
    default_output = script_dir.parent / "Docs" / "Notion"

    input_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else default_input
    output_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else default_output

    # 출력 디렉토리 생성
    output_dir.mkdir(parents=True, exist_ok=True)

    # 변환기 생성
    converter = NotionConverter()

    # .md 파일 찾기
    md_files = list(input_dir.glob("*.md"))

    if not md_files:
        print(f"No .md files found in {input_dir}")
        return

    print("=" * 50)
    print("Markdown to Notion Converter")
    print("=" * 50)
    print(f"Input:  {input_dir}")
    print(f"Output: {output_dir}")
    print(f"Files:  {len(md_files)}")
    print("-" * 50)

    success_count = 0

    for md_file in md_files:
        output_file = output_dir / f"{md_file.stem}_notion.txt"

        print(f"Converting: {md_file.name}", end=" ... ")

        if converter.convert_file(str(md_file), str(output_file)):
            print(f"[OK] -> {output_file.name}")
            success_count += 1
        else:
            print("[FAIL]")

    print("-" * 50)
    print(f"Complete: {success_count}/{len(md_files)} files converted")
    print(f"Output directory: {output_dir}")


if __name__ == "__main__":
    main()
