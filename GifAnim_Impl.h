// when including in other code, allow disabling all those inits
// to avoid double inits.
#ifndef GIFANIM_INCLUDE
#include "animatedgif_config.h"
#endif

// Use NeoMatrix backend? Defined in main ino that calls sav_loop
#ifdef NEOMATRIX
// select which NEOMATRIX config will be selected
    //#define M16BY16T4
    #include "neomatrix_config.h"
// else use SmartMatrix as defined in config.h
#else
    SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
    SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
    #ifdef ENABLE_SCROLLING
    SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
    #endif
#endif // NEOMATRIX


#include "GifDecoder.h"

// Used by SimpleGifAnimViewer2 to work without FilenameFunctions*
#ifdef UNIXFS
    // https://www.programiz.com/c-programming/c-file-input-output
    // https://www.gnu.org/software/libc/manual/html_node/I_002fO-on-Streams.html
    #include <stdio.h>
    #include <stdlib.h>
    #include <dirent.h>
    #include <cerrno>
    FILE *file;
    // This can be used by the caller to retrieve the name by index
    char pathname[2048];

    bool fileSeekCallback(unsigned long position) { return (fseek(file, position, SEEK_SET) != -1); }
    unsigned long filePositionCallback(void) { return ftell(file); }
    int fileReadCallback(void) { return getc(file); }
    int fileReadBlockCallback(void * buffer, int numberOfBytes) { return fread(buffer, 1, numberOfBytes, file); }

    // The functions below belong in FilenameFunctions_Impl.h, but I didn't want
    // to fill them with linux ifdefs, so I just made separate versions here:
    int enumerateGIFFiles(const char *directoryName, boolean displayFilenames) {
	int numberOfFiles = 0;
	Serial.print("Enumerate files in dir ");
	Serial.println(directoryName);
	DIR *d;
	struct dirent *dir;
	if (! (d = opendir(directoryName))) return 0;
	while ((dir = readdir(d)) != NULL) {
	    if (dir->d_name[0] == '.') continue;
	    if (!strstr(dir->d_name, ".gif")) continue;
	    if (displayFilenames) { Serial.print(numberOfFiles); Serial.print(" : "); Serial.println(dir->d_name); }
	    numberOfFiles++;
	}
	closedir(d);
	return numberOfFiles;
    }

    // Get the full path/filename of the GIF file with specified index
    // This doesn't check if the files are gifs like the arduino version
    void getGIFFilenameByIndex(const char *directoryName, int index, char *pnBuffer) {
	DIR *d;
	struct dirent *dir;
	if (! (d = opendir(directoryName))) return;
	dir = readdir(d);
	while (dir && (index >= 0)) {
	    // Serial.print("Check file "); Serial.println(dir->d_name);
	    // this doesn't check where in the string, but good enough
	    if (!(dir->d_name[0] == '.') && strstr(dir->d_name, ".gif")) {
		index--;
		// Serial.print("index "); Serial.println(index);
		// buffer overflow, directory+file size is on you.
		strcpy(pnBuffer, directoryName);
		strcat(pnBuffer, "/");
		strcat(pnBuffer, dir->d_name);
	    }
	    dir = readdir(d);
	}
	Serial.print("Selected file ");
	Serial.println(pnBuffer);
    }

    int openGifFilenameByIndex(const char *directoryName, int index) {
	getGIFFilenameByIndex(directoryName, index, pathname);
	if (file) fclose(file);
	file = fopen(pathname, "r");
	return 0;
    }


#elif defined(BASICARDUINOFS)
    File file;
    bool fileSeekCallback(unsigned long position) { return file.seek(position); }
    unsigned long filePositionCallback(void) { return file.position(); }
    int fileReadCallback(void) { return file.read(); }
    int fileReadBlockCallback(void * buffer, int numberOfBytes) { return file.read((uint8_t*)buffer, numberOfBytes); }
#else
    extern File file;
    #include "FilenameFunctions_Impl.h"
#endif

/* template parameters are maxGifWidth, maxGifHeight, lzwMaxBits
 * defined in config.h
 */
GifDecoder<gif_size, gif_size, lzwMaxBits> decoder;

void screenClearCallback(void) {
#ifdef NEOMATRIX
  matrix->clear();
#else
  backgroundLayer.fillScreen({0,0,0});
#endif
}

void updateScreenCallback(void) {
#ifdef NEOMATRIX
  matrix->show();
#else
  backgroundLayer.swapBuffers();
#endif
}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  red   = matrix->gamma[red];
  green = matrix->gamma[green];
  blue  = matrix->gamma[blue];
#if DEBUGLINE
  if (y == DEBUGLINE) {
      Serial.print(x);
      Serial.print(",");
      Serial.print(y);
      Serial.print(">");
      Serial.print(red);
      Serial.print(",");
      Serial.print(green);
      Serial.print(",");
      Serial.print(blue);
      Serial.println("");
  }
  if (y > DEBUGLINE) return;
#endif
#ifdef NEOMATRIX
  CRGB color = CRGB(red, green, blue);

  uint16_t basex = x*FACTX/10+OFFSETX;
  uint16_t basey = y*FACTY/10+OFFSETY;

  matrix->drawPixel(basex, basey, color);

  if (FACTX > 10 && FACTY > 10) {
      matrix->drawPixel(basex+1, basey,   color);
      matrix->drawPixel(basex,   basey+1, color);
      matrix->drawPixel(basex+1, basey+1, color);
  } else if (FACTY > 10) {
      matrix->drawPixel(basex,   basey+1, color);
  } else if (FACTX > 10) {
      matrix->drawPixel(basex+1, basey,   color);
  }
