#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cfloat>

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
  MyFrame(const wxString &title, string imagePath, double scaleValue, int quantizationBits, int pivot);

private:
  void OnPaint(wxPaintEvent &event);
  wxImage inImage;
  wxScrolledWindow *scrolledWindow;
  int width;
  int height;
};

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height);

/* Utility function to scale image by 3*3 filter
  Pass by refrerence to char pointer, width and height */
void scaleImage(unsigned char *&inData, int &width, int &height, double scaleValue);

/* Utility function to quantize image */
vector<double> quantizeImage(unsigned char *&inData, int width, int height, int quantizationBits, int pivot);

/* Utility function to search best pivot value bass on MSE
  Suppose the error function is unimodal !!! */
vector<double> ternarySearchPivot(unsigned char *&inData, int width, int height, int quantizationBits, int &pivot);

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
  if (wxApp::argc < 4 || wxApp::argc > 5)
  {
    cerr << "The executable should be invoked with 4 or 5 arguments." << endl;
    cerr << "Usage1: ./MyImageApplication imagePath scaleValue quantizationBits" << endl;
    cerr << "Usage2: ./MyImageApplication imagePath scaleValue quantizationBits pivotValue" << endl;
    exit(1);
  }
  cout << "Image Path: " << wxApp::argv[1] << endl;

  double scaleValue;
  if (wxApp::argv[2].ToDouble(&scaleValue) == false || scaleValue <= 0 || scaleValue > 1)
  {
    cerr << "The scale value should be a double between 0 and 1" << endl;
    exit(1);
  }
  cout << "Scale value: " << wxApp::argv[2] << endl;

  int quantizationBits;
  if (wxApp::argv[3].ToInt(&quantizationBits) == false || quantizationBits <= 0 || quantizationBits > 8)
  {
    cerr << "The quantization bits should be a positive integer between 1 to 8" << endl;
    exit(1);
  }
  cout << "Quantization bits: " << wxApp::argv[3] << endl;

  int pivot = -2;
  if (wxApp::argc == 5)
  {
    if (wxApp::argv[4].ToInt(&pivot) == false || pivot < -1 || pivot > 255)
    {
      cerr << "The mode for quantization should be an integer between -1 to 255, where -1 means uniform quantization." << endl;
      exit(1);
    }
    cout << "Pivot for quantization: " << wxApp::argv[4] << endl;
  }

  string imagePath = wxApp::argv[1].ToStdString();
  MyFrame *frame = new MyFrame("My Image", imagePath, scaleValue, quantizationBits, pivot);

  frame->Show(true);

  // return true to continue, false to exit the application
  return true;
}

/**
 * Constructor for the MyFrame class.
 * Here we read the pixel data from the file, and do all the scaling and quantization.
 */
MyFrame::MyFrame(const wxString &title, string imagePath, double scaleValue, int quantizationBits, int pivot) : wxFrame(NULL, wxID_ANY, title)
{

  // Modify the height and width values here to read and display an image with
  // different dimensions.
  width = 512;
  height = 512;

  unsigned char *inData = readImageData(imagePath, width, height);

  scaleImage(inData, width, height, scaleValue);

  vector<double> errs;
  if (pivot == -2)
  {
    errs = ternarySearchPivot(inData, width, height, quantizationBits, pivot);
  }
  else
  {
    errs = quantizeImage(inData, width, height, quantizationBits, pivot);
  }
  if (pivot == -2)
  {
    cout << "Because of 8 bits quantization, we keep the original data so pivot value is not used." << endl;
  }
  else
  {
    cout << "MSE: " << errs[0] << ", MAE: " << errs[1] << ", PSNR: " << errs[2] << endl;
  }

  /* The last argument is static_data, if it is false, after this call the pointer to the data is owned by the wxImage object,
    which will be responsible for deleting it. So this means that you should not delete the data yourself. */
  inImage.SetData(inData, width, height, false);

  // Set up the scrolled window as a child of this frame
  scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
  scrolledWindow->SetScrollbars(10, 10, width, height);
  scrolledWindow->SetVirtualSize(width, height);

  // Bind the paint event to the OnPaint function of the scrolled window
  scrolledWindow->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);

  // Set the frame size
  SetClientSize(width, height);

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

  wxBitmap inImageBitmap = wxBitmap(inImage);
  dc.DrawBitmap(inImageBitmap, 0, 0, false);
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
  inputFile.read(Gbuf.data(), width * height);
  inputFile.read(Bbuf.data(), width * height);

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

/*
 * This function converts the pixel values of the image to the desired scale value.
 * The scale value is a double between 0 and 1.
 * inData, width and height are passed by reference.
 */
