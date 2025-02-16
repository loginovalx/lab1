#include <iostream>
#include <thread>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

std::string caesar_cipher(const std::string& text, int shift) {
    std::string result = "";
    for (char ch : text) {
        if (isalpha(ch)) {
            char base = islower(ch) ? 'a' : 'A';
            ch = static_cast<char>((ch - base + shift) % 26 + base);
        }
        result += ch;
    }
    return result;
}

void receive_messages(SOCKET sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cout << "Соединение с сервером потеряно.\n";
            break;
        }
        std::string encrypted_message(buffer, bytes_received);
        std::string decrypted_message = caesar_cipher(encrypted_message, -3); // Расшифровка
        std::cout << "Сообщение: " << decrypted_message << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации Winsock\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка подключения\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::thread(receive_messages, sock).detach();

    std::cout << "Введите сообщения (exit для выхода):\n";
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "exit") break;
        std::string encrypted_message = caesar_cipher(message, 3); // Шифрование
        send(sock, encrypted_message.c_str(), encrypted_message.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}