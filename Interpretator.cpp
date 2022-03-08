
#include <iostream>
#include <string>
#include <cstdio>
#include <ctype.h>
#include <cstdlib>
#include <vector>
#include <stack>
#include <algorithm>
#include <string.h>

using namespace std;
//выделим следующие типы лексем, введя следующий перечислимый тип данных:
//внутреннее представление лексем — это пара (тип_лексемы, значение_лексемы). Значение лексемы — это номер строки в таблице лексем соответствующего класса, 
//содержащей информацию о лексеме.
enum type_of_lex {
	LEX_NULL,                                                                                   /* 0*/
	LEX_AND, LEX_BEGIN, LEX_BOOL, LEX_DO, LEX_ELSE, LEX_END, LEX_IF, LEX_FALSE, LEX_INT,        /* 9*/
	LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_THEN, LEX_TRUE, LEX_VAR, LEX_WHILE, LEX_WRITE,  /*18*/
	LEX_FOR, LEX_TO,                                                                            /*20*/
	LEX_FIN,                                                                                    /*21*/
	LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS,   /*29*/
	LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH, LEX_LEQ, LEX_NEQ, LEX_GEQ,              /*37*/
	LEX_NUM,                                                                                    /*38*/
	LEX_ID,                                                                                     /*39*/
	POLIZ_LABEL,                                                                                /*40*для ссылок на номера элементов полиз*/
	POLIZ_ADDRESS,                                                                              /*41 для обозначения операндов-адресов, например, для оператора присваивания */
	POLIZ_GO,                                                                                   /*42*/
	POLIZ_FGO                                                                                   /*43*/
};

string name_of_lex[] =
{
"LEX_NULL",
"LEX_AND",
"LEX_BEGIN",
"LEX_BOOL",
"LEX_DO",
"LEX_ELSE",
"LEX_END",
"LEX_IF",
"LEX_FALSE",
"LEX_INT",
"LEX_NOT",
"LEX_OR",
"LEX_PROGRAM",
"LEX_READ",
"LEX_THEN",
"LEX_TRUE",
"LEX_VAR",
"LEX_WHILE",
"LEX_WRITE",
"LEX_FOR",
"LEX_TO",
"LEX_FIN",
"LEX_SEMICOLON",
"LEX_COMMA",
"LEX_COLON",
"LEX_ASSIGN",
"LEX_LPAREN",
"LEX_RPAREN",
"LEX_EQ",
"LEX_LSS",
"LEX_GTR",
"LEX_PLUS",
"LEX_MINUS",
"LEX_TIMES",
"LEX_SLASH",
"LEX_LEQ",
"LEX_NEQ",
"LEX_GEQ",
"LEX_NUM",
"LEX_ID",
"POLIZ_LABEL",
"POLIZ_ADDRESS",
"POLIZ_GO",
"POLIZ_FGO"
};

/////////////////////////  Класс Lex  //////////////////////////
 //возвращает лексемму и номер лексеммы
class Lex {
	type_of_lex   t_lex;//тип лексемы
	int           v_lex;//номер лексемы

public:
	Lex(type_of_lex t = LEX_NULL, int v = 0) : t_lex(t), v_lex(v) { }//если значения не переданы, то вернет LEX_NULL, 0

	type_of_lex  get_type() const { return t_lex; }//геттер возвращает лексемму 

	int get_value() const { return v_lex; }//геттер возвращает номер лексеммы

	friend ostream& operator<< (ostream& s, Lex l)//перегружена как функция-друг операция вывода лексемы << (для отладки)
	{
		s << name_of_lex[l.t_lex] << '(' << l.t_lex << ',' << l.v_lex << ");\n";
		return s;
	}
};

/////////////////////  Класс Ident  ////////////////////////////
//каждый идентификатор представлен в классе Ident
class Ident {
	char* name;          //указатель на строчку в том виде, в каком идент-р встретился в программе
	bool        declare;//описана или не описана переменная name (сем.этап), по умолчанию false
	type_of_lex type;   //какого типа переменная name: int, bool - сем.этап
	bool        assign; //отслеживать на этапе выполнения, переменной было присваивание или не было присваивания (сем.этап), по умолчанию false
	int         value;  //значение переменной name

public:
	Ident() { declare = false; assign = false; }//конструктор устанавливает по умолчанию false

	char* get_name() { return name; }//узнаем имя переменной в том виде, в котором оно встретилось в программе

	void put_name(const char* n) { name = new char[strlen(n) + 1]; strcpy(name, n); }//метод создает строчку и указатель записывает в name

	bool get_declare() { return declare; }

	void put_declare() { declare = true; }

	type_of_lex get_type() { return type; }

	void put_type(type_of_lex t) { type = t; }

	bool get_assign() { return assign; }

	void put_assign() { assign = true; }

	int get_value() { return value; }

