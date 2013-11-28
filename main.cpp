#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <vector>
using namespace std;

struct Element {
	char symbol;
	vector<int> dependsOn;
	
	bool isMainPlace;
	bool isPreMainPlace;

	vector<int> preMainIndexes;

	bool addDependsOn(int d) {
        bool isNew = true;
		for (int i = 0; i < dependsOn.size(); i++) {
			if (dependsOn[i] == d) {
                isNew = false;
				break;
			}
		}
		if (isNew) {
			dependsOn.resize(dependsOn.size() + 1);
			dependsOn[dependsOn.size() - 1] = d;
		}
        return isNew;
	}

};

struct Cortege {
private:
	string X;
	vector <Element> R;

	void loadFromFile() {
		ifstream ifs("RE.txt");
		string field;
		string rStr;
		if (ifs.fail()) {
			cerr << "Ошибка открытия файла.";
			exit(-1);
		}
		do {
			getline(ifs, field, '\n');
			if (field == "X:") {
				getline(ifs, field);
				for (int i = 0; i < field.length(); i++)
				if (field[i] == ' ' || field[i] == '\n')
					field.erase(i, 1);
				X = field;
			}
			else if (field == "R:") {
				getline(ifs, field);
				for (int i = 0; i < field.length(); i++)
				if (field[i] == ' ' || field[i] == '\n')
					field.erase(i, 1);
				rStr = field;
			}
		} while (!ifs.eof());

		R.resize(rStr.length()*2 + 1);

		int k = 0;
		for (int i = 0; i < (rStr.length() * 2 + 1); i++) {
			if (i % 2 == 0) {
				R[i].symbol = '_';
			}
			else {
				R[i].symbol = rStr[k];
				k++;
			}
		}
	}

	void defineDependings() {
		/* Первое правило:
		Начальные места всех термов многочлена помещенных в обычные или
		итерационные скобки подчинены месту,
		расположенному слева от открывающей скобки.
		*/
		int sLvl = 0;
		int iLvl = 0;
		for (int i = 0; i < R.size(); i++) {
			if (R[i].symbol == '(') {
				sLvl = 1;
				iLvl = 0;
				for (int j = i + 1; (j < R.size() && sLvl != 0); j++) {
					if (X.find(R[j].symbol) != string::npos && sLvl == 1 && iLvl == 0)
						R[j - 1].addDependsOn(i - 1);
					else if (R[j].symbol == '(')
						sLvl++;
					else if (R[j].symbol == ')')
						sLvl--;
					else if (R[j].symbol == '<')
						iLvl++;
					else if (R[j].symbol == '>')
						iLvl--;
				}
				continue;
			}
			else if (R[i].symbol == '<') {
				iLvl = 1;
				sLvl = 0;
				for (int j = i + 1; (j < R.size() && iLvl != 0); j++) {
					if (X.find(R[j].symbol) != string::npos && iLvl == 1 && sLvl == 0)
						R[j - 1].addDependsOn(i - 1);
					else if (R[j].symbol == '<')
						iLvl++;
					else if (R[j].symbol == '>')
						iLvl--;
					else if (R[j].symbol == '(')
						sLvl++;
					else if (R[j].symbol == ')')
						sLvl--;
				}
				continue;
			}
		}

		/*
		Второе правило:
		Место расположенное справа от закрывающей скобки
		подчинено конечным местам термов многочлена.
		*/
		for (int i = R.size() - 1; i >= 0; i--) {
			if (R[i].symbol == ')') {
				sLvl = 1;
				iLvl = 0;
				for (int j = i - 1; (j >= 0 && sLvl != 0); j--) {
					if (X.find(R[j].symbol) != string::npos && sLvl == 1 && iLvl == 0)
						R[i + 1].addDependsOn(j + 1);
					else if (R[j].symbol == ')')
						sLvl++;
					else if (R[j].symbol == '(')
						sLvl--;
					else if (R[j].symbol == '>')
						iLvl++;
					else if (R[j].symbol == '<')
						iLvl--;
				}
				continue;
			}
			else if (R[i].symbol == '>') {
				iLvl = 1;
				sLvl = 0;
				for (int j = i - 1; (j >= 0 && iLvl != 0); j--) {
					if (X.find(R[j].symbol) != string::npos && iLvl == 1 && sLvl == 0)
						R[i + 1].addDependsOn(j + 1);
					else if (R[j].symbol == '>')
						iLvl++;
					else if (R[j].symbol == '<') {
						if (iLvl == 1) {
							R[i + 1].addDependsOn(j - 1);
							iLvl--;
						}
					}
					else if (R[j].symbol == ')')
						sLvl++;
					else if (R[j].symbol == '(')
						sLvl--;
				}
				continue;
			}
		}
		
		/*
		Третье правило:
		Начальные места всех термов многочлена
		заключенного в итерационные скобки
		подчинены месту расположенному справа от закрывающей скобки.
		*/
		for (int i = R.size() - 1; i >= 0; i--) {
			if (R[i].symbol == '>') {
				iLvl = 1;
				for (int j = i - 1; (j >= 0 && iLvl != 0); j--) {
					if (X.find(R[j].symbol) != string::npos && iLvl == 1) {
						R[j - 1].addDependsOn(i + 1);
					}
					else if (R[j].symbol == '<') {
						iLvl--;
					}
				}
			}
		}
		
		/*
		Четвертое правило:
		Если месту a подчинено место b,
		и месту b подчинено место c, то
		месту a подчинено место c.
		*/
		bool isSmthChanged = false;
		do {
			isSmthChanged = false;
			// Пробегаем все места выражения
			for (int i = 0; i < R.size(); i += 2)
				// Пробегаем j мест, которым подчинено место i
			for (int j = 0; j < R[i].dependsOn.size(); j++)
				// Пробегаем выражение в поиске места подчиненного месту i
			for (int k = 0; k < R.size(); k += 2)
			if (find(R[k].dependsOn.begin(), R[k].dependsOn.end(), i) != R[k].dependsOn.end())
			if (R[k].addDependsOn(R[i].dependsOn[j]))
				isSmthChanged = true;
		} while (isSmthChanged);

		/*
		Пятое правило:
		Каждое место подчинено самому себе.
		*/
		for (int i = 0; i < R.size(); i++) {
			if (i % 2 == 0) {
				R[i].addDependsOn(i);
			}
		}
	}

	void definePLaces() {
		for (int i = 0; i < R.size(); i++) {
			if (i - 2 >= 0 && (i == 0 || X.find(R[i - 1].symbol) != string::npos)) {
				R[i].isMainPlace = true;
				R[i - 2].isPreMainPlace = true;
			}
		}
	}
 
public:
	void initDefault() {
		loadFromFile();
		defineDependings();
		definePLaces();
	}
};

int main() {
	setlocale(LC_ALL, "rus");

	Cortege C;
	C.initDefault();

	return 0;
}