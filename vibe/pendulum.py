import math
import sys
from dataclasses import dataclass, field
import pygame


def clamp(x, a, b):
    return a if x < a else b if x > b else x


def wrap_deg(d):
    return (d + 180.0) % 360.0 - 180.0


@dataclass
class Config:
    fps: int = 60
    time_scale: float = 1.0
    g: float = 9.81

    width: int = 1500
    height: int = 950
    graph_w: int = 520

    origin_y_div: float = 2.5  # ORIGIN = (sim_w//2, height//origin_y_div)

    m1: float = 1.0
    m2: float = 1.0
    l1: float = 220.0
    l2: float = 180.0

    damp: float = 0.0005
    substeps: int = 6

    plot_seconds: int = 300
    trace_max: int = 2500

    # colors
    bg: tuple = (18, 18, 22)
    white: tuple = (240, 240, 245)
    gray: tuple = (140, 140, 150)
    trace_color: tuple = (90, 170, 255)

    c_grid: tuple = (80, 80, 90)
    c_axis: tuple = (160, 160, 170)
    c_trail: tuple = (235, 210, 120)
    c_dot: tuple = (235, 210, 120)


@dataclass
class DoublePendulum:
    cfg: Config
    theta1: float = math.radians(120)
    theta2: float = math.radians(-10)
    omega1: float = 0.0
    omega2: float = 0.0

    @property
    def origin(self):
        sim_w = self.cfg.width - self.cfg.graph_w
        return (sim_w // 2, self.cfg.height // self.cfg.origin_y_div)

    def accelerations(self, th1, th2, w1, w2):
        g = self.cfg.g * 70.0  # pixel scaling

        delta = th2 - th1
        sin_d = math.sin(delta)
        cos_d = math.cos(delta)

        m1, m2 = self.cfg.m1, self.cfg.m2
        L1, L2 = self.cfg.l1, self.cfg.l2

        denom1 = (m1 + m2) * L1 - m2 * L1 * cos_d * cos_d
        denom2 = (L2 / L1) * denom1

        a1 = (
            m2 * L1 * w1 * w1 * sin_d * cos_d
            + m2 * g * math.sin(th2) * cos_d
            + m2 * L2 * w2 * w2 * sin_d
            - (m1 + m2) * g * math.sin(th1)
        ) / denom1

        a2 = (
            -m2 * L2 * w2 * w2 * sin_d * cos_d
            + (m1 + m2) * (g * math.sin(th1) * cos_d - L1 * w1 * w1 * sin_d - g * math.sin(th2))
        ) / denom2

        a1 -= self.cfg.damp * w1
        a2 -= self.cfg.damp * w2
        return a1, a2

    def rk4_step(self, state, dt):
        def deriv(s):
            th1, th2, w1, w2 = s
            a1, a2 = self.accelerations(th1, th2, w1, w2)
            return (w1, w2, a1, a2)

        def add(s, k, scale):
            return tuple(si + scale * ki for si, ki in zip(s, k))

        s = state
        k1 = deriv(s)
        k2 = deriv(add(s, k1, dt * 0.5))
        k3 = deriv(add(s, k2, dt * 0.5))
        k4 = deriv(add(s, k3, dt))

        return tuple(
            si + (dt / 6.0) * (k1i + 2 * k2i + 2 * k3i + k4i)
            for si, k1i, k2i, k3i, k4i in zip(s, k1, k2, k3, k4)
        )

    def step(self, dt_real):
        state = (self.theta1, self.theta2, self.omega1, self.omega2)
        h = (dt_real * self.cfg.time_scale) / self.cfg.substeps
        for _ in range(self.cfg.substeps):
            state = self.rk4_step(state, h)
        self.theta1, self.theta2, self.omega1, self.omega2 = state

    def positions(self):
        ox, oy = self.origin
        x1 = ox + self.cfg.l1 * math.sin(self.theta1)
        y1 = oy + self.cfg.l1 * math.cos(self.theta1)
        x2 = x1 + self.cfg.l2 * math.sin(self.theta2)
        y2 = y1 + self.cfg.l2 * math.cos(self.theta2)
        return (x1, y1), (x2, y2)

    def angles_from_positions(self, x1, y1, x2, y2):
        ox, oy = self.origin

        th1 = math.atan2(x1 - ox, y1 - oy)
        x1 = ox + self.cfg.l1 * math.sin(th1)
        y1 = oy + self.cfg.l1 * math.cos(th1)

        th2 = math.atan2(x2 - x1, y2 - y1)
        x2 = x1 + self.cfg.l2 * math.sin(th2)
        y2 = y1 + self.cfg.l2 * math.cos(th2)

        return th1, th2, (x1, y1), (x2, y2)

    def reset(self):
        self.theta1 = math.radians(120)
        self.theta2 = math.radians(-10)
        self.omega1 = 0.0
        self.omega2 = 0.0

    def zero_vel(self):
        self.omega1 = 0.0
        self.omega2 = 0.0


@dataclass
class PhasePlot:
    cfg: Config
    points: list = field(default_factory=list)

    @property
    def max_points(self):
        return int(self.cfg.plot_seconds * self.cfg.fps)

    def push(self, theta1_rad, theta2_rad):
        d1 = wrap_deg(math.degrees(theta1_rad))
        d2 = wrap_deg(math.degrees(theta2_rad))
        self.points.append((d1, d2))
        if len(self.points) > self.max_points:
            self.points = self.points[-self.max_points :]

    def clear(self):
        self.points.clear()

    def draw(self, screen, rect, font):
        pygame.draw.rect(screen, (14, 14, 18), rect)

        pad_l, pad_r, pad_t, pad_b = 54, 14, 28, 32
        gx0 = rect.left + pad_l
        gx1 = rect.right - pad_r
        gy0 = rect.top + pad_t
        gy1 = rect.bottom - pad_b

        pygame.draw.rect(screen, (10, 10, 14), (gx0, gy0, gx1 - gx0, gy1 - gy0), border_radius=10)

        def x_from_deg(deg):
            return gx0 + (deg + 180.0) * (gx1 - gx0) / 360.0

        def y_from_deg(deg):
            return gy0 + (180.0 - deg) * (gy1 - gy0) / 360.0

        ticks = [-180, -135, -90, -45, 0, 45, 90, 135, 180]
        for t in ticks:
            x = int(x_from_deg(t))
            y = int(y_from_deg(t))
            pygame.draw.line(screen, self.cfg.c_grid, (x, gy0), (x, gy1), 1)
            pygame.draw.line(screen, self.cfg.c_grid, (gx0, y), (gx1, y), 1)

        x0 = int(x_from_deg(0))
        y0 = int(y_from_deg(0))
        pygame.draw.line(screen, self.cfg.c_axis, (x0, gy0), (x0, gy1), 2)
        pygame.draw.line(screen, self.cfg.c_axis, (gx0, y0), (gx1, y0), 2)

        for t in ticks:
            x = int(x_from_deg(t))
            y = int(y_from_deg(t))
            labx = font.render(f"{t}°", True, (190, 190, 200))
            laby = font.render(f"{t}°", True, (190, 190, 200))
            screen.blit(labx, (x - labx.get_width() // 2, gy1 + 6))
            screen.blit(laby, (rect.left + 6, y - laby.get_height() // 2))

        title = font.render(f"{self.cfg.plot_seconds}-SECOND PLOT  (x=θ1, y=θ2)", True, (230, 210, 140))
        screen.blit(title, (rect.left + 10, rect.top + 6))

        if len(self.points) >= 2:
            pts = [(int(x_from_deg(a)), int(y_from_deg(b))) for (a, b) in self.points]
            pygame.draw.lines(screen, self.cfg.c_trail, False, pts, 2)
            cx, cy = pts[-1]
            pygame.draw.circle(screen, self.cfg.c_dot, (cx, cy), 6)
            pygame.draw.circle(screen, (40, 40, 50), (cx, cy), 8, 2)


@dataclass
class Trail:
    max_len: int
    points: list = field(default_factory=list)

    def push(self, p):
        self.points.append(p)
        if len(self.points) > self.max_len:
            self.points.pop(0)

    def clear(self):
        self.points.clear()


class App:
    def __init__(self):
        self.cfg = Config()
        self.sim_w = self.cfg.width - self.cfg.graph_w

        pygame.init()
        self.screen = pygame.display.set_mode((self.cfg.width, self.cfg.height))
        pygame.display.set_caption("Double Pendulum (RK4) + Phase Plot (θ1 vs θ2) - OOP")
        self.clock = pygame.time.Clock()
        self.font = pygame.font.SysFont("consolas", 16)

        self.pend = DoublePendulum(self.cfg)
        self.phase = PhasePlot(self.cfg)
        self.trace = Trail(self.cfg.trace_max)

        self.dragging1 = False
        self.dragging2 = False
        self.paused = False

        _, p2 = self.pend.positions()
        self.trace.push(p2)

    def reset(self):
        self.pend.reset()
        self.pend.zero_vel()
        self.phase.clear()
        self.trace.clear()
        self.cfg.time_scale = 1.0
        self.paused = False
        self.dragging1 = self.dragging2 = False

    def handle_mouse_down(self, mx, my):
        if mx >= self.sim_w:
            return

        p1, p2 = self.pend.positions()
        d1 = (mx - p1[0]) ** 2 + (my - p1[1]) ** 2
        d2 = (mx - p2[0]) ** 2 + (my - p2[1]) ** 2

        R1 = 28 ** 2
        R2 = 30 ** 2

        self.dragging1 = self.dragging2 = False
        if d1 <= R1 and d1 <= d2:
            self.dragging1 = True
        elif d2 <= R2:
            self.dragging2 = True

    def handle_mouse_up(self):
        self.dragging1 = False
        self.dragging2 = False

    def update_dragging(self):
        if self.dragging1:
            mx, my = pygame.mouse.get_pos()
            mx = clamp(mx, 0, self.sim_w - 1)
            my = clamp(my, 0, self.cfg.height - 1)
            ox, oy = self.pend.origin
            self.pend.theta1 = math.atan2(mx - ox, my - oy)
            self.pend.zero_vel()
            self.trace.clear()
            self.phase.clear()

        elif self.dragging2:
            mx, my = pygame.mouse.get_pos()
            mx = clamp(mx, 0, self.sim_w - 1)
            my = clamp(my, 0, self.cfg.height - 1)
            p1, _ = self.pend.positions()
            th1, th2, _, _ = self.pend.angles_from_positions(p1[0], p1[1], mx, my)
            self.pend.theta1, self.pend.theta2 = th1, th2
            self.pend.zero_vel()
            self.trace.clear()
            self.phase.clear()

    def update(self, dt_real):
        # dragging works even paused
        self.update_dragging()

        if (not self.paused) and (not self.dragging1) and (not self.dragging2):
            self.pend.step(dt_real)

        p1, p2 = self.pend.positions()
        self.trace.push(p2)
        self.phase.push(self.pend.theta1, self.pend.theta2)

        return p1, p2

    def draw(self, p1, p2):
        self.screen.fill(self.cfg.bg)

        if len(self.trace.points) > 2:
            pygame.draw.lines(self.screen, self.cfg.trace_color, False, self.trace.points, 2)

        pygame.draw.line(self.screen, self.cfg.gray, self.pend.origin, p1, 4)
        pygame.draw.line(self.screen, self.cfg.gray, p1, p2, 4)

        pygame.draw.circle(self.screen, self.cfg.white, (int(self.pend.origin[0]), int(self.pend.origin[1])), 6)
        pygame.draw.circle(self.screen, self.cfg.white, (int(p1[0]), int(p1[1])), 14)
        pygame.draw.circle(self.screen, self.cfg.white, (int(p2[0]), int(p2[1])), 16)

        pygame.draw.line(self.screen, (60, 60, 70), (self.sim_w, 0), (self.sim_w, self.cfg.height), 2)

        ui = [
            "R: reset | SPACE: stop | P: pause | UP/DOWN: speed",
            "LMB: click/drag near p1 or p2",
            "PAUSE" if self.paused else "",
            f"time x{self.cfg.time_scale:.3g}",
            f"theta1={math.degrees(self.pend.theta1):.1f}°  theta2={math.degrees(self.pend.theta2):.1f}°",
            f"omega1={self.pend.omega1:.2f}  omega2={self.pend.omega2:.2f}",
        ]
        y = 10
        for line in ui:
            if not line:
                continue
            self.screen.blit(self.font.render(line, True, (220, 220, 230)), (10, y))
            y += 18

        graph_rect = pygame.Rect(self.sim_w, 0, self.cfg.graph_w, self.cfg.height)
        self.phase.draw(self.screen, graph_rect, self.font)

        pygame.display.flip()

    def run(self):
        while True:
            dt_real = self.clock.tick(self.cfg.fps) / 1000.0

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()

                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_r:
                        self.reset()
                    elif event.key == pygame.K_SPACE:
                        self.pend.zero_vel()
                    elif event.key == pygame.K_p:
                        self.paused = not self.paused
                    elif event.key == pygame.K_UP:
                        self.cfg.time_scale = min(self.cfg.time_scale * 2.0, 32.0)
                        self.cfg.fps = min(self.cfg.fps * 2, 1920)
                    elif event.key == pygame.K_DOWN:
                        self.cfg.time_scale = max(self.cfg.time_scale / 2.0, 0.125)
                        self.cfg.fps = max(self.cfg.fps // 2, 60)

                if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                    self.handle_mouse_down(*event.pos)
                if event.type == pygame.MOUSEBUTTONUP and event.button == 1:
                    self.handle_mouse_up()

            p1, p2 = self.update(dt_real)
            self.draw(p1, p2)


if __name__ == "__main__":
    App().run()
