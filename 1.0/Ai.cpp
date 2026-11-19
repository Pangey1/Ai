#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")

#include <windows.h>
#include <gdiplus.h>
#include <commdlg.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <random>
#include <algorithm>
#include <cwctype>

namespace fs = std::filesystem;
using namespace Gdiplus;

#define IDI_APP_ICON        1
#define IDR_BACKUP_HTML     101

#define ID_EDIT_FIRST_NAME  101
#define ID_BTN_CREATE_FIRST 102
#define ID_BTN_EDIT_PROFILE 103
#define ID_LIST_CHATS       104
#define ID_BTN_NEW_CHAT     105
#define ID_EDIT_CHAT_HIST   106
#define ID_EDIT_MSG_INPUT   107
#define ID_BTN_SEND_MSG     108
#define ID_BTN_TRAIN_AI     109
#define ID_BTN_OPEN_HTML    110
#define ID_BTN_IMPORT_CHAT  111
#define ID_BTN_OPEN_FOLDER  112
#define TIMER_MAIN_GIF      1

#define ID_LIST_USERS       201
#define ID_BTN_LOGIN_USER   202
#define ID_BTN_NEW_USER_DLG 203

#define ID_PROF_NAME        301
#define ID_PROF_DESC        302
#define ID_PROF_AVATAR_BTN  303
#define ID_PROF_SWITCH_USER 304
#define ID_PROF_OK          305
#define ID_PROF_CANCEL      306
#define TIMER_TEMP_GIF      2

#define ID_TRAIN_TEXT       401
#define ID_TRAIN_LOAD_FILE  402
#define ID_TRAIN_OK         403
#define ID_TRAIN_CANCEL     404

#define ID_NEWCHAT_TITLE    501
#define ID_NEWCHAT_DESC     502
#define ID_NEWCHAT_OK       503
#define ID_NEWCHAT_CANCEL   504
bool g_isRu = true;

std::wstring T(const std::wstring& ru, const std::wstring& en) {
    return g_isRu ? ru : en;
}

typedef std::vector<BYTE> ByteVec;
typedef std::vector<std::wstring> StrVec;
typedef std::vector<HBITMAP> BmpVec;
typedef std::vector<GUID> GuidVec;
typedef std::vector<UINT32> UintVec;

struct SafeFrame {
    UINT32 delayMs = 100;
    ByteVec pngData;
};
typedef std::vector<SafeFrame> SafeFrameVec;

struct SafeProfile {
    bool exists = false;
    std::wstring username;
    std::wstring description;
    SafeFrameVec frames;
};

struct ChatMessage {
    bool isUser = true;
    std::wstring text;
};
typedef std::vector<ChatMessage> MsgVec;

struct ChatSession {
    std::wstring id;
    std::wstring title;
    std::wstring description;
    MsgVec messages;
};
typedef std::vector<ChatSession> ChatVec;

// Системный перевод в нижний регистр с поддержкой кириллицы (Windows API)
std::wstring ToLowerSystem(std::wstring str) {
    if (!str.empty()) {
        CharLowerBuffW(str.data(), (DWORD)str.size());
    }
    return str;
}
class MiniAIBrain {
public:
    std::unordered_map<std::wstring, StrVec> markovChain;
    StrVec knowledgeSentences;

    void Clear() {
        markovChain.clear();
        knowledgeSentences.clear();
    }

    static std::wstring NormalizeWord(const std::wstring& w) {
        std::wstring raw;
        for (wchar_t c : w) {
            if (iswalnum(c)) raw += c;
        }
        return ToLowerSystem(raw);
    }

    static std::wstring StemWord(const std::wstring& w) {
        std::wstring res = NormalizeWord(w);
        if (res.size() > 5) {
            res.resize(res.size() - 2);
        }
        else if (res.size() == 5) {
            res.resize(4);
        }
        return res;
    }

    static bool TrySolveMath(const std::wstring& text, double& outResult) {
        std::wstring expr;
        bool hasOp = false, hasDigit = false;
        for (wchar_t c : text) {
            if (iswdigit(c) || c == L'.') { expr += c; hasDigit = true; }
            else if (c == L'+' || c == L'-' || c == L'*' || c == L'/' || c == L'(' || c == L')') {
                expr += c;
                if (c != L'(' && c != L')') hasOp = true;
            }
        }
        if (!hasDigit || !hasOp || expr.size() < 3) return false;

        std::vector<double> nums;
        std::vector<wchar_t> ops;
        auto applyOp = [&]() -> bool {
            if (nums.size() < 2 || ops.empty()) return false;
            double b = nums.back(); nums.pop_back();
            double a = nums.back(); nums.pop_back();
            wchar_t op = ops.back(); ops.pop_back();
            if (op == L'+') nums.push_back(a + b);
            else if (op == L'-') nums.push_back(a - b);
            else if (op == L'*') nums.push_back(a * b);
            else if (op == L'/') { if (b == 0.0) return false; nums.push_back(a / b); }
            return true;
            };
        auto prec = [](wchar_t op) { return (op == L'*' || op == L'/') ? 2 : (op == L'+' || op == L'-') ? 1 : 0; };

        for (size_t i = 0; i < expr.size(); ++i) {
            if (iswdigit(expr[i]) || expr[i] == L'.') {
                size_t nextIdx = 0;
                try {
                    nums.push_back(std::stod(expr.substr(i), &nextIdx));
                    i += nextIdx - 1;
                }
                catch (...) { return false; }
            }
            else if (expr[i] == L'(') {
                ops.push_back(L'(');
            }
            else if (expr[i] == L')') {
                while (!ops.empty() && ops.back() != L'(') if (!applyOp()) return false;
                if (!ops.empty()) ops.pop_back();
            }
            else {
                while (!ops.empty() && prec(ops.back()) >= prec(expr[i])) if (!applyOp()) return false;
                ops.push_back(expr[i]);
            }
        }
        while (!ops.empty()) if (!applyOp()) return false;
        if (nums.size() == 1) { outResult = nums.front(); return true; }
        return false;
    }

    void TrainOnText(const std::wstring& rawText) {
        if (rawText.empty()) return;

        std::wstring currentSent;
        for (wchar_t c : rawText) {
            currentSent += c;
            if (c == L'.' || c == L'!' || c == L'?' || c == L'\n') {
                size_t start = currentSent.find_first_not_of(L" \t\r\n");
                if (start != std::wstring::npos) {
                    std::wstring clean = currentSent.substr(start);
                    while (!clean.empty() && (clean.back() == L'\r' || clean.back() == L'\n')) clean.pop_back();
                    if (clean.size() > 2) {
                        knowledgeSentences.push_back(clean);
                    }
                }
                currentSent.clear();
            }
        }
        if (currentSent.size() > 2) knowledgeSentences.push_back(currentSent);

        std::wstringstream ss(rawText);
        StrVec words;
        std::wstring token;
        while (ss >> token) words.push_back(token);

        if (words.size() >= 2) {
            for (size_t i = 0; i + 1 < words.size(); ++i) {
                std::wstring key = NormalizeWord(words[i]);
                if (!key.empty()) markovChain[key].push_back(words[i + 1]);
            }
        }
    }
    std::wstring GenerateResponse(const std::wstring& userInput) {
        double mathVal = 0.0;
        if (TrySolveMath(userInput, mathVal)) {
            std::wstringstream res;
            res << mathVal;
            return T(L"Точный расчёт: " + res.str(), L"Exact calculation: " + res.str());
        }

        std::wstringstream ss(userInput);
        StrVec inputStems;
        std::wstring tok;
        while (ss >> tok) {
            std::wstring norm = NormalizeWord(tok);
            if (norm == L"привет" || norm == L"приветик" || norm == L"ку" || norm == L"хай" || norm == L"hello" || norm == L"hi") {
                inputStems.push_back(StemWord(L"здравствуй"));
                inputStems.push_back(StemWord(L"привет"));
            }
            else if (norm == L"ии") {
                inputStems.push_back(StemWord(L"искусственный"));
                inputStems.push_back(StemWord(L"интеллект"));
            }
            if (norm.size() >= 3) {
                inputStems.push_back(StemWord(norm));
            }
        }

        static std::mt19937 rng(GetTickCount());
        int bestScore = 0;
        StrVec bestReplies;

        for (size_t i = 0; i < knowledgeSentences.size(); ++i) {
            const auto& sent = knowledgeSentences[i];
            std::wstring lowerSent = ToLowerSystem(sent);
            int score = 0;

            for (const auto& stem : inputStems) {
                if (stem.size() >= 3 && lowerSent.find(stem) != std::wstring::npos) {
                    score += (int)stem.size();
                }
            }

            if (score > 0) {
                std::wstring candidate = sent;
                if (!sent.empty() && sent.back() == L'?' && (i + 1 < knowledgeSentences.size())) {
                    candidate = knowledgeSentences[i + 1];
                }

                if (score > bestScore) {
                    bestScore = score;
                    bestReplies.clear();
                    bestReplies.push_back(candidate);
                }
                else if (score == bestScore) {
                    bestReplies.push_back(candidate);
                }
            }
        }

        if (!bestReplies.empty()) {
            std::uniform_int_distribution<size_t> dist(0, bestReplies.size() - 1);
            return bestReplies[dist(rng)];
        }

        return T(
            L"В моей текущей базе Ai.dat пока нет ответа на этот вопрос. Нажми «Обучить ИИ» сверху, чтобы дописать знания или загрузить готовый файл Ai.dat!",
            L"My current Ai.dat base doesn't have an answer for this yet. Click 'Train AI' above to add text or load a ready Ai.dat file!"
        );
    }

