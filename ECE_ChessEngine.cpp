#include "ECE_ChessEngine.hpp"

HANDLE hInputWrite, hInputRead;
HANDLE hOutputWrite, hOutputRead;

bool InitializeEngine() 
{
    // Create pipes for input and output
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    CreatePipe(&hOutputRead, &hOutputWrite, &sa, 0);
    CreatePipe(&hInputRead, &hInputWrite, &sa, 0);

    // Start the Komodo engine
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hInputRead;
    si.hStdOutput = hOutputWrite;
    si.hStdError = hOutputWrite;

    // Path to Komodo executable
    std::string enginePath = "dragon-64bit.exe";
    if (!CreateProcess(NULL, const_cast<char*>(enginePath.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to start engine" << std::endl;
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    sendMove("uci");
    std::cout << "Engine Response: " << ReadFromEngine() << std::endl;

    sendMove("isready");
    std::cout << "Engine Response: " << ReadFromEngine() << std::endl;

	return true;
}

bool sendMove(const std::string& strMove)
{
    DWORD written;
    WriteFile(hInputWrite, strMove.c_str(), strMove.length(), &written, NULL);
    WriteFile(hInputWrite, "\n", 1, &written, NULL);

	return true;
}

bool getResponseMove(std::string& strMove)
{
    // use the output to interact with the movement object
    std::string response;
    while ((response = ReadFromEngine()).find("bestmove") == std::string::npos) {
        std::cout << "Engine Response: " << response << std::endl;
    }

    std::cout << "Engine best move: " << response << std::endl;
    strMove = response;
    std::cout << strMove << std::endl;

    std::regex moveRegex("^bestmove ([a-h][1-8])([a-h][1-8])$");

    //if (std::reg)

    // Return true on returning call from object

	return true;
}

std::string ReadFromEngine() {
    char buffer[4096];
    DWORD read;
    std::string output;
    if (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0)
    {
        buffer[read] = '\0';
        output = buffer;
    }
    return output;
}