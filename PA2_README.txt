I used the cross-platform C++ imageDisplay starter project, and implemented and tested on an Apple M1 Silicon mac, using Mac OS 15.3 (24D60) with clang version 19.1.7 compiler!

My submission contains three files:

	1. ImageDisplay_C++_cross_platform/src/PA2_Main.cpp: source code, this is the entrance of MyCompression program.
	2. ImageDisplay_C++_cross_platform/dependency/wxWidgets/src/osx/carbon/dcscreen.cpp: the provided file in starter code is outdated, incuring errors in CMkae Build.
		I modified provided file base on this newer version (https://github.com/wxWidgets/wxWidgets/blob/7e32de0e7eb8df37f3cf3c78d4f736fc7111f899/src/osx/carbon/dcscreen.cpp).
	3. ImageDisplay_C++_cross_platform/CMakeLists.txt: the CMake file I used to compile the program.
   4. ImageDisplay_C++_cross_platform/PA2_README.txt: this instruction file.

IMPORTANT: 

	I did finish the extending implementation for M = perfect square. 
	My implementation support usage: ./MyCompression imagePath vectorSize vectorNumber
   The imagePath support both ".raw" and ".rgb" file. For ".raw" file, I copied the one channel pixel value for the other two channels to display grayscale image via wxWidgets.
   The vectorSize should be either 2 or a perfect square integer.
   The vector number should be an integer of power of 2.

Detailed Explanation:

	MyCompression would check the input parameters are legal or not before doing vector quantization.

	(1) read the image data with unsigned char *inData = readImageData(imagePath, width = 352, height = 288);
      where the imagePath ending with ".raw" was converted to RGBRGB as well with same pixel value for three channels.

	(2) vector<VectorData> vectorSpace = buildVectorSpaceAndFrequency(inData, width = 352, height = 288, blockWidth, blockHeight, channelCnt, vectorFreq);
      where width is 352 and height is 288 by default.
      blockWidth and blockHeight is base on input vectorSize M. i.e. [2, 1] for M = 2, and [sqrt(M), sqrt(M)] for perfect square number M > 2.
      channelCnt is 1 for ".raw" file and 3 for ".rgb" file.
      this returns the vector space for all blocks with each vector element contains 3 or 1 channel data, i.e. the defined VectorData is a vector of M vector<double>, each of size channelCnt;
      Also returns an unordered_map for frequency count of all vectors in the space;

   (3) vector<VectorData> centroids = centroidsInitializationWithFrequency(vectorFreq, vectorNumber);
      this use the unordered_map build above to initialize the centroids by choosing K VectorData with highest frequency.

   (4) vector<int> assignments = kMeansClustering(vectorSpace, centroids, 200);
      this use meanSquareDistance to find the nearest centroid for each vector point and record this assigned centroid.
      Update the centroids for each clusters.
      Continue above two steps until there is no change to the assigned centroid for each vector and the meanSquareDistance change of centroids is less than convergenceThreshold = 1e-9.
      (The program would output the current centroids pixel value whenever the iteration % 50 == 0 and the final centroids after termination.)

	(5) vectorQuantization(compressedData, vectorSpace.size(), assignments, centroids, channelCnt, blocksPerRow, blockWidth, blockHeight, width, height);
      It would map and update the pixel value for each block of size M into the pixel value of their assigned centroid.
      Display the original inData and compressedData side by side in MyFrame initialization. 

   PS: I would record the time consumed for (1) ~ (3) and (4) ~ (5) process, and output them in following format:
      Time taken for Reading Image Data, Building Vector Space and Initializing Centroids:: XXX milliseconds
      Time taken for K-Means Clustering and Vector Quantization: XXX milliseconds
