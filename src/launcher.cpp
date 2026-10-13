#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <tlhelp32.h>
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>
#include <algorithm>
#include <regex>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <cwctype>
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "user32.lib")

namespace fs = std::filesystem;
constexpr wchar_t kDefaultGame[] = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\ACE COMBAT 8\\Game\\Binaries\\Win64\\AceCombat8.exe";
constexpr char kExpectedHash[] = "51510E2A520565DBE81FB0D569E95CD4393077ACAAA859371489B80B8128829F";
constexpr uint64_t kExpectedSize = 555102832;
constexpr uint64_t kImageBase = 0x140000000;
constexpr char kBuild[] = "25201480";
constexpr char kVersion[] = "0.1.1";

struct Handle {
    HANDLE value = nullptr;
    Handle() = default;
    explicit Handle(HANDLE h) : value(h) {}
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    ~Handle() { if (value && value != INVALID_HANDLE_VALUE) CloseHandle(value); }
    bool valid() const { return value && value != INVALID_HANDLE_VALUE; }
    operator HANDLE() const { return value; }
};
std::string winError(const char* action) {
    return std::string(action) + " (Windows error " + std::to_string(GetLastError()) + ")";
}
// Capture GetLastError only after the API expression has completed. Passing an
// eagerly formatted message alongside an API call has unspecified argument order.
void winCheck(bool condition,const char* action) {if(!condition)throw std::runtime_error(winError(action));}
void require(bool condition, const std::string& reason) { if (!condition) throw std::runtime_error(reason); }
std::string utf8(const std::wstring& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0, nullptr, nullptr);
    std::string out(size_t(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), int(s.size()), out.data(), n, nullptr, nullptr);
    return out;
}
std::string escapeJson(const std::string& s) {
    std::ostringstream o;
    for (unsigned char c : s) {
        switch (c) {
        case '\\': o << "\\\\"; break; case '"': o << "\\\""; break;
        case '\n': o << "\\n"; break; case '\r': o << "\\r"; break; case '\t': o << "\\t"; break;
        default: if (c < 32) o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(c); else o << c;
        }
    }
    return o.str();
}
fs::path ownPath() {
    std::vector<wchar_t> buf(32768);
    DWORD n = GetModuleFileNameW(nullptr, buf.data(), DWORD(buf.size()));
    require(n && n < buf.size(), "Cannot locate launcher executable.");
    return fs::path(std::wstring(buf.data(), n));
}
std::string utcStamp() {
    SYSTEMTIME st{}; GetSystemTime(&st);
    std::ostringstream o;
    o << std::setfill('0') << std::setw(4) << st.wYear << std::setw(2) << st.wMonth << std::setw(2) << st.wDay
      << '-' << std::setw(2) << st.wHour << std::setw(2) << st.wMinute << std::setw(2) << st.wSecond;
    return o.str();
}
struct Report {
    fs::path folder;
    std::ofstream log;
    fs::path game;
    std::string hash;
    DWORD pid = 0;
    bool verified = false, patched = false;
    explicit Report(fs::path directory) : folder(std::move(directory)) {
        fs::create_directories(folder);
        log.open(folder / ("launcher-" + utcStamp() + "-" + std::to_string(GetCurrentProcessId()) + ".log"));
        require(log.good(), "Cannot open launcher log.");
    }
    void line(const std::string& s) { std::cout << s << std::endl; log << utcStamp() << " " << s << std::endl; }
    void status(const std::string& state, const std::string& detail) {
        std::ofstream out(folder / "launcher-last-status.json", std::ios::trunc);
        require(out.good(), "Cannot write launcher status.");
        out << "{\n  \"version\": \"" << kVersion << "\",\n  \"state\": \"" << escapeJson(state)
            << "\",\n  \"detail\": \"" << escapeJson(detail) << "\",\n  \"timestamp_utc\": \"" << utcStamp()
            << "\",\n  \"supported_steam_build\": \"" << kBuild
            << "\",\n  \"game_executable\": \"" << escapeJson(utf8(game.wstring()))
            << "\",\n  \"sha256\": \"" << hash << "\",\n  \"process_id\": " << pid
            << ",\n  \"static_verified\": " << (verified ? "true" : "false")
            << ",\n  \"memory_patch_active\": " << (patched ? "true" : "false")
            << ",\n  \"visual_runtime_verified\": false,\n  \"on_disk_game_files_modified\": false,\n  \"saves_modified_by_launcher\": false\n}\n";
    }
};
std::vector<uint8_t> hexBytes(const char* text) {
    std::istringstream in(text); std::vector<uint8_t> out; unsigned x;
    while (in >> std::hex >> x) out.push_back(uint8_t(x));
    return out;
}
struct Site { const char* name; uint32_t guardRva; uint32_t patchRva; uint8_t original, replacement; std::vector<uint8_t> guard; };
const std::vector<Site> kSites = {
    {"pitch", 0x72F15BC, 0x72F15CD, 2, 3, hexBytes("00 48 8b cf e8 9b 4f 62 00 48 8b 8f 60 04 00 00 3c 02 48 8b 01 72 19 b2 01 48 8b 9c 24 c0 00 00 00 48 81 c4 b0 00 00 00 5f 48 ff a0 c8 02 00 00")},
    {"compass", 0x7243E0A, 0x7243E1B, 2, 3, hexBytes("cd 02 00 00 48 8b cb e8 4a 27 6d 00 48 8b 4e 10 3c 02 48 8b 01 72 28 b2 01 ff 90 c8 02 00 00 48 8b 4e 18 b2 01 48 8b 01 48 8b 9c 24 90 00 00 00")},
    {"speed_altitude_tapes", 0x72F0E4A, 0x72F0E5B, 1, 2, hexBytes("85 ff 0f 84 be 04 00 00 48 8b cb e8 06 57 62 00 3c 01 0f 87 d3 04 00 00 48 8b ce e8 56 aa a2 00 84 c0 0f 84 c3 04 00 00 48 8b 07 b2 04 4c 89 a4")},
    {"whisky_mark", 0x72F0576, 0x72F0587, 1, 2, hexBytes("20 49 8b ce e8 e1 5f 62 00 49 8b 8e 58 04 00 00 3c 01 b2 04 48 8b 01 76 02 b2 01 ff 90 c8 02 00 00 49 8b ce e8 a1 0f 00 00 49 8b 06 49 8b ce ff")}
};

