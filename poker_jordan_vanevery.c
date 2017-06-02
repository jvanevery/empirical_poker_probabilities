/********************************************************************
* Empirical Poker Hand Probability Generator
* Jordan VanEvery ~ Last Modified: 2017-3-2 
*
* The program in its current state determines the probability of 
* improving a five card poker hand when allowed to discard a single
* card from the hand and replace it with a remaining card in the deck
* An empirical method is used, where a card it replaced at random 
* many times, and upon each replacement it is determined if the 
* hand state improved.
* 
* The precision of the probabilities determined arbitrarily at this
* point. No check for the convergence of the probability is made, 
* rather a "sampleNumber" is preset that determines what number of
* times a new card will be drawn. If time permits: the difference in
* subsequent probabilities could be checked upon each replacement
* until the change in a prob. is less than some desired precision.
*
* Example input/output: 2D 2C 5H 2H 2S 
* --->2D 2C 5H 2H 2S >>>Four of a Kind 0.0% 0.0% 0.0% 0.0% 0.0%
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define DECK_SIZE  52
#define HAND_SIZE  5
#define SUIT_COUNT 4
#define RANK_COUNT 13
#define FALSE 0
#define TRUE  1
#define HIGH_CARD       1
#define ONE_PAIR        2
#define TWO_PAIR        3
#define THREE_OF_A_KIND 4
#define STRAIGHT        5
#define FLUSH           6
#define FULL_HOUSE      7
#define FOUR_OF_A_KIND  8
#define STRAIGHT_FLUSH  9
#define DIGITS_IN_POKER_HAND_ID 3
#define MAJOR_RANK_DIGIT 0
#define MINOR_RANK_DIGIT 1
#define LOW_PAIR_DIGIT   2

/*********************************************************************
* Global constants
*
*   SUIT_LIST[]: Clubs, Diamonds, Hearts, Spades
*   RANK_LIST[]: Cards ranks: 0->10, J->Jack, Q->Queen, K->king, A->Ace
**********************************************************************/
const char SUIT_LIST[] = "CDHS";
const char  RANK_LIST[] = "234567890JQKA";
/********************************************************************
* External Variables
*
*   handRank[]: contains set of integers representing the rank of 
*     each card in a HAND_SIZE size hand. This array is altered upon
*     every iteration of the inner getProbabilities loop, and is 
*     constantly re-sorted. 
*   handSuit[]: contains a character suit corresponding to the suit
*     of the card in a HAND_SIZE size hand.This array is altered upon
*     every iteration of the inner getProbabilities loop, and is 
*     constantly re-sorted. 
*   pokerHandID[]: A 3 digit identification number
*     Digit 1 ~ Major Rank ~ Pair, Full House, etc. ~ values 1-9
*     Digit 2 ~ Minor Rank ~ Different for different Major Ranks,
*               takes values of integer card ranks.
*     Digit 3 ~ Two Pair Rank ~ 0 if not a Two pair, otherwise holds
*               the card rank of the lower pair of the two.
*     Example: 590 ~ Straight with high card 9
*   copyHandrank[]: A copy of the original imput hand ranks before
*     sorting or other manipulation.
*   copyHandSuit[]: A copy of the original input hand suits befor
*     sorting or other manipulation.
********************************************************************/
int  handRank[HAND_SIZE];
char handSuit[HAND_SIZE];
int  copyHandRank[HAND_SIZE];
char copyHandSuit[HAND_SIZE];
float probabilities[HAND_SIZE];
int  pokerHandID[DIGITS_IN_POKER_HAND_ID];

int  readLine(void);
int  isSuit(char c);
int  isRank(char c);
void loadCopyHand(void);
int  rankToInt(char rank);
char trashToEndOfLine(void);
int  repeatCards(void);
void sortHand();
void printHandRank(void);
void getProbabilities(void);
void printProbabilities(void);
int  isFlush(void);
int  isStraight(void);
int  isXOfAKind(int);
int  isFullHouse(void);
int  isTwoPair(void);
int  highCard(void);
void getHandRank(void);
int  isBetterHand(void);
void printHand(void);