    void InitDefaultKnowledge() {
        Clear();
        if (g_isRu) {
            TrainOnText(L"Привет, приветствую, здравствуй — рад пообщаться! Чем я могу тебе помочь?\n"
                L"Как дела, настроение — всё отлично, я работаю на полную мощность и готов к задачам!\n"
                L"Кто ты, искусственный интеллект, нейросеть — я твой локальный ИИ Mini AI на языке C++17, работающий с зашифрованной базой Ai.dat.");
        }
        else {
            TrainOnText(L"Hello, hi, greetings — glad to chat! How can I help you today?\n"
                L"How are you — doing great and ready to assist you with your tasks!\n"
                L"Who are you, artificial intelligence — I am your local Mini AI built with C++17 using the encrypted Ai.dat base.");
        }
    }
};
SafeProfile g_user;
SafeProfile g_tempUser;
MiniAIBrain g_brain;
ChatVec g_chats;
int g_currentChatIdx = -1;

BmpVec g_mainAnimBmps;
size_t g_mainFrameIdx = 0;

BmpVec g_tempAnimBmps;
size_t g_tempFrameIdx = 0;

HWND g_hMainWnd = NULL;
HWND g_hEditFirstName = NULL, g_hBtnCreateFirst = NULL;
HWND g_hBtnEditProfile = NULL, g_hListChats = NULL, g_hBtnNewChat = NULL;
HWND g_hBtnImportChat = NULL, g_hBtnOpenFolder = NULL;
HWND g_hEditChatHist = NULL, g_hEditMsgInput = NULL, g_hBtnSendMsg = NULL;
HWND g_hBtnTrainAI = NULL, g_hBtnOpenHtml = NULL;

HFONT g_hFontNormal, g_hFontBold, g_hFontTitle;
HICON g_hAppIcon = NULL;
ULONG_PTR g_gdiplusToken;

fs::path GetBaseAppFolder() {
    std::wstring path(MAX_PATH, L'\0');
    fs::path dir = L"C:\\ProgramData\\MiniAI_Studio";
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path.data()))) {
        dir = fs::path(path.c_str()) / L"MiniAI_Studio";
    }
    std::error_code ec;
    fs::create_directories(dir / L"Users", ec);
    return dir;
}

fs::path GetUserFolder(const std::wstring& username) {
    fs::path userDir = GetBaseAppFolder() / L"Users" / username;
    std::error_code ec;
    fs::create_directories(userDir / L"chats", ec);
    return userDir;
}

void CryptBuffer(ByteVec& data) {
    const BYTE key[] = { 0x6B, 0xB6, 0x2D, 0xD2, 0x8F, 0xF8, 0x4A, 0xA4 };
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= (key[i % 8] + (BYTE)((i * 7) & 0xFF));
    }
}

void WriteUint32(ByteVec& buf, UINT32 val) {
    const BYTE* p = reinterpret_cast<const BYTE*>(&val);
    buf.insert(buf.end(), p, p + 4);
}

void WriteWString(ByteVec& buf, const std::wstring& str) {
    UINT32 len = (UINT32)str.size();
    WriteUint32(buf, len);
    if (len > 0) {
        const BYTE* p = reinterpret_cast<const BYTE*>(str.data());
        buf.insert(buf.end(), p, p + len * sizeof(wchar_t));
    }
}

bool ReadUint32(const ByteVec& buf, size_t& offset, UINT32& val) {
    if (offset + 4 > buf.size()) return false;
    memcpy(&val, buf.data() + offset, 4);
    offset += 4;
    return true;
}

bool ReadWString(const ByteVec& buf, size_t& offset, std::wstring& str) {
    UINT32 len = 0;
    if (!ReadUint32(buf, offset, len)) return false;
    if (offset + len * sizeof(wchar_t) > buf.size()) return false;
    str.assign(reinterpret_cast<const wchar_t*>(buf.data() + offset), len);
    offset += len * sizeof(wchar_t);
    return true;
}

bool SaveEncryptedFile(const fs::path& filePath, ByteVec& rawData) {
    CryptBuffer(rawData);
    std::ofstream out(filePath, std::ios::binary);
    if (!out.is_open()) return false;
    out.write(reinterpret_cast<const char*>(rawData.data()), rawData.size());
    return true;
}

bool LoadDecryptedFile(const fs::path& filePath, ByteVec& outData) {
    std::ifstream in(filePath, std::ios::binary | std::ios::ate);
    if (!in.is_open()) return false;
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);
    if (size <= 0) return false;
    outData.resize((size_t)size);
    in.read(reinterpret_cast<char*>(outData.data()), size);
    CryptBuffer(outData);
    return true;
}
void OpenEmbeddedBackupHTML(HWND hwnd) {
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_BACKUP_HTML), RT_RCDATA);
    if (!hRes) {
        const wchar_t* errMsg = g_isRu
            ? L"Ресурс Ai.html не найден! Проверьте файл resource.rc."
            : L"Ai.html resource not found! Please check resource.rc.";
        MessageBoxW(hwnd, errMsg, L"Info", MB_ICONINFORMATION);
        return;
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    DWORD size = SizeofResource(NULL, hRes);
    void* pPtr = LockResource(hData);

    if (pPtr && size > 0) {
        fs::path htmlPath = GetBaseAppFolder() / L"Ai.html";
        std::ofstream out(htmlPath, std::ios::binary);
        if (out.is_open()) {
            out.write(reinterpret_cast<const char*>(pPtr), size);
            out.close();
            ShellExecuteW(NULL, L"open", htmlPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }
}

int GetPngEncoderClsid(CLSID* pClsid) {
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    ByteVec buf(size);
    ImageCodecInfo* pInfo = reinterpret_cast<ImageCodecInfo*>(buf.data());
    GetImageEncoders(num, size, pInfo);
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pInfo[j].MimeType, L"image/png") == 0) {
            *pClsid = pInfo[j].Clsid;
            return (int)j;
        }
    }
    return -1;
}