std::vector<uint8_t> fileRead(HANDLE file, uint64_t offset, size_t size) {
    LARGE_INTEGER pos{}; pos.QuadPart = offset;
    winCheck(SetFilePointerEx(file, pos, nullptr, FILE_BEGIN)!=FALSE,"File seek failed");
    std::vector<uint8_t> data(size); DWORD read = 0;
    require(size <= MAXDWORD && ReadFile(file, data.data(), DWORD(size), &read, nullptr) && read == size, "File read failed or file is truncated.");
    return data;
}
template<typename T> T fileStruct(HANDLE file, uint64_t offset) {
    auto data = fileRead(file, offset, sizeof(T)); T out{}; memcpy(&out, data.data(), sizeof(T)); return out;
}
std::string sha256(HANDLE file) {
    BCRYPT_ALG_HANDLE alg = nullptr; BCRYPT_HASH_HANDLE hash = nullptr;
    require(BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0, "BCrypt SHA256 provider failed.");
    struct CloseAlg { BCRYPT_ALG_HANDLE a; ~CloseAlg() { BCryptCloseAlgorithmProvider(a, 0); } } closeAlg{alg};
    DWORD objectSize=0, resultSize=0;
    require(BCryptGetProperty(alg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objectSize), sizeof(objectSize), &resultSize, 0) >= 0, "BCrypt property failed.");
    std::vector<uint8_t> object(objectSize);
    require(BCryptCreateHash(alg, &hash, object.data(), objectSize, nullptr, 0, 0) >= 0, "BCryptCreateHash failed.");
    struct CloseHash { BCRYPT_HASH_HANDLE h; ~CloseHash() { BCryptDestroyHash(h); } } closeHash{hash};
    LARGE_INTEGER zero{}; require(SetFilePointerEx(file, zero, nullptr, FILE_BEGIN), "Hash seek failed.");
    std::vector<uint8_t> chunk(1024*1024); DWORD n=0;
    for (;;) {
        require(ReadFile(file, chunk.data(), DWORD(chunk.size()), &n, nullptr), "Hash file read failed.");
        if (!n) break;
        require(BCryptHashData(hash, chunk.data(), n, 0) >= 0, "BCryptHashData failed.");
    }
    std::array<uint8_t,32> result{};
    require(BCryptFinishHash(hash, result.data(), DWORD(result.size()), 0) >= 0, "BCryptFinishHash failed.");
    std::ostringstream out; out << std::uppercase << std::hex << std::setfill('0');
    for (auto c : result) out << std::setw(2) << unsigned(c);
    return out.str();
}
void hashGuard(const std::string& actual) { require(actual == kExpectedHash, "Unsupported executable SHA256; no process memory was changed."); }
struct PEInfo { IMAGE_NT_HEADERS64 header; std::vector<IMAGE_SECTION_HEADER> sections; };
PEInfo filePE(HANDLE file) {
    auto dos=fileStruct<IMAGE_DOS_HEADER>(file,0);
    require(dos.e_magic==IMAGE_DOS_SIGNATURE && dos.e_lfanew>0 && dos.e_lfanew<0x100000, "Invalid DOS/PE header.");
    PEInfo pe{}; pe.header=fileStruct<IMAGE_NT_HEADERS64>(file,dos.e_lfanew);
    require(pe.header.Signature==IMAGE_NT_SIGNATURE && pe.header.FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64 && pe.header.OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR64_MAGIC, "Requires a Windows AMD64 PE image.");
    require(pe.header.OptionalHeader.ImageBase==kImageBase && pe.header.OptionalHeader.SizeOfImage>0x72F1600, "Unexpected executable PE layout.");
    require(pe.header.FileHeader.NumberOfSections>0 && pe.header.FileHeader.NumberOfSections<100, "Invalid PE section count.");
    auto offset=uint64_t(dos.e_lfanew)+24+pe.header.FileHeader.SizeOfOptionalHeader;
    for (unsigned i=0;i<pe.header.FileHeader.NumberOfSections;i++) pe.sections.push_back(fileStruct<IMAGE_SECTION_HEADER>(file,offset+i*sizeof(IMAGE_SECTION_HEADER)));
    return pe;
}
uint64_t rawOffset(const PEInfo& pe, uint32_t rva, size_t len) {
    for (auto& s:pe.sections) if (rva>=s.VirtualAddress && uint64_t(rva)+len<=uint64_t(s.VirtualAddress)+s.SizeOfRawData) {
        require((s.Characteristics&IMAGE_SCN_MEM_EXECUTE)!=0, "Patch guard is outside an executable section.");
        return uint64_t(s.PointerToRawData)+rva-s.VirtualAddress;
    }
    throw std::runtime_error("Patch guard RVA is not backed by a PE file section.");
}
void verifyBuildManifest(const fs::path& game, Report& report) {
    auto dir=game.parent_path();
    for (int i=0;i<9 && !dir.empty();i++,dir=dir.parent_path()) {
        auto path=dir/L"appmanifest_2288340.acf";
        if (!fs::exists(path)) continue;
        std::ifstream in(path); std::string contents((std::istreambuf_iterator<char>(in)),{});
        std::smatch m;
        require(std::regex_search(contents,m,std::regex("\"buildid\"\\s+\"([0-9]+)\"")), "Cannot read Steam manifest buildid.");
        require(m[1].str()==kBuild,"Unsupported Steam buildid: "+m[1].str());
        report.line("Steam manifest buildid verified: "+m[1].str()); return;
    }
    report.line("Steam manifest unavailable; exact SHA256 identifies supported build 25201480.");
}
void verifyFile(HANDLE file, const fs::path& game, Report& report) {
    LARGE_INTEGER size{}; require(GetFileSizeEx(file,&size),"Cannot read executable size.");
    require(uint64_t(size.QuadPart)==kExpectedSize,"Unsupported executable size.");
    report.line("Computing full executable SHA256..."); report.hash=sha256(file); hashGuard(report.hash);
    auto pe=filePE(file);
    for (auto& site:kSites) require(fileRead(file,rawOffset(pe,site.guardRva,site.guard.size()),site.guard.size())==site.guard,std::string("On-disk context mismatch: ")+site.name);
    verifyBuildManifest(game,report); report.verified=true;
    report.line("Exact executable SHA256, x64 PE layout, and all four 48-byte gate contexts verified.");
}

