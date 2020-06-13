#include "hand.h"
#include "card.h"
#include <cstdlib>
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include "main.h"
#include <string>


struct Card* GenDeck(int deckSize, int suits)
{
    struct Card* Deck = static_cast<struct Card*>(malloc(sizeof(struct Card) * deckSize));
    if (Deck == NULL)
    {
        return NULL;
    }
    int i{ 0 };
    int j{ 0 };
    int k{ 0 };

    for (i = 0; i < (deckSize / suits); i++)
    {
        for (j = 0; j < suits; j++)
        {
            *(Deck + k++) = { j, i };
        }
    }
    return Deck;
}

void Shuffle(struct Card* deck, int deckSize)
{
    unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
    std::shuffle(deck, (deck + deckSize), std::default_random_engine(seed));
    return;
}


int main()
{
    const int SUITS = 4;
    const int GAMES = 1000;
    const int DECK_SIZE = 208;

    // const int INCREMENT = 50;
    // const int MAX_BUCKET_IDX = 10000;

    // int BUCKETS = MAX_BUCKET_IDX / INCREMENT;

    int NUM_GAMES_TRACKED = (10 * 2);
    int TURNS_TRACKED = 20000;

    int i;
    int j;
    int turns;

    std::ofstream outputFile;

    outputFile.open("tracking_players_handsize_large.csv");
    outputFile << "Turn Number,";
    for (j = 0; j < (NUM_GAMES_TRACKED / 2); j++) {
        outputFile << "Game " << (j + 1) << " Player 1,";
        outputFile << "Game " << (j + 1) << " Player 2,";

    }
    outputFile << "\n";

    // outputFile << "Deck Size,Turns,Count\n";
    //std::cout << "Deck Size\tGame\tRounds\tTurns\tTurns/Rounds\tPlayer 1 Score - Player 2 Score\tWinner\n";
    //outputFile << "Deck Size,Game,Rounds,Turns,Turns/Rounds,Player 1 Score - Player 2 Score,Winner\n";
    Hand player1(DECK_SIZE);
    Hand player2(DECK_SIZE);
    struct Card* deck;

    //int bucket_idx;

    //int** histogramLists = static_cast<int **>(malloc(sizeof(int*) * RANGE_MAX/INCREMENT));

    //for (i = 0; i < RANGE_MAX / INCREMENT; i++) {
    //    histogramLists[i] = static_cast<int*>(calloc(BUCKETS, sizeof(int)));
    //}

    int** scoreLists = static_cast<int**>(malloc(sizeof(int*) * NUM_GAMES_TRACKED));

    for (i = 0; i < NUM_GAMES_TRACKED; i++) {
        scoreLists[i] = static_cast<int*>(malloc(sizeof(int) * TURNS_TRACKED));
    }

    struct Card** player1WarCards = static_cast<struct Card**>(malloc(sizeof(struct Card*) * DECK_SIZE));
    struct Card** player2WarCards = static_cast<struct Card**>(malloc(sizeof(struct Card*) * DECK_SIZE));


    deck = GenDeck(DECK_SIZE, SUITS);
    Shuffle(deck, DECK_SIZE);
    std::string winner;

    int game_num;

    for (game_num = 0; game_num < NUM_GAMES_TRACKED; game_num += 2)
    {
        resetDeck(&player1, &player2, deck, DECK_SIZE);

        player1.StartRound();
        player2.StartRound();

        turns = 0;
        i = 0;
        int runoff_cards;

        while ((player1.handSize() > 0 && player2.handSize() > 0))
        {
            player1.StartRound();
            player2.StartRound();
            i += 1;
            round(&player1, &player2, player1WarCards, player2WarCards, 0, &turns, scoreLists[game_num], scoreLists[game_num + 1], TURNS_TRACKED);
        }
            
        if (player1.handSize() == 0)
        {
            winner = "Player 2";
            runoff_cards = 0;
        }
        else {
            winner = "Player 1";
            runoff_cards = DECK_SIZE;
        }

        turns += 1;
        for (i = turns; i < TURNS_TRACKED; i++) {
            scoreLists[game_num][i] = runoff_cards;
            scoreLists[game_num + 1][i] = (DECK_SIZE - runoff_cards);
        }
    }

    for (i = 0; i < TURNS_TRACKED; i++) {
        outputFile << "Turn " << i << ",";
        for (j = 0; j < NUM_GAMES_TRACKED; j++) {
            outputFile << scoreLists[j][i] << ", ";
        }
        outputFile << "\n";
    }
    outputFile.close();
}

void resetDeck(Hand* player1, Hand* player2, struct Card* deck, int deck_size)
{
    player1->ClearCards();
    player2->ClearCards();
    Shuffle(deck, deck_size);

    int i;
    for (i = 0; i < deck_size; i += 2)
    {
        player1->AddActiveCard((deck + i));
        player2->AddActiveCard((deck + i + 1));
    }

    return;
}