	void put_value(int v) { value = v; }
};

//////////////////////  Table_Ident  ///////////////////////
//таблица идентификаторов анализируемой программы, строится в процессе ЛА 
class tabl_ident {
	Ident* p;    //указатель p на массив идентификаторов Ident, указатель на строчку в том виде, в котором идентификатор встретился в программе
	int        size; //размер таблицы
	int        top;  //при формировании таблицы top указывает на очередное свободное место, куда помещать следующий элемент таблицы

public:
	tabl_ident(int max_size) //конструктор имеет параметр максимального размера таблицы имен
	{
		p = new Ident[size = max_size];
		top = 1;
	}

	~tabl_ident() { delete[]p; }//очищаем массив, созданный конструктором

	Ident& operator[] (int k) { return p[k]; } //операция перегрузка [] возвращает к-ый элемент массива Ident

	int put(const char* buf); //метод put добавляет в таблицу имен имя, записанное в буфере buf (сначала проверит, нет ли такого имени в таблице, потом добавляет)
};                            //реализован ниже

int tabl_ident::put(const char* buf) //пробегаем по строчкам таблицы и сравниваем каждый элемент класса Ident, у него спрашиваем имя get_name и сравниваем с тем,
//что лежит в буфере. Если уже есть, возвращаем позицию - номер элемента массива, в котором данное имя уже находится. 
//Если не нашли, то переходим в позицию top, который показывает следующую свободную позицию,  к оператору put_name, кладем новое имя, top увеличим, 
//но возвратить нужно предыдущее значение - куда положили
{
	for (int j = 1; j < top; ++j)
		if (!strcmp(buf, p[j].get_name()))
			return j;
	p[top].put_name(buf);
	++top;
	return top - 1;
}

//////////////////Класс Scanner///////////////////////////////////////////////
 //ЛЕКСИЧЕСКИЙ АНАЛИЗ МКА 
class Scanner
{
	enum state { H, IDENT, NUMB, COM, ALE, DELIM, NEQ }; //множество состояний
	static const char* TW[]; //статическая таблица служебных слов (как в тексте)
	static type_of_lex words[]; //статическая таблица служебных слов в закодированном виде type_of_lex
	static const char* TD[]; //статическая таблица служебных разделителей (как в тексте)
	static type_of_lex dlms[]; //статическая таблица служебных разделителей в закодированном виде type_of_lex
	state CS; //текущее состояние
	FILE* fp; //файловый указатель, где хранится текущая программа на М-языке
	char c; //текущий символ
	char buf[80]; //буфер для накопления лексемы посимвольно
	int buf_top; //очередная свободная позиция в буфере - куда кладем очередной символ при накоплении лексемы

	void clear() { //очистка буфера нулями
		buf_top = 0;
		for (int j = 0; j < 80; ++j)
			buf[j] = '\0';
	}

	void add() { //добавляем очередной символ в позицию buf_top
		buf[buf_top++] = c;
	}

	int look(const char* buf, const char** list) //метод look позволяет найти строчку, записанную в буфере buf, среди массива
												 //строк, обозначенных как list, ищет в TW, TD
	{
		int i = 0;
		while (list[i])
		{
			if (!strcmp(buf, list[i]))
				return i;
			++i;
		}
		return 0;
	}

	void gc() { //читаем очередной символ из файла
		c = fgetc(fp);
	}

public:
	Lex get_lex();//ОСНОВНОЙ МЕТОД КЛАССА, выдает очередную лексему

	Scanner(const char* program) {
		fopen_s(&fp, program, "r");//конструктор получает в качестве параметра имя программы на чтение
		CS = H;//
		clear();//очищаем буфер
		gc();//считываем первый символ
	}
};
//Таблицы лексем М-языка
const char*
Scanner::TW[] = { //таблица служебных слов
"",//позиция 0 не используется
"and",// 1
"begin",// 2
"bool",// 3
"do",// 4
"else",// 5
"end",// 6
"if",// 7
"false",// 8
"int",// 9
"not",// 10
"or",// 11
"program",// 12
"read",// 13
"then",// 14
"true",// 15
"var",// 16
"while",// 17
"write",// 18
"for",// 19
"to",// 20
NULL
};

const char*
Scanner::TD[] = { //ограничители
"",//позиция 0 не используется
"@",// 1
";",// 2
",",// 3
":",// 4
":=",// 5
"(",// 6
")",// 7
"=",// 8
"<",// 9
">",// 10
"+",// 11
"-",// 12
"*",// 13
"/",// 14
"<=",// 15
"!=",// 16
">=",// 17
NULL
};