int main(void)
{ int lineStatus,i;
  //Seed rand number generator for later    
  srand((unsigned long)time(NULL));
  // Check for input error, echo input
  while((lineStatus=readLine())!=EOF)
  { //copy original input hand for reference to ordering
    for(i=0; i<HAND_SIZE; ++i)
    { copyHandRank[i]=handRank[i];
      copyHandSuit[i]=handSuit[i];
    }
    printf(" >>>");
    //Got a good line 
    if(lineStatus==1)
    { sortHand();
      getHandRank();
      getProbabilities();
      printHandRank();
      //print probabilities
      for(int i=0; i<HAND_SIZE; ++i)
      { printf(" %.1f%%",probabilities[i]);
      }
    }
    //Bad line
    else
    { printf("Error");
    }
    printf("\n");
  }
  return 0;
}
/****************************************************
 * Reads a line from standard input.
 * Returns
 *   1 if no errors
 *   0 if error
 *   EOF if last line is read
 * Vars
 *   isError: error flag, also can possibly hold EOF
 *   ____Count: counters for various detections
 *
 ****************************************************/
int readLine(void)
{
char c;
int isError=1,charCount=0,inputCount=0,rankCount=0,suitCount=0, branchNum=0;
  while((c=getchar())!='\n' && c!=EOF)
  { printf("%c",c);
    charCount+=1;
    branchNum+=1;
    if(branchNum>3) branchNum=1;
    //rank card should be here
    if(branchNum==1)
    { 
      if(isRank(c)==TRUE)
      { handRank[rankCount]=rankToInt(c);
        rankCount+=1;
      }
      else
      { isError=0;
        c=trashToEndOfLine();
      }
    }
    //suit card should be here
    else if(branchNum==2)
    { 
      if(isSuit(c)==TRUE)
      { handSuit[suitCount]=c;
        suitCount+=1;
	inputCount+=1;
      }
      else
      { isError=0;
        c=trashToEndOfLine();
      }
    }
    //space should be here
    else if(branchNum==3)
    {
      if(c==' ')
      {
      }
      else
      { isError=0;
        c=trashToEndOfLine();
      }
    }
    if((c=='\n'||c==EOF))
    { break;
    }
  }
  //Some last checks that make a hand invalid
  if(inputCount!=HAND_SIZE) isError=0;
  if(repeatCards()==TRUE) isError=0;
  if(c==EOF) isError=EOF;

  return isError; 
}
/********************************************************************
* isSuit determines if one of the input suits is a valid suit
* 
* Returns an integer TRUE or FALSE
* Takes a character as a parameter
********************************************************************/
int isSuit(char inputSuit)
{ int i, isValidSuit=FALSE;
  for(i=0; i<SUIT_COUNT; ++i)
  { if(inputSuit==SUIT_LIST[i])
    { isValidSuit=TRUE;
      break;
    }
  }
  return isValidSuit;
}
/********************************************************************
* isRank determines if the char handed to it is a valid rank
* for a card in a typical 52 card deck
* 
* Returns an int TRUE=1 or FALSE=0
* Takes a character that represents a rank
********************************************************************/
int isRank(char inputRank)
{ int i, isValidRank=FALSE;
  for(i=0; i<RANK_COUNT; ++i)
  { if(inputRank==RANK_LIST[i])
    { isValidRank=TRUE;
      break;
    }
  }
  return isValidRank;
}
/***********************************************************************
* Reads rest of line into temp character to avoid storing massive input
* Prints trashed characters. 
*
* Returns either a newline or EOF character
* no paramters
***********************************************************************/
char trashToEndOfLine(void)
{
  char c;
  while((c=getchar())!='\n' && c!=EOF)
  { printf("%c",c);
  }
}
/********************************************************************
* Takes a char that is interpreted as a card rank and maps it to an
* integer such that lower rank cards have a lower integer mapping
* than a higher one. This will make sorting a straightforward task
*
* Returns an integer that is the chars' integer mapping
* Takes a char that gets mapped to an integer
* 
* Range of integers will be 2-14, with 14 corresponding to the ace.
* (In general the ace will be 14 except in the case that we have a
* special straight of the form A 2 3 4 5, in which we will regard 
* the ace as a 1 for practical reasons)
********************************************************************/
int rankToInt(char c)
{ int rankAsInteger;
  if(c>='2' && c<='9')
  { rankAsInteger = c - '0';
  }
  else
  { int i;
    //8 is the index where the 10 rank is held
    for(i=8; i<RANK_COUNT; ++i)
    { if(c==RANK_LIST[i])
      { rankAsInteger = i+2;
        break;
      } 
    }
  }
  return (int)rankAsInteger;
}
/********************************************************************
* loadCopyHand copies the elements of copyHandRank and copyHandSuit
* into handRank and handSuit respectively
*
* No returns, no paramters
********************************************************************/
void loadCopyHand(void)
{ int i;
  for(i=0; i<HAND_SIZE; ++i)
  { handRank[i]=copyHandRank[i];
    handSuit[i]=copyHandSuit[i];
  }
}
/********************************************************************
* Prints the rank of the hand encoded in pokerHandID. Looks at the
* major rank digit to make a decision in the switch.
*
* No returns no paramters
*********************************************************************/
void printHandRank(void)
{ int handRankAsInt;
    switch(pokerHandID[MAJOR_RANK_DIGIT])
    { case HIGH_CARD:
        printf("High Card");
        break;
      case ONE_PAIR:
        printf("Pair");
        break;
      case TWO_PAIR:
        printf("Two Pair");
        break;
      case THREE_OF_A_KIND:
        printf("Three of a Kind"); 
        break;
      case STRAIGHT:
        printf("Straight");
        break;
      case FLUSH:
        printf("Flush");
        break;
      case FULL_HOUSE: 
        printf("Full House");
        break;
      case FOUR_OF_A_KIND:
        printf("Four of a Kind");
        break;
      case STRAIGHT_FLUSH:
        printf("Straight Flush");
        break; 
    }
}
/********************************************************************
* Determines whether or not a repeat card is in handRank and handSuit
* 
* Returns TRUE if hand has repeated cards
*         FALSE if not
*************************************/
int repeatCards(void)
{ char rank, suit;
  int i, j,repCard=FALSE;
  for(i=0;i<HAND_SIZE;++i)
  { rank = handRank[i];
    suit = handSuit[i];
    for(j=i+1;j<HAND_SIZE;++j)
    { 
      if(rank==handRank[j] && suit==handSuit[j]) repCard=TRUE;
    }
  }
  return repCard;
}
/************************************************************************
* Calculates a series probabilities for the likelihood of improving the
* rank of your hand by discarding a certain card. These values are stored
* in a global array and is a parallel array with handRank and handSuit.
*
* No returns no parameters, resulting probabilities are stored into a 
* global variable.
**************************************************************************/
void getProbabilities(void)
{ char tempSuit, randSuit;
  int sampleNumber=750000, numOfImprovements;
  int i, j, k, sameHand, tempRank, swapIndex, randRank;
  //Loop over possible cards to discard
  for(i=0; i<HAND_SIZE; ++i)
  { /*Load in a copy of the input hand this is a simple way to 
    * avoid a nasty bug that occurs when hands have cards 
    * that share a rank*/
    loadCopyHand();
    sortHand();
    tempSuit = handSuit[i];
    tempRank = handRank[i];
    numOfImprovements=0;
    swapIndex=i;
    for(j=0; j<sampleNumber; ++j) 
    { do{
        sameHand=FALSE;
        //Offset of 2 on rank because of the way the rank system is defined
        randRank=rand()%RANK_COUNT + 2;
        randSuit=SUIT_LIST[rand()%SUIT_COUNT];
        handRank[swapIndex]=randRank;
        handSuit[swapIndex]=randSuit;
        if((randRank==tempRank)&&(randSuit==tempSuit))
        { sameHand=TRUE;
        }
      }while(repeatCards()==TRUE||sameHand==TRUE);
      sortHand();
      //After sorting we lose track of the card that got swapped
      for(k=0;k<HAND_SIZE;++k)
      { if((randRank==handRank[k]) && (randSuit==handSuit[k])) 
        { swapIndex=k;
          break;
        } 
      }
      if(isBetterHand()==TRUE){
        ++numOfImprovements;
        //printHand();
      }
    }
    //printf("\n");
    //Locate where this card was in the  original arrays
    for(j=0; j<HAND_SIZE; ++j)
    { if((tempSuit==copyHandSuit[j]) && (tempRank==copyHandRank[j]))
      { probabilities[j] = 100*((float)numOfImprovements/sampleNumber);
        break;
      }
    }
  }
}
/********************************************************************
* Sorts current hand according to rank from lowest to highest and 
* suit according to an arbitrarily chosen suit order. This will make
* the process of determining the rank of hands much simpler
*
* No returns no parameters                                 
* Currently bubble sorting. If a swap takes place in a sweep
* of the array, swapDetector is set to 1 to alert the program that 
* it needs to sweep over the array again. Bubble sort is actually
* quite appropriate in this context since most of the time it is
* sorting small nearly-sorted hands.       
********************************************************************/
void sortHand(void)
{ int swapDetector=TRUE, tempRank, tempSuit, i;
  //1st sort by rank
  while(swapDetector==TRUE)
  { swapDetector=FALSE;
    for(i=0; i<HAND_SIZE-1; ++i)
    { 
      if(handRank[i]>handRank[i+1])
      { tempRank = handRank[i];
        handRank[i] = handRank[i+1];
        handRank[i+1] = tempRank;
        //Make sure suits stick with ranks
        tempSuit = handSuit[i];
        handSuit[i]=handSuit[i+1];
        handSuit[i+1] = tempSuit;
        swapDetector = TRUE;
      }	
    }
    --i;
  }
}
/********************************************************************
* getHandRank modifies the global pokerHandID variable for Major
* and minor ranks. Major rank is most critical and gets an integer 
* from 1-9. Better hands have higher integer values:
* 1 ~ High card (Junk) : Minor rank is high card
* 2 ~ One pair         : Minor rank is pair rank
* 3 ~ Two pair         : Minor ranks are both pair ranks
* 4 ~ Three of a kind  : Minor rank is 3 of a kind rank
* 5 ~ Straight         : Minor rank is high card
* 6 ~ Flush            : Minor rank is high card
* 7 ~ Full house       : Minor rank is three of a kind rank
* 8 ~ Four of a kind   : Minor rank is four of a kind rank
* 9 ~ Straight Flush   : Minor rank is high card
*
* No parameters, no returns
********************************************************************/
void getHandRank(void)
{ int straightMinorRank, flushMinorRank, tempMinorRank;
  //Default is that two pair digit is zero, unless the hand is a two pair
  //Apologizing now for code golf
  pokerHandID[LOW_PAIR_DIGIT]=0;
  straightMinorRank = isStraight();
  flushMinorRank = isFlush();
  if((flushMinorRank!=0) && (straightMinorRank!=0)) 
  { pokerHandID[MAJOR_RANK_DIGIT]=STRAIGHT_FLUSH;
    pokerHandID[MINOR_RANK_DIGIT]=straightMinorRank;
  }
  else if((tempMinorRank=isXOfAKind(4))!=0) 
  { pokerHandID[MAJOR_RANK_DIGIT]=FOUR_OF_A_KIND;
    pokerHandID[MINOR_RANK_DIGIT]=tempMinorRank;
  }
  else if((tempMinorRank=isFullHouse())!=0)
  { pokerHandID[MAJOR_RANK_DIGIT]=FULL_HOUSE;
    pokerHandID[MINOR_RANK_DIGIT]=tempMinorRank;
  }
  else if(flushMinorRank!=0)        
  { pokerHandID[MAJOR_RANK_DIGIT]=FLUSH;
    pokerHandID[MINOR_RANK_DIGIT]=flushMinorRank;
  }
  else if(straightMinorRank!=0)    
  { pokerHandID[MAJOR_RANK_DIGIT]=STRAIGHT;
    pokerHandID[MINOR_RANK_DIGIT]=straightMinorRank;
  }
  else if((tempMinorRank=isXOfAKind(3))!=0)
  { pokerHandID[MAJOR_RANK_DIGIT]=THREE_OF_A_KIND;
    pokerHandID[MINOR_RANK_DIGIT]=tempMinorRank;
  }
  else if((tempMinorRank=isTwoPair())!=0)
  { pokerHandID[MAJOR_RANK_DIGIT]=TWO_PAIR;
    pokerHandID[MINOR_RANK_DIGIT]=tempMinorRank;
    pokerHandID[LOW_PAIR_DIGIT]=isXOfAKind(2);
  }
  else if((tempMinorRank=isXOfAKind(2))!=0) 
  { pokerHandID[MAJOR_RANK_DIGIT]=ONE_PAIR;
    pokerHandID[MINOR_RANK_DIGIT]=tempMinorRank;
  }
  else
  { pokerHandID[MAJOR_RANK_DIGIT]=HIGH_CARD;
    pokerHandID[MINOR_RANK_DIGIT]=highCard();
  }
 
}
/********************************************************************
* isBetterHand compares the major and minor ranks stored in 
* pokerHandId to that of handRank[] and handSuit[]. It implements 
* a switch so that cases that are ruled out for a better hand are not
* checked. For example, if we know the original hand was a straight,
* we need not check if the new hand is a pair, that would not yield 
* a better hand.
*
* Returns TRUE if a better hand was drawn, FALSE otherwise
* No parameters
********************************************************************/
int isBetterHand (void)
{ int majorRank = pokerHandID[MAJOR_RANK_DIGIT];
  int minorRank = pokerHandID[MINOR_RANK_DIGIT];
  int tempMinor, straightMinorRank=0, flushMinorRank=0,  betterHandDetected=FALSE;

  switch (majorRank)
  { case HIGH_CARD:
      //printf("entered high card switch\n");
      if(highCard()>minorRank)
      { betterHandDetected=TRUE;
        break;
      }
    case ONE_PAIR:
      //printf("entered one pair switch\n");
      tempMinor = isXOfAKind(2);
      //Check if new hand is One Pair
      if(tempMinor!=0)
      { if(ONE_PAIR>majorRank)
        { betterHandDetected=TRUE;
          break;
        }
        else
        { if(tempMinor>minorRank)
          { betterHandDetected=TRUE;
            break; 
          }
        }
      }
    case TWO_PAIR:
      //printf("Entered two pair switch\n");
      tempMinor = isTwoPair();
      if(tempMinor!=0)
      { if(TWO_PAIR>majorRank)
        { betterHandDetected=TRUE;
          break;
        }
        else
        { if(tempMinor>minorRank)
          { betterHandDetected=TRUE;
            break;
          }
          //ONLY REACHED IF 1ST PAIR MATCHES
          else if(tempMinor==minorRank)
          { //Looks at 1st pair ranks
            if(isXOfAKind(2)>pokerHandID[LOW_PAIR_DIGIT])
            { betterHandDetected=TRUE;
              break;
            }
          }
        }
      }
    case THREE_OF_A_KIND:
      //printf("entered three of a kind switch\n");
      tempMinor = isXOfAKind(3);
      if(tempMinor!=0)
      { if(THREE_OF_A_KIND>majorRank)
        { betterHandDetected=TRUE;
          break;
        }
        else if(tempMinor>minorRank)
        { betterHandDetected=TRUE;
          break; 
        }
      }
    case STRAIGHT:
      straightMinorRank = isStraight();
      //printf("entered straight switch\n");
      if(straightMinorRank!=0)
      { if(STRAIGHT>majorRank)
        { betterHandDetected=TRUE;
          break;
        }
        else if(straightMinorRank>minorRank)
        { betterHandDetected=TRUE;
          break; 
        }
      }
    case FLUSH:
      //printf("entered flush switch\n");
      flushMinorRank = isFlush();
      if(flushMinorRank!=0) 
      { if(FLUSH>majorRank)
        { betterHandDetected=TRUE;
          break;
        }
        else if(flushMinorRank>minorRank)
        { betterHandDetected=TRUE;
          break;
        }
      }
    case FULL_HOUSE:
      //printf("entered full house switch\n");
      tempMinor = isFullHouse();
      if(tempMinor!=0)
      { if(FULL_HOUSE>majorRank)
        { betterHandDetected=TRUE;
          break;
        }
        else if(tempMinor>minorRank)
        { betterHandDetected=TRUE;
          break; 
        }
      } 
    case FOUR_OF_A_KIND:
      tempMinor = isXOfAKind(4);
      //printf("entered four of a kind switch\n");
      if(tempMinor!=0)
      { if(FOUR_OF_A_KIND>majorRank)
        { betterHandDetected=TRUE;
          break;
        }
        else if(tempMinor>minorRank)
        { betterHandDetected=TRUE;
          break; 
        }
      }
    case STRAIGHT_FLUSH:
      //printf("entered straight flush switch\n");
      //in the case that isStraight and isFlush have not been called
      if(straightMinorRank==0)
      { straightMinorRank=isStraight();
      }
      if(flushMinorRank==0)
      { flushMinorRank=isFlush();
      }
      if(straightMinorRank!=0 && flushMinorRank!=0)
      { if(majorRank==STRAIGHT_FLUSH)
        { if(straightMinorRank>minorRank)
            { betterHandDetected=TRUE;
              break;
            }
        }
        else
        { betterHandDetected=TRUE;
          break;
        }
      }
  }
  return betterHandDetected;
}

