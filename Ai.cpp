#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")

#include <windows.h>
#include <gdiplus.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
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

#define ID_EDIT_FIRST_NAME  101
#define ID_EDIT_FIRST_PASS  102
#define ID_BTN_CREATE_FIRST 103
#define ID_BTN_EDIT_PROFILE 104
#define ID_LIST_CHATS       105
#define ID_BTN_NEW_CHAT     106
#define ID_EDIT_CHAT_HIST   107
#define ID_EDIT_MSG_INPUT   108
#define ID_BTN_SEND_MSG     109
#define ID_BTN_TRAIN_AI     110
#define ID_BTN_IMPORT_CHAT  111
#define ID_BTN_OPEN_FOLDER  112
#define ID_BTN_EXPORT_ZIP   113
#define ID_BTN_IMPORT_ZIP   114
#define ID_COMBO_LANG       115
#define ID_BTN_ATTACH_IMG   116
#define TIMER_MAIN_GIF      1

#define ID_LIST_USERS       201
#define ID_BTN_LOGIN_USER   202
#define ID_BTN_NEW_USER_DLG 203
#define ID_BTN_GUEST_USER   204
#define ID_BTN_IMPORT_USER  205

#define ID_PROF_NAME        301
#define ID_PROF_PASS        302
#define ID_PROF_DESC        303
#define ID_PROF_AVATAR_BTN  304
#define ID_PROF_DEL_AVATAR  305
#define ID_PROF_SWITCH_USER 306
#define ID_PROF_DELETE_USER 307
#define ID_PROF_OK          308
#define ID_PROF_CANCEL      309
#define TIMER_TEMP_GIF      2

#define ID_TRAIN_TEXT       401
#define ID_TRAIN_LOAD_FILE  402
#define ID_TRAIN_OK         403
#define ID_TRAIN_CANCEL     404

#define ID_NEWCHAT_TITLE    501
#define ID_NEWCHAT_DESC     502
#define ID_NEWCHAT_OK       503
#define ID_NEWCHAT_CANCEL   504

#define ID_PASS_INPUT       601
#define ID_PASS_OK          602
#define ID_PASS_CANCEL      603

#define ID_LANG_EDIT        701
#define ID_LANG_OK          702
#define ID_LANG_CANCEL      703

#define ID_FIND_INPUT       801
#define ID_FIND_NEXT        802
#define ID_FIND_CLOSE       803
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
    bool isTemporary = false;
    std::wstring username;
    std::wstring password;
    std::wstring description;
    SafeFrameVec frames;
};

struct ChatMessage {
    bool isUser = true;
    std::wstring text;
    std::wstring imageFile;
    std::wstring timestamp;
};
typedef std::vector<ChatMessage> MsgVec;

struct ChatSession {
    std::wstring id;
    std::wstring title;
    std::wstring description;
    MsgVec messages;
};
typedef std::vector<ChatSession> ChatVec;

int g_langIndex = 0; // 0:RU, 1:EN, 2:DE, 3:ES, 4:FR, 5:UK, 6:ZH, 7:JA, 8:Custom
std::unordered_map<std::wstring, std::wstring> g_customLangMap;

std::wstring T(const std::wstring& key, const std::wstring& ru, const std::wstring& en) {
    if (g_langIndex == 8 && g_customLangMap.count(key)) return g_customLangMap[key];
    if (g_langIndex == 0) return ru;
    if (g_langIndex == 2) {
        if (key == L"train") return L"KI trainieren";
        if (key == L"folder") return L"Datenordner";
        if (key == L"expzip") return L"ZIP Export";
        if (key == L"impzip") return L"ZIP Import";
        if (key == L"prof") return L"Profil";
        if (key == L"newchat") return L"+ Neuer Chat";
        if (key == L"impchat") return L"Chat importieren";
        if (key == L"send") return L"Senden";
        if (key == L"screen") return L"Bild";
    }
    else if (g_langIndex == 3) {
        if (key == L"train") return L"Entrenar IA";
        if (key == L"folder") return L"Carpeta datos";
        if (key == L"expzip") return L"Exportar ZIP";
        if (key == L"impzip") return L"Importar ZIP";
        if (key == L"prof") return L"Perfil";
        if (key == L"newchat") return L"+ Nuevo chat";
        if (key == L"impchat") return L"Importar chat";
        if (key == L"send") return L"Enviar";
        if (key == L"screen") return L"Captura";
    }
    else if (g_langIndex == 4) {
        if (key == L"train") return L"Entraîner IA";
        if (key == L"folder") return L"Dossier données";
        if (key == L"expzip") return L"Exporter ZIP";
        if (key == L"impzip") return L"Importer ZIP";
        if (key == L"prof") return L"Profil";
        if (key == L"newchat") return L"+ Nouveau chat";
        if (key == L"impchat") return L"Importer chat";
        if (key == L"send") return L"Envoyer";
        if (key == L"screen") return L"Image";
    }
    else if (g_langIndex == 5) {
        if (key == L"train") return L"Навчити ШІ";
        if (key == L"folder") return L"Папка даних";
        if (key == L"expzip") return L"Експорт ZIP";
        if (key == L"impzip") return L"Імпорт ZIP";
        if (key == L"prof") return L"Профіль";
        if (key == L"newchat") return L"+ Новий чат";
        if (key == L"impchat") return L"Імпорт чату";
        if (key == L"send") return L"Надіслати";
        if (key == L"screen") return L"Скріншот";
    }
    else if (g_langIndex == 6) {
        if (key == L"train") return L"训练 AI";
        if (key == L"folder") return L"数据文件夹";
        if (key == L"expzip") return L"导出 ZIP";
        if (key == L"impzip") return L"导入 ZIP";
        if (key == L"prof") return L"个人资料";
        if (key == L"newchat") return L"+ 新建聊天";
        if (key == L"impchat") return L"导入聊天";
        if (key == L"send") return L"发送";
        if (key == L"screen") return L"截图";
    }
    else if (g_langIndex == 7) {
        if (key == L"train") return L"AI 学習";
        if (key == L"folder") return L"データフォルダ";
        if (key == L"expzip") return L"ZIP 出力";
        if (key == L"impzip") return L"ZIP 読込";
        if (key == L"prof") return L"プロフィール";
        if (key == L"newchat") return L"+ 新規チャット";
        if (key == L"impchat") return L"チャット読込";
        if (key == L"send") return L"送信";
        if (key == L"screen") return L"画像";
    }
    return en;
}

std::wstring ToLowerSystem(std::wstring str) {
    if (!str.empty()) CharLowerBuffW(str.data(), (DWORD)str.size());
    return str;
}

std::wstring GetCurrentTimestampStr() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::wstring buf(32, L'\0');
    int len = swprintf_s(buf.data(), buf.size(), L"%04d-%02d-%02d %02d:%02d:%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    if (len > 0) buf.resize(len);
    return buf;
}

