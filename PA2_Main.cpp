#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cfloat>
#include <numeric>
#include <chrono> // Include the chrono library for timing measurements
#include <random>

using namespace std;
namespace fs = std::filesystem;

/**
 * Display an image using WxWidgets.
 * https://www.wxwidgets.org/
 */

/** Declarations*/

/**
 * Class that implements wxApp
 */
class MyApp : public wxApp
{
public:
  bool OnInit() override;
};

/**
 * Class that implements wxFrame.
 * This frame serves as the top level window for the program
 */
class MyFrame : public wxFrame
{
public:
  MyFrame(const wxString &title, string imagePath, int vectorSize, int vectorNumber);

private:
  void OnPaint(wxPaintEvent &event);
  wxImage originalImage;
  wxImage compressedImage;
  wxScrolledWindow *scrolledWindow;
  int width;
  int height;
};

// Structure to hold a vector of integers
struct VectorData
{
  vector<vector<double>> vec; // M-dimensional vector, with inner vecter corresponding to each channel for one pixel
  // Define equality comparison for unordered_map
  bool operator==(const VectorData &other) const
  {
    return vec == other.vec;
  }
};

// Custom hash function for unordered_map
struct VectorDataHash
{
  size_t operator()(const VectorData &v) const
  {
    size_t hashVal = 0;
    for (const auto &row : v.vec)
    {
      for (double val : row)
      {
        hashVal ^= hash<double>()(val) + 0x9e3779b9 + (hashVal << 6) + (hashVal >> 2);
      }
    }
    return hashVal;
  }
};

// Custom comparator for the min-heap
struct Compare
{
  bool operator()(const pair<int, VectorData> &a, const pair<int, VectorData> &b)
  {
    return a.first > b.first; // Min-heap based on frequency
  }
};

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height);

/* Build the sample vector space with vector size blockWidth * blockHeight for each channel */
vector<VectorData> buildVectorSpaceAndFrequency(unsigned char *&inData, int width, int height, int blockWidth, int blockHeight, int chanelCnt, unordered_map<VectorData, int, VectorDataHash> &vectorFreq);

vector<VectorData> centroidsInitializationRandom(int K, int M, int channelCnt);

vector<VectorData> centroidsInitializationWithFrequency(const unordered_map<VectorData, int, VectorDataHash> &vectorFreq, int K);

/* K-Means Clustering for one channel */
vector<int> kMeansClustering(const vector<VectorData> &vectors, vector<VectorData> &centroids, int maxIterations);

/* Map to original vector to the assigned centroid */
void vectorQuantization(unsigned char *&compressedData, int vectorNumber, vector<int> &assignments, vector<VectorData> &centroids, int channelCnt, int blocksPerRow, int blockWidth, int blockHeight, int width, int height);

/** Definitions */
/**
 * Init method for the app.
 * Here we process the command line arguments and
 * instantiate the frame.
 */
bool MyApp::OnInit()
{
  wxInitAllImageHandlers();

  // deal with command line arguments here
  cout << "Number of command line arguments: " << wxApp::argc << endl;
  if (wxApp::argc != 4)
  {
    cerr << "The executable should be invoked with 4 arguments." << endl;
    cerr << "Usage: ./MyCompression imagePath vectorSize vectorNumber" << endl;
    exit(1);
  }
  string imagePath = wxApp::argv[1].ToStdString();
  if (!fs::exists(imagePath))
  {
    cerr << "The image path does not exist." << endl;
    exit(1);
  }
  cout << "Image Path: " << imagePath << endl;

  int vectorSize;
  if (wxApp::argv[2].ToInt(&vectorSize) == false || !(vectorSize == 2 || ceil((double)sqrt(vectorSize)) == floor((double)sqrt(vectorSize))))
  {
    cerr << "The vectorSize should be either 2 or a perfect square integer." << endl;
    exit(1);
  }
  cout << "Vector Size: " << vectorSize << endl;

  int vectorNumber;
  if (wxApp::argv[3].ToInt(&vectorNumber) == false || vectorNumber <= 1 || (vectorNumber & (vectorNumber - 1)) != 0)
  {
    cerr << "The vector number should be an integer of power of 2." << endl;
    exit(1);
  }
  cout << "Vector Number: " << vectorNumber << endl;

  MyFrame *frame = new MyFrame("My Image", imagePath, vectorSize, vectorNumber);

  frame->Show(true);

  // return true to continue, false to exit the application
  return true;
}

/**
 * Constructor for the MyFrame class.
 * Here we read the pixel data from the file, and do all the scaling and quantization.
 */
