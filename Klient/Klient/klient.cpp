#include <SFML\Network.hpp>
#include <iostream>
#include <bitset>
#include <string>
#include <limits>

uint8_t operacja = 1;
uint8_t odpowiedz = 0;
uint8_t sesja = 0;
uint8_t liczba = 0;
bool stop = false;

void AbyKontynuowac()
{

	std::cout << "\n\nAby kontynuowac, wprowadz \" \' \" (apostrof).\n\n";

	char c;

	do {
		std::cout << "> ";
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		c = char(std::cin.get());
	} while (c != '\'');
}
void odczyt(const std::bitset<24> &a)
{
	//std::cout << "\n\n(Odczyt) Bitset przed zlozeniem: " << a;

	std::string sub1 = a.to_string().substr(0, 8);
	std::string sub2 = a.to_string().substr(8, 8);
	std::string sub3 = a.to_string().substr(16, 8);

	/*std::cout << "\n\n(Odczyt) Substring 1: " << sub1
	<< "\n(Odczyt) Substring 2: " << sub2
	<< "\n(Odczyt) Substring 3: " << sub3;*/

	std::bitset<24> a_2(sub3 + sub2 + sub1);
	std::bitset<24> a_kopia(a_2);

	//std::cout << "\n\n(Odczyt) Bitset po zlozeniu: " << a_kopia;


	a_kopia &= 0b111100000000000000000000;
	operacja = static_cast<uint8_t>((a_kopia >> 20).to_ulong());
	a_kopia = a_2;
	a_kopia &= 0b000011100000000000000000;
	odpowiedz = static_cast<uint8_t>((a_kopia >> 17).to_ulong());
	a_kopia = a_2;
	a_kopia &= 0b000000011110000000000000;
	sesja = static_cast<uint8_t>((a_kopia >> 13).to_ulong());
	a_kopia = a_2;
	a_kopia &= 0b000000000001111111100000;
	liczba = static_cast<uint8_t>((a_kopia >> 5).to_ulong());
	a_kopia = a_2;
	stop = a_kopia.test(4);

	std::cout << "\n\n(Odczyt) Bitset po odczycie: " << a_kopia;

	std::cout << "\n\nKod operacji: " << (int)operacja << "\nKod odpowiedzi: " << (int)odpowiedz << "\nID sesji: " << (int)sesja << "\nLiczba: " << (int)liczba << "\nFlaga stopu: " << stop;

}
void zapis(std::bitset<24> &a)
{
	//std::cout << "\n\nPakiet przez zapisem: " << a;

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

	a = std::bitset<24>(sub3 + sub2 + sub1);

	if (stop) { a.set(20); }
	else { a.reset(20); }

	//std::cout << "\n\nPakiet po 2. zapisie (bitset): " << a;
}

