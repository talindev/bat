from PIL import Image

palette = [
    (0, 0, 0),
    (15, 10, 15),
    (103, 67, 112),
    (87, 58, 94),
    (194, 130, 186),
    (250, 192, 243),
    (242, 240, 206),
    (250, 36, 68),
    (159, 32, 53),
    (254, 254, 254),
    (212, 210, 212),
    (181, 179, 181)
]

def get_color_index(rgb):
    # allows margin for error (image compression, etc)
    min_dist = float('inf')
    best_idx = 0
    for i, p_color in enumerate(palette):
        dist = sum((a - b) ** 2 for a, b in zip(rgb[:3], p_color))
        if dist < min_dist:
            min_dist = dist
            best_idx = i
    return best_idx

# filename search factor
frames_files = [f"assets/frame{i}.png" for i in range(1, 9)]

print("{")

for frame_idx, filename in enumerate(frames_files):
    try:
        img = Image.open(filename).convert("RGB")

        if img.size != (37, 16):
            print(f"  // ERROR: {filename} does not have 37x16 pixels. Actual size: {img.size}")
            continue

        print("    {")
        for y in range(16):
            row_data = []
            for x in range(37):
                pixel = img.getpixel((x, y))
                # converts background color to transparent
                if pixel == (157, 101, 171):
                    idx = 0
                else:
                    idx = get_color_index(pixel)
                row_data.append(str(idx))

            comma = "," if y < 15 else ""
            print(f"        {{{','.join(row_data)}}}{comma}")

        comma_frame = "," if frame_idx < 7 else ""
        print(f"    }}{comma_frame}")

    except FileNotFoundError:
        print(f"  // FILE NOT FOUND: {filename}")

print("};")