MyFrame::MyFrame(const wxString &title, string imagePath, int vectorSize, int vectorNumber) : wxFrame(NULL, wxID_ANY, title)
{

  // Modify the height and width values here to read and display an image with
  // different dimensions.
  width = 352;
  height = 288;
  int channelCnt = (imagePath.find(".rgb") != string::npos) ? 3 : 1; // RGB or grayscale image

  auto start = std::chrono::high_resolution_clock::now(); // Start timing measurements
  unsigned char *inData = readImageData(imagePath, width, height);
  /* The fourth argument is static_data.
  If it is false, after this call the pointer to the data is owned by the wxImage object, which will be responsible for deleting it.
  If true, m_data is pointer to static data and shouldn't be freed by wxImage. */
  originalImage.SetData(inData, width, height, false);
  int blockWidth = 2, blockHeight = 1;
  if (vectorSize > 2)
  {
    blockWidth = blockHeight = sqrt(vectorSize);
  }
  unordered_map<VectorData, int, VectorDataHash> vectorFreq;
  vector<VectorData> vectorSpace = buildVectorSpaceAndFrequency(inData, width, height, blockWidth, blockHeight, channelCnt, vectorFreq);
  vector<VectorData> centroids = centroidsInitializationWithFrequency(vectorFreq, vectorNumber);

  auto end = std::chrono::high_resolution_clock::now(); // End timing
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  cout << "Time taken for Reading Image Data, Building Vector Space and Initializing Centroids: " << duration << " milliseconds" << endl;
  cout << endl;

  // Perform K-means clustering for all channels
  start = std::chrono::high_resolution_clock::now();
  // Create a copy of inData for later modifications
  unsigned char *compressedData = new unsigned char[width * height * 3]; // Assuming RGB image
  std::memcpy(compressedData, inData, width * height * 3);
  int blocksPerRow = ceil(width / (float)blockWidth); // should use ceil to handle edge cases

  vector<int> assignments = kMeansClustering(vectorSpace, centroids, 200);
  vectorQuantization(compressedData, vectorSpace.size(), assignments, centroids, channelCnt, blocksPerRow, blockWidth, blockHeight, width, height);

  if (channelCnt == 1)
  {
    // If the image is grayscale, copy the pixel values to the other channels
    for (int i = 0; i < width * height; i++)
    {
      compressedData[3 * i + 1] = compressedData[3 * i];
      compressedData[3 * i + 2] = compressedData[3 * i];
    }
  }
  end = std::chrono::high_resolution_clock::now(); // End timing
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  cout << "Time taken for K-Means Clustering and Vector Quantization: " << duration << " milliseconds" << endl;
  cout << endl;

  compressedImage.SetData(compressedData, width, height, false);
  // Set up the scrolled window as a child of this frame
  scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
  scrolledWindow->SetScrollbars(10, 10, 2 * width, height);
  scrolledWindow->SetVirtualSize(2 * width, height);
  // Bind the paint event to the OnPaint function of the scrolled window
  scrolledWindow->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);
  // Set the frame size
  SetClientSize(2 * width, height);
  // Set the frame background color
  SetBackgroundColour(*wxBLACK);
}

/**
 * The OnPaint handler that paints the UI.
 * Here we paint the image pixels into the scrollable window.
 */
void MyFrame::OnPaint(wxPaintEvent &event)
{
  wxBufferedPaintDC dc(scrolledWindow);
  scrolledWindow->DoPrepareDC(dc);

  wxBitmap inImageBitmap = wxBitmap(originalImage);
  wxBitmap outImageBitmap = wxBitmap(compressedImage);
  dc.DrawBitmap(inImageBitmap, 0, 0, false);
  dc.DrawBitmap(outImageBitmap, width, 0, false);
}

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height)
{

  // Open the file in binary mode
  ifstream inputFile(imagePath, ios::binary);

  if (!inputFile.is_open())
  {
    cerr << "Error Opening File for Reading" << endl;
    exit(1);
  }

  // Create and populate RGB buffers
  vector<char> Rbuf(width * height);
  vector<char> Gbuf(width * height);
  vector<char> Bbuf(width * height);

  /**
   * The input RGB file is formatted as RRRR.....GGGG....BBBB.
   * i.e the R values of all the pixels followed by the G values
   * of all the pixels followed by the B values of all pixels.
   * Hence we read the data in that order.
   */

  inputFile.read(Rbuf.data(), width * height);
  if (imagePath.find(".rgb") != string::npos)
  { // If the file is in RGB format
    inputFile.read(Gbuf.data(), width * height);
    inputFile.read(Bbuf.data(), width * height);
  }
  else
  { // If the file is in .raw format with single channel
    Gbuf = Rbuf;
    Bbuf = Rbuf;
  }

  inputFile.close();

  /**
   * Allocate a buffer to store the pixel values.
   * Char values in the range [-128, 127], while unsigned char values in the range [0, 255] (8-bit)
   * The data must be allocated with malloc(), NOT with operator new. wxWidgets library requires this.
   */
  unsigned char *inData = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));

  for (int i = 0; i < height * width; i++)
  {
    // We populate RGB values of each pixel in that order RGB.RGB.RGB and so on for all pixels
    inData[3 * i] = Rbuf[i];
    inData[3 * i + 1] = Gbuf[i];
    inData[3 * i + 2] = Bbuf[i];
  }

  return inData;
}

