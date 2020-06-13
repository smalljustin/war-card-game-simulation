#pragma once

class Hand
{
private:
    struct Card** activeCards;
    struct Card** wonCards;
    int endActiveIdx;
    int endWonIdx;
    int deckSize;
    unsigned seed;
public:
    Hand(int deckSize);
    void AddWonCard(struct Card* card);
    void AddActiveCard(struct Card* card);
    void ClearCards(void);
    void StartRound();
    int TotalCards();
    struct Card* GetActiveCard();
    struct Card* GetWonCard();
    void Shuffle();
    int handSize();
    void printCards();
    int scoreHand();
};