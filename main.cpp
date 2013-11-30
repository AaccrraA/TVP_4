#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <vector>

using namespace std;

struct Element {
	char symbol;
	vector<int> dependsOn;
	
	bool isMainPlace = false;
	bool isPreMainPlace = false;

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
			dependsOn.push_back(d);
		}
        return isNew;
	}
};

struct Cortege {
private:
	string X;
	vector <Element> R;

	vector<int> MPI; // вектор основных индексов в выражении
	vector<int> A; // вектор переходов в другие состояния по соответствующим символам, размера алфавита X
    vector<int> FSS; // массив конечный состояний
	vector<vector<int>> FSM; // Finite State Machine - вектор состояний(0, 1, ...)

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

		R.resize(rStr.length() * 2 + 1);

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
    
    void firstRule() {
        /*
         Первое правило:
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
					else if (R[j].symbol == '(') {
						sLvl++;
                        if (sLvl == 2 && iLvl == 0)
                            R[j - 1].addDependsOn(i - 1);
                    }
					else if (R[j].symbol == ')')
						sLvl--;
					else if (R[j].symbol == '<') {
						iLvl++;
                        if (sLvl == 1 && iLvl == 1)
                            R[j - 1].addDependsOn(i - 1);
                    }
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
					else if (R[j].symbol == '<') {
						iLvl++;
                        if (iLvl == 2 && sLvl == 0)
                            R[j - 1].addDependsOn(i - 1);
                    }
					else if (R[j].symbol == '>')
						iLvl--;
					else if (R[j].symbol == '(') {
						sLvl++;
                        if (iLvl == 1 && sLvl == 1)
                            R[j - 1].addDependsOn(i - 1);
                    }
					else if (R[j].symbol == ')')
						sLvl--;
				}
				continue;
			}
		}
    }
    
    void secondRule() {
        /*
         Второе правило:
         Место расположенное справа от закрывающей скобки
         подчинено конечным местам термов многочлена. А в итерационных
         скобках еще и месту находящемуся слева от открывающей
         */
        int sLvl = 0;
        int iLvl = 0;
		for (int i = R.size() - 1; i >= 0; i--) {
			if (R[i].symbol == ')') {
				sLvl = 1;
				iLvl = 0;
				for (int j = i - 1; (j >= 0 && sLvl != 0); j--) {
					if (X.find(R[j].symbol) != string::npos && sLvl == 1 && iLvl == 0)
						R[i + 1].addDependsOn(j + 1);
					else if (R[j].symbol == ')') {
						sLvl++;
                        if (sLvl == 2 && iLvl == 0)
                            R[i + 1].addDependsOn(j - 1);
                    }
					else if (R[j].symbol == '(')
						sLvl--;
					else if (R[j].symbol == '>') {
						iLvl++;
                        if (sLvl == 1 && iLvl == 1)
                            R[i + 1].addDependsOn(j - 1);
                    }
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
					else if (R[j].symbol == '>') {
						iLvl++;
                        if (iLvl == 2 && sLvl == 0)
                            R[i + 1].addDependsOn(j + 1);
                    }
					else if (R[j].symbol == '<') {
                        iLvl--;
						if (iLvl == 0) {
							R[i + 1].addDependsOn(j - 1);
                        }
					}
					else if (R[j].symbol == ')') {
						sLvl++;
                        if (iLvl == 1 && sLvl == 1)
                            R[i + 1].addDependsOn(j + 1);
                    }
					else if (R[j].symbol == '(')
						sLvl--;
				}
				continue;
			}
		}
    }
    
