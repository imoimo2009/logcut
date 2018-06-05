#include "Header.h"

// �O���[�o���ϐ���`
int giMode;
int giView;
int giLines;
char gsInFile[STRING_MAX];
char gsOutFile[STRING_MAX];
char gsOpt[STRING_MAX];
sBuffer *sbList;

// �W�����͂��當������擾����
void get_stdin(char *str,const char *msg) {
	char s[256];

	printf("%s", msg);
	fgets(s, sizeof(s) - 1, stdin);
	s[strlen(s) - 1] = '\0';
	strcpy_s(str, STRING_MAX, s);
}

// �����O�o�b�t�@���쐬����
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

// �����O�o�b�t�@�ɒl������
sBuffer* add_ring(sBuffer *b,const char *str) {
	strcpy_s(b->s, LINE_LENGTH, str);
	return b->next;
}

// �P�������X�g���쐬����
sBuffer* make_list(void) {
	sBuffer *b;

	b = new sBuffer;
	b->s[0] = '\0';
	b->next = NULL;
	return b;
}

// ���X�g�ɒl��ǉ����A���X�g��L������
sBuffer* add_list(sBuffer *b, const char *str) {
	strcpy_s(b->s, LINE_LENGTH, str);
	b->next = new sBuffer;
	b->next->s[0] = '\0';
	b->next->next = NULL;
	return b->next;
}

// ���X�g���J������
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

// �I�v�V�����w�b�_�����o����
int chk_opt(const char *str) {
	return (str[0] == OPT_HEAD || str[0] == OPT_HEAD2);
}

// �I�v�V��������͂��A�p�����[�^�ɔ��f����
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
					if (strlen(gsOpt) == 0) get_stdin(gsOpt,"�������[�h����͂��Ă��������B:");
					if (strlen(gsOpt) == 0) {
						fprintf(stderr, "ERROR:�������[�h����ł��B\n");
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
					if (strlen(gsOutFile) == 0) get_stdin(gsOutFile,"�o�̓t�@�C��������͂��Ă��������B:");
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

// �t���p�X����f�B���N�g������Ԃ�
void dirname(char *d, const char *s) {
	strcpy_s(d, STRING_MAX, s);
	for (int i = strlen(d) - 1; i >= 0; i--) {
		if (d[i] == '\\') {
			d[i] = '\0';
			break;
		}
	}
}


// �f�B���N�g�������Ȃ��ꍇ�A�J�����g�f�B���N�g�����p�X�ɒǉ�����
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

// �����񌟍�����
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

// ���������s����
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

// �f�o�b�O�p�p�����[�^�\��
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

// ���X�g��\������
void print_list(FILE *fp,sBuffer *e) {
	sBuffer *b;

	b = sbList;
	do {
		fprintf(fp,"%s", b->s);
		b = b->next;
	} while (b != e);
}

// ���X�g���e�L�X�g�t�@�C���ɕۑ�����B
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
	fprintf(stderr, "    -t [linenum]   Tail���[�h�Œ��o���܂��B(linenum�ōs�����w�肵�܂��B)\n");
	fprintf(stderr, "    -g [searchkey] Grep���[�h�Œ��o���܂��B(searchkey�Ō����L�[���w�肵�܂��B)\n");
	fprintf(stderr, "    -o [outfile]   �t�@�C���ɏo�͂��܂��B(outfile�Ńt�@�C�������w�肵�܂��B)\n");
	fprintf(stderr, "    -v [outfile]   �t�@�C���o�̓��[�h�ł������I�Ɍ��ʂ�\�������܂��B\n");
}

// �G���g���|�C���g
int main(int argc,char *argv[]) {
	sBuffer *e = NULL;

	fprintf(stderr, "���O�J�b�g�c�[��\n\n");
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
				fprintf(stderr, "�����L�[�������ƏI�����܂��B");
				getchar();
			}
		}
		else {
			fprintf(stderr, "ERROR:���̓t�@�C����������܂���ł����B\n");
		}
		clr_list(e);
	}
	return 0;
}