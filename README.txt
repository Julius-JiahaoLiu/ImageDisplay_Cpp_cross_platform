I used the cross-platform C++ imageDisplay starter project, and implemented and tested on an Apple M1 Silicon mac, using Mac OS 15.3 (24D60) with clang version 19.1.7 compiler!

My submission contains three files:

	1. ImageDisplay_C++_cross_platform/src/Main.cpp: source code, this is the entrance of MyImageApplication.
	2. ImageDisplay_C++_cross_platform/dependency/wxWidgets/src/osx/carbon/dcscreen.cpp: the provided file in starter code is outdated, incuring errors in CMkae Build.
		I modified provided file base on this newer version (https://github.com/wxWidgets/wxWidgets/blob/7e32de0e7eb8df37f3cf3c78d4f736fc7111f899/src/osx/carbon/dcscreen.cpp).
	3. ImageDisplay_C++_cross_platform/README.txt: this instruction file.

IMPORTANT: 

	I did implement extra credit part with usage1: ./MyImageApplication imagePath scaleValue quantizationBits
	Of course, my implementation support usage2: ./MyImageApplication imagePath scaleValue quantizationBits pivotValue

Detailed Explanation:

	MyImageApplication would check the input parameters are legal or not, i.e. the scaleValue in (0, 1.0], quantizationBits in (0, 8], pivotValue in [-1, 255].

	First scale the image with 3 * 3 filter, I divide the target pixel coordinate by the scaleValue to get the corresponding source coordinate, 
		then fetch and average the 3 * 3 block of the source pixel to get the target pixel value, R G B respectively.

	Then quantize with given pivot value, I first calculate the boundaries of levels for uniform or logarithmic quantization.
		Then given channel values, use binary search to quickly locate its levels and corresponding quantized value. Calculate the quantization error along the way.
		Obviously, time complexity of above process would be O[(log2^quantizationBits) * pixels of image]

	Finally for optimal pivot searching in extra credit, I have implemented following two sultions: 
		1. suppose there is an unimodal distribution of MSE, i.e. decrease and increase among differnt pivot in [0, 255]
			I directly use ternary search for the unique optimal pivot value among all pixel with RGB values.
			The time complexity of this approach would be O[log256 * (log2^quantizationBits) * pixels of image]
		2.  Since human vision is more sensitive to brightness (luminance) than color,
			I first convert each pixel to grayscale using the luminance formula: I = 0.299R + 0.587G + 0.114B
			Then, create a 256-bin array (histogram[256]) where each bin counts the number of pixels with that intensity.
			Finally, use Otsu’s method to minimize the intra-class variance between two groups: [0, pivot] and [pivot, 255].
			The time complexity of this approach would be O(256) + O[(log2^quantizationBits) * pixels of image].
			Even with faster speed, this solution only works well on images with distinct dark and bright regions, e.g. lake-forest_512_512.rgb.
	You can switch between above two solutions by comment out line Main.cpp:153 and uncomment line Main.cpp:160~161.

------------------------------------------------------------------------------------------------------------------------------------------------------

The following is the original guidance on build and run from the start code:

1. Install GCC Complier
	
	Windows: You can install mingw-w64 from https://sourceforge.net/projects/mingw-w64/
	Mac: You can install using Homebrew. You may already have clang installed (which is invoked
		when gcc command is run). This should be fine. You can verify this using `gcc --version`.
	Linux: You can install gcc using apt-get (or apt)

	Verify GCC installation by running `gcc --version`. GCC version 12.2 or higher is recommended.
	On Mac, clang version 15.0.0 or higher is recommended
	You may need to add gcc to the PATH environment variable.

2. Install CMake

	Windows: You can install cmake from https://cmake.org/download/
	Mac: You can install using Homebrew
	Linux: You can install using apt-get (or apt)

	Verify CMake installation by running `cmake --version`. CMake version 3.28.1 or higher is recommended.
	You may need to add cmake to the PATH environment variable.

3. Open the project folder in vscode.

4. Install and activate the following vscode extensions from the extensions marketplace.
	https://marketplace.visualstudio.com/

	a. C / C++ for VS Code - https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools
	b. CMake for VS Code - https://marketplace.visualstudio.com/items?itemName=twxs.cmake
	c. CMake Tools - https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools

	Ensure that all these extensions are active. Reload or restart vscode if needed.

5. Configure CMake

	Note: All vscode commands are run from the vscode command palette. To activate the Command Palette,
	Windows / Linux: Ctrl+Shift+P,
 	Mac:  Cmd+Shift+P

	Command Palette > Type in "CMake: Configure" and execute it.
	
	1. Find your compiler installation (gcc or clang) in the options that appear.
	   You may use the [scan for kits] option to scan the device for compiler installations.

	2. If prompted, Choose the correct location for CMakeLists
	   file - ${workspaceFolder}/ImageDisplay_C++_cross_platform/CMakeLists.txt

	At this point, the Configure command should execute and you may see some output in vscode output window.

6. Set up Configuration Provider

	Command Palette > Type in "C/C++: Change Configuration Provider" and run it.
	'CMake Tools' should be an option listed if the previous configure step was successful. Choose that.

	Command Palette > Type in "C/C++: Edit Configurations (UI)" and run it.
	This opens up a UI where you can edit the configurations. 
	
	1. Change compiler path and intellisense mode accordingly.
	2. Change C standard and C++ standard to c17 and c++17 respectively.

7. Build

	Command Palette > Type in "CMake: Set build target" and run it.
	Choose the target you want to build. You may choose MyImageApplication to create an executable
	with that name.

	Command Palette > Type in "CMake: Delete cache and reconfigure" and run it.
	This just cleans up the repository to prepare for a clean rebuild.

	Command Palette > Type in "CMake: Build" and run it.
	
	- Initial build might take some time, because the dependencies need to be built first,
	but later builds should be faster.
	- Rarely, you may want to run "CMake: Clean rebuild", but this would
	rebuild the whole project, including the dependencies, which may take a long time.


8. Run

	- In the /build folder, you will find the executable (MyImageApplication).
	- To run this, navigate to the build folder in a terminal and run the executable file.
	- The given starter code takes exactly one argument - a file path to a 512x512 rgb image file
	- This should be invoked as ./MyImageApplication '<path to rgb file>'
	- Example - ./MyImageApplication '../../Lena_512_512.rgb'

9. Rebuilds

	After making changes to the source code, you need to build the executable again.
	Command Palette > Type in "CMake: Build" and run it.

	On restarting vscode, you may need to configure CMake by running "CMake: Configure"
	After that, continue building using "CMake: Build".