#ifndef MASA_RUTU_H
#define MASA_RUTU_H

#include <Arduino.h>

// ============================================================
// TELUGU PANCHANGA MASA
// ============================================================

const char* getTeluguMasaName(int masa);

// ============================================================
// TELUGU PANCHANGA RUTU
// ============================================================

const char* getTeluguRutuName(int masa);

// ============================================================
// GET RUTU NUMBER
//
// 0 = Vasanta
// 1 = Grishma
// 2 = Varsha
// 3 = Sharad
// 4 = Hemanta
// 5 = Shishira
// ============================================================

int getTeluguRutu(int masa);

#endif