std::wstring FormatMessageTimeForUI(const std::wstring& rawTs) {
    if (rawTs.size() < 19) return rawTs.empty() ? L"" : (L"[" + rawTs + L"] ");
    int y = 0, m = 0, d = 0, hh = 0, mm = 0, ss = 0;
    if (swscanf_s(rawTs.c_str(), L"%d-%d-%d %d:%d:%d", &y, &m, &d, &hh, &mm, &ss) != 6) {
        return L"[" + rawTs + L"] ";
    }
    SYSTEMTIME now;
    GetLocalTime(&now);
    if (now.wYear == y && now.wMonth == m && now.wDay == d) {
        std::wstring timeOnly(24, L'\0');
        int len = swprintf_s(timeOnly.data(), timeOnly.size(), L"[%02d:%02d:%02d] ", hh, mm, ss);
        if (len > 0) timeOnly.resize(len);
        return timeOnly;
    }
    SYSTEMTIME msgSt = {};
    msgSt.wYear = (WORD)y;
    msgSt.wMonth = (WORD)m;
    msgSt.wDay = (WORD)d;
    FILETIME ft;
    SystemTimeToFileTime(&msgSt, &ft);
    FileTimeToSystemTime(&ft, &msgSt);

    const wchar_t* daysRu[] = { L"Вс", L"Пн", L"Вт", L"Ср", L"Чт", L"Пт", L"Сб" };
    const wchar_t* daysEn[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
    const wchar_t* dayName = (g_langIndex == 0) ? daysRu[msgSt.wDayOfWeek % 7] : daysEn[msgSt.wDayOfWeek % 7];

    std::wstring fullBuf(48, L'\0');
    int len = swprintf_s(fullBuf.data(), fullBuf.size(), L"[%s, %02d.%02d.%04d %02d:%02d:%02d] ",
        dayName, d, m, y, hh, mm, ss);
    if (len > 0) fullBuf.resize(len);
    return fullBuf;
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
        for (wchar_t c : w) if (iswalnum(c)) raw += c;
        return ToLowerSystem(raw);
    }

    static std::wstring StemWord(const std::wstring& w) {
        std::wstring res = NormalizeWord(w);
        if (res.size() > 5) res.resize(res.size() - 2);
        else if (res.size() == 5) res.resize(4);
        return res;
    }

    static std::wstring AnalyzeImageAsBinaryPixels(const fs::path& imgPath, const std::wstring& fileName) {
        Bitmap bmp(imgPath.c_str());
        if (bmp.GetLastStatus() != Ok) {
            return T(L"img_err", L"Не удалось прочитать пиксели изображения.", L"Failed to read image pixels.");
        }

        UINT origW = bmp.GetWidth();
        UINT origH = bmp.GetHeight();
        const int gridW = 24;
        const int gridH = 12;

        Bitmap smallBmp(gridW, gridH, PixelFormat32bppARGB);
        Graphics g(&smallBmp);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.DrawImage(&bmp, 0, 0, gridW, gridH);

        int onesCount = 0;
        int zerosCount = 0;
        std::wstring matrixStr;

        for (int y = 0; y < gridH; ++y) {
            for (int x = 0; x < gridW; ++x) {
                Color px;
                smallBmp.GetPixel(x, y, &px);
                int lum = (int)(0.299 * px.GetR() + 0.587 * px.GetG() + 0.114 * px.GetB());
                if (px.GetA() < 128 || lum >= 128) {
                    matrixStr += L'1';
                    onesCount++;
                }
                else {
                    matrixStr += L'0';
                    zerosCount++;
                }
            }
            matrixStr += L"\r\n";
        }

        std::wstringstream ss;
        if (g_langIndex == 0) {
            ss << L"Изображение «" << fileName << L"» (" << origW << L"x" << origH << L" px) проанализировано на уровне пикселей!\r\n"
                << L"Светлых зон (1): " << onesCount << L" | Тёмных зон (0): " << zerosCount << L"\r\n"
                << L"Бинарная матрица пикселей (24x12):\r\n" << matrixStr;
        }
        else {
            ss << L"Image '" << fileName << L"' (" << origW << L"x" << origH << L" px) analyzed at pixel level!\r\n"
                << L"Bright pixels (1): " << onesCount << L" | Dark pixels (0): " << zerosCount << L"\r\n"
                << L"Binary pixel matrix (24x12):\r\n" << matrixStr;
        }
        return ss.str();
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
            else if (expr[i] == L'(') ops.push_back(L'(');
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
                    if (clean.size() > 2) knowledgeSentences.push_back(clean);
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
    std::wstring GenerateResponse(const std::wstring& userInput, const std::wstring& binaryVisionReport = L"") {
        if (!binaryVisionReport.empty()) {
            return binaryVisionReport;
        }

        std::wstring lowRaw = ToLowerSystem(userInput);
        std::wstring cleanCmd;
        for (wchar_t c : lowRaw) {
            if (iswalnum(c) || c == L'/') cleanCmd += c;
        }

        if (cleanCmd == L"помощь" || cleanCmd == L"справка" || cleanCmd == L"инструкция" || cleanCmd == L"какпользоваться" ||
            cleanCmd == L"help" || cleanCmd == L"/help" || cleanCmd == L"hilfe" || cleanCmd == L"ayuda" ||
            cleanCmd == L"aide" || cleanCmd == L"допомога" || cleanCmd == L"帮助" || cleanCmd == L"ヘルプ") {

            if (g_langIndex == 0) {
                return L"📖 ИНСТРУКЦИЯ ПО MINI AI STUDIO 6.0:\r\n"
                    L"• Обучение (Ai.dat): Нажми «Обучить ИИ» сверху, чтобы редактировать базу знаний (Ctrl+A — выделить всё) или загрузить готовый файл Ai.dat.\r\n"
                    L"• Скриншоты: Перетащи картинку в окно или нажми «Скрин» (до 25 МБ) — она сохранится как 0.png, 1.png и разложится на карту пикселей (0 и 1).\r\n"
                    L"• Поиск и горячие клавиши: Ctrl+F — поиск слов в чате; Enter — отправить сообщение; Delete на чате слева — удалить диалог.\r\n"
                    L"• Профили и архивы: В «Профиле» можно задать пароль и GIF-аватар (512x512), а кнопки «Экспорт/Импорт ZIP» сохраняют или переносят весь аккаунт целиком.";
            }
            else if (g_langIndex == 2) {
                return L"📖 ANLEITUNG FÜR MINI AI STUDIO 6.0:\r\n"
                    L"• KI trainieren: Klicken Sie oben auf «KI trainieren», um Ai.dat zu bearbeiten (Ctrl+A = Alles auswählen) oder eine fertige Ai.dat zu laden.\r\n"
                    L"• Screenshots: Ziehen Sie ein Bild ins Fenster (max. 25 MB) — es wird als 0.png, 1.png gespeichert und als Pixelkarte (0 und 1) analysiert.\r\n"
                    L"• Tastenkürzel: Ctrl+F = Im Chat suchen; Enter = Senden; Entf (Delete) auf einem Chat = Chat löschen.\r\n"
                    L"• Profile & ZIP: Im Profil können Sie ein Passwort und GIF-Avatar festlegen sowie alles als ZIP exportieren/importieren.";
            }
            else if (g_langIndex == 3) {
                return L"📖 GUÍA DE USO DE MINI AI STUDIO 6.0:\r\n"
                    L"• Entrenar IA: Pulsa «Entrenar IA» arriba para editar la base Ai.dat (Ctrl+A = seleccionar todo) o cargar un archivo Ai.dat.\r\n"
                    L"• Capturas: Arrastra una imagen a la ventana (máx. 25 MB) — se guardará como 0.png, 1.png y se analizará en píxeles (0 y 1).\r\n"
                    L"• Atajos: Ctrl+F = buscar en el chat; Enter = enviar; Supr (Delete) en la lista = eliminar chat.\r\n"
                    L"• Perfiles y ZIP: Configura contraseña y avatar GIF en «Perfil», y usa Exportar/Importar ZIP para respaldar tus datos.";
            }
            else if (g_langIndex == 4) {
                return L"📖 GUIDE D'UTILISATION DE MINI AI STUDIO 6.0:\r\n"
                    L"• Entraîner l'IA : Cliquez sur «Entraîner IA» pour modifier Ai.dat (Ctrl+A = tout sélectionner) ou charger un fichier Ai.dat.\r\n"
                    L"• Captures d'écran : Glissez une image dans la fenêtre (max 25 Mo) — enregistrée en 0.png, 1.png et analysée en pixels (0 et 1).\r\n"
                    L"• Raccourcis : Ctrl+F = rechercher dans le chat ; Entrée = envoyer ; Suppr (Delete) = supprimer la discussion.\r\n"
                    L"• Profils & ZIP : Définissez un mot de passe et un avatar GIF dans «Profil», et utilisez Exporter/Importer ZIP.";
            }
            else if (g_langIndex == 5) {
                return L"📖 ІНСТРУКЦІЯ ДО MINI AI STUDIO 6.0:\r\n"
                    L"• Навчання (Ai.dat): Натисни «Навчити ШІ» зверху для редагування бази знань (Ctrl+A — виділити все) або завантаження готового Ai.dat.\r\n"
                    L"• Скріншоти: Перетягни картинку у вікно (до 25 МБ) — вона збережеться як 0.png, 1.png та розкладеться на карту пікселів (0 і 1).\r\n"
                    L"• Гарячі клавіші: Ctrl+F — пошук у чаті; Enter — надіслати; Delete на чаті зліва — видалити діалог.\r\n"
                    L"• Профілі та ZIP: У «Профілі» можна встановити пароль та GIF-аватар, а Експорт/Імпорт ZIP переносить увесь акаунт.";
            }
            else if (g_langIndex == 6) {
                return L"📖 MINI AI STUDIO 6.0 使用指南：\r\n"
                    L"• 训练 AI：点击顶部的“训练 AI”编辑 Ai.dat 知识库（Ctrl+A 全选）或加载现成的 Ai.dat 文件。\r\n"
                    L"• 截图分析：将图片拖入窗口（最大 25 MB），系统会自动保存为 0.png、1.png 并生成 0 和 1 像素矩阵。\r\n"
                    L"• 快捷键：Ctrl+F 在聊天中搜索；Enter 发送消息；选中左侧聊天按 Delete 删除对话。\r\n"
                    L"• 个人资料与 ZIP：可在“个人资料”中设置密码和 GIF 头像，支持一键导出/导入 ZIP 备份。";
            }
            else if (g_langIndex == 7) {
                return L"📖 MINI AI STUDIO 6.0 使い方ガイド：\r\n"
                    L"• AI学習：上部の「AI 学習」から Ai.dat 知識ベースを編集（Ctrl+A で全選択）または既存の Ai.dat を読み込めます。\r\n"
                    L"• スクリーンショット：画像をウィンドウにドラッグ（最大 25MB）すると 0.png、1.png として保存され、0 と 1 のピクセルマップを解析します。\r\n"
                    L"• ショートカット：Ctrl+F でチャット内検索、Enter で送信、左リストで Delete キーを押すとチャットを削除します。\r\n"
                    L"• プロフィールとZIP：「プロフィール」でパスワードやGIFアバターを設定でき、ZIPで丸ごと出力・読込が可能です。";
            }
            else {
                return L"📖 MINI AI STUDIO 6.0 USER GUIDE:\r\n"
                    L"• Train AI (Ai.dat): Click 'Train AI' at the top to edit the knowledge base (Ctrl+A to select all) or load an existing Ai.dat file.\r\n"
                    L"• Screenshots: Drag & drop an image into the window or click 'Image' (up to 25 MB) — saved as 0.png, 1.png and analyzed as a 0/1 pixel map.\r\n"
                    L"• Shortcuts: Ctrl+F to search inside the chat; Enter to send; Delete on a chat in the left list to remove it.\r\n"
                    L"• Profiles & ZIP: Set a password and GIF avatar (512x512) in 'Profile', and use Export/Import ZIP to back up or transfer your account.";
            }
        }

        double mathVal = 0.0;
        if (TrySolveMath(userInput, mathVal)) {
            std::wstringstream res;
            res << mathVal;
            return T(L"math", L"Точный расчёт: " + res.str(), L"Exact calculation: " + res.str());
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
            if (norm.size() >= 3) inputStems.push_back(StemWord(norm));
        }

        static std::mt19937 rng((unsigned int)GetTickCount64());
        int bestScore = 0;
        StrVec bestReplies;

        for (size_t i = 0; i < knowledgeSentences.size(); ++i) {
            const auto& sent = knowledgeSentences[i];
            std::wstring lowerSent = ToLowerSystem(sent);
            int score = 0;

            for (const auto& stem : inputStems) {
                if (stem.size() >= 3 && lowerSent.find(stem) != std::wstring::npos) score += (int)stem.size();
            }

            if (score > 0) {
                std::wstring candidate = sent;
                if ((i % 2 == 0) && (i + 1 < knowledgeSentences.size())) {
                    candidate = knowledgeSentences[i + 1];
                }
                if (score > bestScore) {
                    bestScore = score;
                    bestReplies.clear();
                    bestReplies.push_back(candidate);
                }
                else if (score == bestScore) bestReplies.push_back(candidate);
            }
        }

        if (!bestReplies.empty()) {
            std::uniform_int_distribution<size_t> dist(0, bestReplies.size() - 1);
            return bestReplies[dist(rng)];
        }

        return T(L"no_ans",
            L"В моей текущей базе Ai.dat пока нет ответа на этот вопрос. Нажми «Обучить ИИ» сверху или напиши «помощь» для инструкции!",
            L"My current Ai.dat base doesn't have an answer for this yet. Click 'Train AI' above or type 'help' for instructions!");
    }

    void InitDefaultKnowledge() {
        Clear();
        TrainOnText(L"Привет!\n"
            L"Рад тебя видеть, чем могу тебе помочь?\n"
            L"Как дела?\n"
            L"Всё отлично, работаю на полную мощность и готов к задачам!");
    }
};
SafeProfile g_user;
SafeProfile g_tempUser;
MiniAIBrain g_brain;
ChatVec g_chats;
int g_currentChatIdx = -1;
std::wstring g_pendingUserPassCheck;
fs::path g_pendingImagePath;

size_t g_lastFindPos = 0;
std::wstring g_lastFindQuery;

BmpVec g_mainAnimBmps;
size_t g_mainFrameIdx = 0;

BmpVec g_tempAnimBmps;
size_t g_tempFrameIdx = 0;

HWND g_hMainWnd = NULL;
HWND g_hEditFirstName = NULL, g_hEditFirstPass = NULL, g_hBtnCreateFirst = NULL;
HWND g_hBtnEditProfile = NULL, g_hListChats = NULL, g_hBtnNewChat = NULL;
HWND g_hBtnImportChat = NULL, g_hBtnOpenFolder = NULL;
HWND g_hBtnExportZip = NULL, g_hBtnImportZip = NULL, g_hComboLang = NULL;
HWND g_hEditChatHist = NULL, g_hEditMsgInput = NULL, g_hBtnSendMsg = NULL, g_hBtnAttachImg = NULL;
HWND g_hBtnTrainAI = NULL;

HFONT g_hFontNormal, g_hFontBold, g_hFontTitle;
HICON g_hAppIcon = NULL;
ULONG_PTR g_gdiplusToken;

WNDPROC g_origMsgInputProc = NULL;
WNDPROC g_origChatListProc = NULL;
WNDPROC g_origTrainEditProc = NULL;

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
    fs::create_directories(userDir / L"screenshots", ec);
    return userDir;
}