tabl_ident TID(100);//таблица пользовательских имен заполняется в процессе работы ЛА
//Таблицы кодов
type_of_lex Scanner::words[] = {
LEX_NULL, LEX_AND, LEX_BEGIN, LEX_BOOL, LEX_DO, LEX_ELSE, LEX_END, LEX_IF, LEX_FALSE, LEX_INT, LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_THEN, LEX_TRUE, LEX_VAR,
LEX_WHILE, LEX_WRITE, LEX_FOR, LEX_TO
};

type_of_lex Scanner::dlms[] = {
LEX_NULL, LEX_FIN, LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS, LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH,
LEX_LEQ, LEX_NEQ, LEX_GEQ
};

Lex Scanner::get_lex()// основная работа, выдает очередную лексему
{
	int d, j;
	CS = H;//начинаем с состояния H
	do
	{
		switch (CS)
		{
		case H:
			if (c == ' ' || c == '\n' || c == '\r' || c == '\t')//пропускаем пробелы
				gc();
			else if (isalpha(c)) {
				clear();//очищаем буфер
				add();//добавляем букву
				gc();//считываем следующий символ
				CS = IDENT;//меняем состояние на IDENT
			}

			else if (isdigit(c)) {
				d = c - '0';
				gc();
				CS = NUMB;
			}

			else if (c == '{') {
				gc();
				CS = COM;
			}

			else if (c == ':' || c == '<' || c == '>') {
				clear();
				add();
				gc();
				CS = ALE;
			}

			else if (c == '@')
				return Lex(LEX_FIN);

			else if (c == '!') {
				clear();
				add();
				gc();
				CS = NEQ;
			}

			else
				CS = DELIM;
			break;

		case IDENT:
			if (isalpha(c) || isdigit(c)) {
				add();
				gc();
			}

			else //считываем до тех пор, пока не буква/цифра - получилось букв-цифровое слово в buf
				if (j = look(buf, TW))//сначала ищем найденное слово в TW. Если нашли, возвращаем позицию j, по номеру позиции из таблицы сформируем лексему, соотв. этому ключевому слову
					return Lex(words[j], j);
				else
				{
					j = TID.put(buf);//если не нашли в TW, значит это пользовательское имя. С помощью put добавляем в таблицу имен и 
								     //put возвращает позицию в TID, эту позицию мы используем, чтобы сформировать лексему 
									 //другого сорта: тип лексемы, номер позиции
					return Lex(LEX_ID, j);
				}
			break;

		case NUMB:
			if (isdigit(c)) {
				d = d * 10 + (c - '0');
				gc();
			}
			else
				return Lex(LEX_NUM, d);
			break;

		case COM://выбросили комментарии и вернулись в Н
			if (c == '}') {
				gc();
				CS = H;
			}
			else if (c == '@' || c == '{')//если внутри комментария конец программы @
				throw c;
			else
				gc();
			break;

		case ALE:
			if (c == '=')//возвращаем := | <= | >=
			{
				add();
				gc();
				j = look(buf, TD);
				return Lex(dlms[j], j);
			}
			else//возвращаем : | < | > 
			{
				j = look(buf, TD);
				return Lex(dlms[j], j);
			}
			break;

		case NEQ:
			if (c == '=') {
				add();
				gc();
				j = look(buf, TD);
				return Lex(LEX_NEQ, j);//возвращаем !=
			}
			else
				throw '!';//ошибка
			break;

		case DELIM:
			clear();
			add();
			if (j = look(buf, TD)) {
				gc();
				return Lex(dlms[j], j);
			}
			else
				throw c;
			break;
		}
		// end switch
	} while (true);
}




/////////////////////class Poliz///////////////////////////////////////////
//описание внутреннего устройства генерируемой программы
class Poliz
{
	Lex* p; //указатель на массив из 1000 элементов, где будет выписан полиз программы
	int size; //реальный размер массива <= 1000, на который указывает p
	int free; //текущая позиция в полизе, куда можно положить очередную лексему
public:
	Poliz(int max_size) { //конструктор класса Poliz получает размер и отводит память в динам.памяти
		p = new Lex[size = max_size];//для массива лексем размера max_size
		free = 0; //начинаем заполнять массив с нулевой позиции, удобно возвращать ноль, если в таблице элемент не найден
	};
	~Poliz() { delete[]p; }; //удаление массива, хранящего программу

	void put_lex(Lex l) { p[free] = l; ++free; }; //помещаем лексему, заданную в качестве параметра метода put_lex 
												  //в позицию free

	void put_lex(Lex l, int place) { p[place] = l; }; //перегруженный метод, помещает лексему не в текущую свободную позицию,
													  // а в позицию l, необходимо для пропущенных символов ПОЛИЗ

	void blank() { ++free; }; //пропустить свободную позицию

	int get_free() { return free; }; //получить текущее значение позиции free - узнаем какая сейчас текущая позиция

