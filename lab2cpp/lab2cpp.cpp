#include <iostream>
#include <vector>
#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <bitset>
#include <locale>
#include <codecvt>
#include <stdexcept> 
#include <iomanip>
using namespace std;


union EncodedChar 
{
    struct 
    {
        uint16_t rowBits : 4;        
        uint16_t lowAsciiBits : 4;    
        uint16_t parity1 : 1;         
        uint16_t highAsciiBits : 4;  
        uint16_t positionBits : 2;     
        uint16_t parity2 : 1;         
    } bits;
    uint16_t value;                   
};

int Multiply(int a, int b) {
    int result = 0;
    bool negative = false;

   
    if ((a < 0 && b > 0) || (a > 0 && b < 0)) {
        negative = true;
    }

  
    a = abs(a);
    b = abs(b);

   
    while (b > 0) {
        if (b & 1) { 
            result += a;
        }
        a <<= 1;
        b >>= 1;
    }

    return negative ? -result : result;
}

float Divide(int dividend, int divisor) {
    if (divisor == 0) {
        throw runtime_error("Division by zero is not allowed.");
    }

    bool negative = false;


    if ((dividend < 0 && divisor > 0) || (dividend > 0 && divisor < 0)) {
        negative = true;
    }


    int absDividend = abs(dividend);
    int absDivisor = abs(divisor);

    float quotient = 0.0f;

    // Ціла частина ділення
    while (absDividend >= absDivisor) {
        int power = 0;
        int temp = absDivisor;


        while (temp <= absDividend && temp > 0) {
            temp <<= 1;
            power++;
        }


        temp >>= 1;
        power--;

        absDividend -= temp;

      
        quotient += (1 << power);
    }

    // Дробова частина ділення
    const int precision = 6; 
    float fraction = 0.0f;
    float factor = 0.5f; 

    for (int i = 0; i < precision; ++i) {
        absDividend <<= 1; 
        if (absDividend >= absDivisor) {
            fraction += factor;
            absDividend -= absDivisor;
        }
        factor /= 2.0f; 
    }

    quotient += fraction;

    return negative ? -quotient : quotient;
}

void Task1() 
{
    int a, b, c, d;
    float res;

    wcout << L"Введіть значення a, b, c, d: ";
    wcin >> a >> b >> c >> d;

    try {
        res = Divide((Multiply(a, 18) + Multiply(312, d)), 512)
            - Multiply(122, b)
            + Multiply(c, 123);

        wcout << L"Результат: " << fixed << setprecision(6) << res << endl;
    }
    catch (const exception& e) {
        wcerr << L"Помилка: " << e.what() << endl;
    }
}

int parityBit(int value, int bits = 8)
{
    int count = 0;
    for (int i = 0; i < bits; i++)
    {
        count += (value >> i) & 1;
    }
    return count % 2; 
}


vector<uint8_t> encodeBytes(const vector<uint8_t>& input) 
{

    if (input.size() % 8 != 0) 
    {
        throw invalid_argument("Довжина вхідного масиву повинна бути кратна 8.");
    }
    vector<uint8_t> output(input.size(), 0);
    for (size_t i = 0; i < input.size(); i += 8) 
    {
        for (size_t j = 0; j < 8; j++) {
           
            for (size_t k = 0; k < 8; k++) {
                uint8_t bit = (input[i + j] >> k) & 1; 
                output[i + k] |= (bit << j); 
            }
        }
    }

    return output;
}


void encryptAndSave(const wstring& binaryFile) {
    vector<wstring> inputLines(16, wstring(16, L' ')); 

    wcout << L"Введіть текст (16 рядків, кожен до 16 символів):" << endl;
    for (int i = 0; i < 16; i++) {
        wcout << L"Рядок " << i + 1 << L": ";
        wcin >> inputLines[i];
        if (inputLines[i].size() < 16) {
            inputLines[i].resize(16, L' '); 
        }
        else {
            inputLines[i] = inputLines[i].substr(0, 16); 
        }
    }

    ofstream outFile(binaryFile, ios::binary);
    if (!outFile) {
        wcerr << L"Помилка: Не вдалося відкрити бінарний файл для запису!" << endl;
        return;
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            wchar_t ch = inputLines[i][j];
            int asciiCode = static_cast<int>(ch);

            EncodedChar encodedChar;

            encodedChar.bits.rowBits = i & 0b1111;
            encodedChar.bits.lowAsciiBits = asciiCode & 0b1111;
            encodedChar.bits.highAsciiBits = (asciiCode >> 4) & 0b1111;
            encodedChar.bits.positionBits = j & 0b11;
            encodedChar.bits.parity1 = parityBit(encodedChar.bits.rowBits | (encodedChar.bits.lowAsciiBits << 4), 8);
            encodedChar.bits.parity2 = parityBit(encodedChar.bits.highAsciiBits | (encodedChar.bits.positionBits << 4), 8);

            outFile.write(reinterpret_cast<char*>(&encodedChar.value), sizeof(uint16_t));
        }
    }

    outFile.close();
    wcout << L"Шифрування завершено! Дані збережено у " << binaryFile << endl;
}