void CleanupTempProfileIfNeeded() {
    if (g_user.exists && g_user.isTemporary) {
        std::error_code ec;
        fs::remove_all(GetBaseAppFolder() / L"Users" / g_user.username, ec);
        g_user.isTemporary = false;
    }
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

bool SaveBitmapToPngBytes(Bitmap& bmp, ByteVec& outPng) {
    IStream* pStream = NULL;
    if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) != S_OK) return false;
    CLSID pngClsid;
    GetPngEncoderClsid(&pngClsid);
    if (bmp.Save(pStream, &pngClsid, NULL) != Ok) {
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

void DrawDefaultWin8Avatar(HDC hdc, RECT rc) {
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    SolidBrush bgBrush(Color(255, 224, 224, 224));
    g.FillRectangle(&bgBrush, rc.left, rc.top, w, h);

    SolidBrush personBrush(Color(255, 165, 165, 165));
    float headSize = w * 0.36f;
    float headX = rc.left + (w - headSize) / 2.0f;
    float headY = rc.top + h * 0.18f;
    g.FillEllipse(&personBrush, headX, headY, headSize, headSize);

    g.SetClip(Rect(rc.left, rc.top, w, h));
    float bodyW = w * 0.72f;
    float bodyH = h * 0.50f;
    float bodyX = rc.left + (w - bodyW) / 2.0f;
    float bodyY = rc.top + h * 0.60f;
    g.FillPie(&personBrush, bodyX, bodyY, bodyW, bodyH, 180.0f, 180.0f);
    g.ResetClip();
}

bool EncodeCurrentFrameToPng512(Image& img, ByteVec& outPng) {
    Bitmap resizedBmp(512, 512, PixelFormat32bppARGB);
    Graphics g(&resizedBmp);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.DrawImage(&img, 0, 0, 512, 512);
    return SaveBitmapToPngBytes(resizedBmp, outPng);
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
        if (EncodeCurrentFrameToPng512(img, frame.pngData)) outFrames.push_back(std::move(frame));
    }
    return !outFrames.empty();
}

HBITMAP CreateScaledHBitmap(const ByteVec& pngBytes, int targetSize) {
    if (pngBytes.empty()) return NULL;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, pngBytes.size());
    if (!hMem) return NULL;
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
    for (const auto& f : frames) outBmps.push_back(CreateScaledHBitmap(f.pngData, size));
}
bool ValidateAndCopyImageToProfile(HWND hwnd, const fs::path& srcPath, fs::path& outSavedPath, std::wstring& outFileName) {
    std::error_code ec;
    if (!fs::exists(srcPath, ec) || !fs::is_regular_file(srcPath, ec)) return false;

    uintmax_t fileSize = fs::file_size(srcPath, ec);
    const uintmax_t max25MB = 25ULL * 1024ULL * 1024ULL;
    if (fileSize > max25MB) {
        MessageBoxW(hwnd,
            T(L"err_size", L"Ошибка: размер изображения превышает лимит 25 МБ!", L"Error: image file size exceeds 25 MB limit!").c_str(),
            L"25 MB Limit", MB_ICONWARNING);
        return false;
    }

    fs::path scrDir = GetUserFolder(g_user.username) / L"screenshots";
    int nextIndex = 0;
    while (fs::exists(scrDir / (std::to_wstring(nextIndex) + L".png"), ec)) {
        nextIndex++;
    }

    outFileName = std::to_wstring(nextIndex) + L".png";
    outSavedPath = scrDir / outFileName;

    Image img(srcPath.c_str());
    if (img.GetLastStatus() != Ok) return false;

    CLSID pngClsid;
    GetPngEncoderClsid(&pngClsid);
    return (img.Save(outSavedPath.c_str(), &pngClsid, NULL) == Ok);
}

void ExportProfileAsZip(HWND hwnd) {
    std::wstring szFile(MAX_PATH, L'\0');
    wcscpy_s(szFile.data(), MAX_PATH, (g_user.username + L"_Backup.zip").c_str());

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"ZIP Archive (*.zip)\0*.zip\0";
    ofn.lpstrDefExt = L"zip";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameW(&ofn)) {
        fs::path userDir = GetUserFolder(g_user.username);
        std::wstring cmd = L"tar.exe -a -cf \"" + std::wstring(szFile.c_str()) + L"\" -C \"" + userDir.wstring() + L"\" .";
        STARTUPINFOW si = { sizeof(si) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};
        if (CreateProcessW(NULL, cmd.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 10000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            MessageBoxW(hwnd,
                T(L"zip_ok", L"Профиль, чаты, скриншоты и Ai.dat успешно упакованы в ZIP-архив!", L"Profile, chats, screenshots, and Ai.dat exported to ZIP!").c_str(),
                L"ZIP", MB_ICONINFORMATION);
        }
    }
}
void SaveCustomLanguage() {
    fs::path file = GetBaseAppFolder() / L"lang.dat";
    ByteVec buf;
    WriteUint32(buf, 0x4C414E47); // "LANG"
    WriteUint32(buf, (UINT32)g_customLangMap.size());
    for (const auto& kv : g_customLangMap) {
        WriteWString(buf, kv.first);
        WriteWString(buf, kv.second);
    }
    SaveEncryptedFile(file, buf);
}

void LoadCustomLanguage() {
    fs::path file = GetBaseAppFolder() / L"lang.dat";
    ByteVec buf;
    if (!LoadDecryptedFile(file, buf)) return;
    size_t offset = 0;
    UINT32 magic = 0, count = 0;
    if (!ReadUint32(buf, offset, magic) || magic != 0x4C414E47) return;
    if (!ReadUint32(buf, offset, count)) return;
    g_customLangMap.clear();
    for (UINT32 i = 0; i < count; ++i) {
        std::wstring k, v;
        if (ReadWString(buf, offset, k) && ReadWString(buf, offset, v)) g_customLangMap[k] = v;
    }
}

void SaveUserProfile(const SafeProfile& user) {
    fs::path file = GetUserFolder(user.username) / (user.username + L".dat");
    ByteVec buf;
    WriteUint32(buf, 0x50524F33); // "PRO3"
    WriteWString(buf, user.username);
    WriteWString(buf, user.password);
    WriteWString(buf, user.description);

    WriteUint32(buf, (UINT32)user.frames.size());
    for (const auto& f : user.frames) {
        WriteUint32(buf, f.delayMs);
        WriteUint32(buf, (UINT32)f.pngData.size());
        if (!f.pngData.empty()) buf.insert(buf.end(), f.pngData.begin(), f.pngData.end());
    }
    SaveEncryptedFile(file, buf);
}

bool LoadUserProfile(const std::wstring& username, SafeProfile& outUser) {
    fs::path file = GetBaseAppFolder() / L"Users" / username / (username + L".dat");
    ByteVec buf;
    if (!LoadDecryptedFile(file, buf)) return false;

    size_t offset = 0;
    UINT32 magic = 0;
    if (!ReadUint32(buf, offset, magic)) return false;

    if (magic == 0x50524F33) {
        if (!ReadWString(buf, offset, outUser.username)) return false;
        if (!ReadWString(buf, offset, outUser.password)) return false;
        if (!ReadWString(buf, offset, outUser.description)) return false;
    }
    else if (magic == 0x50524F46) {
        if (!ReadWString(buf, offset, outUser.username)) return false;
        outUser.password.clear();
        if (!ReadWString(buf, offset, outUser.description)) return false;
    }
    else {
        return false;
    }

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
    outUser.isTemporary = false;
    return true;
}

