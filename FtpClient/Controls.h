#pragma once

#include "Headers.h"


// �������� ����������� ����
HWND ClientCreateWindow(HINSTANCE, WNDPROC, LPCSTR, LPCSTR, long, long);
// �������� ����� �����
BOOL ClientCreateEdits(HINSTANCE, HWND, HWND &, HWND &, HWND &);
// �������� ������
BOOL ClientCreateButtons(HINSTANCE, HWND, HWND &, HWND &, HWND &, HWND &, HWND &);
// �������� �������
BOOL ClientCreateListViews(HINSTANCE, HWND, HWND &, HWND &);
// ������ ���
BOOL ClientCreateStatusBar(HINSTANCE, HWND, int, int, HWND &);
// ����� ����
BOOL ClientCreateComboBox(HINSTANCE, HWND, HWND &);