	Lex& operator[] (int index) { //оператор [] позволяет добраться до i-го элемента ПОЛИЗ и возвращает ссылку на него
		if (index > size)
			throw "POLIZ:out of array"; //если промахнулись с диапазоном - ошибка
		else
			if (index > free)
				throw "POLIZ:indefinite element of array";
			else
				return p[index]; //возвращается соотв.лексема, номер которой указан в [] при обращении к этой операции
	};

	void print() { //печатаем полиз
		cout << "\nPoliz:\n";
		for (int i = 0; i < free; ++i)
			cout << i << "\t" << p[i];
	};
};


template <class T, int max_size > class Stack
{
	T s[max_size];
	int top;
public:
	Stack() { top = 0; }
	void reset() { top = 0; }
	void push(T i);
	T pop();
	bool is_empty() { return top == 0; }
	bool is_full() { return top == max_size; }
};
template <class T, int max_size >
void Stack <T, max_size >::push(T i)
{
	if (!is_full())
	{
		s[top] = i;
		++top;
	}
	else
		throw "Stack_is_full";
}
template <class T, int max_size >
T Stack <T, max_size >::pop()
{
	if (!is_empty())
	{
		--top;
		return s[top];
	}
	else
		throw "Stack_is_empty";
}
//////////////////////////  Класс Parser  /////////////////////////////////
//СИНТАКСИЧЕСКИЙ ЭТАП, АНАЛИЗАТОР МЕТОДОМ РС

class Parser
{
	Lex        curr_lex; //текущая лексема
	type_of_lex c_type;//тип curr_lex
	int        c_val;//значение curr_lex
	Scanner        scan;//ЛА - подобъект СА
	Stack < int, 100 > st_int;//сем.этап-переменным типа int и bool предшествует список идентификаторов-переменных.
							//Эти идентификаторы (вернее, номера соответствующих им строк таблицы TID) запоминаются 
							// в этом стеке целых чисел, 
							//а когда будет проанализирован тип, надо заполнять поля declare и type в этих строках
	Stack < type_of_lex, 100 > st_lex;//сем.этап

	// процедуры РС-метода
	void P(); // процедура распознает program->запустит процедуру D1->распознает ';'->
			  //запустит процедуру B (кот.распознает тело программы)->распознает признак конца программы @
	void D1(); //var D {,D}
	void D(); //I {, I}: [ int | bool ]
	void B(); //begin S {;S} end
	void S(); //I := E | if E then S else S| while E do S | write (E) | for E do S
	void E(); //E1 [ = | < | > | != | <= | >= ] E1 | E1
	void E1(); //T {[ + | - | or ] T}
	void T(); //F {[ * | / | and ] F}
	void F(); // not F | (E)

	void dec(type_of_lex type); //для переменных, например, a, b: int, смотри в class Stack
								//считывает из стека номера строк таблицы TID и заносить в них инфо о типе соотв.переменных, 
								//о наличии их описаний + будет контролировать их повторное описание
	void check_id();
	void check_op();
	void check_not();
	void eq_type();
	void eq_bool();
	void check_id_in_read();

	void gl() //запрашивает у Scanner очередную лексему
	{
		curr_lex = scan.get_lex();//обращается к Scanner и запрашивает очередную лексему, кот.записывается в curr_lex 
		cout << curr_lex;//*
		c_type = curr_lex.get_type();//в c_type get_type
		c_val = curr_lex.get_value();//сем.этап
	}
public:
	Poliz prog;
	Parser(const char* program) : scan(program), prog(1000) {}//конструктор имеет один параметр - имя файла, 
	                                                          //передает конструктору ЛА scan, 
														      //выделили память под программу prog в 1000 лексем - сем.этап
	void analyze();//ГЛАВНАЯ ФУНКЦИЯ, анализатор с действиями, запускает методом РС систему РС процедур
};
void Parser::analyze()
{
	gl();//analyze считывет первую лексему и запускает процедуру, соотв. нач.символу грамматики М-языка - P
	P();//запускаем процедуру P
	prog.print();
	cout << endl << "Yes!!!" << endl;
}

void Parser::P()
{
	if (c_type == LEX_PROGRAM)
		gl();
	else
		throw curr_lex;//нет на первом месте слова program, бросаем исключение в качестве объекта кидаем ошибочную лексему
	D1();//вызываем D1, она распознает описание
	if (c_type == LEX_SEMICOLON)//проверяем, что после описания есть ';'
		gl();
	else
		throw curr_lex;//если после описания нет ";", бросаем ошибку
	B();//вызывем B, она обработает тело программы
	if (c_type != LEX_FIN) //если нет конца программы, бросаем ошибку
		throw curr_lex;
}
void Parser::D1()
{
	if (c_type == LEX_VAR)//распознали переменную
	{
		gl();//считали следующую лексему
		D();//запустили в первый раз D
		while (c_type == LEX_COMMA)//пока есть "," запускаем gl и D
		{
			gl();
			D();
		}
	}
	else
		throw curr_lex;//var не встретился, ошибка
}