void round(Hand* player1, Hand* player2, struct Card** player1WarCards, struct Card** player2WarCards, int verbose, int* turns, int* turnTrackerPlayer1, int* turnTrackerPlayer2, int maxTurns)
{
    int turn;
    int i;
    struct Card *player1Card, *player2Card;
    Hand* winningPlayer;

    turnTrackerPlayer1[*turns] = player1->TotalCards();
    turnTrackerPlayer2[*turns] = player2->TotalCards();

    for (turn = 0; ((turn <= player1->handSize() && turn <= player2->handSize()) && turn <= maxTurns); turn++)
    {
        *(turns) += 1;

        if (*(turns) > maxTurns) {
            player1->ClearCards();
            player2->ClearCards();
            return;
        }
        turnTrackerPlayer1[*turns] = player1->TotalCards();
        turnTrackerPlayer2[*turns] = player2->TotalCards();

        player1Card = player1->GetActiveCard();
        player2Card = player2->GetActiveCard();

        // Check for "WAR" case. 

        if (verbose)
        {
            std::cout << "Player 1 card: " << player1Card->number << "\tPlayer 2 card: " << player2Card->number << "\n";
        }

        if (player1Card->number == player2Card->number)
        {
            if (verbose)
            {
                std::cout << "War!\n";
            }
            player1WarCards[0] = player1Card;
            player2WarCards[0] = player2Card;
            int idx = 1;

            winningPlayer = war(player1, player2, player1WarCards, player2WarCards, &idx);

            for (i = 0; i < idx; i++)
            {
                if (player1WarCards[i] != nullptr) {
                    if (verbose) {
                        std::cout << "Player 1 Card: " << player1WarCards[i]->number << "\n";
                    }
                    winningPlayer->AddWonCard(player1WarCards[i]);
                    player1WarCards[i] = nullptr;
                }

                if (player2WarCards[i] != nullptr) {
                    if (verbose) {
                        std::cout << "Player 2 Card: " << player2WarCards[i]->number << "\n";
                    }
                    winningPlayer->AddWonCard(player2WarCards[i]);
                    player2WarCards[i] = nullptr;
                }
            }
            continue;
        }

        if (player1Card->number > player2Card->number)
        {
            if (verbose) {
                std::cout << "Player 1 wins!\n";
            }
            winningPlayer = player1;
        }
        else {
            if (verbose) {
                std::cout << "Player 2 wins!\n";
            }
            winningPlayer = player2;
        }
        winningPlayer->AddWonCard(player1Card);
        winningPlayer->AddWonCard(player2Card);
    }
    return;
}

Hand* war(Hand* player1, Hand* player2, struct Card** player1WarCards, struct Card** player2WarCards, int* idx)
{
    struct Card* t_p1_card = player1->GetActiveCard();
    struct Card* t_p2_card = player2->GetActiveCard();

    if ((t_p1_card == nullptr) && (t_p2_card == nullptr)) {
        return player1;
    }
    else if (t_p1_card != nullptr && t_p2_card == nullptr) {
        player1WarCards[*idx] = t_p1_card;
        player2WarCards[*idx] = nullptr;
        *(idx) = *idx + 1;
        return player2;
    }
    else if (t_p1_card == nullptr && t_p2_card != nullptr) {
        player2WarCards[*idx] = t_p2_card;
        player1WarCards[*idx] = nullptr;
        *(idx) = *idx + 1;
        return player1;
    }
    else {
        player1WarCards[*idx] = t_p1_card;
        player2WarCards[*idx] = t_p2_card;
        *(idx) = *idx + 1;
    }

    t_p1_card = player1->GetActiveCard();
    t_p2_card = player2->GetActiveCard();

    if ((t_p1_card == nullptr) && (t_p2_card == nullptr)) {
        return player1;
    }
    else if (t_p1_card != nullptr && t_p2_card == nullptr) {
        player1WarCards[*idx] = t_p1_card;
        player2WarCards[*idx] = nullptr;
        *(idx) = *idx + 1;

        return player2;
    }
    else if (t_p1_card == nullptr && t_p2_card != nullptr) {
        player2WarCards[*idx] = t_p2_card;
        player1WarCards[*idx] = nullptr;
        *(idx) = *idx + 1;
        return player1;
    }
    else {
        player1WarCards[*idx] = t_p1_card;
        player2WarCards[*idx] = t_p2_card;
        *(idx) = *idx + 1;
    }


    if (t_p1_card->number == t_p2_card->number)
    {
        // std::cout << "War again!\n";
        return war(player1, player2, player1WarCards, player2WarCards, idx);
    }
    else if (t_p1_card->number > t_p2_card->number)
    {
        return player1;
    }
    else
    {
        return player2;
    }



}

