#include <SFML/Network.hpp>
#include <iostream>
#include <bitset>
#include <random>
#include <chrono>

// ------------------------------------------------- Socket
sf::UdpSocket socket;
unsigned short localPort = 54000;
sf::IpAddress localIP = "127.0.0.2";

// ------------------------------------------------- K1:
sf::IpAddress K1_IP;
unsigned short K1_port;
uint8_t K1_ID = 0;
uint8_t K1_init;
uint8_t K1_proba = 1;
uint8_t K1_maks_liczba_prob = 0;
bool K1_stop = false;
bool K1_sukces = false;

// ------------------------------------------------- K2:
sf::IpAddress K2_IP;
unsigned short K2_port;
uint8_t K2_ID = 0;
uint8_t K2_init;
uint8_t K2_proba = 1;
uint8_t K2_maks_liczba_prob = 0;
bool K2_stop = false;
bool K2_sukces = false;

// ------------------------------------------------- Tajna liczba:
uint8_t tajnaLiczba = 0;

// ------------------------------------------------- Funkcja losujÂąca:
uint8_t losuj(const unsigned short &min, const unsigned short &max) {

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	static std::default_random_engine e{ seed1 };
	std::uniform_int_distribution <unsigned short> d(min, max);
	return static_cast<uint8_t>(d(e));
}

// ------------------------------------------------- Zmienne do odbioru i wysyÂłania danych:
std::bitset<24> in;
std::bitset<24> out;
size_t bytesReceived = 0;
unsigned short bytesToSendOrReceive = 3;
sf::IpAddress senderIP;
unsigned short senderPort = 0;

// ------------------------------------------------- Zmienne do odczytu danych:
uint8_t operacja;
uint8_t odpowiedz;
uint8_t sesja;
uint8_t liczba;
bool stop = false;

// ------------------------------------------------- Funkcja do odbioru pakietu:
void odbierz() {

	bool stop_01 = false;
	while (!stop_01) {

		if (socket.receive(&in, bytesToSendOrReceive, bytesReceived, senderIP, senderPort) != sf::Socket::Done) {

			std::cout << "\n\nBlad gniazda.";
			continue;
		}
		stop_01 = true;
	}
}

unsigned short wyslijCounter = 0;
unsigned short whileCounter = 0;

// ------------------------------------------------- Funkcja do wysÂłania pakietu:
void wyslij() {

	bool stop_02 = false;
	while (!stop_02) {

		if (socket.send(&out, bytesToSendOrReceive, senderIP, senderPort) != sf::Socket::Done) { std::cout << "\n\nSOCKET ERROR!\n\n"; exit(0); };
		++whileCounter;
		stop_02 = true;
	}
}

// ------------------------------------------------- Funkcja do odczytu pakietu:
void odczyt() {

	//std::cout << "\n\n(Odczyt) Bitset przed zlozeniem: " << in;

	std::string sub1 = in.to_string().substr(0, 8);
	std::string sub2 = in.to_string().substr(8, 8);
	std::string sub3 = in.to_string().substr(16, 8);

	/*std::cout << "\n\n(Odczyt) Substring 1: " << sub1
	<< "\n(Odczyt) Substring 2: " << sub2
	<< "\n(Odczyt) Substring 3: " << sub3;*/

	std::bitset<24> in_2(sub3 + sub2 + sub1);
	std::bitset<24> in_kopia(in_2);

	//std::cout << "\n\n(Odczyt) Bitset po zlozeniu: " << in_kopia;

	in_kopia &= 0b111100000000000000000000;
	operacja = static_cast<uint8_t>((in_kopia >> 20).to_ulong());

	in_kopia = in_2;
	in_kopia &= 0b000011100000000000000000;
	odpowiedz = static_cast<uint8_t>((in_kopia >> 17).to_ulong());

	in_kopia = in_2;
	in_kopia &= 0b000000011110000000000000;
	sesja = static_cast<uint8_t>((in_kopia >> 13).to_ulong());

	in_kopia = in_2;
	in_kopia &= 0b000000000001111111100000;
	liczba = static_cast<uint8_t>((in_kopia >> 5).to_ulong());

	in_kopia = in_2;
	stop = in_kopia.test(4);

	std::cout << "\n\n(Odczyt) Bitset po odczycie: " << in_kopia;

	std::cout << "\n\nOdczytano zawartosc komunikatu od klienta o ID = " << int(sesja) << ":"
		"\n\tOperacja: " << int(operacja) <<
		"\n\tOdpowiedz: " << int(odpowiedz) <<
		"\n\tLiczba: " << int(liczba) <<
		"\n\tFlaga stopu: " << stop;
}