void Parser::D()
{
	st_int.reset();//перед тем как обработать первый идентификатор, очистим стек (в котором номера идентификаторов из TID)
					//a, b: int
	if (c_type != LEX_ID) //если не идентификатор - ошибка
		throw curr_lex;
	else
	{
		st_int.push(c_val);//обработаем очередной идентификатор, добавим его номер c_val в таблице имен в стек
		gl();
		while (c_type == LEX_COMMA)//пока есть идентификаторы через ',' делаем то же самое с очередны идентификатором
		{
			gl();
			if (c_type != LEX_ID)
				throw curr_lex;
			else
			{
				st_int.push(c_val);
				gl();
			}
		}
		if (c_type != LEX_COLON) //проверяем, что после идентфиикаторов есть ':'
			throw curr_lex;
		else//после ':'
		{
			gl();
			if (c_type == LEX_INT)//если int - обращаемся к методу dec(LEX_INT) - проставляем тип всем идентификаторам, перечисленным через запятую в разделе описаний
			{
				dec(LEX_INT);
				gl();
			}
			else
				if (c_type == LEX_BOOL)//если int - обращаемся к методу dec(LEX_BOOL)
				{
					dec(LEX_BOOL);
					gl();
				}
				else throw curr_lex;
		}
	}
}

void Parser::B()
{
	if (c_type == LEX_BEGIN)
	{
		gl();
		S();
		while (c_type == LEX_SEMICOLON)
		{
			gl();
			S();
		}
		if (c_type == LEX_END)
			gl();
		else
			throw curr_lex;
	}
	else
		throw curr_lex;
}
void Parser::S()
{
	int pl0, pl1, pl2, pl3, pl4, pl5;
	if (c_type == LEX_IF)
	{
		gl();
		E();
		eq_bool();
		pl2 = prog.get_free();
		prog.blank();
		prog.put_lex(Lex(POLIZ_FGO));
		if (c_type == LEX_THEN)
		{
			gl();
			S();
			pl3 = prog.get_free();
			prog.blank();
			prog.put_lex(Lex(POLIZ_GO));
			prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl2);
			if (c_type == LEX_ELSE)
			{
				gl();
				S();
				prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl3);
			}
			else
				throw curr_lex;
		}
		else
			throw curr_lex;
	} //end if
	else
		if (c_type == LEX_WHILE)
		{
			pl0 = prog.get_free();
			gl();
			E();
			eq_bool();
			pl1 = prog.get_free();
			prog.blank();
			prog.put_lex(Lex(POLIZ_FGO));
			if (c_type == LEX_DO)
			{
				gl();
				S();
				prog.put_lex(Lex(POLIZ_LABEL, pl0));
				prog.put_lex(Lex(POLIZ_GO));
				prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl1);
			}
			else
				throw curr_lex;
		} //end while
	else
		if (c_type == LEX_FOR)
		{

			gl();
			
			pl1 = c_val; // "номер переменной в TID"
			S();
			if (c_type == LEX_TO)
			{
				gl(); // взять "докуда" 3
				if (pl1 <= c_val) // если "докуда" < "откуда" то ошибка
				{
					pl2 = c_val; // запомнить "докуда"
				}
				else throw "Syntax error: for: 'from' greater than 'to'";

				pl3 = prog.get_free(); // узнать следующую позицию в структуре полиз
				prog.put_lex(Lex(LEX_ID, pl1)); //кладем номер переменной с в TID в полиз
				prog.put_lex(Lex(LEX_NUM, pl2));//кладем правое значение в полиз
				prog.put_lex(Lex(LEX_LEQ)); // кладем операнд <= в полиз
				pl0 = prog.get_free(); //узнаем номер следующей ячейки полиз
				prog.blank(); //пропускаем позицию для перехода по !F
				prog.put_lex(Lex(POLIZ_FGO));//кладем !F в полиз
				gl();
				if (c_type == LEX_DO)
				{
					gl();
					S(); //проверяем после DO

					prog.put_lex(Lex(POLIZ_ADDRESS, pl1)); //кладем ADDRESS с в полиз
					prog.put_lex(Lex(LEX_ID, pl1)); //кладем номер переменной с в TID в полиз
					prog.put_lex(Lex(LEX_NUM, 1));//кладем 1 в полиз
					prog.put_lex(Lex(LEX_PLUS)); //кладем + в полиз
					prog.put_lex(Lex(LEX_ASSIGN));//кладем := в полиз
					prog.put_lex(Lex(POLIZ_LABEL, pl3)); //кладем номер позиции для перехода ! в полиз
					prog.put_lex(Lex(POLIZ_GO));//кладем переход ! в полиз
				}
				prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl0);//узнаем куда переходить по !F
			}
			else
				throw curr_lex;
			
		}//end for
			else
				if (c_type == LEX_READ)
				{
					gl();
					if (c_type == LEX_LPAREN)
					{
						gl();
						if (c_type == LEX_ID)
						{
							check_id_in_read();
							prog.put_lex(Lex(POLIZ_ADDRESS, c_val));
							gl();
						}
						else
							throw curr_lex;
						if (c_type == LEX_RPAREN)
						{
							gl();
							prog.put_lex(Lex(LEX_READ));
						}
						else
							throw curr_lex;
					}
					else
						throw curr_lex;
				} //end read
				else
					if (c_type == LEX_WRITE)
					{
						gl();
						if (c_type == LEX_LPAREN)
						{
							gl();
							E();
							if (c_type == LEX_RPAREN)
							{
								gl();
								prog.put_lex(Lex(LEX_WRITE));
							}
							else
								throw curr_lex;
						}
						else
							throw curr_lex;
					} //end write
					else
						if (c_type == LEX_ID)
						{
							check_id();//проверим, объявлена ли переменная
										//если объявлена, заносим тип в стек
							prog.put_lex(Lex(POLIZ_ADDRESS, c_val));//помещаем POLIZ_ADDRESS в стек
							gl();
							if (c_type == LEX_ASSIGN)
							{
								gl();
								E();
								eq_type();
								prog.put_lex(Lex(LEX_ASSIGN));
							}
							else
								throw curr_lex;
						} //assign-end
						else B();
}
void Parser::E()
{
	E1();
	if (c_type == LEX_EQ || c_type == LEX_LSS || c_type == LEX_GTR ||
		c_type == LEX_LEQ || c_type == LEX_GEQ || c_type == LEX_NEQ)
	{
		st_lex.push(c_type);
		gl();
		E1();
		check_op();
	}
}
void Parser::E1()
{
	T();
	while (c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_OR)
	{
		st_lex.push(c_type);
		gl();
		T();
		check_op();
	}
}
void Parser::T()
{
	F();
	while (c_type == LEX_TIMES || c_type == LEX_SLASH || c_type == LEX_AND)
	{
		st_lex.push(c_type);
		gl();
		F();
		check_op();
	}
}
void Parser::F()
{
	if (c_type == LEX_ID) //если имеем дело с идентификатором
	{
		check_id(); //то проверяем, что идентификатор описан
		prog.put_lex(Lex(LEX_ID, c_val)); //check_id помещает лексему в полиз
		gl();
	}
	else
		if (c_type == LEX_NUM) //если числовая константа
		{
			st_lex.push(LEX_INT); //тип константы кладем в сем.стек
			prog.put_lex(curr_lex); //в полиз записываем лексему
			gl();
		}
		else
			if (c_type == LEX_TRUE) //если TRUE
			{
				st_lex.push(LEX_BOOL); //в сем. стек тип bool
				prog.put_lex(Lex(LEX_TRUE, 1)); //формируем логическую константу LEX_TRUE со значением 1 и записваем в полиз
				gl();
			}
			else
				if (c_type == LEX_FALSE) //если FALSE
				{
					st_lex.push(LEX_BOOL); //в сем. стек тип bool
					prog.put_lex(Lex(LEX_FALSE, 0)); //формируем логическую константу LEX_FALSE со значением 0 и записваем в полиз
					gl();
				}
				else
					if (c_type == LEX_NOT) //если NOT
					{
						gl(); //читаем следующую лексему
						F(); //в сем.стеке окажется тип операнда для NOT
						check_not(); //check_not проверит, логический ли тип операнда, если тип логический - 
									 //положит в сем.стек, в полиз положит операцию NOT
					}
					else
						if (c_type == LEX_LPAREN) //если левая скобка
						{
							gl(); //пропускаем, читаем след.лексему
							E(); //вызываем процедуру Е, она обрабатывает то, что в скобках, после того как вернулись
							if (c_type == LEX_RPAREN) //проверяем, есть ли правая скобка
								gl();
							else
								throw curr_lex;
						}
						else
							throw curr_lex; //если ни один вариант не подошел - ошибка
}


