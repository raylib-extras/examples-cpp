# Cards-cpp
A simple example of how to build up data for cards and keep them stacks and hands.
Includes code to select and drag the cards around.

A deck is a unquie set of cards, and owns the card data. Represents the cards in the box.
A stack is a set of card pointers, used for things like draw decks. Represents a stack of some subset of the cards from a deck.
A hand is a set of card pointers that are manipulatable and dragable. These represent the cards a player has drawn or are in play.

![cards](https://user-images.githubusercontent.com/322174/138608557-5b1dfeb3-33a3-409c-8635-0dac7f7cdf36.gif)