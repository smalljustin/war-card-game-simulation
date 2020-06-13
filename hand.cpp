#include "hand.h"
#include "card.h"
#include <cstdlib>
#include <algorithm>
#include <random>
#include <chrono>
#include <ctime>
#include <iostream>


Hand::Hand(int given_deck_size)
{
    activeCards = static_cast<struct Card**>(malloc(sizeof(struct Card*) * given_deck_size));
    wonCards = static_cast<struct Card**>(malloc(sizeof(struct Card*) * given_deck_size));

    deckSize = given_deck_size;
    endActiveIdx = -1;
    endWonIdx = -1;
    seed = 0;
}

void Hand::printCards()
{
    int i;
    std::cout << "Active cards:\n";
    for (i = 0; i <= endActiveIdx; i++)
    {
        std::cout << "Suit: " << (activeCards[i]->suit) << "\tNumber: " << (activeCards[i]->number) << "\n";
    }
    std::cout << "Won cards:\n";
    for (i = 0; i <= endWonIdx; i++)
    {
        std::cout << "Suit: " << (wonCards[i]->suit) << "\tNumber: " << (wonCards[i]->number) << "\n";
    }
}

int Hand::scoreHand()
{
    int score{ 0 };
    int i;
    for (i = 0; i <= endActiveIdx; i++) {
        score += wonCards[i]->number;
    }
    return score;
}

void Hand::AddWonCard(struct Card* card)
{
    wonCards[++endWonIdx] = card;
    return;
}

void Hand::AddActiveCard(struct Card* card)
{
    activeCards[++endActiveIdx] = card;
    return;
}

void Hand::ClearCards(void)
{
    int i;
    for (i = 0; i < deckSize; i++)
    {
        activeCards[i] = nullptr;
        wonCards[i] = nullptr;
    }

    endActiveIdx = -1;
    endWonIdx = -1;
    return;
}

void Hand::StartRound()
{
    struct Card* card;
    card = GetWonCard();
    while (card != nullptr)
    {
        AddActiveCard(card);
        card = GetWonCard();
    };
    Shuffle();

    return;
}

int Hand::TotalCards()
{
    return (endWonIdx + endActiveIdx + 2);

}

struct Card* Hand::GetActiveCard()
{
    if (endActiveIdx > -1)
    {
        return activeCards[endActiveIdx--];
    }
    else
    {
        return nullptr;
    }
}

struct Card* Hand::GetWonCard()
{
    if (endWonIdx > -1)
    {
        return wonCards[endWonIdx--];
    }
    else
    {
        return nullptr;
    }
}

void Hand::Shuffle()
{
    if (endActiveIdx > -1) {
        seed += 1;
        std::shuffle(activeCards, (activeCards + endActiveIdx), std::default_random_engine(seed));
    }
}

int Hand::handSize()
{
    return endActiveIdx + 1;
}
