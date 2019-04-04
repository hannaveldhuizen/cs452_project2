/* CS 452 - Project 2
 * Hanna Veldhuizen
 * blackjack.c - implementation of black jack using card_driver.c

 INSTRUCTIONS TO RUN PROGRAM:
 - run "make" to build the driver
 - run "insmod card_driver.ko" to link the driver
 - run "make blackjack" to build the source code
 - run "./blackjack" to play
 */

#include <stdio.h>
#include <string.h>

FILE *deck;
// global variables to keep track of how many aces in each persons hands
// whenever we change an ace from "11" to "1", we decrease this number
int numPlayerAces = 0;
int numDealerAces = 0;

/*
 * Method to intelligently determine an ace's value. If adding 11 would make the user bust, we
 * pick 1. 11 otherwise.
 */
int ace(int score) {
  if (score + 11 > 21) {
    return 1;
  }

  return 11;
}

/*
 * "draw" a card from the deck by reading a byte from /dev/cards. Return the value of that card.
 */
int draw(int score) {

  char card[1];

  fread(card, 1, 1, deck);

  // mod for suit
  card[0] = card[0] % 13;

  // ace
  if (card[0] == 0) {
    return ace(score);
  }

  // face card
  else if (card[0] >=10 && card[0] <=12) {
    return 10;
  }

  return card[0] + 1;
}

/*
 * Adjusts ace values from 11 to 1 if the player busts. While the player is over 21 and has leftover aces,
 * this function recurses. An ace is not counted if we have already counted it's value as 1.
 */
int adjustAce(int score, int *numAces) {
    // change aces from counting as 11 to counting as 1
    if (score > 21 && *numAces > 0) {
        score -= 10;
        *numAces = *numAces - 1;
        return adjustAce(score, numAces);
    }

    return score;
}

/*
 * Runs one full turn for the human player. This draws a card, adjusts ace values if the user busts,
 * then asks them if they would like to hit or stand. If the user reaches 21 or over, it returns
 * their score and the game ends.
 *
 * @param score - players current score
 * @param lastTurn - 0 if the user has already hit once this round, 1 otherwise
 */
int playerTurn(int score, int lastTurn) {
    char input[10];
    int card = draw(score);

    if (card == 11) {
        numPlayerAces++;
    }

    score += card;

    score = adjustAce(score, &numPlayerAces);

    if (score > 0) {
         printf("You:\n %d + %d = %d\n\n", score-card, card, score);
    }

    if (score < 21 && lastTurn == 0) {
        // have the option to hit or stand
        printf("Would you like to 'hit' or 'stand'? ");
        scanf(" %s", input);

        if (strcmp(input, "hit") == 0) {
            return playerTurn(score, 1);
        }
    }

    return score;
}

/*
 * Runs one full turn for the dealer. This draws a card, adjusts ace values if the dealer busts,
 * hits if their score is under 17, returns the score otherwise.
 *
 * @param score - dealers current score
 * @param lastTurn - 0 if the dealer has already hit once this round, 1 otherwise
 */
int dealerTurn(int score, int lastTurn) {
    int card = draw(score);

    if (card == 11) {
        numDealerAces++;
    }

    score += card;

    score = adjustAce(score, &numDealerAces);

    if (score > 0) {
        printf("The dealer:\n ? + %d\n\n", score-card, card, score);
    }

    // hit again if dealer's score is less than 17
    if (score < 17 && lastTurn == 0) {
        return dealerTurn(score, 1);
    }

    return score;
}

/*
 * Plays the game of black jack until someone wins. This uses the /dev/cards driver set up in
 * card_driver.c to simulate a deck of cards. Drawing a card is done by reading bytes from the
 * /dev/cards file.
 */
void playBlackJack() {

  deck = fopen("/dev/cards", "r");

  int inGame = 1;
  int numRounds = 0;

  // score for user and dealer
  int playerScore = 0;
  int dealerScore = 0;

    while (inGame == 1) {
        if (numRounds != 0) {
            dealerScore = dealerTurn(dealerScore, 0);
        } else {
            playerScore = playerTurn(0, 1);
            dealerScore = dealerTurn(0, 0);
        }

        if (dealerScore == 21) {
            printf("Dealer wins!\n");
            break;
        } else if (dealerScore > 21) {
            printf("Dealer busted. You win.\n");
            break;
        }

        playerScore = playerTurn(playerScore, 0);

        if (playerScore == 21) {
            printf("You win!\n");
            break;
        } else if (playerScore > 21) {
            printf("You busted. Dealer wins.\n");
            break;
        }

        numRounds++;
    }
}

int main() {
  playBlackJack();
  return 0;
}