void SaveUserBrain(const std::wstring& username) {
    fs::path file = GetUserFolder(username) / L"Ai.dat";
    ByteVec buf;
    WriteUint32(buf, 0x41494441); // "AIDA"

    WriteUint32(buf, (UINT32)g_brain.knowledgeSentences.size());
    for (const auto& s : g_brain.knowledgeSentences) WriteWString(buf, s);

    WriteUint32(buf, (UINT32)g_brain.markovChain.size());
    for (const auto& pair : g_brain.markovChain) {
        WriteWString(buf, pair.first);
        WriteUint32(buf, (UINT32)pair.second.size());
        for (const auto& nextW : pair.second) WriteWString(buf, nextW);
    }
    SaveEncryptedFile(file, buf);
}

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
    WriteUint32(buf, 0x43484134); // "CHA4"
    WriteWString(buf, chat.title);
    WriteWString(buf, chat.description);
    WriteUint32(buf, (UINT32)chat.messages.size());
    for (const auto& m : chat.messages) {
        WriteUint32(buf, m.isUser ? 1 : 0);
        WriteWString(buf, m.text);
        WriteWString(buf, m.imageFile);
        WriteWString(buf, m.timestamp);
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
            if (!ReadUint32(buf, offset, magic)) continue;
            if (magic != 0x43484134 && magic != 0x43484133 && magic != 0x43484132) continue;

            ChatSession chat;
            chat.id = entry.path().stem().wstring();
            if (!ReadWString(buf, offset, chat.title)) continue;
            if (!ReadWString(buf, offset, chat.description)) continue;

            UINT32 msgCount = 0;
            if (!ReadUint32(buf, offset, msgCount)) continue;
            for (UINT32 i = 0; i < msgCount; ++i) {
                UINT32 isU = 0;
                std::wstring txt, imgF, ts;
                if (!ReadUint32(buf, offset, isU) || !ReadWString(buf, offset, txt)) break;
                if (magic >= 0x43484133) ReadWString(buf, offset, imgF);
                if (magic >= 0x43484134) ReadWString(buf, offset, ts);
                chat.messages.push_back({ isU == 1, std::move(txt), std::move(imgF), std::move(ts) });
            }
            g_chats.push_back(std::move(chat));
        }
    }

    if (g_chats.empty()) {
        ChatSession first;
        first.id = L"chat_1";
        first.title = T(L"def_chat", L"Основной диалог", L"Main Chat");
        first.messages.push_back({ false, T(L"welcome_msg",
            L"Привет! Я готов к работе. Напиши «помощь» для инструкции, ищи по чату через Ctrl+F или перетаскивай скриншоты!",
            L"Hello! Ready to work. Type 'help' for guide, search with Ctrl+F, or drop screenshots!"), L"", GetCurrentTimestampStr() });
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
            if (uname == L"TempGuest") continue;
            if (fs::exists(entry.path() / (uname + L".dat"), ec)) users.push_back(uname);
        }
    }
    return users;
}
void StartMainAvatarAnimation(HWND hwnd) {
    KillTimer(hwnd, TIMER_MAIN_GIF);
    g_mainFrameIdx = 0;
    RebuildBitmapList(g_user.frames, g_mainAnimBmps, 44);
    if (g_user.frames.size() > 1) SetTimer(hwnd, TIMER_MAIN_GIF, g_user.frames.front().delayMs, NULL);
}

void StartTempAvatarAnimation(HWND hwnd) {
    KillTimer(hwnd, TIMER_TEMP_GIF);
    g_tempFrameIdx = 0;
    RebuildBitmapList(g_tempUser.frames, g_tempAnimBmps, 120);
    if (g_tempUser.frames.size() > 1) SetTimer(hwnd, TIMER_TEMP_GIF, g_tempUser.frames.front().delayMs, NULL);
}

void RefreshChatUI() {
    SendMessageW(g_hListChats, LB_RESETCONTENT, 0, 0);
    for (const auto& c : g_chats) SendMessageW(g_hListChats, LB_ADDSTRING, 0, (LPARAM)c.title.c_str());

    if (g_currentChatIdx >= 0 && g_currentChatIdx < (int)g_chats.size()) {
        SendMessageW(g_hListChats, LB_SETCURSEL, g_currentChatIdx, 0);

        std::wstring fullText;
        if (!g_chats[g_currentChatIdx].description.empty()) {
            fullText += L"[" + g_chats[g_currentChatIdx].description + L"]\r\n----------------------------------------\r\n\r\n";
        }
        for (const auto& m : g_chats[g_currentChatIdx].messages) {
            fullText += FormatMessageTimeForUI(m.timestamp);
            fullText += (m.isUser ? (g_user.username + L": ") : L"AI: ");
            if (!m.imageFile.empty()) fullText += L"[Screenshot: " + m.imageFile + L"] ";
            fullText += m.text + L"\r\n\r\n";
        }
        SetWindowTextW(g_hEditChatHist, fullText.c_str());
        int len = GetWindowTextLengthW(g_hEditChatHist);
        SendMessageW(g_hEditChatHist, EM_SETSEL, len, len);
        SendMessageW(g_hEditChatHist, EM_SCROLLCARET, 0, 0);
    }
    else {
        SetWindowTextW(g_hEditChatHist, L"");
    }
}

void CompleteUserLogin(HWND hwnd, const SafeProfile& loadedProf) {
    CleanupTempProfileIfNeeded();
    g_user = loadedProf;
    LoadUserBrain(g_user.username);
    LoadUserChats(g_user.username);
    StartMainAvatarAnimation(hwnd);
    RefreshChatUI();
}

void SwitchToUser(HWND hwnd, const std::wstring& username) {
    SafeProfile tempProf;
    if (!LoadUserProfile(username, tempProf)) return;

    if (!tempProf.password.empty()) {
        g_pendingUserPassCheck = username;
        EnableWindow(hwnd, FALSE);
        CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAIPasswordClass", L"Password",
            WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 340, 175,
            hwnd, NULL, GetModuleHandle(NULL), NULL);
    }
    else {
        CompleteUserLogin(hwnd, tempProf);
    }
}

void StartGuestProfile(HWND hwnd) {
    CleanupTempProfileIfNeeded();
    std::error_code ec;
    fs::remove_all(GetBaseAppFolder() / L"Users" / L"TempGuest", ec);

    g_user = SafeProfile{};
    g_user.exists = true;
    g_user.isTemporary = true;
    g_user.username = L"TempGuest";
    g_user.description = T(L"guest_bio", L"Временный гостевой аккаунт", L"Temporary guest account");

    SaveUserProfile(g_user);
    g_brain.InitDefaultKnowledge();
    SaveUserBrain(g_user.username);
    LoadUserChats(g_user.username);
    StartMainAvatarAnimation(hwnd);
    RefreshChatUI();
}

void ImportExternalChat(HWND hwnd) {
    std::wstring szFile(MAX_PATH, L'\0');
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Encrypted Chat (*.dat)\0*.dat\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        ByteVec buf;
        if (!LoadDecryptedFile(szFile.c_str(), buf)) return;
        size_t offset = 0;
        UINT32 magic = 0;
        if (!ReadUint32(buf, offset, magic) || (magic != 0x43484134 && magic != 0x43484133 && magic != 0x43484132)) {
            MessageBoxW(hwnd, L"Invalid chat .dat file!", L"Error", MB_ICONERROR);
            return;
        }
        ChatSession imported;
        imported.id = L"chat_" + std::to_wstring(GetTickCount64());
        if (!ReadWString(buf, offset, imported.title) || !ReadWString(buf, offset, imported.description)) return;
        UINT32 msgCount = 0;
        if (!ReadUint32(buf, offset, msgCount)) return;
        for (UINT32 i = 0; i < msgCount; ++i) {
            UINT32 isU = 0;
            std::wstring txt, imgF, ts;
            if (!ReadUint32(buf, offset, isU) || !ReadWString(buf, offset, txt)) break;
            if (magic >= 0x43484133) ReadWString(buf, offset, imgF);
            if (magic >= 0x43484134) ReadWString(buf, offset, ts);
            imported.messages.push_back({ isU == 1, std::move(txt), std::move(imgF), std::move(ts) });
        }
        SaveChatSession(g_user.username, imported);
        g_chats.push_back(std::move(imported));
        g_currentChatIdx = (int)g_chats.size() - 1;
        RefreshChatUI();
    }
}

