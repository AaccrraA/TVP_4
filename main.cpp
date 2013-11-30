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

	vector<int> MPI; // ������ �������� �������� � ���������
	vector<int> A; // ������ ��������� � ������ ��������� �� ��������������� ��������, ������� �������� X
	vector<vector<int>> FSM; // Finite State Machine - ������ ���������(0, 1, ...)
    vector<int> FSS; // ������ �������� ���������

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
		������ �������:
		��������� ����� ���� ������ ���������� ���������� � ������� ���
		������������ ������ ��������� �����,
		�������������� ����� �� ����������� ������.
		*/

		int sLvl = 0;
		int iLvl = 0;
		for (int i = 0; i < R.size(); i++) {
			if (R[i].symbol == '(') {
				sLvl = 1;
				iLvl = 0;
				R[i + 1].addDependsOn(i - 1);
				for (int j = i + 1; (j < R.size() && sLvl != 0); j++) {
					if (R[j].symbol == '|' && sLvl == 1 && iLvl == 0) {
						R[j + 1].addDependsOn(i - 1);
					}
					else if (R[j].symbol == '(') {
						sLvl++;
					}
					else if (R[j].symbol == ')')
						sLvl--;
					else if (R[j].symbol == '<') {
						iLvl++;
					}
					else if (R[j].symbol == '>')
						iLvl--;
				}
				continue;
			}
			else if (R[i].symbol == '<') {
				iLvl = 1;
				sLvl = 0;
				R[i + 1].addDependsOn(i - 1);
				for (int j = i + 1; (j < R.size() && iLvl != 0); j++) {
					if (R[j].symbol == '|' && iLvl == 1 && sLvl == 0) {
						R[j + 1].addDependsOn(i - 1);
					}
					else if (R[j].symbol == '<') {
						iLvl++;
					}
					else if (R[j].symbol == '>')
						iLvl--;
					else if (R[j].symbol == '(') {
						sLvl++;
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
		������ �������:
		����� ������������� ������ �� ����������� ������
		��������� �������� ������ ������ ����������. � ��� ������������
		��� � ����� �������������� ���������������
		����� �� ����������� ������.
		*/
        int sLvl = 0;
        int iLvl = 0;
		for (int i = R.size() - 1; i >= 0; i--) {
			if (R[i].symbol == ')') {
				sLvl = 1;
				iLvl = 0;
				R[i + 1].addDependsOn(i - 1);
				for (int j = i - 1; (j >= 0 && sLvl != 0); j--) {
					if (R[j].symbol == '|' && sLvl == 1 && iLvl == 0) {
						R[i + 1].addDependsOn(j - 1);
					}
					else if (R[j].symbol == ')') {
						sLvl++;
                    }
					else if (R[j].symbol == '(')
						sLvl--;
					else if (R[j].symbol == '>') {
						iLvl++;
                    }
					else if (R[j].symbol == '<')
						iLvl--;
				}
				continue;
			}
			else if (R[i].symbol == '>') {
				iLvl = 1;
				sLvl = 0;
				R[i + 1].addDependsOn(i - 1);
				for (int j = i - 1; (j >= 0 && iLvl != 0); j--) {
					if (R[j].symbol == '|' && iLvl == 1 && sLvl == 0)
						R[i + 1].addDependsOn(j - 1);
					else if (R[j].symbol == '>') {
						iLvl++;
                    }
					else if (R[j].symbol == '<') {
                        iLvl--;
						if (iLvl == 0 && sLvl == 0) {
							R[i + 1].addDependsOn(j - 1);
						}
					}
					else if (R[j].symbol == ')') {
						sLvl++;
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
		������ �������:
		��������� ����� ���� ������ ����������
		������������ � ������������ ������
		��������� ����� �������������� ������ �� ����������� ������.
		*/
        int iLvl = 0;
        int sLvl = 0;
		for (int i = R.size() - 1; i >= 0; i--) {
			if (R[i].symbol == '>') {
				iLvl = 1;
                sLvl = 0;
				for (int j = i - 1; (j >= 0 && iLvl != 0); j--) {
					if (R[j].symbol == '|' && iLvl == 1 && sLvl == 0) {
						R[j + 1].addDependsOn(i + 1);
					}
                    else if (R[j].symbol == '>') {
                        iLvl++;
                    }
					else if (R[j].symbol == '<') {
						iLvl--;
						if (iLvl == 0 && sLvl == 0) {
							R[j + 1].addDependsOn(i + 1);
						}
					}
                    else if (R[j].symbol == ')') {
                        sLvl++;
                    }
                    else if (R[j].symbol == '(') {
                        sLvl--;
                    }
				}
			}
		}
    }
    
    void forthRule() {
		/*
		��������� �������:
		���� ����� a ��������� ����� b,
		� ����� b ��������� ����� c, ��
		����� a ��������� ����� c.
		*/
		bool isSmthChanged = false;
		do {
			isSmthChanged = false;
			// ��������� ��� ����� ���������
			for (int i = 0; i < R.size(); i ++)
				// ��������� j ����, ������� ��������� ����� i
                for (int j = 0; j < R[i].dependsOn.size(); j++)
					// ��������� ��������� � ������ ����� ������������ ����� i
                    for (int k = 0; k < R.size(); k ++)
                        if (find(R[k].dependsOn.begin(), R[k].dependsOn.end(), i) != R[k].dependsOn.end())
                            if (R[k].addDependsOn(R[i].dependsOn[j]))
                                isSmthChanged = true;
		} while (isSmthChanged);
    }
    
    void fifthRule() {
		/*
		����� �������:
		������ ����� ��������� ������ ����.
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
		// ��������� ��� ����� ���������
		for (int i = 2; i < R.size(); i++) {
			if (i % 2 == 0 && X.find(R[i - 1].symbol) != string::npos) {
				R[i].isMainPlace = true;
				R[i - 2].isPreMainPlace = true;
				MPI.push_back(i);
			}
		}
		// ��������� ��� ����������� ����� ������������� ����� i-2
		for (int i = 0; i < R.size(); i++) {
			if (R[i].isPreMainPlace) {
				for (int j = 0; j < R[i].dependsOn.size(); j++) {
					// ��������� ��� ����� ��������� ��� ���������� �������� ���� ����� �����������
					for (int k = 0; k < R.size(); k++) {
						if (R[i].dependsOn[j] == k && R[k].isMainPlace == true) {
							// ��������� ������� �������� �������� ����� ������� �� � ������������
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
		// �������������� ������ ��������� � ������ ���������
		for (int i = 0; i < X.length(); i++) {
			// -1 - отсутствует переход в другое состояние
			A.push_back(-1);
		}

		// �������������� �������� �������
		for (int i = 0; i < MPI.size(); i++) {
			FSM.push_back(A);
		}

		// � 0-�� ��������� ������� ����������� ������������ �������
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
                            if (FSM[k][z] == i2) {
								FSM[k][z] = i1;
                            }
							else if (FSM[k][z] > i2) {
								FSM[k][z]--;
							}
                        }
                    }
                    FSM.erase(FSM.begin()+i2);
                    i2--;
                }
            }
        }
        
        // �������������� ������ �������� ���������
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
    
    bool isExpressionCorrespond(string e) {
        bool isCorrespond = false;
        bool isEnd = true;
        
        int ei = 0;
        int q = 0; // ���������
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
	setlocale(LC_ALL, "rus");
	Cortege C;
	C.initDefault();
    
	//<x<e>f>abc(x|<l|m>)
    // nm(c|d)<k>n<n|m>
    string re = C.getRegularExpression();
    cout << "���������� ���������:\n" << re << endl;
    
	C.buildFSM();
    cout << "�������� �������:" << endl;
    C.printFSM();
    cout << "���������� �������� �������:" << endl;
    C.optimizeFSM();
    C.printFSM();
	
    string e;
    
    int cmd = -1;
    do {
        cmd = getCommand();
        switch (cmd) {
			case 1:
				cout << "�������: " << re << endl;
				cout << "����� ���������: ";
				cin >> e;
				cout << "��������� \"" << e;
				C.isExpressionCorrespond(e) ? cout << "\" " : cout << "\" �� ";
				cout << "��������." << endl;
				cout << endl;
				break;
			case 2:
				C.printFSM();
				break;
			case 0:
				cout << "����!" << endl;
				break;
			default:
				cerr << "����������� ������!" << endl;
				break;
		}
    } while (cmd);
    return 0;
}

int getCommand() {
	int cmd;
	cout << "������ ����� ����: " << endl;
	cout << "1. ��������� ���������." << endl;
	cout << "2. ������� �������� �������." << endl;
	cout << "0. �����." << endl;
	cin >> cmd;
	while (!cin.good() || (cmd < 0 || cmd > 2)) {
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cerr << "������ ������ ���� ���! ��������� ����: ";
		cin >> cmd;
	}
	return cmd;
}