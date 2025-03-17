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

int parityBit(int value, int bits = 8)
{
    int count = 0;
    for (int i = 0; i < bits; i++) {
        count += (value >> i) & 1;
    }
    return count % 2; 
}


vector<uint8_t> encodeBytes(const wstring& input) {
    vector<uint8_t> output;

    for (const wchar_t& ch : input) {
        uint16_t character = static_cast<uint16_t>(ch);
        for (int bitPos = 0; bitPos < 16; bitPos++) { 
            uint8_t bit = (character >> bitPos) & 1;
            output.push_back(bit);
        }
    }

    return output;
}


void saveToFile(const string& filename, const vector<uint8_t>& data) 
{
    ofstream outFile(filename, ios::binary);
    outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
    outFile.close();
}


wstring readFromFile(const string& filename)
{
    wifstream inFile(filename);
    inFile.imbue(locale(inFile.getloc(), new codecvt_utf8<wchar_t>()));
    wstring data;
    wchar_t ch;
    while (inFile.get(ch)) 
    {
        data += ch;
    }
    inFile.close();
    return data;
}



void encryptAndSave(const wstring& inputFile, const wstring& binaryFile) 
{
    // Перевіряємо, чи існує вхідний файл
    {
        wifstream checkFile(inputFile);
        if (!checkFile) {
            wcout << L"Вхідний файл не знайдено. Створення нового файлу з прикладом тексту." << endl;

            // Створюємо вхідний файл з прикладом тексту
            wofstream createInputFile(inputFile);
            if (createInputFile) 
            {
                createInputFile << L"Hello world!\nThis is a test.\nShort\nAnother line.";
                createInputFile.close();
            }
            else 
            {
                wcerr << L"Не вдалося створити вхідний файл!" << endl;
                return;
            }
        }
    }


    {
        ifstream checkFile(binaryFile, ios::binary);
        if (!checkFile) 
        {
            wcout << L"Бінарний файл не знайдено. Він буде створений під час шифрування." << endl;
        }
    }

    wifstream inFile(inputFile);
    ofstream outFile(binaryFile, ios::binary);

    inFile.imbue(locale(locale(), new codecvt_utf8<wchar_t>()));

    if (!inFile || !outFile) {
        wcerr << L"Помилка відкриття файлу!" << endl;
        return;
    }

    vector<wstring> lines(16, wstring(16, L' ')); 
    int row = 0;

    while (row < 16) 
    {
        wchar_t ch;
        int col = 0;

        while (inFile.get(ch)) 
        {
            if (ch == L'\n') break; 
            if (col >= 16) break;  
            lines[row][col++] = ch;
        }

        row++;
        if (inFile.eof()) break; 
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            wchar_t ch = lines[i][j];
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

    inFile.close();
    outFile.close();
    wcout << L"Шифрування завершено! Дані збережено у " << binaryFile << endl;
}


void decryptAndShow(const wstring& binaryFile, const wstring& outputFile) {
    {
        ifstream checkFile(binaryFile, ios::binary);
        if (!checkFile) {
            wcout << L"Бінарний файл не знайдено. Створення порожнього файлу." << endl;

  
            ofstream createBinaryFile(binaryFile, ios::binary);
            if (!createBinaryFile) {
                wcerr << L"Не вдалося створити бінарний файл!" << endl;
                return;
            }
            createBinaryFile.close();
        }
    }


    {
        wofstream checkFile(outputFile);
        if (!checkFile) {
            wcout << L"Вихідний файл не знайдено. Він буде створений під час розшифрування." << endl;
        }
    }

    ifstream inFile(binaryFile, ios::binary);
    wofstream outFile(outputFile);

    outFile.imbue(locale(locale(), new codecvt_utf8<wchar_t>()));

    if (!inFile || !outFile) {
        wcerr << L"Помилка відкриття файлу!" << endl;
        return;
    }

    vector<wstring> lines(16, wstring(16, L' '));
    EncodedChar encodedChar;

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            inFile.read(reinterpret_cast<char*>(&encodedChar.value), sizeof(uint16_t));

            int asciiCode = (encodedChar.bits.highAsciiBits << 4) | encodedChar.bits.lowAsciiBits;
            wchar_t ch = static_cast<wchar_t>(asciiCode);

            lines[encodedChar.bits.rowBits][encodedChar.bits.positionBits] = ch;
        }
    }

    wcout << L"Розшифрований текст:\n";
    for (const auto& line : lines) {
        wcout << line << endl;
        outFile << line << endl;
    }

    inFile.close();
    outFile.close();
    wcout << L"Розшифрування завершено! Дані збережено у " << outputFile << endl;
}


void Task1() {
    int a, b, c, d;
    float res;

    wcout << L"Введіть значення a, b, c, d: ";
    wcin >> a >> b >> c >> d;

    try {
        // Обчислюємо результат за заданою формулою
        res = Divide((Multiply(a, 18) + Multiply(312, d)), 512)
            - Multiply(122, b)
            + Multiply(c, 123);

        // Виводимо результат з точністю до 6 знаків після коми
        wcout << L"Результат: " << fixed << setprecision(6) << res << endl;
    }
    catch (const exception& e) {
        wcerr << L"Помилка: " << e.what() << endl;
    }
}

void Task2()
{
    wstring inputTextFile = L"input.txt";
    wstring encryptedFile = L"encrypted.bin";
    encryptAndSave(inputTextFile, encryptedFile);
}
void Task3() {
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

    wcout << L"Вхідний текст: " << input << endl;

    vector<uint8_t> encoded = encodeBytes(input);

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
        case 0: break;
        default: wcout << L"Невірний вибір!" << endl;
        }
    } while (choice != 0);

    return 0;
}

