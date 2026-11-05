#ifdef _WIN32

#include "etapki/core/analysis.h"
#include "etapki/core/catalog.h"
#include "etapki/core/midi.h"
#include "etapki/core/preview.h"
#include "etapki/core/transformation.h"
#include "etapki/core/variant_registry.h"

#include <windows.h>
#include <commdlg.h>

#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace {

constexpr int kOpenButton = 1001;
constexpr int kAnalyzeButton = 1002;
constexpr int kBeforeButton = 1003;
constexpr int kAfterButton = 1004;
constexpr int kTargetButton = 1005;
constexpr int kStopButton = 1006;
constexpr int kRandomButton = 1007;
constexpr int kLabelList = 1101;
constexpr int kTypeList = 1102;
constexpr int kVariantList = 1103;
constexpr int kSeedEdit = 1201;
constexpr int kStatusText = 1301;

HWND g_labelList = nullptr;
HWND g_typeList = nullptr;
HWND g_variantList = nullptr;
HWND g_seedEdit = nullptr;
HWND g_status = nullptr;
HFONT g_titleFont = nullptr;
HFONT g_bodyFont = nullptr;
std::optional<etapki::core::MidiDocument> g_document;
std::vector<std::string> g_labelIds;
std::vector<etapki::core::RegisteredVariant> g_visibleVariants;