// ------------------------------------------------- Funkcja do tworzenia pakietu:
void generuj() {

	//std::cout << "\n\nPakiet przez zapisem: " << out;

	std::bitset<4> operacja_bity(operacja);
	std::bitset<3> odpowiedz_bity(odpowiedz);
	std::bitset<4> sesja_bity(sesja);
	std::bitset<8> liczba_bity(liczba);

	std::string pakiet = operacja_bity.to_string() + odpowiedz_bity.to_string() + sesja_bity.to_string() + liczba_bity.to_string() + "00000";
	std::cout << "\n\nPakiet po 1. zapisie (string): " << pakiet;

	std::string sub1 = pakiet.substr(0, 8);
	std::string sub2 = pakiet.substr(8, 8);
	std::string sub3 = pakiet.substr(16, 8);

	/*std::cout << "\n\nSubstring 1: " << sub1
	<< "\nSubstring 2: " << sub2
	<< "\nSubstring 3: " << sub3;*/

	out = std::bitset<24>(sub3 + sub2 + sub1);
	//std::cout << "\n\nPakiet po 2. zapisie (bitset): " << out;

	if (stop) { out.set(20); }
	else { out.reset(20); }
}

// ------------------------------------------------- Funkcja do wysÂłania potwierdzenia:
void potwierdzOdebranie() {

	odpowiedz = 1;

	generuj();
	wyslij();
}

