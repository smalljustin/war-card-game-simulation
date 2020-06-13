#pragma once
#include "hand.h"

struct Card* GenDeck(int deckSize, int suits);
void Shuffle(struct Card** deck, int deckSize);
int main();
void resetDeck(Hand* player1, Hand* player2, Card* deck, int deck_size);
void round(Hand* player1, Hand* player2, struct Card** player1WarCards, struct Card** player2WarCards, int verbose, int* turns, int* turnTrackerPlayer1, int* turnTrackerPlayer2, int maxTurns);
Hand* war(Hand* player1, Hand* player2, struct Card** player1WarCards, struct Card** player2WarCards, int *idx);