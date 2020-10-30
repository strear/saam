#include "../imageArray.hpp"

#undef UNICODE
#include <windows.h>
#include <wincodec.h>
#include <commdlg.h>

/* 修改：引用相关头文件，提供符号定义 */
#include "../imageArray.hpp"
/* 修改：结束 */

namespace {
    template <typename T>
    inline void SafeRelease(T*& p) {
        if (nullptr != p) {
            p->Release();
            p = nullptr;
        }
    }

    class ReadImpl {
    public:
        PicReader();
        ~PicReader();

        void readPic(LPCSTR);

        /* void 修改：实现自定义的读取函数 getData() */
        void getData(Array<BYTE>&);
        /* 修改：结束 */

    private:
        void init();
        bool checkHR(HRESULT);
        void quitWithError(LPCSTR);

        HWND                    hWnd;
        HANDLE                  hFile;
        IWICImagingFactory* m_pIWICFactory;
        IWICFormatConverter* m_pConvertedSourceBitmap;

        /* 修改：这里可能会增加你需要的内部成员（未使用） */
    };

    PicReader::PicReader() : hFile(nullptr), m_pIWICFactory(nullptr), m_pConvertedSourceBitmap(nullptr) {
        init();
    }

    PicReader::~PicReader() {
        if (hFile != NULL) CloseHandle(hFile);
        SafeRelease(m_pConvertedSourceBitmap);
        SafeRelease(m_pIWICFactory);
        CoUninitialize();
    }

    bool PicReader::checkHR(HRESULT hr) {
        return (hr < 0);
    }

    void PicReader::quitWithError(LPCSTR message) {
        MessageBoxA(hWnd, message, "Application Error", MB_ICONEXCLAMATION | MB_OK);
        quick_exit(0xffffffff);
    }

    void PicReader::init() {
        hWnd = GetForegroundWindow();

        // Enables the terminate-on-corruption feature.
        HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

        HRESULT hr = S_OK;

        //Init the WIC
        hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        // Create WIC factory
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pIWICFactory)
        );

        // Throw error if create factor failed
        if (checkHR(hr)) { quitWithError("Init Reader Failed"); }
    }

    void PicReader::readPic(LPCSTR fileName) {
        HRESULT hr = S_OK;

        // Create a File Handle (WinAPI method not std c)
        if (hFile != NULL) FindClose(hFile);
        hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            quitWithError("Cannot find such file, please retry or check the access");
        }

        // Create a decoder
        IWICBitmapDecoder* pDecoder = nullptr;
        hr = m_pIWICFactory->CreateDecoderFromFileHandle((ULONG_PTR)hFile, nullptr, WICDecodeMetadataCacheOnDemand, &pDecoder);
        if (checkHR(hr)) { quitWithError("Create Decoder Failed"); }

        // Retrieve the first frame of the image from the decoder
        IWICBitmapFrameDecode* pFrame = nullptr;
        hr = pDecoder->GetFrame(0, &pFrame);
        if (checkHR(hr)) { quitWithError("Get Frame Failed"); }

        // Format convert the frame to 32bppRGBA
        SafeRelease(m_pConvertedSourceBitmap);
        hr = m_pIWICFactory->CreateFormatConverter(&m_pConvertedSourceBitmap);
        if (checkHR(hr)) { quitWithError("Get Format Converter Failed"); }

        hr = m_pConvertedSourceBitmap->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
        if (checkHR(hr)) { quitWithError("Init Bitmap Failed"); }

        // Clean memory
        SafeRelease(pDecoder);
        SafeRelease(pFrame);
    }

    /* 修改：返回自定义类型的读取函数原型 */
    void PicReader::getData(Array<BYTE>& receiver) {
        HRESULT hr = S_OK;

        // Get the size of Image
        UINT x, y;
        hr = m_pConvertedSourceBitmap->GetSize(&x, &y);
        if (checkHR(hr)) { quitWithError("Check Bitmap Size Failed"); }

        // Create the buffer of pixels, the type of BYTE is unsigned char
        BYTE* data;
        data = new BYTE[x * y * 4];
        memset(data, 0, x * y * 4);

        // Copy the pixels to the buffer
        UINT stride = x * 4;
        hr = m_pConvertedSourceBitmap->CopyPixels(nullptr, stride, x * y * 4, data);
        if (checkHR(hr)) { quitWithError("Copy Pixels Failed"); }

        /******************************************************************
        *  TO-DO:                                                         *
        *                                                                 *
        *  实现一个Array类，并将上面的data转存至你的Array内                  *
        *                                                                 *
        *  数据说明：从Bitmap Copy出来的数据，每4个为一组代表一个像素         *
        *           数据为一个长度为图像的(长*宽*4)的一维数组                *
        *           即数据排布为 R G B A R G B A R G B A.....              *
        *                                                                 *
        *  ！注意！  你仅可以只改动从此开始到下一个TO-DO END位置的代码！       *
        ******************************************************************/

        receiver = Array<BYTE>(data, (size_t)x * y * 4);
        receiver.reshape(y, x, 4);

        delete[] data;

        /******************************************************************
        *  TO-DO END                                                      *
        ******************************************************************/

        // Close the file handle
        CloseHandle(hFile);
        hFile = NULL;
    }
}

PicLoader::PicLoader()
    : impl(new ReadImpl()) {}

PicLoader::~PicLoader() {
    delete (ReadImpl*)impl;
}

void PicLoader::loadPic(const char* file) {
    ((ReadImpl*)impl)->readPic(file);
}

void PicLoader::getPixels(Array<int8_t>& receiver) {
    ((ReadImpl*)impl)->getData(file);
}