void ImportProfileFromZip(HWND hwnd) {
    std::wstring szFile(MAX_PATH, L'\0');
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"ZIP Archive (*.zip)\0*.zip\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        fs::path tempDir = GetBaseAppFolder() / L"TempZipImport";
        std::error_code ec;
        fs::remove_all(tempDir, ec);
        fs::create_directories(tempDir, ec);

        std::wstring cmd = L"tar.exe -xf \"" + std::wstring(szFile.c_str()) + L"\" -C \"" + tempDir.wstring() + L"\"";
        STARTUPINFOW si = { sizeof(si) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi = {};
        if (CreateProcessW(NULL, cmd.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 10000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            for (const auto& entry : fs::directory_iterator(tempDir, ec)) {
                if (entry.is_regular_file() && entry.path().extension() == L".dat" && entry.path().filename() != L"Ai.dat") {
                    std::wstring uname = entry.path().stem().wstring();
                    fs::path destUserDir = GetBaseAppFolder() / L"Users" / uname;
                    fs::remove_all(destUserDir, ec);
                    fs::rename(tempDir, destUserDir, ec);
                    SwitchToUser(hwnd, uname);
                    return;
                }
            }
        }
        fs::remove_all(tempDir, ec);
    }
}
void ApplyLanguageToControls() {
    SetWindowTextW(g_hBtnCreateFirst, T(L"create_acc", L"Создать аккаунт", L"Create Account").c_str());
    SetWindowTextW(g_hBtnTrainAI, T(L"train", L"Обучить ИИ", L"Train AI").c_str());
    SetWindowTextW(g_hBtnOpenFolder, T(L"folder", L"Папка данных", L"Data Folder").c_str());
    SetWindowTextW(g_hBtnExportZip, T(L"expzip", L"Экспорт ZIP", L"Export ZIP").c_str());
    SetWindowTextW(g_hBtnImportZip, T(L"impzip", L"Импорт ZIP", L"Import ZIP").c_str());
    SetWindowTextW(g_hBtnEditProfile, T(L"prof", L"Профиль", L"Profile").c_str());
    SetWindowTextW(g_hBtnNewChat, T(L"newchat", L"+ Новый чат", L"+ New Chat").c_str());
    SetWindowTextW(g_hBtnImportChat, T(L"impchat", L"Импорт чата (.dat)", L"Import Chat (.dat)").c_str());
    SetWindowTextW(g_hBtnAttachImg, T(L"screen", L"Скрин", L"Image").c_str());
    SetWindowTextW(g_hBtnSendMsg, T(L"send", L"Отправить", L"Send").c_str());
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
        ShowWindow(g_hBtnExportZip, SW_HIDE);
        ShowWindow(g_hBtnImportZip, SW_HIDE);
        ShowWindow(g_hComboLang, SW_HIDE);
        ShowWindow(g_hEditChatHist, SW_HIDE);
        ShowWindow(g_hEditMsgInput, SW_HIDE);
        ShowWindow(g_hBtnSendMsg, SW_HIDE);
        ShowWindow(g_hBtnAttachImg, SW_HIDE);
        ShowWindow(g_hBtnTrainAI, SW_HIDE);

        ShowWindow(g_hEditFirstName, SW_SHOW);
        ShowWindow(g_hEditFirstPass, SW_SHOW);
        ShowWindow(g_hBtnCreateFirst, SW_SHOW);
        MoveWindow(g_hEditFirstName, (w - 280) / 2, 220, 280, 28, TRUE);
        MoveWindow(g_hEditFirstPass, (w - 280) / 2, 260, 280, 28, TRUE);
        MoveWindow(g_hBtnCreateFirst, (w - 180) / 2, 305, 180, 34, TRUE);
    }
    else {
        ShowWindow(g_hEditFirstName, SW_HIDE);
        ShowWindow(g_hEditFirstPass, SW_HIDE);
        ShowWindow(g_hBtnCreateFirst, SW_HIDE);

        ShowWindow(g_hBtnEditProfile, SW_SHOW);
        ShowWindow(g_hListChats, SW_SHOW);
        ShowWindow(g_hBtnNewChat, SW_SHOW);
        ShowWindow(g_hBtnImportChat, SW_SHOW);
        ShowWindow(g_hBtnOpenFolder, SW_SHOW);
        ShowWindow(g_hBtnExportZip, SW_SHOW);
        ShowWindow(g_hBtnImportZip, SW_SHOW);
        ShowWindow(g_hComboLang, SW_SHOW);
        ShowWindow(g_hEditChatHist, SW_SHOW);
        ShowWindow(g_hEditMsgInput, SW_SHOW);
        ShowWindow(g_hBtnSendMsg, SW_SHOW);
        ShowWindow(g_hBtnAttachImg, SW_SHOW);
        ShowWindow(g_hBtnTrainAI, SW_SHOW);

        MoveWindow(g_hBtnTrainAI, 10, 14, 115, 32, TRUE);
        MoveWindow(g_hBtnOpenFolder, 130, 14, 120, 32, TRUE);
        MoveWindow(g_hBtnExportZip, 255, 14, 105, 32, TRUE);
        MoveWindow(g_hBtnImportZip, 365, 14, 105, 32, TRUE);
        MoveWindow(g_hComboLang, 475, 16, 120, 200, TRUE);
        MoveWindow(g_hBtnEditProfile, w - 165, 14, 105, 32, TRUE);

        int sideW = 200;
        MoveWindow(g_hBtnNewChat, 10, 70, sideW - 20, 30, TRUE);
        MoveWindow(g_hBtnImportChat, 10, 106, sideW - 20, 30, TRUE);
        MoveWindow(g_hListChats, 10, 144, sideW - 20, max(50, h - 154), TRUE);

        int chatX = sideW + 10;
        int chatW = max(100, w - chatX - 15);
        MoveWindow(g_hEditChatHist, chatX, 70, chatW, max(50, h - 135), TRUE);
        MoveWindow(g_hEditMsgInput, chatX, h - 52, max(80, chatW - 185), 36, TRUE);
        MoveWindow(g_hBtnAttachImg, w - 190, h - 52, 70, 36, TRUE);
        MoveWindow(g_hBtnSendMsg, w - 115, h - 52, 100, 36, TRUE);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}
void FindNextInChat(HWND dlgHwnd, const std::wstring& query) {
    if (query.empty()) return;

    int len = GetWindowTextLengthW(g_hEditChatHist);
    if (len <= 0) return;

    std::wstring text(len + 1, L'\0');
    GetWindowTextW(g_hEditChatHist, text.data(), len + 1);
    text.resize(len);

    std::wstring lowText = ToLowerSystem(text);
    std::wstring lowQuery = ToLowerSystem(query);

    if (lowQuery != g_lastFindQuery) {
        g_lastFindQuery = lowQuery;
        g_lastFindPos = 0;
    }

    size_t found = lowText.find(lowQuery, g_lastFindPos);
    if (found == std::wstring::npos && g_lastFindPos > 0) {
        found = lowText.find(lowQuery, 0);
    }

    if (found != std::wstring::npos) {
        g_lastFindPos = found + lowQuery.size();
        SendMessageW(g_hEditChatHist, EM_SETSEL, (WPARAM)found, (LPARAM)(found + lowQuery.size()));
        SendMessageW(g_hEditChatHist, EM_SCROLLCARET, 0, 0);
        SetFocus(g_hEditChatHist);
    }
    else {
        MessageBoxW(dlgHwnd,
            T(L"not_found", L"Совпадений в этом чате не найдено.", L"No matches found in this chat.").c_str(),
            L"Ctrl + F", MB_ICONINFORMATION);
    }
}
LRESULT CALLBACK FindWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditFind;
    switch (uMsg) {
    case WM_CREATE: {
        std::wstring lbl = T(L"fnd_lbl", L"Найти в чате:", L"Find in chat:");
        std::wstring bNx = T(L"fnd_nxt", L"Найти далее", L"Find Next");
        std::wstring bCl = T(L"fnd_cls", L"Закрыть", L"Close");

        CreateWindowW(L"STATIC", lbl.c_str(), WS_VISIBLE | WS_CHILD, 15, 15, 290, 20, hwnd, NULL, NULL, NULL);
        hEditFind = CreateWindowW(L"EDIT", g_lastFindQuery.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            15, 40, 290, 26, hwnd, (HMENU)ID_FIND_INPUT, NULL, NULL);

        CreateWindowW(L"BUTTON", bNx.c_str(), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            105, 80, 105, 30, hwnd, (HMENU)ID_FIND_NEXT, NULL, NULL);
        CreateWindowW(L"BUTTON", bCl.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            220, 80, 85, 30, hwnd, (HMENU)ID_FIND_CLOSE, NULL, NULL);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);

        SetFocus(hEditFind);
        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == ID_FIND_NEXT) {
            int qLen = GetWindowTextLengthW(hEditFind);
            std::wstring q(qLen + 1, L'\0');
            if (qLen > 0) GetWindowTextW(hEditFind, q.data(), qLen + 1);
            q.resize(qLen);
            FindNextInChat(hwnd, q);
        }
        else if (id == ID_FIND_CLOSE) {
            DestroyWindow(hwnd);
        }
        return 0;
    }

    case WM_DESTROY:
        SetForegroundWindow(g_hMainWnd);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void OpenChatSearchDialog(HWND parent) {
    if (!g_user.exists) return;
    std::wstring tit = T(L"fnd_tit", L"Поиск по чату (Ctrl + F)", L"Search in Chat (Ctrl + F)");
    CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, L"MiniAIFindClass", tit.c_str(),
        WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 335, 155,
        parent, NULL, GetModuleHandle(NULL), NULL);
}
bool ImportExternalUserProfile(HWND hwnd) {
    std::wstring szFile(MAX_PATH, L'\0');
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = (g_langIndex == 0)
        ? L"Файл профиля пользователя (*.dat)\0*.dat\0"
        : L"User Profile File (*.dat)\0*.dat\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (!GetOpenFileNameW(&ofn)) return false;

    fs::path srcDatPath(szFile.c_str());
    ByteVec buf;
    if (!LoadDecryptedFile(srcDatPath, buf)) {
        MessageBoxW(hwnd, T(L"err_read", L"Не удалось прочитать файл!", L"Failed to read file!").c_str(), L"Error", MB_ICONERROR);
        return false;
    }

    size_t offset = 0;
    UINT32 magic = 0;
    if (!ReadUint32(buf, offset, magic) || (magic != 0x50524F46 && magic != 0x50524F33)) {
        MessageBoxW(hwnd,
            T(L"err_prof", L"Ошибка: выбранный .dat файл не является профилем пользователя!",
                L"Error: selected .dat file is not a valid user profile!").c_str(),
            L"Error", MB_ICONERROR);
        return false;
    }

    std::wstring importedName;
    if (!ReadWString(buf, offset, importedName) || importedName.empty()) {
        importedName = srcDatPath.stem().wstring();
    }

    fs::path destUserDir = GetUserFolder(importedName);
    fs::path destDatPath = destUserDir / (importedName + L".dat");

    std::error_code ec;
    fs::copy_file(srcDatPath, destDatPath, fs::copy_options::overwrite_existing, ec);

    fs::path srcDir = srcDatPath.parent_path();
    fs::path srcChatsDir = srcDir / L"chats";
    if (fs::exists(srcChatsDir, ec) && fs::is_directory(srcChatsDir, ec)) {
        for (const auto& entry : fs::directory_iterator(srcChatsDir, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == L".dat") {
                fs::copy_file(entry.path(), destUserDir / L"chats" / entry.path().filename(),
                    fs::copy_options::overwrite_existing, ec);
            }
        }
    }

    fs::path srcAiDat = srcDir / L"Ai.dat";
    if (fs::exists(srcAiDat, ec) && fs::is_regular_file(srcAiDat, ec)) {
        fs::copy_file(srcAiDat, destUserDir / L"Ai.dat", fs::copy_options::overwrite_existing, ec);
    }

    std::wstring okMsg = T(L"imp_p1", L"Профиль «", L"Profile '") + importedName + T(L"imp_p2", L"» успешно импортирован!", L"' successfully imported!");
    MessageBoxW(hwnd, okMsg.c_str(), T(L"imp_tit", L"Импорт завершён", L"Import Complete").c_str(), MB_ICONINFORMATION);
    return true;
}

