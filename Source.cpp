#include "Header.h"

// グリーバル変数定義
int giMode;
int giView;
int giLines;
char gsInFile[STRING_MAX];
char gsOutFile[STRING_MAX];
char gsOpt[STRING_MAX];
sBuffer *sbList;

// 標準入力から文字列を取得する
void get_stdin(char *str,const char *msg) {
	char s[256];

	printf("%s", msg);
	fgets(s, sizeof(s) - 1, stdin);
	s[strlen(s) - 1] = '\0';
	strcpy_s(str, STRING_MAX, s);
}

// リングバッファを作成する
sBuffer* make_ring(int num) {
	sBuffer *b,*s;

	b = new sBuffer;
	s = b;
	for (int i = 1; i < num; i++) {
		b->s[0] = '\0';
		b->next = new sBuffer;
		b = b->next;
	}
	b->next = s;
	return s;
}

// リングバッファに値を入れる
sBuffer* add_ring(sBuffer *b,const char *str) {
	strcpy_s(b->s, LINE_LENGTH, str);
	return b->next;
}

// 単方向リストを作成する
sBuffer* make_list(void) {
	sBuffer *b;

	b = new sBuffer;
	b->s[0] = '\0';
	b->next = NULL;
	return b;
}

// リストに値を追加し、リストを伸張する
sBuffer* add_list(sBuffer *b, const char *str) {
	strcpy_s(b->s, LINE_LENGTH, str);
	b->next = new sBuffer;
	b->next->s[0] = '\0';
	b->next->next = NULL;
	return b->next;
}

// リストを開放する
void clr_list(sBuffer *e) {
	sBuffer *n,*b;

	b = sbList;
	do {
		n = b->next;
		delete(b);
		b = n;
	} while (n != e);
}

char chr_ucase(char c) {
	if (c >= 97 && c <= 122) c -= 32;
	return c;
}

char chr_lcase(char c) {
	if (c >= 65 && c <= 90) c += 32;
	return c;
}

// オプションヘッダを検出する
int chk_opt(const char *str) {
	return (str[0] == OPT_HEAD || str[0] == OPT_HEAD2);
}

// オプションを解析し、パラメータに反映する
int init_gvars(int argc,char **argv) {
	int i;

	giMode = MODE_TAIL;
	giView = 0;
	giLines = TAIL_LINES;

	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) > 1 && chk_opt(argv[i])) {
			switch (chr_lcase(argv[i][1])) {
				case OPT_GREP:
					giMode = MODE_GREP;
					if (i < argc - 1 && !chk_opt(argv[i + 1]) && strlen(argv[i + 1]) > 0) {
						strcpy_s(gsOpt,STRING_MAX,argv[i + 1]);
						i++;
					}
					if (strlen(gsOpt) == 0) get_stdin(gsOpt,"検索ワードを入力してください。:");
					if (strlen(gsOpt) == 0) {
						fprintf(stderr, "ERROR:検査ワードが空です。\n");
						return 0;
					}
					break;
				case OPT_TAIL:
					giMode = MODE_TAIL;
					if (i < argc - 1 && !chk_opt(argv[i + 1]) && atoi(argv[i + 1]) > 0) {
						giLines = atoi(argv[i + 1]);
						i++;
					}
					if (giLines <= 0) giLines = TAIL_LINES;
					break;
				case OPT_OUT:
					if (i < argc - 1 && !chk_opt(argv[i + 1]) && strlen(argv[i + 1]) > 0) {
						strcpy_s(gsOutFile,STRING_MAX,argv[i + 1]);
						i++;
					}
					if (strlen(gsOutFile) == 0) get_stdin(gsOutFile,"出力ファイル名を入力してください。:");
					if (strlen(gsOutFile) == 0) strcpy_s(gsOutFile,STRING_MAX,OUT_FILENAME);
					break;
				case OPT_VIEW:
					giView = 1;
			}
		}else {
			strcpy_s(gsInFile,STRING_MAX,argv[i]);
		}
	}
	switch (giMode) {
		case MODE_TAIL:
			sbList = make_ring(giLines);
			break;
		case MODE_GREP:
			sbList = make_list();
			break;
	}
	return 1;
}

// フルパスからディレクトリ名を返す
void dirname(char *d, const char *s) {
	strcpy_s(d, STRING_MAX, s);
	for (int i = strlen(d) - 1; i >= 0; i--) {
		if (d[i] == '\\') {
			d[i] = '\0';
			break;
		}
	}
}