    void thirdRule() {
        /*
         Третье правило:
         Начальные места всех термов многочлена
         заключенного в итерационные скобки
         подчинены месту расположенному справа от закрывающей скобки.
         */
        int iLvl = 0;
        int sLvl = 0;
		for (int i = R.size() - 1; i >= 0; i--) {
			if (R[i].symbol == '>') {
				iLvl = 1;
                sLvl = 0;
				for (int j = i - 1; (j >= 0 && iLvl != 0); j--) {
					if (X.find(R[j].symbol) != string::npos && iLvl == 1 && sLvl == 0) {
						R[j - 1].addDependsOn(i + 1);
					}
                    else if (R[j].symbol == '>') {
                        iLvl++;
                    }
					else if (R[j].symbol == '<') {
						iLvl--;
                        if (iLvl == 1 && sLvl == 0) {
                            R[j - 1].addDependsOn(i + 1);
                        }
					}
                    else if (R[j].symbol == ')') {
                        sLvl++;
                    }
                    else if (R[j].symbol == '(') {
                        sLvl--;
                        if (iLvl == 1 && sLvl == 0) {
                            R[j - 1].addDependsOn(i + 1);
                        }
                    }
				}
			}
		}
    }
    
    void forthRule() {
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
			for (int i = 0; i < R.size(); i ++)
				// Пробегаем j мест, которым подчинено место i
                for (int j = 0; j < R[i].dependsOn.size(); j++)
                    // Пробегаем выражение в поиске места подчиненного месту i
                    for (int k = 0; k < R.size(); k ++)
                        if (find(R[k].dependsOn.begin(), R[k].dependsOn.end(), i) != R[k].dependsOn.end())
                            if (R[k].addDependsOn(R[i].dependsOn[j]))
                                isSmthChanged = true;
		} while (isSmthChanged);
    }
    
    void fifthRule() {
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
    
	void defineDependings() {
        firstRule();
        secondRule();
        thirdRule();
        forthRule();
        fifthRule();
	}

	void definePLaces() {
		R[0].isMainPlace = true;
		MPI.push_back(0);
		// Пробегаем все места выражения
		for (int i = 0; i < R.size(); i++) {
			if (i % 2 == 0 && X.find(R[i - 1].symbol) != string::npos) {
				R[i].isMainPlace = true;
				R[i - 2].isPreMainPlace = true;
				MPI.push_back(i);
			}
		}
		// Пробегаем все подчиняемые места предосновного места i-2
		for (int i = 0; i < R.size(); i++) {
			if (R[i].isPreMainPlace) {
				for (int j = 0; j < R[i].dependsOn.size(); j++) {
					// Пробегаем все места выражения для нахождения основных мест среди подчиняемых
					for (int k = 0; k < R.size(); k++) {
						if (R[i].dependsOn[j] == k && R[k].isMainPlace == true) {
							// Пробегаем индексы основных индексов чтобы занести их в предосновные
							for (int z = 0; z < MPI.size(); z++) {
								if (k == MPI[z]) {
									R[i].preMainIndexes.push_back(z);
								}
							}
						}
					}
				}
			}
		}
	}
    
    bool isSameColumns(int i1, int i2) {
        bool isTheSame = true;
        for (int i = 0; i < FSM[0].size(); i++) {
            if (FSM[i1][i] != FSM[i2][i]) {
                isTheSame = false;
                break;
            }
        }
        return isTheSame;
    }

public:
	void initDefault() {
		loadFromFile();
		defineDependings();
		definePLaces();
	}

	void buildFSM() {
		// Инициализируем вектор переходов в другие состояния
		for (int i = 0; i < X.length(); i++) {
			// -1 - отсутствует переход в другое состояние
			A.push_back(-1);
		}

		// Инициализируем конечный автомат
		for (int i = 0; i < MPI.size(); i++) {
			FSM.push_back(A);
		}

		// У 0-го основного индекса отсутствует предосновные индексы
		for (int i = 1; i < MPI.size(); i++) {
			for (int j = 0; j < R[MPI[i] - 2].preMainIndexes.size(); j++) {
				FSM[R[MPI[i] - 2].preMainIndexes[j]][X.find(R[MPI[i] - 1].symbol)] = i;
			}
		}
	}

	void optimizeFSM() {
        for (int i1 = 0; i1 < FSM.size(); i1++) {
            for (int i2 = i1+1; i2 < FSM.size(); i2++) {
                if (i1 != i2 && isSameColumns(i1, i2)) {
                    for (int k = 0; k < FSM.size(); k++) {
                        for (int z = 0; z < FSM[0].size(); z++) {
                            if (FSM[k][z] >= i2) {
                                if (FSM[k][z] > 0) {
                                    FSM[k][z]--;
                                }
                            }
                        }
                    }
                    FSM.erase(FSM.begin()+i2);
                    i2--;
                }
            }
        }
        
        // Инициализируем массив конечных состояний
        bool isFinitState;
        for (int i = 0; i < FSM.size(); i++) {
            isFinitState = true;
            for (int j = 0; j < FSM[i].size(); j++) {
                if (FSM[i][j] == i) {
                    isFinitState = true;
                }
                else if (FSM[i][j] != -1) {
                    isFinitState = false;
                    break;
                }
            }
            if (isFinitState) {
                FSS.push_back(i);
            }
        }
	}
    
    void printFSM() {
        for (int j = 0; j < FSM[0].size()+1; j++) {
            if (j == 0) {
                cout << "* ";
            }
            else {
                cout << X[j-1] << " ";
            }
            for (int i = 0; i < FSM.size(); i++) {
                if (j == 0) {
                    cout << i;
                    i > 9 ? cout << " " : cout << "  ";
                }
                else if (FSM[i][j-1] == -1) {
                    cout << ".  ";
                }
                else {
                    cout << FSM[i][j-1];
                    FSM[i][j-1] > 9 ? cout << " " : cout << "  ";
                }
            }
            cout << endl;
        }
        cout << endl;
    }
    
    bool isExpressionCorrexspond(string e) {
        bool isCorrespond = false;
        bool isEnd = true;
        
        int ei = 0;
        int q = 0; // состояние
        do {
            isEnd = true;
            for (int j = 0; j < FSM[q].size(); j++) {
                if (e[ei] == X[j]) {
                    if (FSM[q][j] != -1) {
                        q = FSM[q][j];
                        ei++;
                        isEnd = false;
                        break;
                    }
                    else {
                        isEnd = true;
                        isCorrespond = false;
                    }
                }
            }
            if (ei == e.length() && find(FSS.begin(), FSS.end(), q) != FSS.end()) {
                isEnd = true;
                isCorrespond = true;
            }
        }
        while (!isEnd);
        
        return isCorrespond;
    }
    
    string getRegularExpression() {
        string r;
        for (int i = 0; i < R.size(); i++) {
            if (i%2 == 1) {
                r = r+R[i].symbol;
            }
        }
        return r;
    }
};