///////////////////////класс Stack/////////////////////////////////////////
//Семантический этап
void Parser::dec(type_of_lex type) { //для переменных a, b: int
									//считывает из стека номера строк таблицы TID и заносит в них инфо
								//о типе соотв.переменных, о наличии их описаний, а также контролирует повторное описание
	int i;
	while (!st_int.is_empty())//пока стек не пуст
	{
		i = st_int.pop();//записывает его номер в переменную i
		if (TID[i].get_declare()) //ищем в TID соответствующий i-ый элемент
			throw "twice"; //если он уже описан - это ошибка
		else
		{
			TID[i].put_declare();//заполняем поле declare, что идентификатор уже описан
			TID[i].put_type(type);//и он имеет такой тип
		}
	}
}

void Parser::check_id() //контроль контекстных условий в выражении

{
	if (TID[c_val].get_declare())//если операнд - лексема-переменная, проверяем, описана ли она. 
		st_lex.push(TID[c_val].get_type()); //Если описана, ее тип заносится в стек
	else
		throw "not declared";
}

void Parser::check_op()//контроль троек из стека: операнд-операция-операнд (соотв-ие типов операндов друг другу и типу операции)
{
	type_of_lex t1, t2, op, t = LEX_INT, r = LEX_BOOL;
	t2 = st_lex.pop(); //вытаскиваем сперва правый операнд в t2
	op = st_lex.pop(); //вытаскиваем операцию в op
	t1 = st_lex.pop(); //вытаскиваем левый операнд в t1
	if (op == LEX_PLUS || op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH) //если операция + - * /
		r = LEX_INT; //то результирующий тип д.б. int
	if (op == LEX_OR || op == LEX_AND) //если операция OR AND
		t = LEX_BOOL; //тип операндов д.б. bool
	if (t1 == t2 && t1 == t) //если типы операндов равны друг другу и соответствуют типу для данной операции
		st_lex.push(r); //то тип результата помещаем в стек
	else
		throw "wrong types are in operation";//иначе ошибка
	prog.put_lex(Lex(op)); //кладем саму операцию в полиз
}