bool EncodeCurrentFrameToPng512(Image& img, ByteVec& outPng) {
    Bitmap resizedBmp(512, 512, PixelFormat32bppARGB);
    Graphics g(&resizedBmp);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.DrawImage(&img, 0, 0, 512, 512);

    IStream* pStream = NULL;
    if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) != S_OK) return false;

    CLSID pngClsid;
    GetPngEncoderClsid(&pngClsid);
    if (resizedBmp.Save(pStream, &pngClsid, NULL) != Ok) {
        pStream->Release();
        return false;
    }

    HGLOBAL hMem = NULL;
    GetHGlobalFromStream(pStream, &hMem);
    SIZE_T size = GlobalSize(hMem);
    void* pPtr = GlobalLock(hMem);

    outPng.resize(size);
    memcpy(outPng.data(), pPtr, size);

    GlobalUnlock(hMem);
    pStream->Release();
    return true;
}

bool ProcessAvatarFile(const fs::path& filePath, SafeFrameVec& outFrames) {
    Image img(filePath.c_str());
    if (img.GetLastStatus() != Ok) return false;

    outFrames.clear();
    UINT dimCount = img.GetFrameDimensionsCount();
    if (dimCount == 0) return false;

    GuidVec dimIDs(dimCount);
    img.GetFrameDimensionsList(dimIDs.data(), dimCount);
    UINT frameCount = img.GetFrameCount(&dimIDs.front());

    UintVec delays(frameCount, 100);
    UINT propSize = img.GetPropertyItemSize(PropertyTagFrameDelay);
    if (propSize > 0) {
        ByteVec propBuf(propSize);
        PropertyItem* pItem = reinterpret_cast<PropertyItem*>(propBuf.data());
        if (img.GetPropertyItem(PropertyTagFrameDelay, propSize, pItem) == Ok) {
            UINT32* rawDelays = reinterpret_cast<UINT32*>(pItem->value);
            for (UINT i = 0; i < frameCount; ++i) {
                UINT32 ms = rawDelays[i] * 10;
                delays[i] = (ms < 20) ? 100 : ms;
            }
        }
    }

    for (UINT i = 0; i < frameCount; ++i) {
        img.SelectActiveFrame(&dimIDs.front(), i);
        SafeFrame frame;
        frame.delayMs = delays[i];
        if (EncodeCurrentFrameToPng512(img, frame.pngData)) {
            outFrames.push_back(std::move(frame));
        }
    }
    return !outFrames.empty();
}

HBITMAP CreateScaledHBitmap(const ByteVec& pngBytes, int targetSize) {
    if (pngBytes.empty()) return NULL;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, pngBytes.size());
    void* pPtr = GlobalLock(hMem);
    memcpy(pPtr, pngBytes.data(), pngBytes.size());
    GlobalUnlock(hMem);

    IStream* pStream = NULL;
    CreateStreamOnHGlobal(hMem, TRUE, &pStream);
    Bitmap bmp(pStream);

    Bitmap scaled(targetSize, targetSize, PixelFormat32bppARGB);
    Graphics g(&scaled);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.DrawImage(&bmp, 0, 0, targetSize, targetSize);

    HBITMAP hBmp = NULL;
    scaled.GetHBITMAP(Color(240, 240, 240), &hBmp);
    pStream->Release();
    return hBmp;
}

void RebuildBitmapList(const SafeFrameVec& frames, BmpVec& outBmps, int size) {
    for (HBITMAP bmp : outBmps) if (bmp) DeleteObject(bmp);
    outBmps.clear();
    for (const auto& f : frames) {
        outBmps.push_back(CreateScaledHBitmap(f.pngData, size));
    }
}
void SaveUserProfile(const SafeProfile& user) {
    fs::path file = GetUserFolder(user.username) / (user.username + L".dat");
    ByteVec buf;
    WriteUint32(buf, 0x50524F46); // "PROF"
    WriteWString(buf, user.username);
    WriteWString(buf, user.description);

    WriteUint32(buf, (UINT32)user.frames.size());
    for (const auto& f : user.frames) {
        WriteUint32(buf, f.delayMs);
        WriteUint32(buf, (UINT32)f.pngData.size());
        if (!f.pngData.empty()) {
            buf.insert(buf.end(), f.pngData.begin(), f.pngData.end());
        }
    }
    SaveEncryptedFile(file, buf);
}

bool LoadUserProfile(const std::wstring& username, SafeProfile& outUser) {
    fs::path file = GetBaseAppFolder() / L"Users" / username / (username + L".dat");
    ByteVec buf;
    if (!LoadDecryptedFile(file, buf)) return false;

    size_t offset = 0;
    UINT32 magic = 0;
    if (!ReadUint32(buf, offset, magic) || magic != 0x50524F46) return false;
    if (!ReadWString(buf, offset, outUser.username)) return false;
    if (!ReadWString(buf, offset, outUser.description)) return false;

    UINT32 frameCount = 0;
    if (!ReadUint32(buf, offset, frameCount)) return false;
    outUser.frames.clear();

    for (UINT32 i = 0; i < frameCount; ++i) {
        SafeFrame f;
        UINT32 pngSize = 0;
        if (!ReadUint32(buf, offset, f.delayMs) || !ReadUint32(buf, offset, pngSize)) return false;
        if (offset + pngSize > buf.size()) return false;
        f.pngData.assign(buf.begin() + offset, buf.begin() + offset + pngSize);
        offset += pngSize;
        outUser.frames.push_back(std::move(f));
    }
    outUser.exists = true;
    return true;
}

void SaveUserBrain(const std::wstring& username) {
    fs::path file = GetUserFolder(username) / L"Ai.dat";
    ByteVec buf;
    WriteUint32(buf, 0x41494441); // "AIDA"

    WriteUint32(buf, (UINT32)g_brain.knowledgeSentences.size());
    for (const auto& s : g_brain.knowledgeSentences) {
        WriteWString(buf, s);
    }

    WriteUint32(buf, (UINT32)g_brain.markovChain.size());
    for (const auto& pair : g_brain.markovChain) {
        WriteWString(buf, pair.first);
        WriteUint32(buf, (UINT32)pair.second.size());
        for (const auto& nextW : pair.second) {
            WriteWString(buf, nextW);
        }
    }
    SaveEncryptedFile(file, buf);
}

// Расшифровка любого указанного файла Ai.dat (поддерживает загрузку готового Ai.dat с диска)
bool LoadBrainFromFile(const fs::path& file) {
    ByteVec buf;
    if (!LoadDecryptedFile(file, buf)) return false;

    size_t offset = 0;
    UINT32 magic = 0;
    if (!ReadUint32(buf, offset, magic) || magic != 0x41494441) return false;

    g_brain.Clear();
    UINT32 sentCount = 0;
    if (ReadUint32(buf, offset, sentCount)) {
        for (UINT32 i = 0; i < sentCount; ++i) {
            std::wstring s;
            if (ReadWString(buf, offset, s)) g_brain.knowledgeSentences.push_back(std::move(s));
        }
    }

    UINT32 mapCount = 0;
    if (ReadUint32(buf, offset, mapCount)) {
        for (UINT32 i = 0; i < mapCount; ++i) {
            std::wstring key;
            UINT32 vecCount = 0;
            if (!ReadWString(buf, offset, key) || !ReadUint32(buf, offset, vecCount)) break;
            StrVec vals;
            for (UINT32 j = 0; j < vecCount; ++j) {
                std::wstring v;
                if (ReadWString(buf, offset, v)) vals.push_back(std::move(v));
            }
            g_brain.markovChain[key] = std::move(vals);
        }
    }
    return true;
}

void LoadUserBrain(const std::wstring& username) {
    fs::path file = GetUserFolder(username) / L"Ai.dat";
    if (!LoadBrainFromFile(file)) {
        g_brain.InitDefaultKnowledge();
        SaveUserBrain(username);
    }
}
void SaveChatSession(const std::wstring& username, const ChatSession& chat) {
    fs::path file = GetUserFolder(username) / L"chats" / (chat.id + L".dat");
    ByteVec buf;
    WriteUint32(buf, 0x43484132); // "CHA2"
    WriteWString(buf, chat.title);
    WriteWString(buf, chat.description);
    WriteUint32(buf, (UINT32)chat.messages.size());
    for (const auto& m : chat.messages) {
        WriteUint32(buf, m.isUser ? 1 : 0);
        WriteWString(buf, m.text);
    }
    SaveEncryptedFile(file, buf);
}