LRESULT CALLBACK UserSelectWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hListUsers;
    switch (uMsg) {
    case WM_CREATE: {
        std::wstring lbl = T(L"sel_lbl", L"Выберите аккаунт для входа:", L"Select an account to sign in:");
        std::wstring bIn = T(L"sel_in", L"Войти", L"Sign In");
        std::wstring bImp = T(L"sel_imp", L"Импорт профиля", L"Import Profile");
        std::wstring bNw = T(L"sel_new", L"+ Новый аккаунт", L"+ New Account");
        std::wstring bGst = T(L"sel_gst", L"Временный профиль (Гость)", L"Temporary Profile (Guest)");

        CreateWindowW(L"STATIC", lbl.c_str(), WS_VISIBLE | WS_CHILD, 20, 15, 340, 22, hwnd, NULL, NULL, NULL);
        hListUsers = CreateWindowW(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            20, 45, 340, 150, hwnd, (HMENU)ID_LIST_USERS, NULL, NULL);

        CreateWindowW(L"BUTTON", bIn.c_str(), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 205, 165, 32, hwnd, (HMENU)ID_BTN_LOGIN_USER, NULL, NULL);
        CreateWindowW(L"BUTTON", bImp.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            195, 205, 165, 32, hwnd, (HMENU)ID_BTN_IMPORT_USER, NULL, NULL);

        CreateWindowW(L"BUTTON", bNw.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 245, 165, 32, hwnd, (HMENU)ID_BTN_NEW_USER_DLG, NULL, NULL);
        CreateWindowW(L"BUTTON", bGst.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            195, 245, 165, 32, hwnd, (HMENU)ID_BTN_GUEST_USER, NULL, NULL);

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
                    DestroyWindow(hwnd);
                    SwitchToUser(g_hMainWnd, buf.c_str());
                    UpdateMainLayout(g_hMainWnd);
                }
            }
        }
        else if (id == ID_BTN_IMPORT_USER && ev == BN_CLICKED) {
            if (ImportExternalUserProfile(hwnd)) {
                SendMessageW(hListUsers, LB_RESETCONTENT, 0, 0);
                StrVec users = GetAllExistingUsers();
                for (const auto& u : users) SendMessageW(hListUsers, LB_ADDSTRING, 0, (LPARAM)u.c_str());
                if (!users.empty()) SendMessageW(hListUsers, LB_SETCURSEL, (int)users.size() - 1, 0);
            }
        }
        else if (id == ID_BTN_NEW_USER_DLG && ev == BN_CLICKED) {
            CleanupTempProfileIfNeeded();
            g_user.exists = false;
            UpdateMainLayout(g_hMainWnd);
            DestroyWindow(hwnd);
        }
        else if (id == ID_BTN_GUEST_USER && ev == BN_CLICKED) {
            StartGuestProfile(g_hMainWnd);
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
    std::wstring title = T(L"sel_tit", L"Выбор пользователя", L"Choose Account");
    CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAIUserSelectClass", title.c_str(),
        WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 395, 325,
        parent, NULL, GetModuleHandle(NULL), NULL);
}