int main() {
	// ------------------------------------------------- Automatyczne ustawienie lokalnego adresu IP i rĂŞczne portu 54000;
	if (socket.bind(localPort, localIP) != sf::Socket::Done) {

		std::cout << "\n\nNie mozna powiazac podanego adresu i portu z gniazdem.\n\n";
	}

	// ------------------------------------------------- Odbior i obsÂługa 1. requesta o ID:
	bool stop_03 = false;
	while (!stop_03) {

		odbierz();
		odczyt();

		if (operacja == 1 && odpowiedz == 0 && sesja == 0 && liczba == 0 && !stop) {

			K1_IP = senderIP;
			K1_port = senderPort;

			potwierdzOdebranie();

			odpowiedz = 3;
			sesja = ++K1_ID;

			generuj();
			wyslij();

			++wyslijCounter;

			std::cout << "\n\nwhileCounter: " << whileCounter << "\nwyslijCounter: " << wyslijCounter;

			stop_03 = true;

		}
		else {

			potwierdzOdebranie();

			odpowiedz = 2;

			generuj();
			wyslij();
		}
	}

	// ------------------------------------------------- Odbior i obsÂługa 2. requesta o ID oraz liczb inicjalizujÂących:
	bool stop_04 = false;
	bool stop_05 = false;
	bool stop_06 = false;
	while (!stop_04 || !stop_05 || !stop_06) {

		odbierz();
		odczyt();

		if (operacja == 2 && odpowiedz == 0 && (sesja == K1_ID || (sesja == K2_ID && sesja != 0)) && !stop) {

			if (liczba % 2 == 1 || liczba < 1 || liczba > 254) {

				potwierdzOdebranie();

				odpowiedz = 5;

				generuj();
				wyslij();

			}
			else {

				potwierdzOdebranie();

				if (sesja == K1_ID) {

					K1_init = liczba;
					stop_04 = true;

				}
				else if (sesja == K2_ID) {

					K2_init = liczba;
					stop_05 = true;
				}
			}

		}
		else if (operacja == 1 && odpowiedz == 0 && sesja == 0 && !stop) {

			K2_IP = senderIP;
			K2_port = senderPort;

			potwierdzOdebranie();

			odpowiedz = 3;
			K2_ID = K1_ID + 1;
			sesja = K2_ID;

			generuj();
			wyslij();

			stop_06 = true;

		}
		else {

			potwierdzOdebranie();

			odpowiedz = 2;

			generuj();
			wyslij();
		}
	}

	// ------------------------------------------------- Wyznaczenie maksymalnej liczby prĂłb:
	unsigned short temp = static_cast<unsigned short>((static_cast<unsigned short>(K1_init) + static_cast<unsigned short>(K2_init)) / 2);
	K1_maks_liczba_prob = static_cast<uint8_t>(temp);
	K2_maks_liczba_prob = K1_maks_liczba_prob;

	odpowiedz = 4;
	liczba = K1_maks_liczba_prob;

	senderIP = K1_IP;
	senderPort = K1_port;
	sesja = K1_ID;

	generuj();
	wyslij();

	senderIP = K2_IP;
	senderPort = K2_port;
	sesja = K2_ID;

	generuj();
	wyslij();

	// ------------------------------------------------- Wylosowanie tajnej liczby:
	tajnaLiczba = losuj(1, 255);

	std::cout << "\n\nTajna liczba: " << (int)tajnaLiczba << std::endl;

	// ------------------------------------------------- PĂŞtla gry:
	while (!K1_stop || !K2_stop) {

		odbierz();
		odczyt();

		if (operacja == 3 && odpowiedz == 0 && (sesja == K1_ID || sesja == K2_ID) && !stop) {

			if (liczba < 1 || liczba > 255) {

				potwierdzOdebranie();

				odpowiedz = 5;

				generuj();
				wyslij();

			}
			else {

				potwierdzOdebranie();

				if (sesja == K1_ID) {

					if (liczba == tajnaLiczba) {

						K1_stop = true;
						K1_sukces = true;

						odpowiedz = 6;
						stop = 1;

						generuj();
						wyslij();

						if (K1_proba < K2_proba) { K2_maks_liczba_prob = K2_proba; }
						else { K2_maks_liczba_prob = K1_proba; }

					}
					else {

						odpowiedz = 7;
						if (K1_proba == K1_maks_liczba_prob) K1_stop = 1;
						if (K1_stop) { stop = 1; }
						else { ++K1_proba; }

						generuj();
						wyslij();
					}

				}
				else if (sesja == K2_ID) {

					if (liczba == tajnaLiczba) {

						K2_stop = true;
						K2_sukces = true;

						odpowiedz = 6;
						stop = 1;

						generuj();
						wyslij();

						if (K2_proba < K1_proba) { K1_maks_liczba_prob = K1_proba; }
						else { K1_maks_liczba_prob = K2_proba; }

					}
					else {

						odpowiedz = 7;
						if (K2_proba == K2_maks_liczba_prob) K2_stop = 1;
						if (K2_stop) { stop = 1; }
						else { ++K2_proba; }

						generuj();
						wyslij();
					}
				}
			}

		}
		else {

			potwierdzOdebranie();

			odpowiedz = 2;

			generuj();
			wyslij();
		}
	}

	// ------------------------------------------------- Wyniki:
	if (K1_sukces && !K2_sukces) {

		senderIP = K1_IP;
		senderPort = K1_port;
		operacja = 4;
		odpowiedz = 0;
		sesja = K1_ID;
		liczba = 0;
		stop = 1;

		generuj();
		wyslij();

		senderIP = K2_IP;
		senderPort = K2_port;
		operacja = 5;
		sesja = K2_ID;

		generuj();
		wyslij();

	}
	else if (!K1_sukces && K2_sukces) {

		senderIP = K1_IP;
		senderPort = K1_port;
		operacja = 5;
		odpowiedz = 0;
		sesja = K1_ID;
		liczba = 0;
		stop = 1;

		generuj();
		wyslij();

		senderIP = K2_IP;
		senderPort = K2_port;
		operacja = 4;
		sesja = K2_ID;

		generuj();
		wyslij();

	}
	else if (K1_sukces && K2_sukces) {

		if (K1_proba == K2_proba) {

			senderIP = K1_IP;
			senderPort = K1_port;
			operacja = 6;
			odpowiedz = 0;
			sesja = K1_ID;
			liczba = 0;
			stop = 1;

			generuj();
			wyslij();

			senderIP = K2_IP;
			senderPort = K2_port;
			operacja = 6;
			sesja = K2_ID;

			generuj();
			wyslij();

		}
		else if (K1_proba < K2_proba) {

			senderIP = K1_IP;
			senderPort = K1_port;
			operacja = 4;
			odpowiedz = 0;
			sesja = K1_ID;
			liczba = 0;
			stop = 1;

			generuj();
			wyslij();

			senderIP = K2_IP;
			senderPort = K2_port;
			operacja = 5;
			sesja = K2_ID;

			generuj();
			wyslij();

		}
		else if (K1_proba > K2_proba) {

			senderIP = K1_IP;
			senderPort = K1_port;
			operacja = 5;
			odpowiedz = 0;
			sesja = K1_ID;
			liczba = 0;
			stop = 1;

			generuj();
			wyslij();

			senderIP = K2_IP;
			senderPort = K2_port;
			operacja = 4;
			sesja = K2_ID;

			generuj();
			wyslij();
		}

	}
	else if (!K1_sukces && !K2_sukces) {

		senderIP = K1_IP;
		senderPort = K1_port;
		operacja = 6;
		odpowiedz = 0;
		sesja = K1_ID;
		liczba = 0;
		stop = 1;

		generuj();
		wyslij();

		senderIP = K2_IP;
		senderPort = K2_port;
		operacja = 6;
		sesja = K2_ID;

		generuj();
		wyslij();
	}

	for (unsigned short x = 1; x < 3; ++x) {

		odbierz();
		odczyt();

		if ((operacja == 4 || operacja == 5 || operacja == 6) && odpowiedz == 1 && (sesja == K1_ID || sesja == K2_ID) && liczba == 0 && stop) {

			std::cout << "\n\nOtrzymano potwierdzenie dostarczenia wyniku gry od klienta o ID: " << int(sesja);
		}
	}

	std::cout << "\n\n";

	return 0;
}