#else
  backgroundLayer.drawPixel(x, y, {red, green, blue});
#endif
}

// Setup method runs once, when the sketch starts
void sav_setup() {
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);

// when including in other code, allow disabling all those inits
// to avoid double inits.
#ifndef GIFANIM_INCLUDE
    #ifdef NEOMATRIX
	matrix_setup();
    #else
	// neomatrix_config takes care of starting Serial, but we don't call it in the SMARTMATRIX code path
	Serial.begin(115200);
	Serial.println("Running on Native SmartMatrix Backend");
	// Initialize matrix
	matrix.addLayer(&backgroundLayer); 
	matrix.setBrightness(defaultBrightness);

	// for large panels, may want to set the refresh rate lower to leave more CPU time to decoding GIFs (needed if GIFs are playing back slowly)
	//matrix.setRefreshRate(90);
	#if defined(ESP32)
	    // for large panels on ESP32, may want to set the max percentage time dedicated to updating the refresh frames lower, to leave more CPU time to decoding GIFs (needed if GIFs are playing back slowly)
	    //matrix.setMaxCalculationCpuPercentage(50);

	    // alternatively, for large panels on ESP32, may want to set the calculation refresh rate divider lower to leave more CPU time to decoding GIFs (needed if GIFs are playing back slowly) - this has the same effect as matrix.setMaxCalculationCpuPercentage() but is set with a different parameter
	    //matrix.setCalcRefreshRateDivider(4);
	    #if defined(SPI_FFS)
		matrix.begin();
	    #else
		// The ESP32 SD Card library is going to want to malloc about 28000 bytes of DMA-capable RAM
		// make sure at least that much is left free
		matrix.begin(28000);
	    #endif
	#else // ESP32
	    matrix.begin();
	#endif // ESP32
    #endif // NEOMATRIX
#endif // GIFANIM_INCLUDE

#ifdef ARDUINOONPC
#elif !defined(FSOSD)
    // SPIFFS Begin (can crash/conflict with IRRemote on ESP32)
    #ifdef FSOFAT
        // Limit Fat support to a single concurrent file to save RAM
        // 37248 KB are saved by using limiting to 2 file instead of 10
        #ifdef GIFANIM_INCLUDE
	    // 1 file for reading files, and one file for the web server
	    if (!FFat.begin(0, "", 2)) die("Fat FS mount failed. Not enough RAM?");
        #else
        // You might need 2 or 3.
	    if (!FFat.begin(0, "", 2)) die("Fat FS mount failed. Not enough RAM?");
        #endif
	Serial.println("FatFS Directory listing:");
    #else
        if (!FSO.begin()) die("FS mount failed");
	Serial.println("Directory listing:");
    #endif

    // ESP8266 SPIFFS uses special directory objects
    #ifdef ESP8266
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
	    String fileName = dir.fileName();
	    size_t fileSize = dir.fileSize();
	    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
	}
    #else
	File dir;
        #if defined(FSOFAT)
            Serial.printf("Total space: %10u\n", FFat.totalBytes());
            Serial.printf("Free space:  %10u\n", FFat.freeBytes());
            dir = FFat.open(GIF_DIRECTORY);
            if (!dir) die("Can't open  " GIF_DIRECTORY);
            if (!dir.isDirectory()) die( GIF_DIRECTORY ": not a directory");
	    while (File file = dir.openNextFile()) {
		Serial.print("FS File: ");
		Serial.print(file.name());
		Serial.print(" Size: ");
		Serial.println(file.size());
		file.close();
	    }
	    dir.close();
        #endif
	dir = FSO.open("/");
	while (File file = dir.openNextFile()) {
	    Serial.print("FS File: ");
	    Serial.print(file.name());
	    Serial.print(" Size: ");
	    Serial.println(file.size());
	    file.close();
	}
	dir.close();
    #endif // ESP8266
    Serial.println();
#else
    if(initSdCard(SD_CS) < 0) {
	#if ENABLE_SCROLLING == 1
	    scrollingLayer.start("No SD card", -1);
	#endif
	die("No SD card");
    }
#endif
    Serial.print("GifAnim Viewer enabled, lzwMaxBits: ");
    Serial.print(lzwMaxBits);
    Serial.print(", gif_size: ");
    Serial.println(gif_size);
}

bool sav_newgif(const char *pathname) {
    Serial.print(pathname);

#ifdef ARDUINOONPC
    if (file) fclose(file);
    file = fopen(pathname, "r");
#else
    if (file) {
	Serial.print(" (closing previous) ");
	file.close();
    }
    Serial.print(" FSO open ");
    file = FSO.open(pathname 
    // FIXME: should this say ESP8266 instead?
    #ifdef FSOSPIFFS
			     , "r"
    #endif
				   );
#endif
    if (!file) {
        Serial.println(": Error opening GIF file");
#ifdef ARDUINOONPC
        Serial.println(strerror(errno));
#endif
	return 1;
    }
    Serial.print(": Opened GIF file, start decoding (OFFSETX: ");
    Serial.print(OFFSETX);
    Serial.print(", OFFSETY: ");
    Serial.print(OFFSETY);
    Serial.print(", FACTX: ");
    Serial.print(FACTX);
    Serial.print(", FACTY: ");
    Serial.print(FACTY);
    Serial.println(")");
    decoder.startDecoding();
    return 0;
}

bool sav_loop() {
    // ERROR_WAITING means it wasn't time to display the next frame and the display did
    // not get updated (this is important for a neopixel matrix where the display being
    // updated causes a pause in the code).
    if (decoder.decodeFrame() == ERROR_WAITING) return 1;
    return 0;
}

// vim:sts=4:sw=4
