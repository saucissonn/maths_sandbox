import random
import string
import os
from PIL import Image, ImageDraw, ImageFont, ImageFilter

CHARSET = (
    string.ascii_lowercase
    + string.ascii_uppercase
    + string.digits
)
CHARLIST = list(CHARSET)

W, H = 1000, 1000
OUT_IMG = "images"
OUT_TXT = "images_ans"

N  = 100   # clean
N2 = 100   # bruit
N3 = 100  # caractères inclinés

fonts_paths = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf",
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
]

TARGET_H = 20
LINE_GAP = 40
X_MARGIN = 20

os.makedirs(OUT_IMG, exist_ok=True)
os.makedirs(OUT_TXT, exist_ok=True)

def font_for_target_height(draw, font_path, target_h):
    size = target_h
    for _ in range(6):
        try:
            f = ImageFont.truetype(font_path, size)
        except:
            return None
        bbox = draw.textbbox((0, 0), "Hg", font=f)
        h = bbox[3] - bbox[1]
        if h <= 0:
            return None
        size = max(1, int(size * (target_h / h)))
    try:
        return ImageFont.truetype(font_path, size)
    except:
        return None

def add_noise(img):
    px = img.load()
    total = img.size[0] * img.size[1]

    for _ in range(int(total * 0.002)):
        x = random.randrange(img.size[0]); y = random.randrange(img.size[1])
        px[x, y] = (255, 255, 255)
    for _ in range(int(total * 0.002)):
        x = random.randrange(img.size[0]); y = random.randrange(img.size[1])
        px[x, y] = (0, 0, 0)

    noise = Image.effect_noise(img.size, 6).convert("L")
    noise_rgb = Image.merge("RGB", (noise, noise, noise))
    img = Image.blend(img, noise_rgb, alpha=0.12)

    if random.random() < 0.5:
        img = img.filter(ImageFilter.GaussianBlur(radius=random.choice([0.3, 0.6, 1.0])))
    return img

def draw_line_normal(draw, y, W, font):
    # génère une ligne avec espaces, mais on retourne aussi la version sans espaces pour le txt
    bbox_a = draw.textbbox((0, 0), "a", font=font)
    char_w = max(1, bbox_a[2] - bbox_a[0])

    bbox_hg = draw.textbbox((0, 0), "Hg", font=font)
    char_h = max(1, bbox_hg[3] - bbox_hg[1])

    step_x = max(1, char_w * 2)
    n_chars = max(1, W // step_x)

    chars = [random.choice(CHARLIST) for _ in range(n_chars)]
    text = " ".join(chars)
    draw.text((0, y), text, fill="black", font=font)

    return text, char_h

def draw_line_tilted_chars(img, draw, y, W, font, max_angle=5.0):
    # on dessine caractère par caractère (sans espaces dans l'image)
    bbox_a = draw.textbbox((0, 0), "a", font=font)
    base_w = max(X_MARGIN, bbox_a[2] - bbox_a[0])

    bbox_hg = draw.textbbox((0, 0), "Hg", font=font)
    base_h = max(1, bbox_hg[3] - bbox_hg[1])

    # largeur "case" par caractère
    cell_w = max(1, int(base_w * 1.7))
    n_chars = max(1, W // cell_w)

    chars = [random.choice(CHARLIST) for _ in range(n_chars)]
    x = 0

    for ch in chars:
        angle = random.uniform(-max_angle, max_angle)

        # rendu du caractère sur une petite image RGBA
        tmp = Image.new("RGBA", (cell_w * 2, base_h * 2), (0, 0, 0, 0))
        d = ImageDraw.Draw(tmp)
        d.text((cell_w // 2, base_h // 2), ch, fill=(0, 0, 0, 255), font=font)

        rot = tmp.rotate(angle, resample=Image.Resampling.BICUBIC, expand=True)

        # paste alpha
        img.paste(rot, (x, y), rot)
        x += cell_w

        if x >= W - cell_w:
            break

    text = "".join(chars)
    return text, base_h

def generate_one(img_path, txt_path, mode):
    img = Image.new("RGB", (W, H), "white")
    draw = ImageDraw.Draw(img)

    y = 0
    out_lines = []

    while y < H - 40:
        font_path = random.choice(fonts_paths)
        font = font_for_target_height(draw, font_path, TARGET_H)
        if font is None:
            continue

        if mode == "tilt_chars":
            line_txt, char_h = draw_line_tilted_chars(img, draw, y, W - 20, font, max_angle=10.0)
        else:
            line_img, char_h = draw_line_normal(draw, y, W, font)
            line_txt = line_img.replace(" ", "")  # txt sans espaces

        out_lines.append(line_txt)
        y += char_h + LINE_GAP

    if mode == "noise":
        img = add_noise(img)

    img.save(img_path)

    # txt : sans espaces, avec \n entre lignes
    with open(txt_path, "w", encoding="utf-8") as f:
        f.write("".join(out_lines))

def make_all():
    total = N + N2 + N3
    idx = 0

    # clean
    for _ in range(N):
        base = f"random_image_{idx:06d}"
        generate_one(
            os.path.join(OUT_IMG, base + ".png"),
            os.path.join(OUT_TXT, base + ".txt"),
            mode="clean"
        )
        idx += 1

    # noise
    for _ in range(N2):
        base = f"random_image_{idx:06d}"
        generate_one(
            os.path.join(OUT_IMG, base + ".png"),
            os.path.join(OUT_TXT, base + ".txt"),
            mode="noise"
        )
        idx += 1

    # tilted characters
    for _ in range(N3):
        base = f"random_image_{idx:06d}"
        generate_one(
            os.path.join(OUT_IMG, base + ".png"),
            os.path.join(OUT_TXT, base + ".txt"),
            mode="tilt_chars"
        )
        idx += 1

    print(f"Done: {total} images dans {OUT_IMG}/ et {total} txt dans {OUT_TXT}/")

if __name__ == "__main__":
    make_all()