std::vector<uint8_t> remoteRead(HANDLE process, uintptr_t address, size_t size) {
    std::vector<uint8_t> bytes(size); SIZE_T n=0;
    winCheck(ReadProcessMemory(process,reinterpret_cast<LPCVOID>(address),bytes.data(),size,&n)!=FALSE,"ReadProcessMemory failed");
    require(n==size,"ReadProcessMemory returned an incomplete read.");
    return bytes;
}
std::vector<uint8_t> expectedGuard(const Site& s, bool enabled) {
    auto expected=s.guard; require(s.patchRva>=s.guardRva && s.patchRva-s.guardRva<expected.size(),"Invalid internal patch layout.");
    expected[s.patchRva-s.guardRva]=enabled?s.replacement:s.original; return expected;
}
void verifyRemote(HANDLE process, uintptr_t base, const std::vector<Site>& sites, bool enabled) {
    for(auto& s:sites) require(remoteRead(process,base+s.guardRva,s.guard.size())==expectedGuard(s,enabled),std::string("Remote context mismatch: ")+s.name);
}
struct ProtectedPage { uintptr_t address; DWORD original; };
class ProtectionScope {
    HANDLE process; std::vector<ProtectedPage> pages; bool injectPartialRestoreFailure=false;
public:
    explicit ProtectionScope(HANDLE p,bool injectRestoreFailure=false):process(p),injectPartialRestoreFailure(injectRestoreFailure) {}
    void add(uintptr_t address) {
        SYSTEM_INFO info{}; GetSystemInfo(&info);
        uintptr_t page=address-(address%info.dwPageSize);
        if(std::any_of(pages.begin(),pages.end(),[page](auto& p){return p.address==page;})) return;
        MEMORY_BASIC_INFORMATION mb{};
        SIZE_T queried=VirtualQueryEx(process,reinterpret_cast<LPCVOID>(page),&mb,sizeof(mb));
        winCheck(queried!=0,"VirtualQueryEx failed");require(queried==sizeof(mb),"VirtualQueryEx returned an incomplete record.");
        require(mb.State==MEM_COMMIT && (mb.Protect&PAGE_GUARD)==0 && mb.Protect!=PAGE_NOACCESS,"Patch page is guarded, inaccessible, or uncommitted.");
        require(mb.Protect==PAGE_EXECUTE_READ || mb.Protect==PAGE_EXECUTE_READWRITE || mb.Protect==PAGE_EXECUTE_WRITECOPY,"Patch page lacks expected executable protection.");
        DWORD old=0;
        winCheck(VirtualProtectEx(process,reinterpret_cast<LPVOID>(page),info.dwPageSize,PAGE_EXECUTE_READWRITE,&old)!=FALSE,"VirtualProtectEx failed");
        pages.push_back({page,old});
    }
    bool restore() {
        SYSTEM_INFO info{}; GetSystemInfo(&info); bool ok=true;size_t originalCount=pages.size();
        for(size_t i=originalCount;i>0;--i) {
            // Synthetic fault injection skips one restore after another page has
            // already become RX, exercising rollback across mixed protections.
            if(injectPartialRestoreFailure&&originalCount>1&&i==originalCount-1) {injectPartialRestoreFailure=false;ok=false;continue;}
            auto page=pages[i-1];DWORD ignored=0;
            if(VirtualProtectEx(process,reinterpret_cast<LPVOID>(page.address),info.dwPageSize,page.original,&ignored))
                pages.erase(pages.begin()+static_cast<ptrdiff_t>(i-1));
            else ok=false;
        }
        // Successfully restored pages are no longer tracked: rollback add()
        // must explicitly make them writable again while retaining the original.
        return ok;
    }
    ~ProtectionScope(){ restore(); }
};
void remoteByte(HANDLE process,uintptr_t address,uint8_t byte) {
    SIZE_T n=0;winCheck(WriteProcessMemory(process,reinterpret_cast<LPVOID>(address),&byte,1,&n)!=FALSE,"WriteProcessMemory failed");
    require(n==1,"WriteProcessMemory returned an incomplete write.");
    winCheck(FlushInstructionCache(process,reinterpret_cast<LPCVOID>(address),1)!=FALSE,"FlushInstructionCache failed");
    require(remoteRead(process,address,1)[0]==byte,"One-byte patch readback failed.");
}
void transaction(HANDLE process,uintptr_t base,const std::vector<Site>& sites,bool enable,bool injectFailure=false,bool injectRestoreFailure=false) {
    verifyRemote(process,base,sites,!enable);
    ProtectionScope protection(process,injectRestoreFailure);
    std::vector<const Site*> changed;
    try {
        for(auto& s:sites) protection.add(base+s.patchRva);
        verifyRemote(process,base,sites,!enable); // Recheck all contexts immediately before writing.
        for(auto& s:sites) {
            changed.push_back(&s); // Includes a site if write/readback partially fails.
            remoteByte(process,base+s.patchRva,enable?s.replacement:s.original);
            if(injectFailure && changed.size()==1) throw std::runtime_error("Injected transactional failure after first byte write.");
        }
        verifyRemote(process,base,sites,enable);
        require(protection.restore(),"Failed to restore page protections after patch.");
    } catch(const std::exception& initial) {
        std::string message=initial.what(); bool rolledBack=true;
        try {
            for(auto s:changed) protection.add(base+s->patchRva);
            for(auto it=changed.rbegin();it!=changed.rend();++it) remoteByte(process,base+(*it)->patchRva,enable?(*it)->original:(*it)->replacement);
            verifyRemote(process,base,sites,!enable);
        } catch(...) { rolledBack=false; }
        if(!protection.restore()) rolledBack=false;
        throw std::runtime_error(message+(rolledBack?" Rollback verified; page protections restored.":" CRITICAL: rollback or page-protection restoration could not be verified."));
    }
}
// Failure recovery owns only the four known replacement bytes. Never overwrite unknown bytes.
bool ownedRollback(HANDLE process,uintptr_t base,const std::vector<Site>& sites,std::string& detail) {
    ProtectionScope protection(process); bool ok=true;
    try {
        for(auto& s:sites) {
            auto context=remoteRead(process,base+s.guardRva,s.guard.size());auto patchIndex=s.patchRva-s.guardRva;
            auto value=context[patchIndex];
            if(value==s.original) continue;
            if(value!=s.replacement) {ok=false;detail+=std::string(s.name)+": unexpected byte preserved; ";continue;}
            auto surrounding=context;surrounding[patchIndex]=s.original;
            if(surrounding!=s.guard){ok=false;detail+=std::string(s.name)+": changed surrounding context preserved; ";continue;}
            protection.add(base+s.patchRva); remoteByte(process,base+s.patchRva,s.original);
        }
        if(!protection.restore()) {ok=false;detail+="page protections could not be restored; ";}
        for(auto& s:sites) if(remoteRead(process,base+s.patchRva,1)[0]!=s.original) ok=false;
    } catch(const std::exception& e) {ok=false;detail+=e.what();}
    return ok;
}
std::wstring lower(std::wstring s) {std::transform(s.begin(),s.end(),s.begin(),[](wchar_t c){return wchar_t(towlower(c));});return s;}
bool antiCheatName(const std::wstring& name) {
    auto n=lower(name);
    return n.find(L"easyanticheat")!=std::wstring::npos || n.find(L"easy_anti_cheat")!=std::wstring::npos || n.find(L"battleye")!=std::wstring::npos || n==L"beservice.exe" || n==L"beservice_x64.exe" || n==L"eac_launcher.exe" || n==L"start_protected_game.exe";
}
void processPreflight() {
    Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0)); require(snapshot.valid(),"Cannot enumerate existing processes.");
    PROCESSENTRY32W p{};p.dwSize=sizeof(p);bool steam=false;
    require(Process32FirstW(snapshot,&p),"Process enumeration failed.");
    do {
        auto n=lower(p.szExeFile);if(n==L"steam.exe") steam=true;
        require(n!=L"acecombat8.exe" && n!=L"acecombat8-win64-shipping.exe","ACE COMBAT 8 is already running. Close it before using the launcher.");
        require(!antiCheatName(n),"An anti-cheat process is running. Close other protected games before launching this offline/campaign prototype.");
    }while(Process32NextW(snapshot,&p));
    require(steam,"Steam must already be running and signed in.");
}
void moduleGuard(DWORD pid, bool initial=false) {
    DWORD lastError=ERROR_SUCCESS;
    for(int attempt=0;attempt<20;attempt++) {
        Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid));
        if(!snapshot.valid()) {
            lastError=GetLastError();
            if(initial&&lastError==ERROR_PARTIAL_COPY)return; // Suspended loader has not initialized.
            if(lastError==ERROR_BAD_LENGTH||lastError==ERROR_PARTIAL_COPY){Sleep(25);continue;}
            throw std::runtime_error("Cannot enumerate child modules (Windows error "+std::to_string(lastError)+")");
        }
        MODULEENTRY32W m{};m.dwSize=sizeof(m);
        if(!Module32FirstW(snapshot,&m)) {
            lastError=GetLastError();
            if(initial&&lastError==ERROR_NO_MORE_FILES)return;
            if(lastError==ERROR_BAD_LENGTH||lastError==ERROR_PARTIAL_COPY||lastError==ERROR_NO_MORE_FILES){Sleep(25);continue;}
            throw std::runtime_error("Child module enumeration failed (Windows error "+std::to_string(lastError)+")");
        }
        do {require(!antiCheatName(m.szModule)&&!antiCheatName(fs::path(m.szExePath).filename().wstring()),"Anti-cheat module detected; prototype must stop.");}while(Module32NextW(snapshot,&m));
        lastError=GetLastError();
        if(lastError==ERROR_NO_MORE_FILES)return;
        if(lastError==ERROR_BAD_LENGTH||lastError==ERROR_PARTIAL_COPY){Sleep(25);continue;}
        throw std::runtime_error("Child module enumeration ended unexpectedly (Windows error "+std::to_string(lastError)+")");
    }
    if(initial&&(lastError==ERROR_BAD_LENGTH||lastError==ERROR_PARTIAL_COPY||lastError==ERROR_NO_MORE_FILES))return;
    throw std::runtime_error("Child module enumeration remained unavailable after 20 bounded retries (Windows error "+std::to_string(lastError)+")");
}
uintptr_t suspendedImageBase(HANDLE process,HANDLE thread) {
    CONTEXT context{};context.ContextFlags=CONTEXT_INTEGER;
    winCheck(GetThreadContext(thread,&context)!=FALSE,"GetThreadContext failed");
    auto bytes=remoteRead(process,uintptr_t(context.Rdx)+0x10,sizeof(uintptr_t));uintptr_t base=0;memcpy(&base,bytes.data(),sizeof(base));
    require(base!=0,"Suspended child image base is null.");
    auto dosBytes=remoteRead(process,base,sizeof(IMAGE_DOS_HEADER));IMAGE_DOS_HEADER dos{};memcpy(&dos,dosBytes.data(),sizeof(dos));
    require(dos.e_magic==IMAGE_DOS_SIGNATURE&&dos.e_lfanew>0&&dos.e_lfanew<0x100000,"Child DOS header invalid.");
    auto ntBytes=remoteRead(process,base+dos.e_lfanew,sizeof(IMAGE_NT_HEADERS64));IMAGE_NT_HEADERS64 nt{};memcpy(&nt,ntBytes.data(),sizeof(nt));
    require(nt.Signature==IMAGE_NT_SIGNATURE&&nt.FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64&&nt.OptionalHeader.ImageBase==kImageBase&&nt.OptionalHeader.SizeOfImage>0x72F1600,"Child PE image mismatch.");
    PROCESS_PROTECTION_LEVEL_INFORMATION protection{};
    winCheck(GetProcessInformation(process,ProcessProtectionLevelInfo,&protection,sizeof(protection))!=FALSE,"Cannot query child process protection");
    require(protection.ProtectionLevel==PROTECTION_LEVEL_NONE,"Refusing a protected child process.");
    return base;
}
std::vector<wchar_t> childEnvironment() {
    LPWCH raw=GetEnvironmentStringsW();require(raw!=nullptr,"Cannot copy current environment.");
    std::vector<std::wstring> entries;
    for(auto p=raw;*p;p+=wcslen(p)+1) {
        std::wstring e=p;auto key=lower(e.substr(0,e.find(L'=')));
        if(key!=L"steamappid"&&key!=L"steamgameid") entries.push_back(e);
    }
    FreeEnvironmentStringsW(raw);
    entries.push_back(L"SteamAppId=2288340");entries.push_back(L"SteamGameId=2288340");
    std::sort(entries.begin(),entries.end(),[](auto& a,auto& b){return _wcsicmp(a.c_str(),b.c_str())<0;});
    std::vector<wchar_t> block;
    for(auto& e:entries){block.insert(block.end(),e.begin(),e.end());block.push_back(0);}block.push_back(0);return block;
}