void LoadUserChats(const std::wstring& username) {
    g_chats.clear();
    fs::path chatsDir = GetUserFolder(username) / L"chats";
    std::error_code ec;

    for (const auto& entry : fs::directory_iterator(chatsDir, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == L".dat") {
            ByteVec buf;
            if (!LoadDecryptedFile(entry.path(), buf)) continue;

            size_t offset = 0;
            UINT32 magic = 0;
            if (!ReadUint32(buf, offset, magic) || magic != 0x43484132) continue;

            ChatSession chat;
            chat.id = entry.path().stem().wstring();
            if (!ReadWString(buf, offset, chat.title)) continue;
            if (!ReadWString(buf, offset, chat.description)) continue;

            UINT32 msgCount = 0;
            if (!ReadUint32(buf, offset, msgCount)) continue;
            for (UINT32 i = 0; i < msgCount; ++i) {
                UINT32 isU = 0;
                std::wstring txt;
                if (!ReadUint32(buf, offset, isU) || !ReadWString(buf, offset, txt)) break;
                chat.messages.push_back({ isU == 1, std::move(txt) });
            }
            g_chats.push_back(std::move(chat));
        }
    }

    if (g_chats.empty()) {
        ChatSession first;
        first.id = L"chat_1";
        first.title = T(L"Основной диалог", L"Main Chat");
        first.description = L"";
        first.messages.push_back({ false, T(
            L"Привет! Я готов к работе. Нажми «Обучить ИИ» (можно загрузить готовый Ai.dat) или задай любой вопрос!",
            L"Hello! I am ready. Click 'Train AI' (you can load a ready Ai.dat) or ask any question!"
        ) });
        SaveChatSession(username, first);
        g_chats.push_back(std::move(first));
    }
    g_currentChatIdx = 0;
}

StrVec GetAllExistingUsers() {
    StrVec users;
    fs::path usersRoot = GetBaseAppFolder() / L"Users";
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(usersRoot, ec)) {
        if (entry.is_directory()) {
            std::wstring uname = entry.path().filename().wstring();
            if (fs::exists(entry.path() / (uname + L".dat"), ec)) {
                users.push_back(uname);
            }
        }
    }
    return users;
}
void StartMainAvatarAnimation(HWND hwnd) {
    KillTimer(hwnd, TIMER_MAIN_GIF);
    g_mainFrameIdx = 0;
    RebuildBitmapList(g_user.frames, g_mainAnimBmps, 44);
    if (g_user.frames.size() > 1) {
        SetTimer(hwnd, TIMER_MAIN_GIF, g_user.frames.front().delayMs, NULL);
    }
}

void StartTempAvatarAnimation(HWND hwnd) {
    KillTimer(hwnd, TIMER_TEMP_GIF);
    g_tempFrameIdx = 0;
    RebuildBitmapList(g_tempUser.frames, g_tempAnimBmps, 120);
    if (g_tempUser.frames.size() > 1) {
        SetTimer(hwnd, TIMER_TEMP_GIF, g_tempUser.frames.front().delayMs, NULL);
    }
}

void RefreshChatUI() {
    SendMessageW(g_hListChats, LB_RESETCONTENT, 0, 0);
    for (const auto& c : g_chats) {
        SendMessageW(g_hListChats, LB_ADDSTRING, 0, (LPARAM)c.title.c_str());
    }
    if (g_currentChatIdx >= 0 && g_currentChatIdx < (int)g_chats.size()) {
        SendMessageW(g_hListChats, LB_SETCURSEL, g_currentChatIdx, 0);

        std::wstring fullText;
        if (!g_chats[g_currentChatIdx].description.empty()) {
            fullText += L"[" + g_chats[g_currentChatIdx].description + L"]\r\n----------------------------------------\r\n\r\n";
        }
        for (const auto& m : g_chats[g_currentChatIdx].messages) {
            fullText += (m.isUser ? (g_user.username + L": ") : L"AI: ");
            fullText += m.text + L"\r\n\r\n";
        }
        SetWindowTextW(g_hEditChatHist, fullText.c_str());
        int len = GetWindowTextLengthW(g_hEditChatHist);
        SendMessageW(g_hEditChatHist, EM_SETSEL, len, len);
        SendMessageW(g_hEditChatHist, EM_SCROLLCARET, 0, 0);
    }
}

void SwitchToUser(HWND hwnd, const std::wstring& username) {
    fs::path userDir = GetBaseAppFolder() / L"Users" / username;
    fs::path chatsDir = userDir / L"chats";
    std::error_code ec;

    bool hasChats = false;
    if (fs::exists(chatsDir, ec)) {
        for (const auto& entry : fs::directory_iterator(chatsDir, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == L".dat") {
                hasChats = true;
                break;
            }
        }
    }

    if (!hasChats) {
        MessageBoxW(hwnd,
            T(L"Ошибка: рядом с профилем этого пользователя не найдены файлы чатов (.dat)!",
                L"Error: no chat (.dat) files found next to this user profile!").c_str(),
            L"Error", MB_ICONERROR);
    }

    if (LoadUserProfile(username, g_user)) {
        LoadUserBrain(username);
        LoadUserChats(username);
        StartMainAvatarAnimation(hwnd);
        RefreshChatUI();
    }
}