void Parser::check_not() { //контроль за типом операнда операции not
	if (st_lex.pop() != LEX_BOOL) //вытаскиваем из стека тип, если не bool
		throw "wrong type is in not"; //сообщение об ошибке
	else {
		st_lex.push(LEX_BOOL); //если bool, обратно в стек кладем LEX_BOOL
		prog.put_lex(Lex(LEX_NOT)); //кладем операцию NOT в полиз
	}
}

void Parser::eq_type() { //контроль, равны ли типы при операции присваивания I := E
	if (st_lex.pop() != st_lex.pop()) throw "wrong types are in :="; //вытаскивает два верхних значения из стека и сравнивает их
}

//в условном операторе и в операторе цикла в качестве условия возможны только логические выражения
//if E then S else S | while E do S
void Parser::eq_bool() { //контроль условных операторов if и while do, for
	if (st_lex.pop() != LEX_BOOL) //извлекаем из стека тип выражения E и проверяем bool
		throw "expression is not boolean";
}

void Parser::check_id_in_read() { //контроль оператора read(I)
	if (!TID[c_val].get_declare()) //по текущему номеру c_val из TID обращаемся к get_declare
		throw "not declared"; //если не описано - ошибка
}



/////////////////////class Executer///////////////////////////////////////////
 //интерпретирует полиз
class Executer {
	Lex pc_el; //данные класса Lex - текущая лексема
public:
	void execute(Poliz& prog); //методу execute дается ссылка на объект класса полиз для интерпретации полиз
};