void decryptAndShow(const wstring& binaryFile) {
    ifstream inFile(binaryFile, ios::binary);
    if (!inFile) {
        wcerr << L"Помилка: Не вдалося відкрити бінарний файл для читання!" << endl;
        return;
    }

    vector<wstring> lines(16, wstring(16, L' ')); 
    EncodedChar encodedChar;

    while (inFile.read(reinterpret_cast<char*>(&encodedChar.value), sizeof(uint16_t))) {
    
        int asciiCode = (encodedChar.bits.highAsciiBits << 4) | encodedChar.bits.lowAsciiBits;
        wchar_t ch = static_cast<wchar_t>(asciiCode);

  
        int row = encodedChar.bits.rowBits;
        int col = encodedChar.bits.positionBits;

        if (row >= 0 && row < 16 && col >= 0 && col < 4) {
            lines[row][col] = ch;
        }
    }

    inFile.close();


    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (lines[i][j] == L'\0') { 
                lines[i][j] = L' ';     
            }
        }
    }

    wcout << L"Розшифрований текст:\n";
    for (const auto& line : lines) {
        wcout << line << endl;
    }

    wcout << L"Розшифрування завершено!" << endl;
}


void saveToFile(const string& filename, const vector<uint16_t>& data) {
    ofstream outFile(filename, ios::binary);
    if (!outFile) {
        cerr << "Помилка: Не вдалося відкрити файл '" << filename << "' для запису!" << endl;
        return;
    }
    outFile.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint16_t));
    outFile.close();
}

void saveToFile(const string& filename, const vector<uint8_t>& data) {
    ofstream outFile(filename, ios::binary);
    if (!outFile) {
        cerr << "Помилка: Не вдалося відкрити файл '" << filename << "' для запису!" << endl;
        return;
    }
    outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
    outFile.close();
}



void encryptAndSave2(const wstring& binaryFile) {
    vector<wstring> inputLines(16, wstring(16, L' ')); 

    wcout << L"Введіть текст (16 рядків, кожен до 16 символів):" << endl;
    for (int i = 0; i < 16; i++) {
        wcout << L"Рядок " << i + 1 << L": ";
        wcin >> inputLines[i];
        if (inputLines[i].size() < 16) {
            inputLines[i].resize(16, L' '); 
        }
        else {
            inputLines[i] = inputLines[i].substr(0, 16); 
        }
    }

    ofstream outFile(binaryFile, ios::binary);
    if (!outFile) {
        wcerr << L"Помилка: Не вдалося відкрити бінарний файл для запису!" << endl;
        return;
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            wchar_t ch = inputLines[i][j];

            if (ch == L' ') continue;

            int asciiCode = static_cast<int>(ch);

            uint16_t encryptedValue = 0;
            encryptedValue |= (i & 0b1111) << 12;
            encryptedValue |= (asciiCode & 0b1111) << 8;
            encryptedValue |= parityBit((i & 0b1111) | ((asciiCode & 0b1111) << 4)) << 7;
            encryptedValue |= ((asciiCode >> 4) & 0b1111) << 3;
            encryptedValue |= (j & 0b11) << 1;
            encryptedValue |= parityBit(((asciiCode >> 4) & 0b1111) | ((j & 0b11) << 4));

            outFile.write(reinterpret_cast<char*>(&encryptedValue), sizeof(uint16_t));
        }
    }

    outFile.close();
    wcout << L"Шифрування завершено! Дані збережено у " << binaryFile << endl;
}