// Function to extract blocks and build M-dimensional vectors for all channel
vector<VectorData> buildVectorSpaceAndFrequency(unsigned char *&inData, int width, int height, int blockWidth, int blockHeight, int channelCnt, unordered_map<VectorData, int, VectorDataHash> &vectorFreq)
{
  vector<VectorData> vectorSpace;
  int blocksPerRow = ceil(width / (float)blockWidth);

  for (int i = 0; i < height; i += blockHeight)
  {
    for (int j = 0; j < width; j += blockWidth)
    {
      vector<vector<double>> vec;
      vector<double> previousValue = vector<double>(channelCnt);
      // i, j is the top-left corner of each block
      for (int k = 0; k < channelCnt; k++)
      {
        previousValue[k] = inData[3 * (i * width + j) + k];
      }
      for (int y = i; y < i + blockHeight; y++)
      {
        for (int x = j; x < j + blockWidth; x++)
        {
          if (x < width && y < height)
          { // Handle edge cases when blockWidth does not divide width or height
            for (int k = 0; k < channelCnt; k++)
            {
              previousValue[k] = inData[3 * (y * width + x) + k];
            }
          }
          vec.push_back(previousValue);
        }
      }
      VectorData vectorData = {vec}; // // Store the vectorData for each pixel block in the image with RGB sequence
      vectorSpace.push_back(vectorData);
      vectorFreq[vectorData]++; // Count occurrences
    }
  }
  return vectorSpace;
}

// Compute Euclidean distance between two vectors of same size
double meanSquareDistance(const VectorData &v1, const VectorData &v2)
{
  if (v1.vec.size() != v2.vec.size())
  {
    cerr << "Vector dimensions mismatch" << endl;
    exit(1);
  }
  double sum = 0.0;
  for (int i = 0; i < v1.vec.size(); i++)
  {
    for (int k = 0; k < v1.vec[i].size(); k++)
    {
      double diff = v1.vec[i][k] - v2.vec[i][k];
      sum += diff * diff;
    }
  }
  return sum / (v1.vec.size() * v1.vec[0].size());
}

// Centroid Update: Compute the mean of a set of vectors
VectorData computeCentroid(const vector<VectorData> &points, const vector<int> &indices)
{
  int M = points[0].vec.size(); // Dimension of vectors
  int channelCnt = points[0].vec[0].size();
  VectorData centroid = {vector<vector<double>>(M, vector<double>(channelCnt, 0.0))};

  // Sum all vectors in the cluster
  for (int idx : indices)
  {
    for (int j = 0; j < M; j++)
    {
      for (int k = 0; k < channelCnt; k++)
      {
        centroid.vec[j][k] += points[idx].vec[j][k];
      }
    }
  }
  // Divide by number of points to get mean
  for (int j = 0; j < M; j++)
  {
    for (int k = 0; k < channelCnt; k++)
    {
      centroid.vec[j][k] /= indices.size();
    }
  }
  return centroid;
}

vector<VectorData> centroidsInitializationRandom(int K, int M, int channelCnt)
{
  /* True uniform initialization: Each centroid is uniformly sampled in the [0,255]^M space.
    This method works for any M and does not assume it is a perfect square. */
  vector<VectorData> centroids(K, {vector<vector<double>>(M, vector<double>(channelCnt, 0.0))});

  random_device rd;
  mt19937 gen(rd());                                  // Random number generator
  uniform_real_distribution<double> dist(0.0, 255.0); // Uniform distribution in [0, 255]

  for (int i = 0; i < K; i++)
  {
    for (int m = 0; m < M; m++)
    {
      for (int k = 0; k < channelCnt; k++)
      {
        centroids[i].vec[m][k] = dist(gen); // Random initialization in the range [0, 255]
      }
    }
  }

  return centroids;
}

vector<VectorData> centroidsInitializationWithFrequency(const unordered_map<VectorData, int, VectorDataHash> &vectorFreq, int K)
{
  if (K > static_cast<int>(vectorFreq.size()))
  {
    cerr << "Invalid vector number for K-Means. Number of clusters must be less than or equal to vector categories in image." << endl;
    exit(1);
  }
  // Min-heap to store top K elements
  priority_queue<pair<int, VectorData>, vector<pair<int, VectorData>>, Compare> minHeap;

  for (const auto &entry : vectorFreq)
  {
    minHeap.push({entry.second, entry.first}); // (frequency, VectorData)
    if (minHeap.size() > K)
    {
      minHeap.pop(); // Remove least frequent element if heap exceeds K
    }
  }
  // Extract centroids from heap
  vector<VectorData> centroids;
  while (!minHeap.empty())
  {
    centroids.push_back(minHeap.top().second);
    minHeap.pop();
  }
  return centroids;
}

