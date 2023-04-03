// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:
    #include <iostream>
    #include <string>

    using namespace std;

    bool check(int num) {
        string str = to_string(num);
        for (char c : str) {
            if(c == '3') {
                return true;
            }
        }
        return false;
    }

    int main() {
        int answ = 0;
        for (int i = 1; i <=1000; ++i) {
            if (check(i)) {
                ++answ;
            }
        }
        cout << answ << endl;
    }
// Закомитьте изменения и отправьте их в свой репозиторий.