LRESULT CALLBACK NewChatWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditTitle, hEditDesc;
    switch (uMsg) {
    case WM_CREATE: {
        std::wstring lTit = T(L"nc_tit", L"Название нового чата:", L"New Chat Title:");
        std::wstring lDsc = T(L"nc_dsc", L"Описание (необязательно):", L"Description (optional):");
        std::wstring bOk = T(L"nc_ok", L"Создать", L"Create");
        std::wstring bCan = T(L"nc_can", L"Отмена", L"Cancel");
        std::wstring defTitle = T(L"nc_def", L"Диалог #", L"Chat #") + std::to_wstring(g_chats.size() + 1);

        CreateWindowW(L"STATIC", lTit.c_str(), WS_VISIBLE | WS_CHILD, 20, 15, 340, 20, hwnd, NULL, NULL, NULL);
        hEditTitle = CreateWindowW(L"EDIT", defTitle.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            20, 40, 340, 26, hwnd, (HMENU)ID_NEWCHAT_TITLE, NULL, NULL);

        CreateWindowW(L"STATIC", lDsc.c_str(), WS_VISIBLE | WS_CHILD, 20, 80, 340, 20, hwnd, NULL, NULL, NULL);
        hEditDesc = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            20, 105, 340, 80, hwnd, (HMENU)ID_NEWCHAT_DESC, NULL, NULL);

        CreateWindowW(L"BUTTON", bOk.c_str(), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            160, 200, 95, 30, hwnd, (HMENU)ID_NEWCHAT_OK, NULL, NULL);
        CreateWindowW(L"BUTTON", bCan.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
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
            std::wstring title(tLen + 1, L'\0'), desc(dLen + 1, L'\0');
            if (tLen > 0) GetWindowTextW(hEditTitle, title.data(), tLen + 1);
            if (dLen > 0) GetWindowTextW(hEditDesc, desc.data(), dLen + 1);
            title.resize(tLen); desc.resize(dLen);
            if (title.empty()) title = T(L"nc_def", L"Диалог #", L"Chat #") + std::to_wstring(g_chats.size() + 1);

            ChatSession nextChat;
            nextChat.id = L"chat_" + std::to_wstring(GetTickCount64());
            nextChat.title = title;
            nextChat.description = desc;
            nextChat.messages.push_back({ false, T(L"nc_msg", L"Чат создан и зашифрован. Задавай вопросы!", L"Chat created and encrypted. Ask questions!"), L"", GetCurrentTimestampStr() });

            SaveChatSession(g_user.username, nextChat);
            g_chats.push_back(std::move(nextChat));
            g_currentChatIdx = (int)g_chats.size() - 1;
            g_lastFindPos = 0;
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
        std::wstring lbl = T(L"tr_lbl", L"Редактор базы Ai.dat (Ctrl+A — выделить всё, без ограничений):", L"Ai.dat Editor (Ctrl+A to select all, unlimited):");
        std::wstring bLd = T(L"tr_ld", L"Загрузить Ai.dat...", L"Load Ai.dat...");
        std::wstring bOk = T(L"tr_ok", L"Сохранить в Ai.dat", L"Save to Ai.dat");
        std::wstring bCn = T(L"tr_cn", L"Отмена", L"Cancel");

        CreateWindowW(L"STATIC", lbl.c_str(), WS_VISIBLE | WS_CHILD, 15, 15, 560, 22, hwnd, NULL, NULL, NULL);

        hEditTrain = CreateWindowW(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            15, 42, 560, 320, hwnd, (HMENU)ID_TRAIN_TEXT, NULL, NULL);

        SendMessageW(hEditTrain, EM_SETLIMITTEXT, 0, 0);
        g_origTrainEditProc = (WNDPROC)SetWindowLongPtrW(hEditTrain, GWLP_WNDPROC, (LONG_PTR)TrainEditProc);

        CreateWindowW(L"BUTTON", bLd.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            15, 375, 155, 32, hwnd, (HMENU)ID_TRAIN_LOAD_FILE, NULL, NULL);
        CreateWindowW(L"BUTTON", bOk.c_str(), WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            325, 375, 145, 32, hwnd, (HMENU)ID_TRAIN_OK, NULL, NULL);
        CreateWindowW(L"BUTTON", bCn.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
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
            ofn.lpstrFilter = L"Ai.dat (*.dat)\0*.dat\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

            if (GetOpenFileNameW(&ofn) && LoadBrainFromFile(szFile.c_str())) {
                std::wstring loadedText;
                for (const auto& s : g_brain.knowledgeSentences) loadedText += s + L"\r\n";
                SetWindowTextW(hEditTrain, loadedText.c_str());
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
    static HWND hEditName, hEditPass, hEditDesc, hBtnDelUser;
    switch (uMsg) {
    case WM_CREATE: {
        std::wstring lblName = T(L"pr_nm", L"Имя профиля:", L"Profile Name:");
        std::wstring lblPass = T(L"pr_ps", L"Пароль (необязательно):", L"Password (optional):");
        std::wstring lblDesc = T(L"pr_ds", L"Описание (о себе):", L"Description (Bio):");
        std::wstring btnAv = T(L"pr_av", L"Выбрать фото / GIF...", L"Choose Image / GIF...");
        std::wstring btnDelAv = T(L"pr_dav", L"Удалить фото", L"Remove Photo");
        std::wstring btnSw = T(L"pr_sw", L"Сменить / Новый юзер", L"Switch / New User");
        std::wstring btnDelU = T(L"pr_du", L"Удалить профиль", L"Delete Profile");

        CreateWindowW(L"STATIC", lblName.c_str(), WS_VISIBLE | WS_CHILD, 20, 15, 150, 20, hwnd, NULL, NULL, NULL);
        hEditName = CreateWindowW(L"EDIT", g_tempUser.username.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            20, 38, 260, 24, hwnd, (HMENU)ID_PROF_NAME, NULL, NULL);

        CreateWindowW(L"STATIC", lblPass.c_str(), WS_VISIBLE | WS_CHILD, 20, 70, 200, 20, hwnd, NULL, NULL, NULL);
        hEditPass = CreateWindowW(L"EDIT", g_tempUser.password.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
            20, 93, 260, 24, hwnd, (HMENU)ID_PROF_PASS, NULL, NULL);

        CreateWindowW(L"STATIC", lblDesc.c_str(), WS_VISIBLE | WS_CHILD, 20, 125, 150, 20, hwnd, NULL, NULL, NULL);
        hEditDesc = CreateWindowW(L"EDIT", g_tempUser.description.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            20, 148, 260, 75, hwnd, (HMENU)ID_PROF_DESC, NULL, NULL);

        CreateWindowW(L"BUTTON", btnAv.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            300, 165, 180, 26, hwnd, (HMENU)ID_PROF_AVATAR_BTN, NULL, NULL);
        CreateWindowW(L"BUTTON", btnDelAv.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            300, 196, 180, 26, hwnd, (HMENU)ID_PROF_DEL_AVATAR, NULL, NULL);

        CreateWindowW(L"BUTTON", btnSw.c_str(), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 235, 165, 30, hwnd, (HMENU)ID_PROF_SWITCH_USER, NULL, NULL);
        hBtnDelUser = CreateWindowW(L"BUTTON", btnDelU.c_str(), WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
            20, 272, 165, 30, hwnd, (HMENU)ID_PROF_DELETE_USER, NULL, NULL);

        CreateWindowW(L"BUTTON", L"OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            280, 272, 95, 30, hwnd, (HMENU)ID_PROF_OK, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            385, 272, 95, 30, hwnd, (HMENU)ID_PROF_CANCEL, NULL, NULL);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);

        StartTempAvatarAnimation(hwnd);
        return 0;
    }

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
        if (pDIS->CtlID == ID_PROF_DELETE_USER) {
            HBRUSH hBrush = CreateSolidBrush((pDIS->itemState & ODS_SELECTED) ? RGB(170, 30, 30) : RGB(215, 45, 45));
            FillRect(pDIS->hDC, &pDIS->rcItem, hBrush);
            DeleteObject(hBrush);

            SetBkMode(pDIS->hDC, TRANSPARENT);
            SetTextColor(pDIS->hDC, RGB(255, 255, 255));
            SelectObject(pDIS->hDC, g_hFontBold);

            std::wstring txt = T(L"pr_du", L"Удалить профиль", L"Delete Profile");
            DrawTextW(pDIS->hDC, txt.c_str(), -1, &pDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            return TRUE;
        }
        break;
    }

    case WM_TIMER:
        if (wParam == TIMER_TEMP_GIF && g_tempAnimBmps.size() > 1) {
            g_tempFrameIdx = (g_tempFrameIdx + 1) % g_tempAnimBmps.size();
            SetTimer(hwnd, TIMER_TEMP_GIF, g_tempUser.frames[g_tempFrameIdx].delayMs, NULL);
            RECT avRect = { 330, 35, 450, 155 };
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
            ofn.lpstrFilter = L"Images (*.png;*.jpg;*.jpeg;*.bmp;*.gif)\0*.png;*.jpg;*.jpeg;*.bmp;*.gif\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            if (GetOpenFileNameW(&ofn) && ProcessAvatarFile(szFile.c_str(), g_tempUser.frames)) {
                StartTempAvatarAnimation(hwnd);
                RECT avRect = { 330, 35, 450, 155 };
                InvalidateRect(hwnd, &avRect, TRUE);
            }
        }
        else if (id == ID_PROF_DEL_AVATAR) {
            KillTimer(hwnd, TIMER_TEMP_GIF);
            g_tempUser.frames.clear();
            for (HBITMAP bmp : g_tempAnimBmps) if (bmp) DeleteObject(bmp);
            g_tempAnimBmps.clear();
            g_tempFrameIdx = 0;
            RECT avRect = { 330, 35, 450, 155 };
            InvalidateRect(hwnd, &avRect, TRUE);
        }
        else if (id == ID_PROF_SWITCH_USER) {
            DestroyWindow(hwnd);
            OpenUserSelectDialog(g_hMainWnd);
        }
        else if (id == ID_PROF_DELETE_USER) {
            int ans = MessageBoxW(hwnd,
                T(L"del_conf", L"Навсегда удалить этот профиль и все его чаты?", L"Permanently delete this profile and all its chats?").c_str(),
                L"Delete", MB_YESNO | MB_ICONWARNING);
            if (ans == IDYES) {
                std::error_code ec;
                fs::remove_all(GetBaseAppFolder() / L"Users" / g_user.username, ec);
                KillTimer(g_hMainWnd, TIMER_MAIN_GIF);
                for (HBITMAP bmp : g_mainAnimBmps) if (bmp) DeleteObject(bmp);
                g_mainAnimBmps.clear();
                g_user = SafeProfile{};
                g_chats.clear();
                g_currentChatIdx = -1;
                DestroyWindow(hwnd);
                UpdateMainLayout(g_hMainWnd);
                StrVec users = GetAllExistingUsers();
                if (!users.empty()) OpenUserSelectDialog(g_hMainWnd);
            }
        }
        else if (id == ID_PROF_OK) {
            int nLen = GetWindowTextLengthW(hEditName);
            int pLen = GetWindowTextLengthW(hEditPass);
            int dLen = GetWindowTextLengthW(hEditDesc);
            std::wstring newName(nLen + 1, L'\0'), newPass(pLen + 1, L'\0'), newDesc(dLen + 1, L'\0');
            if (nLen > 0) GetWindowTextW(hEditName, newName.data(), nLen + 1);
            if (pLen > 0) GetWindowTextW(hEditPass, newPass.data(), pLen + 1);
            if (dLen > 0) GetWindowTextW(hEditDesc, newDesc.data(), dLen + 1);
            newName.resize(nLen); newPass.resize(pLen); newDesc.resize(dLen);

            if (!newName.empty()) {
                if (newName != g_user.username) {
                    std::error_code ec;
                    fs::path oldDir = GetUserFolder(g_user.username);
                    fs::path newDir = GetBaseAppFolder() / L"Users" / newName;
                    fs::rename(oldDir, newDir, ec);
                    fs::rename(newDir / (g_user.username + L".dat"), newDir / (newName + L".dat"), ec);
                    g_tempUser.isTemporary = false;
                }
                g_tempUser.username = newName;
                g_tempUser.password = newPass;
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
        RECT avRect = { 330, 35, 450, 155 };
        if (!g_tempAnimBmps.empty() && g_tempAnimBmps[g_tempFrameIdx]) {
            HDC hMem = CreateCompatibleDC(hdc);
            HBITMAP hOld = (HBITMAP)SelectObject(hMem, g_tempAnimBmps[g_tempFrameIdx]);
            BitBlt(hdc, 330, 35, 120, 120, hMem, 0, 0, SRCCOPY);
            SelectObject(hMem, hOld);
            DeleteDC(hMem);
        }
        else {
            DrawDefaultWin8Avatar(hdc, avRect);
        }
        FrameRect(hdc, &avRect, (HBRUSH)GetStockObject(GRAY_BRUSH));
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

LRESULT CALLBACK MsgInputProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        SendMessageW(g_hMainWnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SEND_MSG, BN_CLICKED), (LPARAM)g_hBtnSendMsg);
        return 0;
    }
    return CallWindowProcW(g_origMsgInputProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ChatListProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN && wParam == VK_DELETE) {
        int sel = (int)SendMessageW(hwnd, LB_GETCURSEL, 0, 0);
        if (sel != LB_ERR && sel >= 0 && sel < (int)g_chats.size()) {
            std::wstring msg = T(L"del_chat", L"Удалить выбранный чат «", L"Delete selected chat '") + g_chats[sel].title + L"»?";
            if (MessageBoxW(g_hMainWnd, msg.c_str(), L"Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                std::error_code ec;
                fs::remove(GetUserFolder(g_user.username) / L"chats" / (g_chats[sel].id + L".dat"), ec);
                g_chats.erase(g_chats.begin() + sel);
                if (g_chats.empty()) g_currentChatIdx = -1;
                else if (g_currentChatIdx >= (int)g_chats.size()) g_currentChatIdx = (int)g_chats.size() - 1;
                RefreshChatUI();
            }
        }
        return 0;
    }
    return CallWindowProcW(g_origChatListProc, hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        DragAcceptFiles(hwnd, TRUE);

        g_hFontNormal = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontBold = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontTitle = CreateFontW(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        g_hEditFirstName = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            260, 220, 280, 28, hwnd, (HMENU)ID_EDIT_FIRST_NAME, NULL, NULL);
        g_hEditFirstPass = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
            260, 260, 280, 28, hwnd, (HMENU)ID_EDIT_FIRST_PASS, NULL, NULL);
        g_hBtnCreateFirst = CreateWindowW(L"BUTTON", L"Create Account", WS_CHILD | BS_DEFPUSHBUTTON,
            310, 305, 180, 34, hwnd, (HMENU)ID_BTN_CREATE_FIRST, NULL, NULL);

        g_hBtnTrainAI = CreateWindowW(L"BUTTON", L"Train AI", WS_CHILD | BS_PUSHBUTTON, 10, 14, 115, 32, hwnd, (HMENU)ID_BTN_TRAIN_AI, NULL, NULL);
        g_hBtnOpenFolder = CreateWindowW(L"BUTTON", L"Data Folder", WS_CHILD | BS_PUSHBUTTON, 130, 14, 120, 32, hwnd, (HMENU)ID_BTN_OPEN_FOLDER, NULL, NULL);
        g_hBtnExportZip = CreateWindowW(L"BUTTON", L"Export ZIP", WS_CHILD | BS_PUSHBUTTON, 255, 14, 105, 32, hwnd, (HMENU)ID_BTN_EXPORT_ZIP, NULL, NULL);
        g_hBtnImportZip = CreateWindowW(L"BUTTON", L"Import ZIP", WS_CHILD | BS_PUSHBUTTON, 365, 14, 105, 32, hwnd, (HMENU)ID_BTN_IMPORT_ZIP, NULL, NULL);

        g_hComboLang = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
            475, 16, 120, 220, hwnd, (HMENU)ID_COMBO_LANG, NULL, NULL);
        const wchar_t* langs[] = { L"Русский", L"English", L"Deutsch", L"Español", L"Français", L"Українська", L"中文", L"日本語", L"Custom..." };
        for (const wchar_t* l : langs) SendMessageW(g_hComboLang, CB_ADDSTRING, 0, (LPARAM)l);
        SendMessageW(g_hComboLang, CB_SETCURSEL, g_langIndex, 0);

        g_hBtnEditProfile = CreateWindowW(L"BUTTON", L"Profile", WS_CHILD | BS_PUSHBUTTON, 700, 14, 105, 32, hwnd, (HMENU)ID_BTN_EDIT_PROFILE, NULL, NULL);
        g_hBtnNewChat = CreateWindowW(L"BUTTON", L"+ New Chat", WS_CHILD | BS_PUSHBUTTON, 10, 70, 180, 30, hwnd, (HMENU)ID_BTN_NEW_CHAT, NULL, NULL);
        g_hBtnImportChat = CreateWindowW(L"BUTTON", L"Import Chat", WS_CHILD | BS_PUSHBUTTON, 10, 106, 180, 30, hwnd, (HMENU)ID_BTN_IMPORT_CHAT, NULL, NULL);
        g_hListChats = CreateWindowW(L"LISTBOX", L"", WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY, 10, 144, 180, 346, hwnd, (HMENU)ID_LIST_CHATS, NULL, NULL);

        g_hEditChatHist = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            210, 70, 580, 370, hwnd, (HMENU)ID_EDIT_CHAT_HIST, NULL, NULL);
        g_hEditMsgInput = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            210, 450, 400, 36, hwnd, (HMENU)ID_EDIT_MSG_INPUT, NULL, NULL);
        g_hBtnAttachImg = CreateWindowW(L"BUTTON", L"Image", WS_CHILD | BS_PUSHBUTTON,
            615, 450, 70, 36, hwnd, (HMENU)ID_BTN_ATTACH_IMG, NULL, NULL);
        g_hBtnSendMsg = CreateWindowW(L"BUTTON", L"Send", WS_CHILD | BS_DEFPUSHBUTTON,
            690, 450, 100, 36, hwnd, (HMENU)ID_BTN_SEND_MSG, NULL, NULL);

        g_origMsgInputProc = (WNDPROC)SetWindowLongPtrW(g_hEditMsgInput, GWLP_WNDPROC, (LONG_PTR)MsgInputProc);
        g_origChatListProc = (WNDPROC)SetWindowLongPtrW(g_hListChats, GWLP_WNDPROC, (LONG_PTR)ChatListProc);

        EnumChildWindows(hwnd, [](HWND hChild, LPARAM lp) -> BOOL {
            SendMessageW(hChild, WM_SETFONT, lp, TRUE);
            return TRUE;
            }, (LPARAM)g_hFontNormal);

        LoadCustomLanguage();
        ApplyLanguageToControls();

        StrVec users = GetAllExistingUsers();
        if (users.size() == 1) SwitchToUser(hwnd, users.front());
        else if (users.size() >= 2) PostMessageW(hwnd, WM_APP + 1, 0, 0);

        UpdateMainLayout(hwnd);
        return 0;
    }

    case WM_DROPFILES: {
        if (g_user.exists && g_currentChatIdx >= 0) {
            HDROP hDrop = (HDROP)wParam;
            UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
            for (UINT i = 0; i < count; ++i) {
                std::wstring filePath(MAX_PATH, L'\0');
                DragQueryFileW(hDrop, i, filePath.data(), MAX_PATH);
                fs::path savedPath;
                std::wstring savedName;
                if (ValidateAndCopyImageToProfile(hwnd, filePath.c_str(), savedPath, savedName)) {
                    std::wstring binReport = MiniAIBrain::AnalyzeImageAsBinaryPixels(savedPath, savedName);
                    g_chats[g_currentChatIdx].messages.push_back({ true, L"(Скриншот прикреплён)", savedName, GetCurrentTimestampStr() });
                    std::wstring reply = g_brain.GenerateResponse(L"", binReport);
                    g_chats[g_currentChatIdx].messages.push_back({ false, reply, L"", GetCurrentTimestampStr() });
                }
            }
            DragFinish(hDrop);
            SaveChatSession(g_user.username, g_chats[g_currentChatIdx]);
            RefreshChatUI();
        }
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

        if (id == ID_COMBO_LANG && ev == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(g_hComboLang, CB_GETCURSEL, 0, 0);
            if (sel == 8) {
                EnableWindow(hwnd, FALSE);
                CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAICustomLangClass", L"Custom Language",
                    WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 425, 355,
                    hwnd, NULL, GetModuleHandle(NULL), NULL);
            }
            else if (sel != CB_ERR) {
                g_langIndex = sel;
                ApplyLanguageToControls();
                RefreshChatUI();
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        else if (id == ID_BTN_CREATE_FIRST && ev == BN_CLICKED) {
            int len = GetWindowTextLengthW(g_hEditFirstName);
            int pLen = GetWindowTextLengthW(g_hEditFirstPass);
            std::wstring name(len + 1, L'\0'), pass(pLen + 1, L'\0');
            if (len > 0) GetWindowTextW(g_hEditFirstName, name.data(), len + 1);
            if (pLen > 0) GetWindowTextW(g_hEditFirstPass, pass.data(), pLen + 1);
            name.resize(len); pass.resize(pLen);

            if (!name.empty()) {
                CleanupTempProfileIfNeeded();
                g_user = SafeProfile{};
                g_user.exists = true;
                g_user.isTemporary = false;
                g_user.username = name;
                g_user.password = pass;
                SaveUserProfile(g_user);

                g_brain.InitDefaultKnowledge();
                SaveUserBrain(name);
                LoadUserChats(name);
                StartMainAvatarAnimation(hwnd);
                RefreshChatUI();
                UpdateMainLayout(hwnd);
            }
        }
        else if (id == ID_BTN_EDIT_PROFILE && ev == BN_CLICKED) {
            g_tempUser = g_user;
            EnableWindow(hwnd, FALSE);
            CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAIProfileEditorClass",
                T(L"prof", L"Профиль", L"Profile").c_str(),
                WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 515, 355,
                hwnd, NULL, GetModuleHandle(NULL), NULL);
        }
        else if (id == ID_BTN_TRAIN_AI && ev == BN_CLICKED) {
            EnableWindow(hwnd, FALSE);
            CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAITrainClass",
                T(L"train", L"Редактор базы Ai.dat", L"Ai.dat Knowledge Editor").c_str(),
                WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 605, 460,
                hwnd, NULL, GetModuleHandle(NULL), NULL);
        }
        else if (id == ID_BTN_OPEN_FOLDER && ev == BN_CLICKED) {
            ShellExecuteW(NULL, L"open", GetBaseAppFolder().c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
        else if (id == ID_BTN_EXPORT_ZIP && ev == BN_CLICKED) {
            ExportProfileAsZip(hwnd);
        }
        else if (id == ID_BTN_IMPORT_ZIP && ev == BN_CLICKED) {
            ImportProfileFromZip(hwnd);
        }
        else if (id == ID_BTN_NEW_CHAT && ev == BN_CLICKED) {
            EnableWindow(hwnd, FALSE);
            CreateWindowExW(WS_EX_DLGMODALFRAME, L"MiniAINewChatClass",
                T(L"newchat", L"Новый чат", L"New Chat").c_str(),
                WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 395, 285,
                hwnd, NULL, GetModuleHandle(NULL), NULL);
        }
        else if (id == ID_BTN_IMPORT_CHAT && ev == BN_CLICKED) {
            ImportExternalChat(hwnd);
        }
        else if (id == ID_BTN_ATTACH_IMG && ev == BN_CLICKED) {
            std::wstring szFile(MAX_PATH, L'\0');
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile.data();
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter = L"Images (.png;.jpg;.jpeg;.bmp;.gif)\0.png;.jpg;.jpeg;.bmp;.gif\0";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            if (GetOpenFileNameW(&ofn)) {
                fs::path savedPath;
                std::wstring savedName;
                if (ValidateAndCopyImageToProfile(hwnd, szFile.c_str(), savedPath, savedName)) {
                    g_pendingImagePath = savedPath;
                    MessageBoxW(hwnd, (L"Image ready: " + savedName).c_str(), L"Image", MB_ICONINFORMATION);
                }
            }
        }
        else if (id == ID_LIST_CHATS && ev == LBN_SELCHANGE) {
            int sel = (int)SendMessageW(g_hListChats, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR && sel < (int)g_chats.size()) {
                g_currentChatIdx = sel;
                g_lastFindPos = 0;
                RefreshChatUI();
            }
        }
        else if (id == ID_BTN_SEND_MSG && ev == BN_CLICKED) {
            int len = GetWindowTextLengthW(g_hEditMsgInput);
            if (len > 0 || !g_pendingImagePath.empty()) {
                if (g_currentChatIdx < 0 || g_chats.empty()) {
                    ChatSession autoChat;
                    autoChat.id = L"chat_" + std::to_wstring(GetTickCount64());
                    autoChat.title = T(L"newchat", L"Новый диалог", L"New Chat");
                    g_chats.push_back(std::move(autoChat));
                    g_currentChatIdx = 0;
                }
                std::wstring userMsg(len + 1, L'\0');
                if (len > 0) GetWindowTextW(g_hEditMsgInput, userMsg.data(), len + 1);
                userMsg.resize(len);
                SetWindowTextW(g_hEditMsgInput, L"");
                LoadUserBrain(g_user.username);
                std::wstring attachedName;
                std::wstring binReport;
                if (!g_pendingImagePath.empty()) {
                    attachedName = g_pendingImagePath.filename().wstring();
                    binReport = MiniAIBrain::AnalyzeImageAsBinaryPixels(g_pendingImagePath, attachedName);
                    g_pendingImagePath.clear();
                }
                g_chats[g_currentChatIdx].messages.push_back({ true, userMsg, attachedName, GetCurrentTimestampStr() });
                std::wstring aiReply = g_brain.GenerateResponse(userMsg, binReport);
                g_chats[g_currentChatIdx].messages.push_back({ false, aiReply, L"", GetCurrentTimestampStr() });
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
            DrawTextW(hdc,
                T(L"reg_title", L"Mini AI Studio 6.0 — Создание профиля", L"Mini AI Studio 6.0 — Create Profile").c_str(),
                -1, &titleRc, DT_CENTER | DT_SINGLELINE);
            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, RGB(100, 100, 100));
            RECT subRc = { 0, 188, rc.right, 215 };
            DrawTextW(hdc,
                T(L"reg_sub", L"Сверху: Имя пользователя | Снизу: Пароль (необязательно)", L"Top: Username | Bottom: Optional Password").c_str(),
                -1, &subRc, DT_CENTER | DT_SINGLELINE);
        }
        else {
            RECT headerRc = { 0, 0, rc.right, 60 };
            FillRect(hdc, &headerRc, (HBRUSH)(COLOR_BTNFACE + 1));
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, RGB(20, 20, 20));
            RECT nameRc = { rc.right - 340, 14, rc.right - 175, 46 };
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
                DrawDefaultWin8Avatar(hdc, avMini);
            }
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        CleanupTempProfileIfNeeded();
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
    WORD primLang = PRIMARYLANGID(langId);
    if (primLang == LANG_ENGLISH) {
        primLang = PRIMARYLANGID(LANGIDFROMLCID(GetUserDefaultLCID()));
    }

    if (primLang == LANG_RUSSIAN) g_langIndex = 0;
    else if (primLang == LANG_GERMAN) g_langIndex = 2;
    else if (primLang == LANG_SPANISH) g_langIndex = 3;
    else if (primLang == LANG_FRENCH) g_langIndex = 4;
    else if (primLang == LANG_UKRAINIAN) g_langIndex = 5;
    else if (primLang == LANG_CHINESE) g_langIndex = 6;
    else if (primLang == LANG_JAPANESE) g_langIndex = 7;
    else g_langIndex = 1;

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
    regClass(L"MiniAIFindClass", FindWndProc, (HBRUSH)(COLOR_BTNFACE + 1));

    g_hMainWnd = CreateWindowExW(
        0, L"MiniAIMainWindowClass", L"Mini AI Studio 6.0",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 580,
        NULL, NULL, hInstance, NULL
    );

    if (g_hMainWnd) {
        SendMessageW(g_hMainWnd, WM_SETICON, ICON_BIG, (LPARAM)g_hAppIcon);
        SendMessageW(g_hMainWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hAppIcon);
        ShowWindow(g_hMainWnd, nCmdShow);

        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0)) {
            if (msg.message == WM_KEYDOWN && msg.wParam == 'F' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                OpenChatSearchDialog(g_hMainWnd);
                continue;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(g_gdiplusToken);
    return 0;
}