// ディレクトリ名がない場合、カレントディレクトリをパスに追加する
void get_path(char *str, const char *parent,const char *path) {
	if (strstr(path, "\\") == NULL) {
		if (parent == NULL) {
			strcpy_s(str, STRING_MAX,_getcwd(NULL,0));
		}
		else {
			char p[STRING_MAX];
			dirname(p, parent);
			strcpy_s(str, STRING_MAX, p);
		}
		strcat_s(str, STRING_MAX, "\\");
		strcat_s(str, STRING_MAX, path);
	} else {
		strcpy_s(str, STRING_MAX, path);
	}
}

// 文字列検索処理
sBuffer* exec_grep(sBuffer *b, const char *str, const char *key) {
	char *p = NULL,*i = NULL,s[STRING_MAX];
	int sc = 0,cnt = 0;
	strcpy_s(s, STRING_MAX, key);
	i = strtok_s(s, " ",&p);
	do {
		if (strstr(str, i)) sc++;
		i = strtok_s(NULL, " ", &p);
		cnt++;
	} while (i != NULL);
	if (sc == cnt) b = add_list(b, str);
	return b;
}

// 検索を実行する
int search_exec(void) {
	FILE *fp;
	sBuffer *b;
	char s[LINE_LENGTH],p[STRING_MAX];
	int cnt = 0, sum = 0, line = 0, last = 0, start = 0;
	get_path(p,NULL,gsInFile);
	if (fopen_s(&fp,p, "r")) return 0;
	b = sbList;
	while (fgets(s, LINE_LENGTH, fp) != NULL) {
		line++;
		switch (giMode) {
			case MODE_GREP:
				b = exec_grep(b, s, gsOpt);
				break;
			case MODE_TAIL:
				b = add_ring(b, s);
				sbList = b;
				break;
		}
	}
	fclose(fp);
	return 1;
}

// デバッグ用パラメータ表示
void print_debug(void) {
	char s[3][STRING_MAX] = {"","GREP","TAIL"};

	fprintf(stderr, "<<<< Param Info >>>>\n");
	fprintf(stderr, "MODE          = %s\n", s[giMode]);
	fprintf(stderr, "LINES         = %d\n", giLines);
	fprintf(stderr, "IN FILE NAME  = %s\n", gsInFile);
	fprintf(stderr, "OUT FILE NAME = %s\n", gsOutFile);
	fprintf(stderr, "OPTIONS       = %s\n", gsOpt);
	fprintf(stderr, "\n");
}

// リストを表示する
void print_list(FILE *fp,sBuffer *e) {
	sBuffer *b;

	b = sbList;
	do {
		fprintf(fp,"%s", b->s);
		b = b->next;
	} while (b != e);
}

// リストをテキストファイルに保存する。
void save_list(sBuffer *e) {
	char p[STRING_MAX];
	FILE *fp;
	get_path(p,gsInFile,gsOutFile);
	if (fopen_s(&fp, p, "w")) return;
	print_list(fp, e);
	fclose(fp);
}

void print_usage(char *argv) {
	char *p;

	p = argv;
	for (int i = strlen(argv) - 1; i >= 0; i--) {
		if (argv[i] == '\\') {
			p = &argv[i + 1];
			break;
		}
	}
	fprintf(stderr, "USAGE: %s [-t [linenum]|-g [searchkey]][-o [outfile]][-v] <infile>\n\n",p);
	fprintf(stderr, "    -t [linenum]   Tailモードで抽出します。(linenumで行数を指定します。)\n");
	fprintf(stderr, "    -g [searchkey] Grepモードで抽出します。(searchkeyで検索キーを指定します。)\n");
	fprintf(stderr, "    -o [outfile]   ファイルに出力します。(outfileでファイル名を指定します。)\n");
	fprintf(stderr, "    -v [outfile]   ファイル出力モードでも強制的に結果を表示させます。\n");
}

// エントリポイント
int main(int argc,char *argv[]) {
	sBuffer *e = NULL;

	fprintf(stderr, "ログカットツール\n\n");
	if (argc == 1) {
		print_usage(argv[0]);
	}
	else if (init_gvars(argc, argv)) {
		print_debug();
		if (giMode == MODE_TAIL) e = sbList;
		if (search_exec()) {
			if (strlen(gsOutFile) > 0) save_list(e);
			if (giView || strlen(gsOutFile) == 0) {
				print_list(stdout, e);
				fprintf(stderr, "何かキーを押すと終了します。");
				getchar();
			}
		}
		else {
			fprintf(stderr, "ERROR:入力ファイルが見つかりませんでした。\n");
		}
		clr_list(e);
	}
	return 0;
}