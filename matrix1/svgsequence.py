import argparse
import os
import subprocess
import xml.etree.ElementTree as ET

parser = argparse.ArgumentParser(description="Create a sequence from svg objects.")
parser.add_argument("inputfile", help="File to be processed")

args = parser.parse_args()

svgfiles = []
pngfiles = []
rgbafiles = []


def CreateOneSvgFilePerObject(inputfile):
    tree = ET.parse(inputfile)
    root = tree.getroot()
    g = root.find("{http://www.w3.org/2000/svg}g")
    steps = len(g)

    filename, file_extension = os.path.splitext(inputfile)
    while steps > 0:
        tree = ET.parse(inputfile)
        root = tree.getroot()
        g = root.find("{http://www.w3.org/2000/svg}g")

        remaining_obj = list(g)[steps - 1]
        for item in list(g):
            if item != remaining_obj:
                g.remove(item)

        # write file
        xml = ET.tostring(root)
        outputfilename = f"{filename}-{steps:02}{file_extension}"
        svgfiles.append(outputfilename)
        outputfile = open(outputfilename, "wb")
        outputfile.write(xml)
        outputfile.close()

        steps = steps - 1


def ConvertToPngFiles():
    for svgfilename in svgfiles:
        filename = os.path.splitext(svgfilename)[0]
        pngfilename = f"{filename}.png"
        pngfiles.append(pngfilename)
        subprocess.run(
            [
                "inkscape",
                "-z",  # no GUI
                svgfilename,  # input file
                "-o",  # export
                pngfilename,  # destination file
                "--export-area-page",  # whole page
                "--export-width=64",
                "--export-height=32",
            ]
        )
        os.remove(svgfilename)


def ConvertToRawFiles():
    for pngfilename in pngfiles:
        filename = os.path.splitext(pngfilename)[0]
        rgbafilename = f"{filename}.rgba"
        rgbafiles.append(rgbafilename)
        subprocess.run(["magick", pngfilename, rgbafilename])
        os.remove(pngfilename)


def MergeRawFiles():
    blended_image = []
    for i in range(64 * 32):
        for color in range(3):
            blended_image.append(0)
        blended_image.append(255)
    rgbafiles.reverse()
    for rgbafilename in rgbafiles:
        filename = os.path.splitext(rgbafilename)[0]
        inputfile = open(rgbafilename, "rb")
        for i in range(64 * 32):
            pixel = inputfile.read(4)
            a = pixel[3]
            for color in range(3):
                blended_image[i * 4 + color] = min(
                    255,
                    int(
                        pixel[color] * (a / 255)
                        + (blended_image[i * 4 + color] * 0.66) * ((255 - a) / 255)
                    ),
                )
        inputfile.close()
        outputfile = open(rgbafilename, "wb")
        outputfile.write(bytes(blended_image))
        outputfile.close()
        subprocess.run(
            [
                "convert",
                "-size",
                "64x32",
                "-depth",
                "8",
                rgbafilename,
                f"{filename}-merged.png",
            ]
        )


# create one svg per object
CreateOneSvgFilePerObject(args.inputfile)
ConvertToPngFiles()
ConvertToRawFiles()
MergeRawFiles()
