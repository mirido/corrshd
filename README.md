# corrshd -- image capture and correct program suitable for hand-drawn line drawings
Copyright (c) 2024 mirido
All rights reserved.

## Screen shot
<img src="https://raw.githubusercontent.com/mirido/corrshd/images/images/screenshot.png" width="800" alt="Screen during automatic correction"><BR>
The left side is the input image, the right side is the image after automatic correction.

## Introduction
This program captures a specified input-image ROI (region of interest)
and corrects for perspective distortion, uneven brightness, and line shading.
The correcting algorithm is particularly suitable to capturing hand-drawn line drawings.

## System requirement
OS: Windows 10 Pro 22H2 (build 19045.3930)<BR>
Processor: Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
Memory: 32 GB

I think it probably works on 64-bit windows PC other than abobe, but I haven't tested it.

## How to use
1. Install corrshd.exe in your PC. (See described steps later.)

2. Take a photo of the handwritten line drawing you want to capture with your smartphone or digital camera.

![Shot image](https://raw.githubusercontent.com/mirido/corrshd/images/images/how_to_use_01_shot_image.JPG)<BR>
This example is taken at a resolution of 4288x3216.<BR>
Target drawing size is the standard paper size B5 (182x257 in mm).<BR>
You can take the photo in any direction. Choose the direction that is easy to shot.<BR>
Instead, try to make the lighting as uniform as possible. (See described examples later.)

3. Copy the taken image file to an appropriate directory on your PC.<BR>
In this example, it is assumed to be copied to<BR> C:\usr2\document\163_0114\IMGP0101.JPG .

4. Type the following command into the command prompt.<BR>
Those command's arguments are file path to the input image file and physical dimensions of the drawing.<BR>
```
corrshd.exe C:\usr2\document\163_0114\IMGP0101.JPG B5
```

5. The photo will be displaied on the screen, so use your mouse to click on the four corners of the target drawing to select it.

![Click ROI corners](https://raw.githubusercontent.com/mirido/corrshd/images/images/how_to_use_02_click_ROI_corners.png)<BR>
You can fine-tune the corner position using the cursor keys.
The position of the corner in a thick red rectangle marker will be moved.<BR>
Also, if you press the TAB key, the thick red marker will be switched to the next corner.

6. The selected target is on its side, so press "R" (Shift+"r") to rotate it 90 degrees counterclockwise. <BR>(If you just press "r" key, the selected target will be rotated 90 degrees clockwise.)

![Rotate ROI image](https://raw.githubusercontent.com/mirido/corrshd/images/images/how_to_use_03_rotate_ROI_image.png)

6. Press "s" key. Press the "s" key. Then, the target line drawing (the area enclosed by clicking the corner above) will be automatically corrected.

<img src="https://raw.githubusercontent.com/mirido/corrshd/images/images/screnshot.png" width="800" alt="Screen during automatic correction"><BR>

6. The image file of the correction results is saved, so retrieve it.<BR>
Since the original image file is C:\usr2\document\163_0114\IMGP0101.JPG ,<BR>the correction result is saved as C:\usr2\document\163_0114\IMGP0101_mod.JPG<BR>in this example.<BR>
(You can change save destination directory, output file name, and image format using the options described later.)

## How to install
This program is just a single EXE file named "corrshd.exe". (OpenCV is statically linked to the EXE file. It is not in DLL form.) Therefore, you can install it using either of the following methods:

### A. Normal method: Setup PATH environment.
Copy corrshd.exe to an appropriate directory and set the PATH environment to it.

### B. Alternative method: Copy to working directory directly.
Copy corrshd.exe to the directory where the photo image file is placed as below.<BR>
And use program there.<BR>
![Direct copy](https://raw.githubusercontent.com/mirido/corrshd/images/images/how_to_install_direct_copy.png)

## How to build
If you want to build this program from source code, first download OpenCV 4.8.0 and generate the solution by specifying BUILD_SHARED_LIBS=OFF with CMake.

After that, when you build the generated solution, a static link library will be built.

Then follow the [readme.txt](./opencv/readme.txt) in the directory $(SolutionDir)opencv .

## Advanced topics
### More fine-tuning of corner points
If you want to more fine-tune the corners of the target drawing, you can adjust the corner position pixel by pixel after pressing the "z" key.

<img src="https://raw.githubusercontent.com/mirido/corrshd/images/images/advanced_topics_fine_tuning_while_zoom_in.jpg" width="800" alt="Fine-tuining while zoom in"><BR>
 Press the "z" key again to return to full view.

1. Double click the file `index.html` in workspace.<br>(Or open the html with Live Server on VSCode.)

### Precautions when shooting
#### Lighting first!
Lighting first! Avoid casting shadows on target drawing. If you are too particular to shot the target drawing from directly in front, you will end up as shown in following NG example.<BR>
It's much better to take a photo from an angle instead of casting a shadow.

![NG example](https://raw.githubusercontent.com/mirido/corrshd/images/images/advanced_topics_NG_example.JPG)<BR>
NG!

![OK example](https://raw.githubusercontent.com/mirido/corrshd/images/images/how_to_use_01_shot_image.JPG)<BR>
GOOD.

#### Prevent anything blacker than the line from appearing
If an object that is too dark falls inside the four corners you clicked, its black color of that object will be used as the standard for automatic correction, and the handwritten lines will become blurred.
​
![NG by two clips](https://raw.githubusercontent.com/mirido/corrshd/images/images/advanced_topics_NG_example_two_clips.JPG)<BR>
NG! Two clips are shot in too dark.

![OK by countermeasure](https://raw.githubusercontent.com/mirido/corrshd/images/images/advanced_topics_OK_by_countermeasure.JPG)<BR>
Countermeasures were taken for above NG case<BR>
The darkness is alleviated by covering it with white paper.<BR>
Alternatively, if you don't feel like it's a hassle, you can delete the clips using software such as GIMP.

## Image format change
The image format is determined by extension in file name.<BR>
If you start the program by giving only the name of the input image file as shown below, the image file resulting from automatic correction will be saved in the same image format.<BR>
```
corrshd.exe C:\usr2\document\163_0114\IMGP0101.JPG B5
```
If you want to use different image formats for the input and output files, use the -outfile option to explicitly specify the output file name along with the desired extension.

Example:<BR>
```
corrshd.exe C:\usr2\document\163_0114\IMGP0101.JPG B5 -outfile=result.png
```
Note: If the file name specified with the outfile option does not include a directory path, it will be saved in the same folder as the input file.

## Irregular image size

### 1. How to shot in Landscape size
Add "L" prefix before the standard size name to specify Landscape.<BR>
For example, if the size is B5 size turned sideways, specify "LB5".

Example:<BR>
```
corrshd.exe C:\usr2\document\163_0114\IMGP0101.JPG LB5
```

<img src="https://raw.githubusercontent.com/mirido/corrshd/images/images/advanced_topics_landscape.jpg" width="800" alt="Fine-tuining by zoom in"><BR>

### 2. How to specify any size
Instead of the standard size name, specify the size in the format "&lt;width in mm&gt;x&lt;height in mm&gt;".

Example:<BR>
```
corrshd.exe C:\usr2\document\163_0114\IMGP0101.JPG 182x257
```
By specifying the decimal point, you can also specify increments smaller than 1 mm.

Example:<BR>
```
corrshd.exe C:\usr2\document\163_0114\IMGP0101.JPG 123.5x567.8
```


# Commandline option
~~~
Usage: corrshd.exe [params] image-file roi-size

        -?, -h
                print this message
        --cutoffonly
                do nothing other than perspective correction
        --dpi (value:96.0)
                output resolution in dot per inch
        --outfile
                output image file name

        image-file
                image file to be corrected
        roi-size (value:B5)
                physical size of ROI
~~~

# Hot key
~~~
Hot keys:
        ESC          - quit the program
        r            - rotate input-image 90 degrees clockwise
        R            - rotate input-image 90 degrees counterclockwise
        z            - zoom in or out the input-image (toggled)
        (cursor key) - move the current corner point of ROI (range of interest) in pixel-wise
        TAB          - switch the current corner point of ROI to neighbor
        s            - correct the input-image ROI and save result
~~~

# License
Copyright (c) mirido. All rights reserved.

Licensed under The 3-Clause BSD License.

# Third party notices
This program incorporates material as listed below or described in the code.<BR>

1. @opencv/opencv 4.8.0 - Apache License Version 2.0<BR>
https://github.com/opencv/opencv

See also [ThirdPartyNotices.txt](./ThirdPartyNotices.txt) for more detail.