void ImportExternalChat(HWND hwnd) {
    std::wstring szFile(MAX_PATH, L'\0');
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = g_isRu ? L"Зашифрованный чат (*.dat)\0*.dat\0" : L"Encrypted Chat (*.dat)\0*.dat\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        ByteVec buf;
        if (!LoadDecryptedFile(szFile.c_str(), buf)) {
            MessageBoxW(hwnd, T(L"Не удалось прочитать файл!", L"Failed to read file!").c_str(), L"Error", MB_ICONERROR);
            return;
        }

        size_t offset = 0;
        UINT32 magic = 0;
        if (!ReadUint32(buf, offset, magic) || magic != 0x43484132) {
            MessageBoxW(hwnd,
                T(L"Ошибка: выбранный .dat файл не является чатом или повреждён!",
                    L"Error: selected .dat file is not a valid chat or is corrupted!").c_str(),
                L"Error", MB_ICONERROR);
            return;
        }

        ChatSession imported;
        imported.id = L"chat_" + std::to_wstring(GetTickCount64());
        if (!ReadWString(buf, offset, imported.title)) return;
        if (!ReadWString(buf, offset, imported.description)) return;

        UINT32 msgCount = 0;
        if (!ReadUint32(buf, offset, msgCount)) return;
        for (UINT32 i = 0; i < msgCount; ++i) {
            UINT32 isU = 0;
            std::wstring txt;
            if (!ReadUint32(buf, offset, isU) || !ReadWString(buf, offset, txt)) break;
            imported.messages.push_back({ isU == 1, std::move(txt) });
        }

        SaveChatSession(g_user.username, imported);
        g_chats.push_back(std::move(imported));
        g_currentChatIdx = (int)g_chats.size() - 1;
        RefreshChatUI();

        MessageBoxW(hwnd,
            T(L"Чат успешно расшифрован и перезаписан в ваш текущий профиль!",
                L"Chat successfully decrypted and saved into your current profile!").c_str(),
            T(L"Готово", L"Done").c_str(), MB_ICONINFORMATION);
    }
}
void UpdateMainLayout(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int w = rc.right;
    int h = rc.bottom;

    if (!g_user.exists) {
        ShowWindow(g_hBtnEditProfile, SW_HIDE);
        ShowWindow(g_hListChats, SW_HIDE);
        ShowWindow(g_hBtnNewChat, SW_HIDE);
        ShowWindow(g_hBtnImportChat, SW_HIDE);
        ShowWindow(g_hBtnOpenFolder, SW_HIDE);
        ShowWindow(g_hEditChatHist, SW_HIDE);
        ShowWindow(g_hEditMsgInput, SW_HIDE);
        ShowWindow(g_hBtnSendMsg, SW_HIDE);
        ShowWindow(g_hBtnTrainAI, SW_HIDE);
        ShowWindow(g_hBtnOpenHtml, SW_HIDE);

        ShowWindow(g_hEditFirstName, SW_SHOW);
        ShowWindow(g_hBtnCreateFirst, SW_SHOW);
        MoveWindow(g_hEditFirstName, (w - 280) / 2, 230, 280, 30, TRUE);
        MoveWindow(g_hBtnCreateFirst, (w - 180) / 2, 275, 180, 34, TRUE);
    }
    else {
        ShowWindow(g_hEditFirstName, SW_HIDE);
        ShowWindow(g_hBtnCreateFirst, SW_HIDE);

        ShowWindow(g_hBtnEditProfile, SW_SHOW);
        ShowWindow(g_hListChats, SW_SHOW);
        ShowWindow(g_hBtnNewChat, SW_SHOW);
        ShowWindow(g_hBtnImportChat, SW_SHOW);
        ShowWindow(g_hBtnOpenFolder, SW_SHOW);
        ShowWindow(g_hEditChatHist, SW_SHOW);
        ShowWindow(g_hEditMsgInput, SW_SHOW);
        ShowWindow(g_hBtnSendMsg, SW_SHOW);
        ShowWindow(g_hBtnTrainAI, SW_SHOW);
        ShowWindow(g_hBtnOpenHtml, SW_SHOW);

        MoveWindow(g_hBtnTrainAI, 15, 14, 120, 32, TRUE);
        MoveWindow(g_hBtnOpenHtml, 145, 14, 135, 32, TRUE);
        MoveWindow(g_hBtnOpenFolder, 290, 14, 135, 32, TRUE);
        MoveWindow(g_hBtnEditProfile, w - 180, 14, 120, 32, TRUE);

        int sideW = 200;
        MoveWindow(g_hBtnNewChat, 10, 70, sideW - 20, 30, TRUE);
        MoveWindow(g_hBtnImportChat, 10, 106, sideW - 20, 30, TRUE);
        MoveWindow(g_hListChats, 10, 144, sideW - 20, max(50, h - 154), TRUE);

        int chatX = sideW + 10;
        int chatW = max(100, w - chatX - 15);
        MoveWindow(g_hEditChatHist, chatX, 70, chatW, max(50, h - 135), TRUE);
        MoveWindow(g_hEditMsgInput, chatX, h - 52, max(80, chatW - 110), 36, TRUE);
        MoveWindow(g_hBtnSendMsg, w - 115, h - 52, 100, 36, TRUE);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK UserSelectWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hListUsers;
    switch (uMsg) {
    case WM_CREATE: {
        const wchar_t* lbl = g_isRu ? L"Выберите аккаунт для входа:" : L"Select an account to sign in:";
        const wchar_t* bIn = g_isRu ? L"Войти" : L"Sign In";
        const wchar_t* bNw = g_isRu ? L"+ Новый аккаунт" : L"+ New Account";

        CreateWindowW(L"STATIC", lbl, WS_VISIBLE | WS_CHILD, 20, 15, 320, 22, hwnd, NULL, NULL, NULL);
        hListUsers = CreateWindowW(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            20, 45, 320, 160, hwnd, (HMENU)ID_LIST_USERS, NULL, NULL);
        CreateWindowW(L"BUTTON", bIn, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 215, 150, 32, hwnd, (HMENU)ID_BTN_LOGIN_USER, NULL, NULL);
        CreateWindowW(L"BUTTON", bNw, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            190, 215, 150, 32, hwnd, (HMENU)ID_BTN_NEW_USER_DLG, NULL, NULL);

        StrVec users = GetAllExistingUsers();
        for (const auto& u : users) SendMessageW(hListUsers, LB_ADDSTRING, 0, (LPARAM)u.c_str());
        if (!users.empty()) SendMessageW(hListUsers, LB_SETCURSEL, 0, 0);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int ev = HIWORD(wParam);
        if ((id == ID_BTN_LOGIN_USER && ev == BN_CLICKED) || (id == ID_LIST_USERS && ev == LBN_DBLCLK)) {
            int sel = (int)SendMessageW(hListUsers, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR) {
                int len = (int)SendMessageW(hListUsers, LB_GETTEXTLEN, sel, 0);
                if (len > 0) {
                    std::wstring buf(len + 1, L'\0');
                    SendMessageW(hListUsers, LB_GETTEXT, sel, (LPARAM)buf.data());
                    SwitchToUser(g_hMainWnd, buf.c_str());
                    UpdateMainLayout(g_hMainWnd);
                    DestroyWindow(hwnd);
                }
            }
        }
        else if (id == ID_BTN_NEW_USER_DLG && ev == BN_CLICKED) {
            g_user.exists = false;
            UpdateMainLayout(g_hMainWnd);
            DestroyWindow(hwnd);
        }
        return 0;
    }

    case WM_DESTROY:
        EnableWindow(g_hMainWnd, TRUE);
        SetForegroundWindow(g_hMainWnd);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void OpenUserSelectDialog(HWND parent) {
    EnableWindow(parent, FALSE);
    const wchar_t* title = g_isRu ? L"Выбор пользователя" : L"Choose Account";
    CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAIUserSelectClass", title,
        WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 375, 295,
        parent, NULL, GetModuleHandle(NULL), NULL);
}
LRESULT CALLBACK NewChatWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditTitle, hEditDesc;
    switch (uMsg) {
    case WM_CREATE: {
        const wchar_t* lTit = g_isRu ? L"Название нового чата:" : L"New Chat Title:";
        const wchar_t* lDsc = g_isRu ? L"Описание (необязательно):" : L"Description (optional):";
        const wchar_t* bOk = g_isRu ? L"Создать" : L"Create";
        const wchar_t* bCan = g_isRu ? L"Отмена" : L"Cancel";

        std::wstring defTitle = T(L"Диалог #", L"Chat #") + std::to_wstring(g_chats.size() + 1);

        CreateWindowW(L"STATIC", lTit, WS_VISIBLE | WS_CHILD, 20, 15, 340, 20, hwnd, NULL, NULL, NULL);
        hEditTitle = CreateWindowW(L"EDIT", defTitle.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            20, 40, 340, 26, hwnd, (HMENU)ID_NEWCHAT_TITLE, NULL, NULL);

        CreateWindowW(L"STATIC", lDsc, WS_VISIBLE | WS_CHILD, 20, 80, 340, 20, hwnd, NULL, NULL, NULL);
        hEditDesc = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            20, 105, 340, 80, hwnd, (HMENU)ID_NEWCHAT_DESC, NULL, NULL);

        CreateWindowW(L"BUTTON", bOk, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            160, 200, 95, 30, hwnd, (HMENU)ID_NEWCHAT_OK, NULL, NULL);
        CreateWindowW(L"BUTTON", bCan, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            265, 200, 95, 30, hwnd, (HMENU)ID_NEWCHAT_CANCEL, NULL, NULL);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_NEWCHAT_OK) {
            int tLen = GetWindowTextLengthW(hEditTitle);
            int dLen = GetWindowTextLengthW(hEditDesc);
            std::wstring title(tLen + 1, L'\0');
            std::wstring desc(dLen + 1, L'\0');
            GetWindowTextW(hEditTitle, title.data(), tLen + 1);
            GetWindowTextW(hEditDesc, desc.data(), dLen + 1);
            title.resize(tLen);
            desc.resize(dLen);

            if (title.empty()) title = T(L"Диалог #", L"Chat #") + std::to_wstring(g_chats.size() + 1);

            ChatSession nextChat;
            nextChat.id = L"chat_" + std::to_wstring(GetTickCount64());
            nextChat.title = title;
            nextChat.description = desc;
            nextChat.messages.push_back({ false, T(L"Чат создан и зашифрован. Задавай вопросы по базе Ai.dat!", L"Chat created and encrypted. Ask questions based on Ai.dat!") });

            SaveChatSession(g_user.username, nextChat);
            g_chats.push_back(std::move(nextChat));
            g_currentChatIdx = (int)g_chats.size() - 1;
            RefreshChatUI();
            DestroyWindow(hwnd);
        }
        else if (id == ID_NEWCHAT_CANCEL) {
            DestroyWindow(hwnd);
        }
        return 0;
    }

    case WM_DESTROY:
        EnableWindow(g_hMainWnd, TRUE);
        SetForegroundWindow(g_hMainWnd);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
WNDPROC g_origTrainEditProc = NULL;

LRESULT CALLBACK TrainEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
        SendMessageW(hwnd, EM_SETSEL, 0, -1);
        return 0;
    }
    return CallWindowProcW(g_origTrainEditProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK TrainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditTrain;
    switch (uMsg) {
    case WM_CREATE: {
        const wchar_t* lbl = g_isRu
            ? L"Редактор базы Ai.dat (Ctrl+A — выделить всё, без ограничений):"
            : L"Ai.dat Knowledge Editor (Ctrl+A to select all, unlimited):";
        const wchar_t* bLd = g_isRu ? L"Загрузить Ai.dat..." : L"Load Ai.dat...";
        const wchar_t* bOk = g_isRu ? L"Сохранить в Ai.dat" : L"Save to Ai.dat";
        const wchar_t* bCn = g_isRu ? L"Отмена" : L"Cancel";

        CreateWindowW(L"STATIC", lbl, WS_VISIBLE | WS_CHILD, 15, 15, 560, 22, hwnd, NULL, NULL, NULL);

        hEditTrain = CreateWindowW(
            L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            15, 42, 560, 320, hwnd, (HMENU)ID_TRAIN_TEXT, NULL, NULL
        );

        SendMessageW(hEditTrain, EM_SETLIMITTEXT, 0, 0);
        g_origTrainEditProc = (WNDPROC)SetWindowLongPtrW(hEditTrain, GWLP_WNDPROC, (LONG_PTR)TrainEditProc);

        CreateWindowW(L"BUTTON", bLd, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            15, 375, 155, 32, hwnd, (HMENU)ID_TRAIN_LOAD_FILE, NULL, NULL);
        CreateWindowW(L"BUTTON", bOk, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            325, 375, 145, 32, hwnd, (HMENU)ID_TRAIN_OK, NULL, NULL);
        CreateWindowW(L"BUTTON", bCn, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            480, 375, 95, 32, hwnd, (HMENU)ID_TRAIN_CANCEL, NULL, NULL);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);

        LoadUserBrain(g_user.username);
        std::wstring existingText;
        for (const auto& s : g_brain.knowledgeSentences) existingText += s + L"\r\n";
        SetWindowTextW(hEditTrain, existingText.c_str());
        SetFocus(hEditTrain);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_TRAIN_LOAD_FILE) {
            std::wstring szFile(MAX_PATH, L'\0');
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile.data();
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = g_isRu ? L"База знаний Ai.dat (*.dat)\0*.dat\0" : L"Ai.dat Knowledge Base (*.dat)\0*.dat\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileNameW(&ofn)) {
                if (LoadBrainFromFile(szFile.c_str())) {
                    std::wstring loadedText;
                    for (const auto& s : g_brain.knowledgeSentences) loadedText += s + L"\r\n";
                    SetWindowTextW(hEditTrain, loadedText.c_str());
                    MessageBoxW(hwnd,
                        T(L"Готовый файл Ai.dat успешно расшифрован и загружен! Нажмите «Сохранить в Ai.dat», чтобы закрепить его за профилем.",
                            L"Ready Ai.dat file decrypted and loaded! Click 'Save to Ai.dat' to bind it to your profile.").c_str(),
                        T(L"Готово", L"Done").c_str(), MB_ICONINFORMATION);
                }
                else {
                    MessageBoxW(hwnd,
                        T(L"Ошибка: выбранный файл не является корректным Ai.dat!",
                            L"Error: selected file is not a valid Ai.dat!").c_str(),
                        L"Error", MB_ICONERROR);
                }
            }
        }
        else if (id == ID_TRAIN_OK) {
            int len = GetWindowTextLengthW(hEditTrain);
            std::wstring text(len + 1, L'\0');
            if (len > 0) GetWindowTextW(hEditTrain, text.data(), len + 1);
            text.resize(len);

            g_brain.Clear();
            g_brain.TrainOnText(text);
            SaveUserBrain(g_user.username);

            MessageBoxW(hwnd,
                T(L"База Ai.dat успешно обновлена и зашифрована!", L"Ai.dat successfully updated and encrypted!").c_str(),
                T(L"Сохранено", L"Saved").c_str(), MB_ICONINFORMATION);
            DestroyWindow(hwnd);
        }
        else if (id == ID_TRAIN_CANCEL) {
            DestroyWindow(hwnd);
        }
        return 0;
    }

    case WM_DESTROY:
        if (g_origTrainEditProc && hEditTrain) {
            SetWindowLongPtrW(hEditTrain, GWLP_WNDPROC, (LONG_PTR)g_origTrainEditProc);
            g_origTrainEditProc = NULL;
        }
        EnableWindow(g_hMainWnd, TRUE);
        SetForegroundWindow(g_hMainWnd);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK ProfileWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditName, hEditDesc;
    switch (uMsg) {
    case WM_CREATE: {
        const wchar_t* lblName = g_isRu ? L"Имя профиля:" : L"Profile Name:";
        const wchar_t* lblDesc = g_isRu ? L"Описание (о себе):" : L"Description (Bio):";
        const wchar_t* btnAv = g_isRu ? L"Выбрать фото / GIF..." : L"Choose Image / GIF...";
        const wchar_t* btnSw = g_isRu ? L"Сменить / Новый юзер" : L"Switch / New User";
        const wchar_t* btnOk = g_isRu ? L"ОК" : L"OK";
        const wchar_t* btnCan = g_isRu ? L"Отмена" : L"Cancel";

        CreateWindowW(L"STATIC", lblName, WS_VISIBLE | WS_CHILD, 20, 20, 150, 20, hwnd, NULL, NULL, NULL);
        hEditName = CreateWindowW(L"EDIT", g_tempUser.username.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            20, 45, 260, 26, hwnd, (HMENU)ID_PROF_NAME, NULL, NULL);

        CreateWindowW(L"STATIC", lblDesc, WS_VISIBLE | WS_CHILD, 20, 85, 150, 20, hwnd, NULL, NULL, NULL);
        hEditDesc = CreateWindowW(L"EDIT", g_tempUser.description.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            20, 110, 260, 90, hwnd, (HMENU)ID_PROF_DESC, NULL, NULL);

        CreateWindowW(L"BUTTON", btnAv, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            300, 175, 180, 28, hwnd, (HMENU)ID_PROF_AVATAR_BTN, NULL, NULL);
        CreateWindowW(L"BUTTON", btnSw, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 225, 165, 30, hwnd, (HMENU)ID_PROF_SWITCH_USER, NULL, NULL);
        CreateWindowW(L"BUTTON", btnOk, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            280, 225, 95, 30, hwnd, (HMENU)ID_PROF_OK, NULL, NULL);
        CreateWindowW(L"BUTTON", btnCan, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            385, 225, 95, 30, hwnd, (HMENU)ID_PROF_CANCEL, NULL, NULL);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);

        StartTempAvatarAnimation(hwnd);
        return 0;
    }

    case WM_TIMER:
        if (wParam == TIMER_TEMP_GIF && g_tempAnimBmps.size() > 1) {
            g_tempFrameIdx = (g_tempFrameIdx + 1) % g_tempAnimBmps.size();
            SetTimer(hwnd, TIMER_TEMP_GIF, g_tempUser.frames[g_tempFrameIdx].delayMs, NULL);
            RECT avRect = { 330, 40, 450, 160 };
            InvalidateRect(hwnd, &avRect, FALSE);
        }
        return 0;

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_PROF_AVATAR_BTN) {
            std::wstring szFile(MAX_PATH, L'\0');
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile.data();
            ofn.nMaxFile = MAX_PATH;
            std::wstring filter = T(
                L"Все файлы изображений (*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff;*.ico)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff;*.ico\0",
                L"All Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff;*.ico)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff;*.ico\0"
            );
            ofn.lpstrFilter = filter.c_str();
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileNameW(&ofn)) {
                if (ProcessAvatarFile(szFile.c_str(), g_tempUser.frames)) {
                    StartTempAvatarAnimation(hwnd);
                    RECT avRect = { 330, 40, 450, 160 };
                    InvalidateRect(hwnd, &avRect, TRUE);
                }
            }
        }
        else if (id == ID_PROF_SWITCH_USER) {
            DestroyWindow(hwnd);
            OpenUserSelectDialog(g_hMainWnd);
        }
        else if (id == ID_PROF_OK) {
            int
                nLen = GetWindowTextLengthW(hEditName);
            int dLen = GetWindowTextLengthW(hEditDesc);
            std::wstring newName(nLen + 1, L'\0');
            std::wstring newDesc(dLen + 1, L'\0');
            GetWindowTextW(hEditName, newName.data(), nLen + 1);
            GetWindowTextW(hEditDesc, newDesc.data(), dLen + 1);
            newName.resize(nLen);
            newDesc.resize(dLen);
            if (!newName.empty()) {
                if (newName != g_user.username) {
                    std::error_code ec;
                    fs::path oldDir = GetUserFolder(g_user.username);
                    fs::path newDir = GetBaseAppFolder() / L"Users" / newName;
                    fs::rename(oldDir, newDir, ec);
                    fs::rename(newDir / (g_user.username + L".dat"), newDir / (newName + L".dat"), ec);
                }
                g_tempUser.username = newName;
                g_tempUser.description = newDesc;
                g_user = g_tempUser;
                SaveUserProfile(g_user);
                StartMainAvatarAnimation(g_hMainWnd);
                RefreshChatUI();
                InvalidateRect(g_hMainWnd, NULL, TRUE);
            }
            DestroyWindow(hwnd);
        }
        else if (id == ID_PROF_CANCEL) {
            DestroyWindow(hwnd);
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT avRect = { 330, 40, 450, 160 };
        FillRect(hdc, &avRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
        FrameRect(hdc, &avRect, (HBRUSH)GetStockObject(GRAY_BRUSH));
        if (!g_tempAnimBmps.empty() && g_tempAnimBmps[g_tempFrameIdx]) {
            HDC hMem = CreateCompatibleDC(hdc);
            HBITMAP hOld = (HBITMAP)SelectObject(hMem, g_tempAnimBmps[g_tempFrameIdx]);
            BitBlt(hdc, 330, 40, 120, 120, hMem, 0, 0, SRCCOPY);
            SelectObject(hMem, hOld);
            DeleteDC(hMem);
        }
        else {
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, g_hFontNormal);
            DrawTextW(hdc, L"512x512\nPhoto / GIF", -1, &avRect, DT_CENTER | DT_VCENTER);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, TIMER_TEMP_GIF);
        for (HBITMAP bmp : g_tempAnimBmps) if (bmp) DeleteObject(bmp);
        g_tempAnimBmps.clear();
        EnableWindow(g_hMainWnd, TRUE);
        SetForegroundWindow(g_hMainWnd);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        g_hFontNormal = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontBold = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontTitle = CreateFontW(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        const wchar_t* bCre = g_isRu ? L"Создать аккаунт" : L"Create Account";
        const wchar_t* bTrn = g_isRu ? L"Обучить ИИ" : L"Train AI";
        const wchar_t* bHtm = g_isRu ? L"Резерв (Ai.html)" : L"Backup (Ai.html)";
        const wchar_t* bFld = g_isRu ? L"Папка данных" : L"Data Folder";
        const wchar_t* bPrf = g_isRu ? L"Профиль" : L"Profile";
        const wchar_t* bNch = g_isRu ? L"+ Новый чат" : L"+ New Chat";
        const wchar_t* bImp = g_isRu ? L"Импорт чата (.dat)" : L"Import Chat (.dat)";
        const wchar_t* bSnd = g_isRu ? L"Отправить" : L"Send";

        g_hEditFirstName = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            260, 230, 280, 30, hwnd, (HMENU)ID_EDIT_FIRST_NAME, NULL, NULL);
        g_hBtnCreateFirst = CreateWindowW(L"BUTTON", bCre, WS_CHILD | BS_DEFPUSHBUTTON,
            310, 275, 180, 34, hwnd, (HMENU)ID_BTN_CREATE_FIRST, NULL, NULL);

        g_hBtnTrainAI = CreateWindowW(L"BUTTON", bTrn, WS_CHILD | BS_PUSHBUTTON,
            15, 14, 120, 32, hwnd, (HMENU)ID_BTN_TRAIN_AI, NULL, NULL);
        g_hBtnOpenHtml = CreateWindowW(L"BUTTON", bHtm, WS_CHILD | BS_PUSHBUTTON,
            145, 14, 135, 32, hwnd, (HMENU)ID_BTN_OPEN_HTML, NULL, NULL);
        g_hBtnOpenFolder = CreateWindowW(L"BUTTON", bFld, WS_CHILD | BS_PUSHBUTTON,
            290, 14, 135, 32, hwnd, (HMENU)ID_BTN_OPEN_FOLDER, NULL, NULL);
        g_hBtnEditProfile = CreateWindowW(L"BUTTON", bPrf, WS_CHILD | BS_PUSHBUTTON,
            620, 14, 120, 32, hwnd, (HMENU)ID_BTN_EDIT_PROFILE, NULL, NULL);

        g_hBtnNewChat = CreateWindowW(L"BUTTON", bNch, WS_CHILD | BS_PUSHBUTTON,
            10, 70, 180, 30, hwnd, (HMENU)ID_BTN_NEW_CHAT, NULL, NULL);
        g_hBtnImportChat = CreateWindowW(L"BUTTON", bImp, WS_CHILD | BS_PUSHBUTTON,
            10, 106, 180, 30, hwnd, (HMENU)ID_BTN_IMPORT_CHAT, NULL, NULL);
        g_hListChats = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            10, 144, 180, 346, hwnd, (HMENU)ID_LIST_CHATS, NULL, NULL);

        g_hEditChatHist = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            210, 70, 580, 370, hwnd, (HMENU)ID_EDIT_CHAT_HIST, NULL, NULL);
        g_hEditMsgInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            210, 450, 470, 36, hwnd, (HMENU)ID_EDIT_MSG_INPUT, NULL, NULL);
        g_hBtnSendMsg = CreateWindowW(L"BUTTON", bSnd, WS_CHILD | BS_DEFPUSHBUTTON,
            690, 450, 100, 36, hwnd, (HMENU)ID_BTN_SEND_MSG, NULL, NULL);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);

        StrVec users = GetAllExistingUsers();
        if (users.size() == 1) {
            SwitchToUser(hwnd, users.front());
        }
        else if (users.size() >= 2) {
            PostMessageW(hwnd, WM_APP + 1, 0, 0);
        }

        UpdateMainLayout(hwnd);
        return 0;
    }

    case WM_APP + 1:
        OpenUserSelectDialog(hwnd);
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_MAIN_GIF && g_mainAnimBmps.size() > 1) {
            g_mainFrameIdx = (g_mainFrameIdx + 1) % g_mainAnimBmps.size();
            SetTimer(hwnd, TIMER_MAIN_GIF, g_user.frames[g_mainFrameIdx].delayMs, NULL);

            RECT rc;
            GetClientRect(hwnd, &rc);
            RECT avMini = { rc.right - 52, 8, rc.right - 8, 52 };
            InvalidateRect(hwnd, &avMini, FALSE);
        }
        return 0;

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int ev = HIWORD(wParam);

        if (id == ID_BTN_CREATE_FIRST && ev == BN_CLICKED) {
            int len = GetWindowTextLengthW(g_hEditFirstName);
            std::wstring name(len + 1, L'\0');
            GetWindowTextW(g_hEditFirstName, name.data(), len + 1);
            name.resize(len);

            if (!name.empty()) {
                g_user = SafeProfile{};
                g_user.exists = true;
                g_user.username = name;
                SaveUserProfile(g_user);

                g_brain.InitDefaultKnowledge();
                SaveUserBrain(name);

                LoadUserChats(name);
                RefreshChatUI();
                UpdateMainLayout(hwnd);
            }
            else {
                MessageBoxW(hwnd, T(L"Введите любое имя для аккаунта!", L"Please enter any account name!").c_str(),
                    T(L"Внимание", L"Notice").c_str(), MB_ICONINFORMATION);
            }
        }
        else if (id == ID_BTN_EDIT_PROFILE && ev == BN_CLICKED) {
            g_tempUser = g_user;
            EnableWindow(hwnd, FALSE);
            const wchar_t* tit = g_isRu ? L"Редактирование профиля" : L"Edit Profile";
            CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAIProfileEditorClass", tit,
                WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 515, 310,
                hwnd, NULL, GetModuleHandle(NULL), NULL);
        }
        else if (id == ID_BTN_TRAIN_AI && ev == BN_CLICKED) {
            EnableWindow(hwnd, FALSE);
            const wchar_t* tit = g_isRu ? L"Редактор базы Ai.dat" : L"Ai.dat Knowledge Editor";
            CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAITrainClass", tit,
                WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 605, 460,
                hwnd, NULL, GetModuleHandle(NULL), NULL);
        }
        else if (id == ID_BTN_OPEN_HTML && ev == BN_CLICKED) {
            OpenEmbeddedBackupHTML(hwnd);
        }
        else if (id == ID_BTN_OPEN_FOLDER && ev == BN_CLICKED) {
            ShellExecuteW(NULL, L"open", GetBaseAppFolder().c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
        else if (id == ID_BTN_NEW_CHAT && ev == BN_CLICKED) {
            EnableWindow(hwnd, FALSE);
            const wchar_t* tit = g_isRu ? L"Создание нового чата" : L"Create New Chat";
            CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAINewChatClass", tit,
                WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 395, 285,
                hwnd, NULL, GetModuleHandle(NULL), NULL);
        }
        else if (id == ID_BTN_IMPORT_CHAT && ev == BN_CLICKED) {
            ImportExternalChat(hwnd);
        }
        else if (id == ID_LIST_CHATS && ev == LBN_SELCHANGE) {
            int sel = (int)SendMessageW(g_hListChats, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR && sel < (int)g_chats.size()) {
                g_currentChatIdx = sel;
                RefreshChatUI();
            }
        }
        else if (id == ID_BTN_SEND_MSG && ev == BN_CLICKED) {
            int len = GetWindowTextLengthW(g_hEditMsgInput);
            if (len > 0 && g_currentChatIdx >= 0) {
                std::wstring userMsg(len + 1, L'\0');
                GetWindowTextW(g_hEditMsgInput, userMsg.data(), len + 1);
                userMsg.resize(len);
                SetWindowTextW(g_hEditMsgInput, L"");

                LoadUserBrain(g_user.username);

                g_chats[g_currentChatIdx].messages.push_back({ true, userMsg });
                std::wstring aiReply = g_brain.GenerateResponse(userMsg);
                g_chats[g_currentChatIdx].messages.push_back({ false, aiReply });

                SaveChatSession(g_user.username, g_chats[g_currentChatIdx]);
                RefreshChatUI();
            }
        }
        return 0;
    }

    case WM_SIZE:
        UpdateMainLayout(hwnd);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);

        FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SetBkMode(hdc, TRANSPARENT);

        if (!g_user.exists) {
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, RGB(30, 30, 30));
            RECT titleRc = { 0, 140, rc.right, 185 };
            DrawTextW(hdc, T(L"Добро пожаловать! Создайте локальный профиль", L"Welcome! Create a Local Profile").c_str(),
                -1, &titleRc, DT_CENTER | DT_SINGLELINE);

            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, RGB(100, 100, 100));
            RECT subRc = { 0, 190, rc.right, 220 };
            DrawTextW(hdc, T(L"Почта не требуется. Введите имя пользователя:", L"No email required. Enter your username:").c_str(),
                -1, &subRc, DT_CENTER | DT_SINGLELINE);
        }
        else {
            RECT headerRc = { 0, 0, rc.right, 60 };
            FillRect(hdc, &headerRc, (HBRUSH)(COLOR_BTNFACE + 1));
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, RGB(20, 20, 20));
            RECT nameRc = { rc.right - 390, 14, rc.right - 190, 46 };
            DrawTextW(hdc, g_user.username.c_str(), -1, &nameRc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            RECT avMini = { rc.right - 52, 8, rc.right - 8, 52 };
            if (!g_mainAnimBmps.empty() && g_mainAnimBmps[g_mainFrameIdx]) {
                HDC hMem = CreateCompatibleDC(hdc);
                HBITMAP hOld = (HBITMAP)SelectObject(hMem, g_mainAnimBmps[g_mainFrameIdx]);
                BitBlt(hdc, avMini.left, avMini.top, 44, 44, hMem, 0, 0, SRCCOPY);
                SelectObject(hMem, hOld);
                DeleteDC(hMem);
            }
            else {
                FillRect(hdc, &avMini, (HBRUSH)GetStockObject(GRAY_BRUSH));
            }
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, TIMER_MAIN_GIF);
        for (HBITMAP bmp : g_mainAnimBmps) if (bmp) DeleteObject(bmp);
        DeleteObject(g_hFontNormal);
        DeleteObject(g_hFontBold);
        DeleteObject(g_hFontTitle);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    LANGID langId = GetUserDefaultUILanguage();
    g_isRu = (PRIMARYLANGID(langId) == LANG_RUSSIAN);
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
    g_hAppIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    if (!g_hAppIcon) g_hAppIcon = LoadIconW(NULL, IDI_APPLICATION);
    auto regClass = [&](const wchar_t* name, WNDPROC proc, HBRUSH bg) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = proc;
        wc.hInstance = hInstance;
        wc.lpszClassName = name;
        wc.hIcon = g_hAppIcon;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = bg;
        RegisterClassW(&wc);
        };
    regClass(L"MiniAIMainWindowClass", MainWndProc, NULL);
    regClass(L"MiniAIProfileEditorClass", ProfileWndProc, (HBRUSH)(COLOR_BTNFACE + 1));
    regClass(L"MiniAIUserSelectClass", UserSelectWndProc, (HBRUSH)(COLOR_BTNFACE + 1));
    regClass(L"MiniAITrainClass", TrainWndProc, (HBRUSH)(COLOR_BTNFACE + 1));
    regClass(L"MiniAINewChatClass", NewChatWndProc, (HBRUSH)(COLOR_BTNFACE + 1));
    g_hMainWnd = CreateWindowExW(
        0, L"MiniAIMainWindowClass", L"Mini AI Studio",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 880, 560,
        NULL, NULL, hInstance, NULL
    );
    if (g_hMainWnd) {
        SendMessageW(g_hMainWnd, WM_SETICON, ICON_BIG, (LPARAM)g_hAppIcon);
        SendMessageW(g_hMainWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hAppIcon);
        ShowWindow(g_hMainWnd, nCmdShow);
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    GdiplusShutdown(g_gdiplusToken);
    return 0;
}