std::wstring utf8ToWide(const std::string& text) {
    if (text.empty()) return {};
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (length <= 0) return {};
    std::wstring result(static_cast<std::size_t>(length - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, result.data(), length);
    return result;
}

std::string wideToUtf8(const std::wstring& text) {
    if (text.empty()) return {};
    const int length = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (length <= 0) return {};
    std::string result(static_cast<std::size_t>(length - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, result.data(), length, nullptr, nullptr);
    return result;
}

void setStatus(const std::wstring& text) {
    if (g_status) SetWindowTextW(g_status, text.c_str());
}

void applyFont(HWND control, HFONT font) {
    if (control) SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

std::vector<etapki::core::Byte> readFile(const std::wstring& path) {
    std::ifstream input(wideToUtf8(path), std::ios::binary);
    if (!input) return {};
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void populateLabels() {
    SendMessageW(g_labelList, LB_RESETCONTENT, 0, 0);
    g_labelIds.clear();
    for (const auto& label : etapki::core::analysisLabels()) {
        g_labelIds.push_back(label.id);
        const std::wstring text = utf8ToWide(label.id + " [" + etapki::core::toString(label.state) + "]");
        SendMessageW(g_labelList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
    }
}

void populateTypes() {
    SendMessageW(g_typeList, LB_RESETCONTENT, 0, 0);
    for (const auto& type : etapki::core::embellishmentTypes()) {
        const std::wstring text = utf8ToWide(type.displayName + " (" + type.stableId + ")");
        SendMessageW(g_typeList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
    }
    SendMessageW(g_typeList, LB_SETCURSEL, 0, 0);
}

void populateVariants(int typeIndex) {
    if (typeIndex < 0 || typeIndex >= static_cast<int>(etapki::core::embellishmentTypes().size())) return;
    SendMessageW(g_variantList, LB_RESETCONTENT, 0, 0);
    const auto type = etapki::core::embellishmentTypes()[static_cast<std::size_t>(typeIndex)].id;
    g_visibleVariants = etapki::core::VariantRegistry::forType(type);
    for (const auto& variant : g_visibleVariants) {
        const std::wstring text = utf8ToWide(variant.displayName + " [" + etapki::core::toString(variant.state) + "]");
        SendMessageW(g_variantList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
    }
}

void openMidi(HWND window) {
    wchar_t path[MAX_PATH]{};
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = window;
    dialog.lpstrFilter = L"MIDI files (*.mid;*.midi)\0*.mid;*.midi\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = path;
    dialog.nMaxFile = MAX_PATH;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_READONLY;
    if (!GetOpenFileNameW(&dialog)) return;
    const auto bytes = readFile(path);
    etapki::core::MidiError error;
    const auto document = etapki::core::MidiDocument::parse(bytes, error);
    if (!document) {
        setStatus(L"MIDI rejected at byte " + std::to_wstring(error.offset) + L": " + utf8ToWide(error.message));
        return;
    }
    g_document = *document;
    setStatus(L"Loaded MIDI: " + std::to_wstring(document->tracks().size()) + L" tracks, " +
              std::to_wstring(document->eventCount()) + L" events. Select labels and analyze.");
}

void analyzeSelected() {
    if (!g_document.has_value()) {
        setStatus(L"Open a MIDI file first.");
        return;
    }
    const int selectedCount = static_cast<int>(SendMessageW(g_labelList, LB_GETSELCOUNT, 0, 0));
    if (selectedCount <= 0) {
        setStatus(L"Select one or more analyzer labels first.");
        return;
    }
    std::vector<int> indexes(static_cast<std::size_t>(selectedCount));
    SendMessageW(g_labelList, LB_GETSELITEMS, static_cast<WPARAM>(selectedCount), reinterpret_cast<LPARAM>(indexes.data()));
    etapki::core::AnalysisOptions options;
    for (const auto index : indexes) {
        if (index >= 0 && index < static_cast<int>(g_labelIds.size())) options.selectedLabels.push_back(g_labelIds[static_cast<std::size_t>(index)]);
    }
    const auto report = etapki::core::analyzeMidi(*g_document, options);
    std::size_t integrated = 0;
    std::size_t placeholders = 0;
    for (const auto& method : report.methods) {
        if (method.state == etapki::core::ImplementationState::Integrated) ++integrated;
        if (method.state == etapki::core::ImplementationState::Placeholder) ++placeholders;
    }
    setStatus(L"Analysis complete: " + std::to_wstring(report.findings.size()) + L" findings; " +
              std::to_wstring(integrated) + L" integrated methods; " + std::to_wstring(placeholders) + L" placeholders.");
}

void selectRandomVariant() {
    const int typeIndex = static_cast<int>(SendMessageW(g_typeList, LB_GETCURSEL, 0, 0));
    if (typeIndex < 0 || typeIndex >= static_cast<int>(etapki::core::embellishmentTypes().size())) return;
    wchar_t seedText[64]{};
    GetWindowTextW(g_seedEdit, seedText, 64);
    try {
        const auto seed = static_cast<std::uint64_t>(std::stoull(seedText));
        const auto type = etapki::core::embellishmentTypes()[static_cast<std::size_t>(typeIndex)].id;
        etapki::core::TransformationRouter router;
        const auto selection = router.chooseRandom(type, seed);
        if (!selection.has_value()) {
            setStatus(L"No selectable direct variants are available for this type.");
            return;
        }
        for (std::size_t index = 0; index < g_visibleVariants.size(); ++index) {
            if (g_visibleVariants[index].id == selection->variant.id) {
                SendMessageW(g_variantList, LB_SETCURSEL, static_cast<WPARAM>(index), 0);
                break;
            }
        }
        setStatus(L"Seed " + std::to_wstring(seed) + L" selected " + utf8ToWide(selection->variant.displayName) + L".");
    } catch (const std::exception&) {
        setStatus(L"Seed must be an unsigned integer.");
    }
}

void requestPreview(etapki::core::PreviewMode mode) {
    if (!g_document.has_value()) {
        setStatus(L"Open a MIDI file before requesting a preview.");
        return;
    }
    etapki::core::PreviewController controller;
    controller.setMode(mode);
    etapki::core::ApprovedLibraryBackend backend;
    etapki::core::SoundLibraryDescriptor library;
    const auto result = controller.render(*g_document, nullptr, library, backend, {});
    setStatus(L"Preview: " + utf8ToWide(result.message));
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        g_titleFont = CreateFontW(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        g_bodyFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        auto title = CreateWindowW(L"STATIC", L"ETAPKI — MIDI Analysis & Embellishment Preview", WS_CHILD | WS_VISIBLE,
                                   24, 18, 800, 40, window, nullptr, nullptr, nullptr);
        applyFont(title, g_titleFont);
        auto subtitle = CreateWindowW(L"STATIC", L"Non-destructive workflow • seven types • 62 analyzer labels", WS_CHILD | WS_VISIBLE,
                                      26, 58, 800, 24, window, nullptr, nullptr, nullptr);
        applyFont(subtitle, g_bodyFont);
        auto open = CreateWindowW(L"BUTTON", L"Open MIDI", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  24, 96, 130, 34, window, reinterpret_cast<HMENU>(kOpenButton), nullptr, nullptr);
        auto analyze = CreateWindowW(L"BUTTON", L"Analyze selected", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     164, 96, 160, 34, window, reinterpret_cast<HMENU>(kAnalyzeButton), nullptr, nullptr);
        auto before = CreateWindowW(L"BUTTON", L"Preview before", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    334, 96, 150, 34, window, reinterpret_cast<HMENU>(kBeforeButton), nullptr, nullptr);
        auto after = CreateWindowW(L"BUTTON", L"Preview after", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   494, 96, 150, 34, window, reinterpret_cast<HMENU>(kAfterButton), nullptr, nullptr);
        auto target = CreateWindowW(L"BUTTON", L"Audition target", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    654, 96, 150, 34, window, reinterpret_cast<HMENU>(kTargetButton), nullptr, nullptr);
        auto stop = CreateWindowW(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                  814, 96, 100, 34, window, reinterpret_cast<HMENU>(kStopButton), nullptr, nullptr);
        g_labelList = CreateWindowW(L"LISTBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_EXTENDEDSEL,
                                    24, 166, 300, 430, window, reinterpret_cast<HMENU>(kLabelList), nullptr, nullptr);
        g_typeList = CreateWindowW(L"LISTBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                                   344, 166, 250, 250, window, reinterpret_cast<HMENU>(kTypeList), nullptr, nullptr);
        g_variantList = CreateWindowW(L"LISTBOX", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                                      614, 166, 440, 430, window, reinterpret_cast<HMENU>(kVariantList), nullptr, nullptr);
        auto seedLabel = CreateWindowW(L"STATIC", L"Random seed", WS_CHILD | WS_VISIBLE, 344, 432, 120, 24, window, nullptr, nullptr, nullptr);
        g_seedEdit = CreateWindowW(L"EDIT", L"42", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 464, 428, 100, 28, window, reinterpret_cast<HMENU>(kSeedEdit), nullptr, nullptr);
        auto random = CreateWindowW(L"BUTTON", L"Choose seeded variant", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    344, 472, 220, 34, window, reinterpret_cast<HMENU>(kRandomButton), nullptr, nullptr);
        g_status = CreateWindowW(L"STATIC", L"Open a MIDI file to begin.", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                 24, 620, 1030, 48, window, reinterpret_cast<HMENU>(kStatusText), nullptr, nullptr);
        for (const auto control : {open, analyze, before, after, target, stop, seedLabel, random, subtitle, g_status}) applyFont(control, g_bodyFont);
        applyFont(g_labelList, g_bodyFont);
        applyFont(g_typeList, g_bodyFont);
        applyFont(g_variantList, g_bodyFont);
        populateLabels();
        populateTypes();
        populateVariants(0);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kOpenButton: openMidi(window); return 0;
        case kAnalyzeButton: analyzeSelected(); return 0;
        case kBeforeButton: requestPreview(etapki::core::PreviewMode::Before); return 0;
        case kAfterButton: requestPreview(etapki::core::PreviewMode::After); return 0;
        case kTargetButton: requestPreview(etapki::core::PreviewMode::Target); return 0;
        case kStopButton: setStatus(L"Playback stopped; current MIDI and revisions remain unchanged."); return 0;
        case kRandomButton: selectRandomVariant(); return 0;
        case kTypeList:
            if (HIWORD(wParam) == LBN_SELCHANGE) populateVariants(static_cast<int>(SendMessageW(g_typeList, LB_GETCURSEL, 0, 0)));
            return 0;
        default: break;
        }
        break;
    case WM_DESTROY:
        if (g_titleFont) DeleteObject(g_titleFont);
        if (g_bodyFont) DeleteObject(g_bodyFont);
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand) {
    const wchar_t className[] = L"EtapkiPhase4Window";
    WNDCLASSW windowClass{};
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = windowProc;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&windowClass)) return 1;
    HWND window = CreateWindowExW(0, className, L"ETAPKI — MIDI Analysis & Embellishment Preview",
                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1100, 740,
                                 nullptr, nullptr, instance, nullptr);
    if (!window) return 2;
    ShowWindow(window, showCommand);
    UpdateWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}

#endif
