from PIL import Image, ImageDraw, ImageFont
import math

# Parameter
width, height = 64, 32
background_color = (0, 0, 0)  # Schwarz
text_color = (255, 0, 0)  # Rot
num_frames = 60  # Anzahl der Frames für die Animation
font_size = 20  # Größe der Schriftart

# Erstellen der Schriftart
font = ImageFont.truetype("arial.ttf", font_size)

# Erstellen der Frames
frames = []

# Berechnung der Bewegung und Drehung
for frame in range(num_frames):
    # Neue Bilddatei mit schwarzem Hintergrund
    img = Image.new('RGB', (width, height), background_color)
    draw = ImageDraw.Draw(img)

    # Berechnung des Fortschritts für die Bewegung von links nach rechts
    progress = frame / (num_frames - 1)
    x_position = int(progress * (width - 1))

    # Berechnung der Drehung: von -90° bis 90°
    rotation_angle = -90 + (180 * progress)

    # Erstellen des Textbilds für den rotierenden Text
    text_image = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    text_draw = ImageDraw.Draw(text_image)
    text_bbox = text_draw.textbbox((0, 0), "3D", font=font)
    text_width = text_bbox[2] - text_bbox[0]
    text_height = text_bbox[3] - text_bbox[1]

    # Positionierung des Texts im Textbild (zentriert)
    text_x = (width - text_width) // 2
    text_y = (height - text_height) // 2
    text_draw.text((text_x, text_y), "3D", font=font, fill=text_color)

    # Rotieren des Texts
    rotated_text = text_image.rotate(rotation_angle, expand=True, resample=Image.BICUBIC)

    # Berechnung der Position, um den Text in der Mitte des Bildes zu platzieren
    paste_x = x_position
    paste_y = (height - rotated_text.height) // 2

    # Sicherstellen, dass der Text nicht außerhalb des Bildes liegt
    if paste_x + rotated_text.width > width:
        paste_x = width - rotated_text.width
    if paste_y < -16:
        paste_y = -16

    # Kopieren des rotierenden Texts in das Hauptbild an der berechneten Position
    img.paste(rotated_text, (paste_x, paste_y), rotated_text)

    # Hinzufügen des Frames zur Liste
    frames.append(img)

    # Speichern als eigene Datei
    with open(f'rot3d{frame:03d}.raw', "wb") as f:
        for y in range(height):
            for x in range(width):
                # Holen der RGBA-Werte für den aktuellen Pixel
                r, g, b = img.getpixel((x, y))
                # Schreiben der Werte in die Datei (4 Bytes pro Pixel)
                f.write(bytes([r, g, b, 0]))

# Speichern der Frames als GIF
frames[0].save('text_animation.gif', save_all=True, append_images=frames[1:], duration=100, loop=0)

print("Animation wurde erfolgreich erstellt!")
