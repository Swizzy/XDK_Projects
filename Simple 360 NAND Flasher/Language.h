#ifndef __S360NF_LANGUAGE_H
#define __S360NF_LANGUAGE_H

#ifdef RUSSIAN_TRANSLATION
#include "Translations\Russian.h"
#elif ITALIAN_TRANSLATION
#include "Translations\Italian.h"
#elif PORTUGUESE_TRANSLATION
#include "Translations\Portuguese.h"
#elif GERMAN_TRANSLATION
#include "Translations\German.h"
#else
#include "Translations\Default.h"
#endif

#endif