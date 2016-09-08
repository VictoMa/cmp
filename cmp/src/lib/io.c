#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

//entry of program
extern int64_t start_program();

void println() {
	wprintf(L"\n");
}

void printI(int64_t v) {
	wprintf(L"%lld", v);
}

void printD(double v) {
	wprintf(L"%lf", v);
}

int main(int argc, char **argv) {
	setlocale(LC_CTYPE, "en_US.UTF-8");
	return start_program();
}