void Executer::execute(Poliz& prog)//реализация void execute
{
	Stack < int, 100 > args; //заводим вспомогательный целочисленный стек с помощью ранее описанного шаблона
	int i, j, index = 0, size = prog.get_free(); //index - текущая позиция полиз, кот.интерпретируем, size-размер, 
												 //который можем узнать с помощью метода get_free из класса полиз, 
											//т.к. после формирования полиза free указывает на последний элемент полиз
	while (index < size) //пока не дошли до конца полиз
	{
		pc_el = prog[index]; //в pc_el вынимаем очередной элемент полиз
		switch (pc_el.get_type()) //проверяем различные случаи
		{
		case LEX_TRUE: //если тип  лексемы - логическая / число / адрес переменной / метка
		case LEX_FALSE:
		case LEX_NUM:
		case POLIZ_ADDRESS:
		case POLIZ_LABEL:
			args.push(pc_el.get_value()); //то считаем аргументом некоторой операции - значит кладем значение лексемы в стек
			break;

		case LEX_ID: //если тип лексемы - идентификатор LEX_ID
			i = pc_el.get_value(); //узнаем под каким номером i зарегистрирован этот идентификатор в TID
			if (TID[i].get_assign()) //проверяем, было ли что-то присвоено ранее этой переменной
			{
				args.push(TID[i].get_value()); //если присвоено значение - кладем значение в стек
				break;
			}
			else //если не присвоено - ошибка "Идентификатор не определен"
				throw "POLIZ: indefinite identifier";

		case LEX_NOT: //если текущая лексема - операция NOT
			args.push(!args.pop()); //вытаскиваем верхний элемент, применяем к нему операцию отрицания и кладем обратно
			break;

		case LEX_OR:
			i = args.pop(); //берем верхний элемент
			args.push(args.pop() || i); //складываем с нижним элементом i
			break;

		case LEX_AND:
			i = args.pop(); //берем верхний элемент
			args.push(args.pop() && i); //умножаем на нижний элемент
			break;

		case POLIZ_GO: //для безусловного перехода
			index = args.pop() - 1; //берем один операнд из стека, вычитаем 1, т.к. в конце цикла ++index
			break;

		case POLIZ_FGO: //для условного перехода выбираем два аргумента
			i = args.pop();
			if (!args.pop()) //если второй аргумент ложь
				index = i - 1; //индекс корректируем и будет осуществлен переход. Если не ложь - коррекции не будет,
							   //автоматически на следующей итерации цикла будет переход на следующий элемент полиз
			break;

		case LEX_WRITE:
			cout << args.pop() << endl;
			break;

		case LEX_READ:
		{
			int k;
			i = args.pop();
			if (TID[i].get_type() == LEX_INT) //если тип i-ой переменной - int
			{
				cout << "Input int value for"; //выводим подсказку ввести целое значение
				cout << TID[i].get_name() << endl;
				cin >> k;//вводим в переменную к это значение
			}
			else //если bool, выводим подсказку
			{
				char j[20];
			rep:
				cout << "Input boolean value; ";//подсказка - ввести логическую переменную
				cout << "(true or false) for";
				cout << TID[i].get_name() << endl;
				cin >> j;//вводим значение
				if (!strcmp(j, "true"))
					k = 1;
				else if (!strcmp(j, "false"))
					k = 0;
				else
				{
					cout << "Error in input:true/false"; //если ввели неправильно
					cout << endl;
					goto rep; //переход на метку rep
				}
			}
			TID[i].put_value(k); //значение  помещается в соответствующую i-ую переменную в таблице имен
			TID[i].put_assign(); //и assign устанавливаем в истину - переменная проинициализирована
			break; }

		case LEX_PLUS:
			args.push(args.pop() + args.pop());
			break;

		case LEX_TIMES:
			args.push(args.pop() * args.pop());
			break;

		case LEX_MINUS:
			i = args.pop();
			args.push(args.pop() - i);
			break;

		case LEX_SLASH:
			i = args.pop();
			if (!i)
			{
				args.push(args.pop() / i);
				break;
			}
			else throw "POLIZ:divide by zero";
		case LEX_EQ:
			args.push(args.pop() == args.pop());
			break;
		case LEX_LSS:
			i = args.pop();
			args.push(args.pop() < i);
			break;
		case LEX_GTR:
			i = args.pop();
			args.push(args.pop() > i);
			break;
		case LEX_LEQ:
			i = args.pop();
			args.push(args.pop() <= i);
			break;
		case LEX_GEQ:
			i = args.pop();
			args.push(args.pop() >= i);
			break;
		case LEX_NEQ:
			i = args.pop();
			args.push(args.pop() != i);
			break;
		case LEX_ASSIGN:
			i = args.pop(); //правая часть
			j = args.pop(); //левая часть
			TID[j].put_value(i);//присваиваем значение переменной J
			TID[j].put_assign(); //поле assign устанавливаем true, чтобы отметить, что данная переменная j инициализирована
			break;
		default:
			throw "POLIZ: unexpected elem";
		}
		// end of switch
		++index; //переходим к следующему элементу полиза
	};
	//end of while
	cout << "Finish of executing!!!" << endl;
}

/////////////////////class Interpretator///////////////////////////////////////////

class Interpretator
	//содержит объект pars класса Parser и объект Е класса Executer
{
	Parser pars;
	Executer E;
public:
	Interpretator(char* program) : pars(program) {}; //имя файла передается конструктору парсера, а тот передает его сканеру, 
												     //кот.открывает его на чтение и выделяет лексемы по просьбе СА
	void interpretation();//основной метод класса
};
void Interpretator::interpretation()
{
	pars.analyze(); //обращаемся к метода analyze объекта pars  - произойдет СА при помощи ЛА по приниципу дай лексему, 
	//заодно произойдет сем.анализ и будет произведена генерация кода в подобъект prog класса Pars
	E.execute(pars.prog); //этот объект хранящий программу в полиз передаем методу execute класса Е,там произойдет цикл,
						  //интерпретирующий элемент за элементом, полученый в результате трансляции полиз
}


int main() {
	try {
		char str[] = "prog_2.txt";
		Interpretator I(str); //описывается объект I интерпретатор, в качестве параметра констр-ра указываем, где хранится исх.прогр-ма
		I.interpretation(); //запускаем метод interpretation
		return 0;
	}
	catch (char c) {
		cout << "unexpected symbol " << c << endl; //ошибка в ЛА - символ
		return 1;
	}
	catch (Lex l) {
		cout << "unexpected lexeme" << l << endl; //ошибка СА - лексема
		return 1;
	}
	catch (const char* source) { //ошибка сем.анализа - типы операндов, операций, ошибки выполнений (деление на 0)
		cout << source << endl;
		return 1;
	}
}