int main()
{
	std::bitset<24> komunikat;
	sf::UdpSocket socket;
	sf::IpAddress receiver = "127.0.0.2";
	sf::IpAddress sender;
	std::size_t received;
	std::string id;
	unsigned short port2;
	unsigned int initialize;
	long long licz = 0;
	unsigned short port = 54000;
	std::bitset<15> send_flag("111111111111111");
	zapis(komunikat);

	if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Done)
	{
		std::cout << "\n\nBlad gniazda (bindowanie).";
	}

	if (socket.send(&komunikat, 3, receiver, port) != sf::Socket::Done)
	{
		std::cout << "\n\nBlad gniazda (wysylanie).";

	}
	else { std::cout << "\n\nWyslano pakiet."; }



	while (true)
	{
		std::cout << "\n\nWaiting...";
		if (socket.receive(&komunikat, 3, received, sender, port2) != sf::Socket::Done)
		{
			std::cout << "\n\nBlad gniazda (odbieranie).";
		}
		odczyt(komunikat);

		if (odpowiedz == 1)
		{
			std::cout << "\n\nOtrzymano potwierdzenie.";
		}
		else if (odpowiedz == 2)
		{
			std::cout << "\n\nNiepoprawna forma lub kolejnosc komunikatu. Przerywam program.";
			return 0;
		}
		else if (odpowiedz == 3)
		{
			std::cout << "\n\nOtrzymano ID sesji.";

			bool stop11 = false;
			while (!stop11) {
				std::cout << "\nPodaj parzysta liczbe z zakresu <2 ; 254>" <<
					"\n> ";
				std::cin >> licz;

				if (licz < 0 || licz > 255) {

					std::cout << "\n\nPodano liczbe spoza akceptowalnego zakresu. Sprobuj ponownie:";

				}
				else {

					stop11 = true;
				}
			}

			initialize = static_cast<unsigned int>(licz);
			operacja = 2;
			odpowiedz = 0;
			liczba = initialize;
			zapis(komunikat);
			if (socket.send(&komunikat, 3, receiver, port) != sf::Socket::Done)
			{
				std::cout << "\n\nBlad gniazda (wysylanie).";
			}
		}
		else if (odpowiedz == 4)
		{
			bool stop_09 = false;
			std::cout << "\n\nMaksymalna liczba prob: " << (int)liczba;

			while (!stop_09) {
				std::cout << "\n\nPodaj liczbe z zakresu <1 ; 255>:" <<
					"\n> ";
				std::cin >> licz;
				if (licz < 0 || licz > 255) {

					std::cout << "\n\nPodano liczbe spoza akceptowalnego zakresu. Sprobuj ponownie:";

				}
				else {

					liczba = static_cast<unsigned int>(licz);
					stop_09 = true;
				}
			}
			operacja = 3;
			odpowiedz = 0;
			zapis(komunikat);
			if (socket.send(&komunikat, 3, receiver, port) != sf::Socket::Done)
			{
				std::cout << "\n\nBlad gniazda (wysylanie).";
			}

		}
		else if (odpowiedz == 5)
		{
			std::cout << "\n\nPodano niedozwolona liczbe. Sprobuj ponownie.";
			if (operacja == 2)
			{
				bool stop12 = false;
				while (!stop12) {
					std::cout << "\nPodaj parzysta liczbe z zakresu <2 ; 254>" <<
						"\n> ";
					std::cin >> licz;

					if (licz < 0 || licz > 255) {

						std::cout << "\n\nPodano liczbe spoza akceptowalnego zakresu. Sprobuj ponownie:";

					}
					else {

						liczba = static_cast<unsigned int>(licz);
						stop12 = true;
					}
				}

				odpowiedz = 0;
				zapis(komunikat);
				if (socket.send(&komunikat, 3, receiver, port) != sf::Socket::Done)
				{
					std::cout << "\n\nBlad gniazda (wysylanie).";
				}
			}
			else if (operacja == 3)
			{
				bool stop14 = false;
				while (!stop14) {
					std::cout << "\nPodaj liczbe z zakresu <1 ; 255>" <<
						"\n> ";
					std::cin >> licz;

					if (licz < 0 || licz > 255) {

						std::cout << "\n\nPodano liczbe spoza akceptowalnego zakresu. Sprobuj ponownie:";

					}
					else {

						liczba = static_cast<unsigned int>(licz);
						stop14 = true;
					}
				}
				odpowiedz = 0;
				zapis(komunikat);
				if (socket.send(&komunikat, 3, receiver, port) != sf::Socket::Done)
				{
					std::cout << "\n\nBlad gniazda (wysylanie).";
				}
			}

		}
		else if (odpowiedz == 6)
		{
			std::cout << "\n\nOdgadles liczbe! Prosze czekac na wyniki.";

		}
		else if (odpowiedz == 7)
		{
			if (!stop)
			{
				std::cout << "\n\nNie odgadles liczby. Sprobuj ponownie.";
				bool stop15 = false;
				while (!stop15) {
					std::cout << "\nPodaj liczbe z zakresu <1 ; 255>" <<
						"\n> ";
					std::cin >> licz;

					if (licz < 0 || licz > 255) {

						std::cout << "\n\nPodano liczbe spoza akceptowalnego zakresu. Sprobuj ponownie:";

					}
					else {

						liczba = static_cast<unsigned int>(licz);
						stop15 = true;
					}
				}
				odpowiedz = 0;
				zapis(komunikat);
				if (socket.send(&komunikat, 3, receiver, port) != sf::Socket::Done)
				{
					std::cout << "\n\nBlad gniazda (wysylanie).";
				}
			}
			else
			{
				std::cout << "\n\nNie odgadles liczby. Prosze czekac na wyniki.";
			}
		}

		if (stop) break;

	}

	if (socket.receive(&komunikat, 3, received, sender, port2) != sf::Socket::Done)
	{
		std::cout << "\n\nBlad gniazda (odbieranie).";
	}
	odczyt(komunikat);

	if (operacja == 4) std::cout << "\n\nGRATULACJE, WYGRALES!";
	else if (operacja == 5) std::cout << "\n\nNIESTETY, PRZEGRALES.";
	else if (operacja == 6) std::cout << "\n\nMAMY REMIS! ";
	odpowiedz = 1;
	zapis(komunikat);

	if (socket.send(&komunikat, 3, receiver, port) != sf::Socket::Done)
	{
		std::cout << "\n\nBlad gniazda (wysylanie).";
	}

	AbyKontynuowac();

	return 0;
}