int launchGame(const fs::path& game,Report& report) {
    processPreflight();
    auto environment=childEnvironment();std::wstring command=L"\""+game.wstring()+L"\" -SaveToUserDir";
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION pi{};
    winCheck(CreateProcessW(game.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT,environment.data(),game.parent_path().c_str(),&startup,&pi)!=FALSE,"CreateProcessW failed");
    Handle process(pi.hProcess),thread(pi.hThread);report.pid=pi.dwProcessId;uintptr_t base=0;bool resumed=false;
    try {
        base=suspendedImageBase(process,thread);moduleGuard(pi.dwProcessId,true);
        transaction(process,base,kSites,true);report.patched=true;
        winCheck(ResumeThread(thread)!=DWORD(-1),"ResumeThread failed");resumed=true;
        report.line("Child resumed with four verified memory-only immediate changes. Use offline/campaign mode.");
        report.line("Keep this launcher open. Ctrl+Alt+F8 while the game is foreground toggles the HUD gates for A/B testing.");
        report.status("running_patched","Memory patches verified; visual HUD behavior still requires in-game validation.");
        constexpr int hotkeyId=0xAC8;
        bool hotkeyRegistered=RegisterHotKey(nullptr,hotkeyId,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,VK_F8)!=FALSE;
        struct HotkeyScope {bool active;int id;~HotkeyScope(){if(active)UnregisterHotKey(nullptr,id);}}hotkeyScope{hotkeyRegistered,hotkeyId};
        if(!hotkeyRegistered)report.line("Ctrl+Alt+F8 is already registered elsewhere; using key-state polling fallback (hold for at least 0.2 seconds). Windows error "+std::to_string(GetLastError()));
        bool keyWasDown=false;ULONGLONG nextCheck=GetTickCount64();
        for(;;) {
            auto wait=WaitForSingleObject(process,100);
            if(wait==WAIT_OBJECT_0) break;
            require(wait==WAIT_TIMEOUT,"Waiting for game process failed.");
            bool key=false,toggleRequested=false;
            if(hotkeyRegistered) {
                MSG message{};
                while(PeekMessageW(&message,nullptr,WM_HOTKEY,WM_HOTKEY,PM_REMOVE))if(message.wParam==hotkeyId)toggleRequested=true;
            }else {
                key=(GetAsyncKeyState(VK_CONTROL)&0x8000)&&(GetAsyncKeyState(VK_MENU)&0x8000)&&(GetAsyncKeyState(VK_F8)&0x8000);
                toggleRequested=key&&!keyWasDown;
            }
            if(toggleRequested) {
                DWORD foregroundPid=0;GetWindowThreadProcessId(GetForegroundWindow(),&foregroundPid);
                if(foregroundPid==pi.dwProcessId) {
                    moduleGuard(pi.dwProcessId);transaction(process,base,kSites,!report.patched);report.patched=!report.patched;
                    report.line(report.patched?"HUD gate patch enabled; four contexts verified.":"HUD gate patch disabled; original contexts verified.");
                    report.status(report.patched?"running_patched":"running_original","Ctrl+Alt+F8 toggle completed with exact memory readback.");
                }
            }
            keyWasDown=key;
            if(GetTickCount64()>=nextCheck) {
                if(WaitForSingleObject(process,0)==WAIT_OBJECT_0) break;
                moduleGuard(pi.dwProcessId);verifyRemote(process,base,kSites,report.patched);nextCheck=GetTickCount64()+1000;
            }
        }
        DWORD exitCode=0;GetExitCodeProcess(process,&exitCode);report.patched=false;
        report.line("Game process exited with code "+std::to_string(exitCode)+". On-disk game files were unchanged.");
        report.status("game_exited","Game exit code "+std::to_string(exitCode)+"; visual runtime verification remains manual.");return 0;
    }catch(const std::exception& e) {
        std::string detail=e.what();
        if(!resumed) {
            if(base&&report.patched){std::string recovery;ownedRollback(process,base,kSites,recovery);}
            TerminateProcess(process,1);WaitForSingleObject(process,5000);report.patched=false;
            detail+=" Suspended startup child stopped before gameplay.";
        }else if(WaitForSingleObject(process,0)==WAIT_OBJECT_0){report.patched=false;detail+=" Child already exited.";}
        else {
            std::string recovery;bool recovered=ownedRollback(process,base,kSites,recovery);
            report.patched=!recovered;detail+=recovered?" Owned patch bytes restored to original; game left running.":" Patch rollback could not be fully verified; game left running. Close the game manually.";
            if(!recovery.empty()) detail+=" "+recovery;
        }
        report.line("STOPPED: "+detail);report.status("monitor_or_startup_failure",detail);return 1;
    }
}