void printVectorData(const vector<VectorData> &vectors)
{
  int K = vectors.size();
  int M = vectors[0].vec.size();
  int channelCnt = vectors[0].vec[0].size();
  for (int k = 0; k < K; k++)
  {
    cout << "Centroid " << (k + 1) << ": [";
    for (int m = 0; m < M; m++)
    {
      cout << "["; // Start of a vector
      for (int n = 0; n < channelCnt; n++)
      {
        printf("%.3f", vectors[k].vec[m][n]);
        if (n < channelCnt - 1)
        {
          cout << ", ";
        }
      }
      cout << "]"; // End of a vector
      if (m < M - 1)
      {
        cout << ", ";
      }
    }
    cout << "]" << endl;
  }
}

// K-means clustering for one channel
vector<int> kMeansClustering(const vector<VectorData> &vectors, vector<VectorData> &centroids, int maxIterations)
{
  int K = centroids.size();       // Number of clusters
  int N = vectors.size();         // Number of vectors
  vector<int> assignments(N, -1); // Cluster assignment for each vector

  // Iterate until convergence or max iterations
  bool changed = true;
  int iteration = 0;
  double convergenceThreshold = 1e-9;

  cout << "------------------------ Begin K-Means Clustering for " << K << " Centroids ------------------------" << endl;
  while (changed && iteration < maxIterations)
  {
    changed = false;
    iteration++;

    // Assign each vector to the nearest centroid
    for (int i = 0; i < N; i++)
    {
      double minDist = numeric_limits<double>::max();
      int bestCluster = -1;
      for (int k = 0; k < K; k++)
      {
        double dist = meanSquareDistance(vectors[i], centroids[k]);
        if (dist < minDist)
        {
          minDist = dist;
          bestCluster = k;
        }
      }
      if (assignments[i] != bestCluster)
      {
        assignments[i] = bestCluster;
        changed = true;
      }
    }

    if (!changed)
    {
      break; // Skip centroid update if no assignment changes
    }

    // Update centroids and check for convergence
    bool centroidsStable = true;
    vector<vector<int>> clusterPoints(K);
    for (int i = 0; i < N; i++)
    {
      clusterPoints[assignments[i]].push_back(i);
    }
    for (int k = 0; k < K; k++)
    {
      if (!clusterPoints[k].empty())
      {
        VectorData newCentroid = computeCentroid(vectors, clusterPoints[k]);
        if (meanSquareDistance(centroids[k], newCentroid) > convergenceThreshold)
        {
          centroidsStable = false;
        }
        centroids[k] = newCentroid;
      }
    }
    if (centroidsStable)
    {
      break; // skip further iterations if centroids are stable
    }

    if (iteration % 50 == 0)
    {
      cout << ">>> Centroids after " << iteration << " iterations" << endl;
      printVectorData(centroids);
      cout << endl;
    }
  }
  cout << ">>> Final Centroids after " << iteration << " iterations" << endl;
  printVectorData(centroids);
  cout << "------------------------ End K-Means Clustering ------------------------" << endl;
  return assignments; // Return cluster assignments and centroids
}

void vectorQuantization(unsigned char *&compressedData, int vectorNumber, vector<int> &assignments, vector<VectorData> &centroids, int channelCnt, int blocksPerRow, int blockWidth, int blockHeight, int width, int height)
{
  for (int vectorIdx = 0; vectorIdx < vectorNumber; vectorIdx++)
  {
    const VectorData &centroid = centroids[assignments[vectorIdx]];
    int blockIdx = vectorIdx; // Block index in the image
    // Compute the top-left corner position of current block in the image
    int blockY = (blockIdx / blocksPerRow) * blockHeight;
    int blockX = (blockIdx % blocksPerRow) * blockWidth;

    // Replace pixel values in the block with centroid values
    int centroidPixelIdx = 0;
    for (int y = blockY; y < blockY + blockHeight; y++)
    {
      if (y >= height) // Handle edge cases when blockHeight does not divide height
        break;
      for (int x = blockX; x < blockX + blockWidth; x++)
      {
        if (x >= width) // Handle edge cases when blockWidth does not divide width
          break;
        for (int channelIdx = 0; channelIdx < channelCnt; channelIdx++)
        {
          int pixelIdx = 3 * (y * width + x) + channelIdx;
          double centroidValue = centroid.vec[centroidPixelIdx][channelIdx];
          compressedData[pixelIdx] = static_cast<unsigned char>(centroidValue);
        }
        centroidPixelIdx++;
      }
    }
  }
}

wxIMPLEMENT_APP(MyApp);