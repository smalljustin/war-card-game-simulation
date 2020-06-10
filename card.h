#pragma once

struct Card
{
    // Suits: 0, 1, 2, 3. Corresponds arbitrarily. 
    // Cards: 0 is Ace, counting numbers are trivial, then 11 is J, 12 is Q, 13 is K. 
    int suit;
    int number;
};
