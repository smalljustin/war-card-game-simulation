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
            *(Deck + k) = { j, i };
            k++;
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
    const int RANGE_MIN = 50;
    const int RANGE_MAX = 500;

    const int INCREMENT = 50;
    const int MAX_BUCKET_IDX = 10000;

    int BUCKETS = MAX_BUCKET_IDX / INCREMENT;

    int deck_size;
    int raw_deck_size;
    int game_num;
    int i;
    int j;
    int verbose;
    int turns;

    std::ofstream outputFile;
    outputFile.open("output_multi_size_deck.csv");
    outputFile << "Deck Size,Turns,Count\n";
    std::cout << "Deck Size\tGame\tRounds\tTurns\tTurns/Rounds\tPlayer 1 Score - Player 2 Score\tWinner\n";
    Hand player1(RANGE_MAX);
    Hand player2(RANGE_MAX);
    struct Card* deck;
    
    int bucket_idx;

    int** histogramLists = static_cast<int **>(malloc(sizeof(int*) * RANGE_MAX/INCREMENT));

    for (i = 0; i < RANGE_MAX / INCREMENT; i++) {
        histogramLists[i] = static_cast<int*>(calloc(BUCKETS, sizeof(int)));
    }


    struct Card** player1WarCards = static_cast<struct Card**>(malloc(sizeof(struct Card*) * RANGE_MAX));
    struct Card** player2WarCards = static_cast<struct Card**>(malloc(sizeof(struct Card*) * RANGE_MAX));


    for (raw_deck_size = RANGE_MIN; raw_deck_size < RANGE_MAX; raw_deck_size += INCREMENT)
    {
        deck_size = (raw_deck_size / 4) * 4;
        deck = GenDeck(deck_size, SUITS);
        Shuffle(deck, deck_size);
        std::string winner;

        for (game_num = 0; game_num < 2500; game_num++)
        {
            for (i = 0; i < deck_size; i += 2)
            {
                player1.AddWonCard((deck + i));
                player2.AddWonCard((deck + i + 1));
            }

            player1.StartRound();
            player2.StartRound();

            turns = 0;
            i = 0;
            verbose = 0;
            

            while ((player1.handSize() > 0 && player2.handSize() > 0))
            {
                if (i > 500000)
                {
                    verbose = 1;
                }

                i += 1;

                if (verbose) {
                    std::cout << "Player 1: " << player1.handSize() << "\tPlayer 2: " << player2.handSize() << "\n";
                    player1.printCards();
                    player2.printCards();
                }
                round(&player1, &player2, player1WarCards, player2WarCards, verbose, &turns);
                player1.StartRound();
                player2.StartRound();
            }

            if (player1.handSize() == 0)
            {
                winner = "Player 2";
            }
            else {
                winner = "Player 1";
            }

            double turns_div_rounds = static_cast<double>(turns) / static_cast<double>(i);
            resetDeck(&player1, &player2, deck, deck_size);
            if (game_num % 50 == 0)
            {
                std::cout << deck_size << "\t\t" << game_num << "\t" << i << "\t" << turns << "\t" << turns_div_rounds << "\t\t" << (player1.scoreHand() - player2.scoreHand()) << "\t\t\t\t" << winner << "\n";
            }
            // outputFile << deck_size << "," << game_num << "," << i << "," << turns << "," << turns_div_rounds << "," << (player1.scoreHand() - player2.scoreHand()) << "," << winner << "\n";
            
            bucket_idx = i / INCREMENT;
            if (bucket_idx > BUCKETS)
            {
                bucket_idx = (BUCKETS - 1); // last bucket
            }

            histogramLists[raw_deck_size / INCREMENT][bucket_idx] += 1;
        }
        
        player1.ClearCards();
        player2.ClearCards();

    }

    for (i = 1; i < RANGE_MAX / INCREMENT; i++) {
        for (j = 0; j < BUCKETS; j++) {
            std::cout << i * INCREMENT << "," << j * INCREMENT << "," << histogramLists[i][j] << "\n";
            outputFile << i * INCREMENT << "," << j * INCREMENT << "," << histogramLists[i][j] << "\n";
        }
    }
    outputFile.close();
}

void resetDeck(Hand* player1, Hand* player2, struct Card* deck, int deck_size)
{
    player1->ClearCards();
    player2->ClearCards();
    Shuffle(deck, deck_size);

    struct Card* player1Card;
    struct Card* player2Card;

    int i;
    for (i = 0; i < deck_size; i += 2)
    {
        player1->AddWonCard((deck + i));
        player2->AddWonCard((deck + i + 1));
    }
    player1->StartRound();
    player2->StartRound();
    return;
}

void round(Hand* player1, Hand* player2, struct Card** player1WarCards, struct Card** player2WarCards, int verbose, int* turns)
{
    int turn;
    int i;
    struct Card* player1Card, * player2Card;
    Hand* winningPlayer;

    for (turn = 0; turn <= player1->handSize() && turn <= player2->handSize(); turn++)
    {
        *(turns) += 1;
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

