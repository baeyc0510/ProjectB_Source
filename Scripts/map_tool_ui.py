import pygame
from map_tool_config import PANEL_BORDER_COLOR, TEXT_COLOR, HIGHLIGHT_COLOR


class Button:
    def __init__(self, x, y, width, height, text, font, callback=None):
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.font = font
        self.callback = callback
        self.hovered = False
        self.active = False

    def handle_event(self, event):
        if event.type == pygame.MOUSEMOTION:
            self.hovered = self.rect.collidepoint(event.pos)
        elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if self.rect.collidepoint(event.pos) and self.callback:
                self.callback()
                return True
        return False

    def draw(self, screen):
        color = (80, 130, 200) if self.hovered else (70, 70, 80)
        if self.active:
            color = (100, 150, 255)
        pygame.draw.rect(screen, color, self.rect)
        pygame.draw.rect(screen, PANEL_BORDER_COLOR, self.rect, 1)
        text_surf = self.font.render(self.text, True, TEXT_COLOR)
        text_rect = text_surf.get_rect(center=self.rect.center)
        screen.blit(text_surf, text_rect)


class InputField:
    def __init__(self, x, y, width, height, font, label="", value=""):
        self.rect = pygame.Rect(x, y, width, height)
        self.font = font
        self.label = label
        self.text = str(value)
        self.active = False
        self.cursor_visible = True
        self.cursor_timer = 0

    def set_value(self, value):
        self.text = str(value)

    def get_value(self):
        return self.text

    def get_int(self):
        try:
            return int(self.text)
        except:
            return 0

    def get_float(self):
        try:
            return float(self.text)
        except:
            return 0.0

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            self.active = self.rect.collidepoint(event.pos)
            return self.active
        elif event.type == pygame.KEYDOWN and self.active:
            if event.key == pygame.K_BACKSPACE:
                self.text = self.text[:-1]
            elif event.key == pygame.K_RETURN:
                self.active = False
            elif event.unicode.isprintable():
                self.text += event.unicode
            return True
        return False

    def update(self, dt):
        self.cursor_timer += dt
        if self.cursor_timer >= 500:
            self.cursor_visible = not self.cursor_visible
            self.cursor_timer = 0

    def draw(self, screen, offset_x=0):
        # Label
        if self.label:
            label_surf = self.font.render(self.label + ":", True, TEXT_COLOR)
            screen.blit(label_surf, (self.rect.x + offset_x - label_surf.get_width() - 5, self.rect.y + 3))

        # Input box
        actual_rect = pygame.Rect(self.rect.x + offset_x, self.rect.y, self.rect.width, self.rect.height)
        bg_color = (70, 70, 90) if self.active else (60, 60, 65)
        pygame.draw.rect(screen, bg_color, actual_rect)
        pygame.draw.rect(screen, HIGHLIGHT_COLOR if self.active else PANEL_BORDER_COLOR, actual_rect, 1)

        # Text
        text_surf = self.font.render(self.text, True, TEXT_COLOR)
        screen.blit(text_surf, (actual_rect.x + 5, actual_rect.y + 3))

        # Cursor
        if self.active and self.cursor_visible:
            cursor_x = actual_rect.x + 5 + text_surf.get_width()
            pygame.draw.line(screen, TEXT_COLOR, (cursor_x, actual_rect.y + 3), (cursor_x, actual_rect.y + actual_rect.height - 3))


class Dropdown:
    def __init__(self, x, y, width, height, font, options, default=0):
        self.rect = pygame.Rect(x, y, width, height)
        self.font = font
        self.options = options
        self.selected = default
        self.expanded = False

    def get_value(self):
        return self.options[self.selected] if self.options else ""

    def set_value(self, value):
        if value in self.options:
            self.selected = self.options.index(value)

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if self.expanded:
                # Check option clicks
                for i, opt in enumerate(self.options):
                    opt_rect = pygame.Rect(self.rect.x, self.rect.y + (i + 1) * self.rect.height, self.rect.width, self.rect.height)
                    if opt_rect.collidepoint(event.pos):
                        self.selected = i
                        self.expanded = False
                        return True
                self.expanded = False
            elif self.rect.collidepoint(event.pos):
                self.expanded = True
                return True
        return False

    def draw(self, screen, offset_x=0):
        actual_rect = pygame.Rect(self.rect.x + offset_x, self.rect.y, self.rect.width, self.rect.height)
        pygame.draw.rect(screen, (60, 60, 65), actual_rect)
        pygame.draw.rect(screen, PANEL_BORDER_COLOR, actual_rect, 1)

        text = self.options[self.selected] if self.options else ""
        text_surf = self.font.render(text, True, TEXT_COLOR)
        screen.blit(text_surf, (actual_rect.x + 5, actual_rect.y + 3))

        # Arrow
        arrow_points = [(actual_rect.right - 15, actual_rect.centery - 3),
                       (actual_rect.right - 5, actual_rect.centery - 3),
                       (actual_rect.right - 10, actual_rect.centery + 3)]
        pygame.draw.polygon(screen, TEXT_COLOR, arrow_points)

        # Expanded options
        if self.expanded:
            for i, opt in enumerate(self.options):
                opt_rect = pygame.Rect(actual_rect.x, actual_rect.y + (i + 1) * self.rect.height, self.rect.width, self.rect.height)
                pygame.draw.rect(screen, (70, 70, 80), opt_rect)
                pygame.draw.rect(screen, PANEL_BORDER_COLOR, opt_rect, 1)
                opt_surf = self.font.render(opt, True, TEXT_COLOR)
                screen.blit(opt_surf, (opt_rect.x + 5, opt_rect.y + 3))


class Checkbox:
    def __init__(self, x, y, size, font, label="", checked=False):
        self.rect = pygame.Rect(x, y, size, size)
        self.font = font
        self.label = label
        self.checked = checked

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if self.rect.collidepoint(event.pos):
                self.checked = not self.checked
                return True
        return False

    def draw(self, screen, offset_x=0):
        actual_rect = pygame.Rect(self.rect.x + offset_x, self.rect.y, self.rect.width, self.rect.height)
        pygame.draw.rect(screen, (60, 60, 65), actual_rect)
        pygame.draw.rect(screen, PANEL_BORDER_COLOR, actual_rect, 1)

        if self.checked:
            inner = actual_rect.inflate(-6, -6)
            pygame.draw.rect(screen, HIGHLIGHT_COLOR, inner)

        if self.label:
            label_surf = self.font.render(self.label, True, TEXT_COLOR)
            screen.blit(label_surf, (actual_rect.right + 5, actual_rect.y + 2))