/********************************************************************
* HERE BEGINS THE SLEW OF "isSomething" FUNCTIONS.
********************************************************************/

/********************************************************************
* isFlush determines whether or not all the suits of a hand match
* 
* Returns a the value of the minor rank, in this case we will use the
* No paramters
********************************************************************/
int isFlush(void)
{ int i=0, flushSum=0; //default false
  char flushSuit=handSuit[i];
  for(i=1; i<HAND_SIZE; ++i)
  { if(flushSuit!=handSuit[i])
    { return FALSE;
    }
  } 
  if(i==HAND_SIZE)
  { for(i=0;i<HAND_SIZE;++i)
    { flushSum+=handRank[i];
    }
  }
  return flushSum;
}
/********************************************************************
* isStraight determines whether or not the hand is of rank straight.
* A straight occurs when the ranks of your cards can be arranged in
* a contiguous order. EX: 8 9 10 J Q / 8 9 10 11 12
* There is a special straight that is the sequence 2 3 4 5 A.
*
* Returns high card in straight if it is a straight, otherwise 0
* No parameters.
********************************************************************/
int isStraight(void)
{ int i,j, highCard=0;
  int specialStraight[HAND_SIZE]= {2,3,4,5,14};
    for(i=0; i<HAND_SIZE; ++i)
    { if(handRank[i]!=specialStraight[i])
      { break;
      }
    }
  for(j=0; j<HAND_SIZE-1; ++j)
  { if(handRank[j]!=handRank[j+1]-1)
    { break;
    }
  }
  if(j==HAND_SIZE-1) highCard=handRank[HAND_SIZE-1];
  if(i==HAND_SIZE)   highCard=handRank[HAND_SIZE-2];
  return highCard;
}
/********************************************************************
* isXOfAKind determines whether or not X cards in the hand all
* share a rank. When x=4 this is the 2nd highest ranking hand.
* This algorithm will not keep looking through the hand if it has 
* found more than HAND_SIZE-x+1 unique ranks in the hand.(efficiency)
*
* Returns the minor rank of the four of a kind if found, otherwise 0
* Takes an integer x that corresponds to the # of matching ranks
* desired.
********************************************************************/
int isXOfAKind(int x)
{ int i=0, diffCounter, matchCounter, rankOfMatchingCards=0;
  matchCounter=diffCounter=1;
  while(diffCounter<=HAND_SIZE-x+1)
  { if(handRank[i]==handRank[i+1]) ++matchCounter;
    else 
    { ++diffCounter;
      matchCounter=1;
    }
    if(matchCounter==x)
    { rankOfMatchingCards=handRank[i];
      break;
    }
    ++i;
  }
  return rankOfMatchingCards;
}
/********************************************************************
* isFullHouse determine if the hand has a 3 of a kind AND a pair, 
* where each occurs in a unique rank. EX: 5 5 A A A
* 
* Returns the minor rank of the three of a kind in the full house
* if a full house is found, otherwise 0.
*
* matchCounter keeps track of the numbers of repeated cards. If the
* content of the array is 2 3 or 3 2, then we have a full house.
********************************************************************/
int isFullHouse(void)
{ int i, uniqueCards, counter[2], cardRankOfTriplet=0;
  counter[0]=counter[1]=1;
  uniqueCards=1;
  i=0;
  while(uniqueCards<3)
  { if(handRank[i]==handRank[i+1])
    { ++counter[uniqueCards-1]; 
    }
    else
    { ++uniqueCards;
    }

    ++i;
    if((counter[0]==2 && counter[1]==3))
    { cardRankOfTriplet=handRank[HAND_SIZE-1];
    }
    else if((counter[0]==3 && counter[1]==2))
    { cardRankOfTriplet=handRank[0];
    }
  }
  return cardRankOfTriplet;
}
/********************************************************************
* isTwoPair determines if two pairs occur in the ranks of the hand.
* Ex: Q Q J J 2
*
* Returns the card rank of the higher pair if found, and 0 otherwise
********************************************************************/
int isTwoPair(void)
{ int i, uniqueCardCount, counter[3], highPairRank=0;
  counter[0]=counter[1]=counter[2]=1;
  uniqueCardCount=1;
  i=0;
  while((uniqueCardCount<4) && i<HAND_SIZE-1)
  { if(handRank[i]==handRank[i+1])
    { ++counter[uniqueCardCount-1]; 
    }
    else ++uniqueCardCount;
    ++i;
  }
  //High pair in middle
  if((counter[0]==2 && counter[1]==2))
  { highPairRank = handRank[HAND_SIZE/2];
  }
  //highpair on end
  else if((counter[0]==2 && counter[2]==2)||(counter[1]==2 && counter[2]==2))
  { highPairRank = handRank[HAND_SIZE-1];
  }
 
  return highPairRank; 
}
/********************************************************************
* highCard determines the highest ranking card in the hand. This
* will be useful for comparing the relative superiority of hands with
* an identical rank. EX: 8 8 10 10 A  vs. 8 8 J J 3, 2nd hand wins
* because J>10 in rank. (MINOR RANKS)
*
* Returns ranking of high card in integer mapping. 
********************************************************************/
int highCard(void)
{ int i, highCardRank=0;
  for(i=0; i<HAND_SIZE; ++i)
  { if(highCardRank<handRank[i])
    { highCardRank = handRank[i];
    }
  }
  return highCardRank;
}
/********************************************************************
* Test function that prints hand in its current state
*
********************************************************************/

void printHand(void)
{ int i;
  for(i=0; i<HAND_SIZE; ++i)
  { printf("%d%c ",handRank[i],handSuit[i]);
  }
  printf("\n");
}
