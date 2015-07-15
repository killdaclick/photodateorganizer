# DESCRIPTION
Corrects modification date file's attribute using original EXIF metadata "camera taken date" timestamp tag.
Organizes photo files in folder structure (e.g. with dates in folder name) and changes their file's name using configurable template.

Modification date file's attribute changes during write operation like rotation of images in Windows preview application. This might be anoying if you like to have your photos sorted. You can revert date of your images to original "camera taken date" timestamp working on original files or creating new ones in templated folder structure with templated new names. Application shows full image's EXIF information in brief and extended mode and additionaly you can preview and rotate your images.

**LANGUAGES: English, Polish**

*Software uses Qt library and exiv2 binary.*



# FEATURES

* Corrects photo's modification date attribute - reverts to original "camera taken date"
* Creates subfolders and modifies files name according to configurable templates
* Option to preserve original folder structure when creating new folders with dates in name
* Filters source files list using date and time from/to
* Automatically changes destination file name if file already exists (does not overwrites it)
* Shows full EXIF image information - in brief and extended format
* Previews images and allows to rotate them
* Contains configuration presets for easy and fast conversions
* Shows: total source files count and size, converted files count and size, conversion speed in MB/s and time to finish



# CHANGES

**v1.5.0**

* New "Image information" tab added. Displays full EXIF image information briefly and in extended mode
* New "Image preview" tab added. Shows image preview and contains right-click menu for image rotation

**v1.4.x**

* English translation
* Input files date/time filter

**v1.3.x**

* Preserve original folder structure option in "Options" tab
* Shows files summary like: total file size, total file count, conversion speed, time to finish conversion, etc., etc.



## TODO LIST
* JPEG loseless rotation in "Image preview" tab and in conversion options
* Image automatic rotation based on EXIF orientation tag
* Linux version (?)



### License

GNU GENERAL PUBLIC LICENSE Version 2