struct TestIPC { uintptr_t base; LONG ready; };
std::vector<Site> testSites() {
    auto sites=kSites;const uint32_t offsets[]={0x100,0x1100,0x2100,0x2200};
    for(size_t i=0;i<sites.size();i++){sites[i].guardRva=offsets[i];sites[i].patchRva=offsets[i]+17;}return sites;
}
int testChild(const wchar_t* mappingName) {
    Handle map(OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,mappingName));if(!map.valid())return 20;
    auto ipc=static_cast<TestIPC*>(MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(TestIPC)));if(!ipc)return 21;
    auto memory=static_cast<uint8_t*>(VirtualAlloc(nullptr,0x4000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));if(!memory)return 22;
    for(auto& s:testSites())memcpy(memory+s.guardRva,s.guard.data(),s.guard.size());
    DWORD old=0;if(!VirtualProtect(memory,0x4000,PAGE_EXECUTE_READ,&old))return 23;
    ipc->base=reinterpret_cast<uintptr_t>(memory);InterlockedExchange(&ipc->ready,1);
    Sleep(INFINITE);return 0;
}
DWORD pageProtection(HANDLE process,uintptr_t address) {
    MEMORY_BASIC_INFORMATION mb{};require(VirtualQueryEx(process,reinterpret_cast<LPCVOID>(address),&mb,sizeof(mb))==sizeof(mb),"Test VirtualQueryEx failed.");return mb.Protect;
}
void corruptTestByte(HANDLE process,uintptr_t address,uint8_t value) {ProtectionScope p(process);p.add(address);remoteByte(process,address,value);require(p.restore(),"Test protection restore failed.");}
template<typename F> void expectFailure(F&& operation,const char* label) {
    bool rejected=false;try{operation();}catch(const std::exception&){rejected=true;}require(rejected,std::string("Self-test failed to reject: ")+label);
}
int selfTest(Report& report) {
    auto name=L"Local\\AC8HUDTest-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());
    Handle map(CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(TestIPC),name.c_str()));require(map.valid(),"Self-test mapping failed.");
    auto ipc=static_cast<TestIPC*>(MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(TestIPC)));require(ipc!=nullptr,"Self-test mapping view failed.");
    struct Unmap {void* p;~Unmap(){UnmapViewOfFile(p);}}unmap{ipc};ipc->base=0;ipc->ready=0;
    auto own=ownPath();std::wstring command=L"\""+own.wstring()+L"\" --test-child \""+name+L"\"";
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION pi{};
    require(CreateProcessW(own.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,own.parent_path().c_str(),&startup,&pi),"Self-test child creation failed.");
    Handle process(pi.hProcess),thread(pi.hThread);
    struct StopChild{HANDLE p;~StopChild(){TerminateProcess(p,0);WaitForSingleObject(p,5000);}}stop{process};
    for(int i=0;i<100 && InterlockedCompareExchange(&ipc->ready,0,0)!=1;i++) {require(WaitForSingleObject(process,50)==WAIT_TIMEOUT,"Self-test child exited early.");}
    require(ipc->ready==1&&ipc->base,"Self-test child did not publish its memory buffer.");
    auto sites=testSites();uintptr_t base=ipc->base;int passed=0;
    auto pass=[&](const char* label){report.line(std::string("PASS: ")+label);passed++;};
    for(size_t i=0;i<kSites.size();i++) {
        auto& s=kSites[i];require(s.guard[16]==0x3c&&s.guard[17]==s.original,"Self-test unexpected compare opcode.");
        unsigned changedModes=0;
        for(unsigned mode=0;mode<256;mode++) {
            bool before=false,after=false;
            if(i<2) {require(s.guard[21]==0x72,"Self-test unexpected jb opcode.");before=mode<s.original;after=mode<s.replacement;}
            else if(i==2) {require(s.guard[18]==0x0f&&s.guard[19]==0x87,"Self-test unexpected ja opcode.");before=mode<=s.original;after=mode<=s.replacement;}
            else {require(s.guard[23]==0x76,"Self-test unexpected jbe opcode.");before=mode<=s.original;after=mode<=s.replacement;}
            if(before!=after){changedModes++;require(mode==2&&!before&&after,"Gate semantics changed an unintended camera value.");}
        }
        require(changedModes==1,"Gate semantics did not change exactly one camera value.");
    }
    pass("all 256 camera values: only mode 2 acceptance changes at each gate");
    verifyRemote(process,base,sites,false);pass("cross-process original context read");
    auto hashTestPath=report.folder/L"sha256-synthetic-test.tmp";
    Handle hashTest(CreateFileW(hashTestPath.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,nullptr));
    require(hashTest.valid(),"Cannot open synthetic hash test file.");DWORD written=0;
    require(WriteFile(hashTest,"abc",3,&written,nullptr)&&written==3,"Cannot write synthetic hash test file.");
    auto syntheticHash=sha256(hashTest);
    require(syntheticHash=="BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD","BCrypt full-file SHA256 failed the known abc vector.");
    pass("BCrypt full-file SHA256 matches the known abc test vector");
    expectFailure([&]{hashGuard(syntheticHash);},"unsupported hash");pass("unsupported computed full SHA256 rejected");
    hashGuard(kExpectedHash);pass("supported full SHA256 accepted");
    corruptTestByte(process,base+sites[2].patchRva,0x7f);
    expectFailure([&]{transaction(process,base,sites,true);},"unexpected immediate");
    require(remoteRead(process,base+sites[0].patchRva,1)[0]==sites[0].original,"Preflight changed an earlier patch.");
    corruptTestByte(process,base+sites[2].patchRva,sites[2].original);pass("preflight rejects unexpected immediate without partial writes");
    uint32_t branchOffset=sites[0].patchRva+4;uint8_t branch=remoteRead(process,base+branchOffset,1)[0];
    corruptTestByte(process,base+branchOffset,uint8_t(branch^1));
    expectFailure([&]{transaction(process,base,sites,true);},"modified surrounding instruction");
    corruptTestByte(process,base+branchOffset,branch);pass("48-byte guard rejects modified branch instruction");
    expectFailure([&]{transaction(process,base,sites,true,true);},"injected failure");
    verifyRemote(process,base,sites,false);for(auto& s:sites)require(pageProtection(process,base+s.patchRva)==PAGE_EXECUTE_READ,"Rollback did not restore page protections.");
    pass("injected write failure rolls back all bytes and page protections");
    expectFailure([&]{transaction(process,base,sites,true,false,true);},"injected partial page-protection restore failure");
    verifyRemote(process,base,sites,false);for(auto& s:sites)require(pageProtection(process,base+s.patchRva)==PAGE_EXECUTE_READ,"Partial protection restore rollback did not restore RX pages.");
    pass("partial protection restore failure reopens restored RX pages and rolls back safely");
    transaction(process,base,sites,true);verifyRemote(process,base,sites,true);
    for(auto& s:sites)require(pageProtection(process,base+s.patchRva)==PAGE_EXECUTE_READ,"Success did not restore page protection.");
    pass("four remote one-byte writes, exact readback, and page protection restoration");
    transaction(process,base,sites,false);verifyRemote(process,base,sites,false);pass("A/B disable transaction restores all original guarded contexts");
    transaction(process,base,sites,true);corruptTestByte(process,base+sites[0].guardRva,0x7f);
    std::string contextDetail;require(!ownedRollback(process,base,sites,contextDetail),"Owned rollback accepted changed surrounding instructions.");
    require(remoteRead(process,base+sites[0].patchRva,1)[0]==sites[0].replacement,"Owned rollback modified a site with rewritten context.");
    require(remoteRead(process,base+sites[0].guardRva,1)[0]==0x7f,"Owned rollback modified rewritten surrounding bytes.");
    for(size_t i:{size_t(1),size_t(2),size_t(3)})require(remoteRead(process,base+sites[i].patchRva,1)[0]==sites[i].original,"Owned rollback failed for an intact context.");
    corruptTestByte(process,base+sites[0].guardRva,sites[0].guard[0]);corruptTestByte(process,base+sites[0].patchRva,sites[0].original);
    pass("failure recovery preserves replacement bytes when surrounding code was rewritten");
    transaction(process,base,sites,true);corruptTestByte(process,base+sites[1].patchRva,0x7f);
    std::string detail;require(!ownedRollback(process,base,sites,detail),"Owned rollback accepted an unknown byte.");
    require(remoteRead(process,base+sites[1].patchRva,1)[0]==0x7f,"Owned rollback overwrote an unknown byte.");
    for(size_t i:{size_t(0),size_t(2),size_t(3)})require(remoteRead(process,base+sites[i].patchRva,1)[0]==sites[i].original,"Owned rollback failed at known replacement.");
    pass("failure recovery restores owned bytes and preserves unexpected bytes");
    report.line(std::to_string(passed)+" synthetic cross-process checks passed. The game was not launched.");
    report.status("self_test_passed",std::to_string(passed)+" synthetic checks passed; game was not launched.");return 0;
}