void scaleImage(unsigned char *&inData, int &width, int &height, double scaleValue)
{
  if (scaleValue == 1)
  {
    return;
  }
  int newWidth = static_cast<int>(width * scaleValue);
  int newHeight = static_cast<int>(height * scaleValue);
  unsigned char *newData = (unsigned char *)malloc(newWidth * newHeight * 3 * sizeof(unsigned char));

  // 3 * 3 averaging filter with nearest neighbor interpolation
  for (int y = 0; y < newHeight; y++)
  {
    for (int x = 0; x < newWidth; x++)
    {
      int srcX = min(static_cast<int>(round(x / scaleValue)), width - 1); // static_cast performs compile-time type checking
      int srcY = min(static_cast<int>(round(y / scaleValue)), height - 1);
      int sumR = 0, sumG = 0, sumB = 0;
      int cnt = 0;
      for (int dy = -1; dy <= 1; dy++)
      {
        for (int dx = -1; dx <= 1; dx++)
        {
          int nx = srcX + dx;
          int ny = srcY + dy;
          if (nx >= 0 && nx < width && ny >= 0 && ny < height)
          {
            sumR += inData[3 * (ny * width + nx)]; // inData[...] is promoted to int.
            sumG += inData[3 * (ny * width + nx) + 1];
            sumB += inData[3 * (ny * width + nx) + 2];
            cnt++;
          }
        }
      }
      newData[3 * (y * newWidth + x)] = static_cast<unsigned char>(sumR / cnt);
      newData[3 * (y * newWidth + x) + 1] = static_cast<unsigned char>(sumG / cnt);
      newData[3 * (y * newWidth + x) + 2] = static_cast<unsigned char>(sumB / cnt);
    }
  }
  free(inData);
  // Update the width, height and data of the image
  inData = newData;
  width = newWidth;
  height = newHeight;
}

vector<double> quantizeImage(unsigned char *&inData, int width, int height, int quantizationBits, int pivot)
{
  if (quantizationBits == 8)
  {
    cout << "No need for quantization with 8 bits, otherwise might introduce more error!" << endl;
    return {0, 0, 100};
  }
  /* Mean squared error, Mean absolute error, Peak Signal-to-Noise Ratio */
  double mse = 0, mae = 0, psnr = 0;
  int quantizationLevels = 1 << quantizationBits;
  vector<int> levelBoundaries(quantizationLevels + 1);

  /* Ensure each half [0, pivot], [pivot, 255] has at least 2 levels */
  int levelsLow = max(static_cast<int>(quantizationLevels * pivot / 255.0), 2);
  levelsLow = min(levelsLow, quantizationLevels - 2);
  int levelsHigh = quantizationLevels - levelsLow;
  for (int i = 0; i <= quantizationLevels; i++)
  {
    if (pivot == -1) // Uniform quantization
    {
      levelBoundaries[i] = static_cast<int>(255 * i / quantizationLevels);
    }
    else if (pivot == 0) // Dark-end logarithmic quantization
    {
      levelBoundaries[i] = static_cast<int>(255 * (pow(2, i) - 1) / (pow(2, quantizationLevels) - 1));
    }
    else if (pivot == 255) // Bright-end logarithmic quantization
    {
      levelBoundaries[i] = static_cast<int>(255 - (255 * (pow(2, quantizationLevels - i) - 1) / (pow(2, quantizationLevels) - 1)));
    }
    else // Proportional logarithmic quantization with pivot
    {
      if (i <= levelsLow)
      {
        levelBoundaries[i] = static_cast<int>(pivot - pivot * (pow(2, levelsLow - i) - 1) / (pow(2, levelsLow) - 1));
      }
      else
      {
        levelBoundaries[i] = static_cast<int>(pivot + (255 - pivot) * (pow(2, i - levelsLow) - 1) / (pow(2, levelsHigh) - 1));
      }
    }
  }
  // Apply quantization
  for (int i = 0; i < width * height * 3; i++)
  {
    auto it = lower_bound(levelBoundaries.begin(), levelBoundaries.end(), inData[i]);
    int old = inData[i];
    if (it == levelBoundaries.begin())
    {
      inData[i] = (*(it + 1) + *it) / 2;
    }
    else
    {
      inData[i] = (*it + *(it - 1)) / 2;
    }
    mse += pow(inData[i] - old, 2);
    mae += abs(inData[i] - old);
  }
  mse /= width * height * 3;
  mae /= width * height * 3;
  psnr = (mse > 0) ? 10 * log10(255 * 255 / mse) : 100;
  return {mse, mae, psnr};
}

vector<double> ternarySearchPivot(unsigned char *&inData, int width, int height, int quantizationBits, int &pivot)
{
  if (quantizationBits == 8)
  {
    cout << "No need for quantization with 8 bits, otherwise might introduce more error!" << endl;
    return {0, 0, 100};
  }
  int left = 0, right = 255;
  while (right - left > 2)
  {
    int m1 = left + (right - left) / 3;
    int m2 = right - (right - left) / 3;
    unsigned char *cp1 = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
    unsigned char *cp2 = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
    memcpy(cp1, inData, width * height * 3 * sizeof(unsigned char));
    memcpy(cp2, inData, width * height * 3 * sizeof(unsigned char));

    double mse1 = quantizeImage(cp1, width, height, quantizationBits, m1)[0];
    double mse2 = quantizeImage(cp2, width, height, quantizationBits, m2)[0];
    free(cp1);
    free(cp2);

    if (mse1 < mse2)
    {
      right = m2;
    }
    else
    {
      left = m1;
    }
  }

  /* Assume the current middle point is the best.
    There might be minor error difference between left, left+1 and right, that should be acceptable. */
  pivot = (left + right) / 2;
  vector<double> errs = quantizeImage(inData, width, height, quantizationBits, pivot);
  cout << "Best pivot: " << pivot << endl;
  return errs;
}

wxIMPLEMENT_APP(MyApp);