void decryptAndShow2(const wstring& binaryFile) {
    ifstream inFile(binaryFile, ios::binary);
    if (!inFile) {
        wcerr << L"Помилка: Не вдалося відкрити бінарний файл для читання!" << endl;
        return;
    }

    vector<wstring> lines(16, wstring(16, L' ')); 

    while (true) {
        uint16_t encryptedValue;
        inFile.read(reinterpret_cast<char*>(&encryptedValue), sizeof(uint16_t));
        if (!inFile) break; 


        int rowBits = (encryptedValue >> 12) & 0b1111; 
        int lowAsciiBits = (encryptedValue >> 8) & 0b1111; 
        int parity1 = (encryptedValue >> 7) & 0b1; 
        int highAsciiBits = (encryptedValue >> 3) & 0b1111; 
        int positionBits = (encryptedValue >> 1) & 0b11; 
        int parity2 = encryptedValue & 0b1; 

   
        bool isValidParity1 = (parity1 == parityBit((rowBits & 0b1111) | (lowAsciiBits << 4)));
        bool isValidParity2 = (parity2 == parityBit((highAsciiBits & 0b1111) | (positionBits << 4)));

        if (!isValidParity1 || !isValidParity2) {
            wcerr << L"Помилка: Неправильна парність у зашифрованих даних!" << endl;
            continue; 
        }

    
        int asciiCode = (highAsciiBits << 4) | lowAsciiBits;
        wchar_t ch = static_cast<wchar_t>(asciiCode);


  
        if (rowBits >= 0 && rowBits < 16 && positionBits >= 0 && positionBits < 16) {
            lines[rowBits][positionBits] = ch;
        }
    }

    inFile.close();


    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (lines[i][j] == L'\0') { 
                lines[i][j] = L' ';    
            }
        }
    }

    wcout << L"Розшифрований текст:\n";
    for (const auto& line : lines) {
        wcout << line << endl;
    }

    wcout << L"Розшифрування завершено!" << endl;
}

void Task2() {
    wstring encryptedFile = L"encrypted.bin";
    encryptAndSave2(encryptedFile);
    decryptAndShow2(encryptedFile);
}


void Task3() {
    wstring encryptedFile = L"encrypted.bin";
    encryptAndSave(encryptedFile);
    decryptAndShow(encryptedFile);
}

void Task4() {
    wstring input;
    wcout << L"Введіть текст (натисніть '#' і Enter, щоб завершити): ";

    wchar_t ch;
    while (wcin.get(ch)) {
        if (ch == L'#') break;
        if (ch == L'\n') continue;
        input += ch;


        if (input.size() >= 1024) {
            wcout << L"\nДосягнута максимальна довжина тексту (1024 символи). Введення завершено." << endl;
            break;
        }
    }

    if (input.empty()) {
        wcout << L"Введений текст порожній!" << endl;
        return;
    }

    vector<uint8_t> byteData;
    for (wchar_t ch : input) {
        uint16_t unicodeValue = static_cast<uint16_t>(ch); 
        byteData.push_back(static_cast<uint8_t>(unicodeValue & 0xFF));       
        byteData.push_back(static_cast<uint8_t>((unicodeValue >> 8) & 0xFF));
    }


    while (byteData.size() % 8 != 0) {
        byteData.push_back(0);
    }

    wcout << L"Вхідний текст: " << input << endl;

    vector<uint8_t> encoded = encodeBytes(byteData);

    wcout << L"Закодовані біти:\n";
    for (size_t i = 0; i < encoded.size(); i++) {
        if (i % 16 == 0 && i != 0) wcout << endl;
        wcout << encoded[i] << L" ";
    }
    wcout << endl;

    saveToFile("encoded.bin", encoded);
    wcout << L"Дані збережено у файл 'encoded.bin'" << endl;
}


int main()
{
    locale::global(locale(locale(), new codecvt_utf8<wchar_t>()));
    _setmode(_fileno(stdout), _O_U8TEXT);
    _setmode(_fileno(stdin), _O_U8TEXT);

    int choice;
    do 
    {
        wcout << L"\nВиберіть завдання (1-3) або 0 для виходу: ";
        wcin >> choice;

        switch (choice) {
        case 1: Task1(); break;
        case 2: Task2(); break;
        case 3: Task3(); break;
        case 4: Task4(); break;
        case 0: break;
        default: wcout << L"Невірний вибір!" << endl;
        }
    } while (choice != 0);

    return 0;
}