int getCommand();

int main() {
//	setlocale(LC_ALL, "rus");
	Cortege C;
	C.initDefault();
    
    //nm(c|d)<k>n<n|m>
    string re = C.getRegularExpression();
    cout << "Регулярное выражение:\n" << re << endl;
    
	C.buildFSM();
    cout << "Конечный автомат:" << endl;
    C.printFSM();
    cout << "Конечный автомат после упрощения:" << endl;
    C.optimizeFSM();
    C.printFSM();
	
    string e;
    
    int cmd = -1;
    do {
        cmd = getCommand();
        switch (cmd) {
            case 1:
                cout << "Правило: " << re << endl;
                cout << "Введи выражение: ";
                cin >> e;
                cout << "Выражение \"" << e;
                C.isExpressionCorrexspond(e) ? cout << "\" " : cout << "\" не ";
                cout << "подходит." << endl;
                cout << endl;
                break;
            case 2:
                C.printFSM();
                break;
            case 0:
                cout << "Пока!" << endl;
                break;
            default:
                cerr << "Неизвестная ошибка!" << endl;
                break;
        }
    } while (cmd);
    return 0;
}

int getCommand() {
    int cmd;
    cout << "Выбери пункт меню: " << endl;
    cout << "1. Проверить выражение." << endl;
    cout << "2. Вывести конечный автомат." << endl;
    cout << "0. Выход." << endl;
    cin >> cmd;
    while (!cin.good() || (cmd < 0 || cmd > 2)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cerr << "Такого пункта меню нет! Повторите ввод: ";
        cin >> cmd;
    }
    return cmd;
}