int wmain(int argc,wchar_t** argv) {
    SetConsoleOutputCP(CP_UTF8);
    if(argc==3&&std::wstring(argv[1])==L"--test-child")return testChild(argv[2]);
    fs::path game=kDefaultGame;fs::path output=ownPath().parent_path()/L"logs";bool verifyOnly=false,test=false,help=false;
    try {
        for(int i=1;i<argc;i++) {
            std::wstring arg=argv[i];
            if(arg==L"--verify")verifyOnly=true;
            else if(arg==L"--self-test")test=true;
            else if(arg==L"--help"||arg==L"-h")help=true;
            else if(arg==L"--game"&&i+1<argc)game=argv[++i];
            else if(arg==L"--output"&&i+1<argc)output=argv[++i];
            else throw std::runtime_error("Unknown or incomplete argument: "+utf8(arg));
        }
        if(help) {
            std::cout << "AC8 HUD Launcher " << kVersion << " (Windows x64, offline/campaign prototype)\n"
                "No arguments: verify supported installed game, start suspended, patch four memory bytes, monitor until exit.\n"
                "--verify     Read-only full-file and instruction verification; never launch game.\n"
                "--self-test  Exercise remote transactions against synthetic child; never launch game.\n"
                "--game PATH  Override game executable; exact supported SHA256 is still required.\n"
                "--output DIR Store this launcher's log/status files in DIR (default: logs beside launcher).\n"
                "Ctrl+Alt+F8  Enable/disable gates while game is foreground and launcher remains open.\n";return 0;
        }
        Report report(fs::absolute(output));report.game=fs::absolute(game);
        try {
            report.line(std::string("AC8 HUD Launcher ")+kVersion+" | supported Steam build "+kBuild);
            if(test){require(!verifyOnly,"Use --self-test and --verify separately.");return selfTest(report);}
            Handle file(CreateFileW(report.game.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr));
            winCheck(file.valid(),"Cannot open game executable for read-only verification");
            verifyFile(file,report.game,report);
            if(verifyOnly){report.status("static_verified","Read-only executable and all four instruction contexts verified; game was not launched.");return 0;}
            return launchGame(report.game,report);
        }catch(const std::exception& e){report.line(std::string("FAILED: ")+e.what());report.status("failed",e.what());return 1;}
    }catch(const std::exception& e){std::cerr<<"FAILED: "<<e.what()<<std::endl;return 1;}
}
