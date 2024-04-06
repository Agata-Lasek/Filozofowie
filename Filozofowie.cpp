#include <iostream>             // Biblioteka do obsługi strumieni wejścia/wyjścia
#include <thread>               // Biblioteka do obsługi wielowątkowości
#include <mutex>                // Biblioteka do obsługi mutexów
#include <chrono>               // Biblioteka do obsługi czasu
#include <condition_variable>   // Biblioteka do obsługi zmiennych warunkowych
#include <queue>                // Biblioteka do obsługi kolejek
#include <random>               // Biblioteka do obsługi generatorów liczb losowych
using namespace std;

const int liczba_filozofow = 5;     // Liczba filozofów
int siedzenia = 4;                  // Limit miejsc przy stole

mutex widelec[liczba_filozofow];    // Mutexy reprezentujące widelce (tylko jeden wątek na raz może mieć dostęp do chronionego zasobu "widelca")
mutex kolejka_mutex;                // Mutex do kontrolowania dostępu do kolejki
condition_variable wolne_miejsce;   // Warunek dla wolnych miejsc przy stole (umożliwia wątkowi czekanie na określony sygnał)
queue<int> kolejka;                 // Kolejka filozofów oczekujących na dostęp, klasa w C++, FIFO
int ile_je = 0;                     // Liczba filozofów aktualnie jedzących

// Generator liczb losowych, aby zatrzymywali widelce przez różną długość czasu
random_device rd;       //random_device generuje losowe liczby na podstawie zróżnicowanych źródeł, takich jak ruch myszy, czas systemowy, szumy urządzeń itp. Jest to najbardziej nieprzewidywalne źródło losowości
mt19937 gen(rd());      //Tworzy generator liczb pseudolosowych

// Dystrybucja równomierna na przedziale [1, 3] sekund
uniform_int_distribution<> losowy_czas(1000, 3000);

void filozof(int id)    // Funkcja reprezentująca działanie pojedynczego filozofa
{
    while (true)        // Pętla nieskończona - filozof będzie jadł lub myślał przez cały czas
        { 
            {
                unique_lock<mutex> lock(kolejka_mutex);         // klasa C++ unique_lock może być blokowany i odblokowywany wielokrotnie w różnych miejscach w kodzie
                kolejka.push(id);                               // Dodaj filozofa do kolejki, po id, filozofowie dodawani są do kolejki w kolejności, w jakiej zgłaszają swoje żądania dostępu do stołu.
            }

            int lewy_widelec = id;                              // Numer lewego widelca dla danego filozofa
            int prawy_widelec = (id + 1) % liczba_filozofow;    // Numer prawego widelca dla danego filozofa

            {
                unique_lock<mutex> lock(kolejka_mutex);
                wolne_miejsce.wait(lock, [id] { return ile_je < siedzenia && kolejka.front() == id; }); // wątek czeka, dopóki nie będzie wolnego miejsca,                                                                                                                                      funkcja lambda, która zwraca wartość logiczną w zależności od spełnienia warunku oczekiwania
                ile_je++;
                kolejka.pop(); // Usuń filozofa z kolejki
            }

            // Każdy filozof czeka na dostęp do widelców w określonej kolejności
            {
                lock_guard<mutex> lock(widelec[lewy_widelec]);      // Zablokuj lewy widelec, automatyczne zarządzanie mutexem, mutex zostanie automatycznie zablokowany na czas życia obiektu lock_guard, i odblokowany
                lock_guard<mutex> lock2(widelec[prawy_widelec]);    // Zablokuj prawy widelec
                cout << "Filozof " << id << " sięga po widelec " << lewy_widelec << " i widelec " << prawy_widelec << endl; // Wyświetl informację o sięganiu po widelce
                cout << "Filozof " << id << " je" << endl;          // Wyświetl informację o jedzeniu

                // Losowy czas trzymania widelców
                int czas = losowy_czas(gen);                        // Losuj czas
                this_thread::sleep_for(chrono::milliseconds(czas)); // Poczekaj przez wylosowany czas

                cout << "Filozof " << id << " kończy jeść" << endl; // Wyświetl informację o kończeniu jedzenia
            }

            {
                unique_lock<mutex> lock(kolejka_mutex);
                ile_je--;
                wolne_miejsce.notify_all(); // Powiadom o zwolnieniu miejsca przy stole
            }
        }
}

int main()  // Funkcja główna programu
 {
    thread filozofowie[liczba_filozofow];       // Tablica wątków dla filozofów, thread - klasa C++, reprezentuje wątek wykonywania (pojedynczy strumień) 

    for (int i = 0; i < liczba_filozofow; ++i)  // Pętla tworząca wątki dla każdego filozofa
    { 
        filozofowie[i] = thread(filozof, i);    // Stwórz wątek dla filozofa o numerze i
    }

    for (int i = 0; i < liczba_filozofow; ++i)  // Pętla czekająca na zakończenie wszystkich wątków
    {
            filozofowie[i].join();              // Połącz się z wątkiem filozofa i poczekaj na jego zakończenie
    }
    return 0; // Zakończ program
}
